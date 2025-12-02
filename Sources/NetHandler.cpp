/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: NetHandler.cpp
Open source lines: 461/564 (81.74%)
*****************************************************/

#include "NetHandler.hpp"
#include "main.hpp"
#include "SaveHandler.hpp"
#include "httpcPatch.h"

namespace CTRPluginFramework {
	u64 NetHandler::ConsoleSecureHash[2] = {0, 0};
	std::vector<std::shared_ptr<NetHandler::Session>> NetHandler::Session::pendingSessions;
	Mutex NetHandler::Session::pendingSessionsMutex{};
	ThreadEx* NetHandler::Session::sessionThread;
	LightEvent NetHandler::Session::threadEvent;
	LightEvent NetHandler::Session::threadFinishEvent;
	bool NetHandler::Session::runThread = false;


	u64 NetHandler::ConsoleUniqueHash = 0;
	std::string NetHandler::ConsoleUniquePassword = "";
	std::string NetHandler::UserUniqueName = "";
	std::pair<void*, size_t> NetHandler::httpcStolenMemory;

	const char* NetHandler::RequestHandler::reqStr[] = {
		"req_betaver",
		"put_generalstatsv2",
		"req_login",
		"req_logout",
		"put_onlsearch",
		"put_onlprepare",
		"put_onlrace",
		"put_onlwatch",
		"put_onlleaveroom",
		"put_hrtbt",
		"put_onlracefinish",
		"req_discordinfo",
		"put_miiicon",
		"req_onlinetoken",
		"req_uniquepidv2",
		"req_roomcharids",
		"req_message",
		"put_ultrashortcut",
		"req_badges",
		"req_ptsweekcfg",
		"req_ptsweekbrd",
		"put_ptsweekscore",
		"put_savebackup",
		"req_savebackup",
	};

	NetHandler::Session::Session(const std::string& url) : remoteUrl(url)
	{
		LightEvent_Init(&waitEvent, ResetType::RESET_STICKY);
		LightEvent_Signal(&waitEvent);
		finishedCallback = nullptr;
		finishedCallbackData = nullptr;
		isFinished = true;
		Initialize(false);
	}

	void NetHandler::Session::Initialize()
	{
		runThread = true;
		LightEvent_Init(&threadEvent, RESET_STICKY);
		LightEvent_Init(&threadFinishEvent, RESET_ONESHOT);
		#if CITRA_MODE == 0
		sessionThread = new ThreadEx(sessionFunc, 0x800, 0x20, 1);
		#else
		sessionThread = new ThreadEx(sessionFunc, 0x800, 0x20, 1);
		#endif
		sessionThread->Start(nullptr);
	}

	void NetHandler::Session::Terminate()
	{
		runThread = false;
		LightEvent_Signal(&threadEvent);
		LightEvent_WaitTimeout(&threadFinishEvent, 10LL * 1000LL * 1000LL * 1000LL);
	}

	void NetHandler::Session::Initialize(bool wait)
	{
		if (wait && status == Status::PROCESSING)
			Wait();
		status = Status::IDLE;
		ClearInputData();
		lastRes = 0;
		outDocument.clear();
	}

	NetHandler::Session::~Session()
	{
		Initialize(true);
	}

	void NetHandler::Session::SetData(minibson::document&& data)
	{
		bsontopayload(std::move(data));
	}

	void NetHandler::Session::setFinishedCallback(bool(*callback)(void*), void* arg)
	{
		finishedCallback = callback;
		finishedCallbackData = arg;
	}

	void NetHandler::Session::ClearInputData()
	{
		rawencbson.Clear();
	}

	void NetHandler::Session::Cleanup()
	{
		Initialize(true);
	}	

	minibson::document& NetHandler::Session::GetData()
	{
		if (GetStatus() == Status::SUCCESS)
			return outDocument;
		else {
			outDocument.clear();
			return outDocument;
		}
	}

	void NetHandler::Session::PrepareStart()
	{
		status = Status::PROCESSING;
		isFinished = false;
		LightEvent_Clear(&waitEvent);
	}

	void NetHandler::Session::Wait()
	{
		LightEvent_Wait(&waitEvent);
	}

	void NetHandler::Session::WaitTimeout(const Time& time)
	{
		LightEvent_WaitTimeout(&waitEvent, time.AsMicroseconds() * 1000);
	}

	NetHandler::Session::Status NetHandler::Session::GetStatus()
	{
		Lock lock(pendingSessionsMutex);
		return status;
	}

	bool NetHandler::Session::HasFinished()
	{
		return isFinished;
	}

	void NetHandler::Session::bsontopayload(minibson::document&& data)
	{
		minibson::document encData(std::move(data));


		auto res = minibson::crypto::encrypt(encData);
		if (res.has_value())
			rawencbson = std::move(res.value());
	}

	s32 NetHandler::Session::payloadtobson(u8* data, u32 size)
	{
		outDocument.clear();
		auto outDocOpt = minibson::crypto::decrypt(data, size);
		if (!outDocOpt.has_value()) {
			return ErrInvalidServerResponse;
		}
		outDocument = std::move(outDocOpt.value());
		if (
			outDocument.get<int>("res", -1) < 0
		) {
			outDocument.clear();
			return ErrInvalidServerResponse;
		}


		return 0;
	}

	void NetHandler::Session::sessionFunc(void* arg)
	{		
		std::shared_ptr<Session> currS;
		u32				downSize = 0;
		u32             responseCode = 0;
		u32             totalSize = 0, bytesRead = 0;
		Result          res = 0, initres = 0;
		httpcContext    context;
		char            userAgent[128];
		bool repeatSession = false;

		while (true) {
			LightEvent_Wait(&threadEvent);
			LightEvent_Clear(&threadEvent);

			if (!runThread)
				break;

			const auto& mem = GetHttpcStolenMemory();
			initres = httpcPatchInit((u32)mem.first, mem.second);

			while (true)
			{
				MiscUtils::Buffer buf;
				downSize = 0; responseCode = 0; totalSize = 0; bytesRead = 0;
				if (!repeatSession)
				{
					Lock lock(pendingSessionsMutex);
					if (pendingSessions.empty())
						break;
					currS = pendingSessions.back();
					pendingSessions.pop_back();
				}

				if (R_FAILED(initres) || !currS->rawencbson || !SaveHandler::saveData.flags1.serverCommunication) {
					repeatSession = false;
					currS->status = Status::FAILURE;
					if (R_FAILED(initres))
						currS->lastRes = initres;
					else if (!SaveHandler::saveData.flags1.serverCommunication)
						currS->lastRes = ErrComDis;
					else
						currS->lastRes = ErrNoInput;
					if (currS->finishedCallback)
						repeatSession = currS->finishedCallback(currS->finishedCallbackData);
					if (!repeatSession) currS->isFinished = true;
					if (!repeatSession) LightEvent_Signal(&currS->waitEvent);
					continue;
				}

				if (R_SUCCEEDED(res = httpcOpenContext(&context, HTTPC_METHOD_POST, currS->remoteUrl.c_str(), 0)))
				{
					snprintf(userAgent, sizeof(userAgent), "CTGP7/1.1");
					if (R_SUCCEEDED(res = httpcSetSSLOpt(&context, SSLCOPT_DisableVerify))
						&& R_SUCCEEDED(res = httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_DISABLED))
						&& R_SUCCEEDED(res = httpcAddRequestHeaderField(&context, "User-Agent", userAgent))
						&& R_SUCCEEDED(res = httpcAddRequestHeaderField(&context, "Content-Type", "application/octet-stream"))
						#if CITRA_MODE == 1
						&& R_SUCCEEDED(res = httpcAddRequestHeaderField(&context, "Citra", "1"))
						#endif
						&& R_SUCCEEDED(res = httpcPatchSetPostDataType(&context, HTTPC_PostDataType::HTTP_POSTDATATYPE_RAW))
						&& R_SUCCEEDED(res = httpcBeginRequest(&context))
						&& R_SUCCEEDED(res = httpcPatchSendPostDataRawTimeout(&context, (u32*)currS->rawencbson.Data(), currS->rawencbson.Size(), 10ULL * 1000ULL * 1000ULL * 1000ULL))
						&& R_SUCCEEDED(res = httpcPatchNotifyFinishSendPostData(&context))
						&& R_SUCCEEDED(res = httpcGetResponseStatusCodeTimeout(&context, &responseCode, 10ULL * 1000ULL * 1000ULL * 1000ULL))
						&& R_SUCCEEDED(res = httpcGetDownloadSizeState(&context, NULL, &totalSize)))
					{
						if (responseCode == 200)
						{
							if (totalSize == 0 || totalSize > 0x8000) {
								res = ErrInvalidSize;
							}
							else {
								buf.Resize(totalSize);
								res = httpcDownloadData(&context, buf.Data(), totalSize, &bytesRead);
								if (bytesRead != totalSize)
									res = ErrInvalidDownload;
							}
						}
						else res = NET_ERROR_CODE(responseCode);
					}
					httpcCloseContext(&context);
				}

				currS->ClearInputData();

				if (R_SUCCEEDED(res))
					res = currS->payloadtobson(buf.Data(), buf.Size());

				buf.Clear();

				currS->lastRes = res;

				if (R_SUCCEEDED(res))
					currS->status = Status::SUCCESS;
				else
					currS->status = Status::FAILURE;

				repeatSession = false;
				if (currS->finishedCallback)
					repeatSession = currS->finishedCallback(currS->finishedCallbackData);

				if (!repeatSession) currS->isFinished = true;
				if (!repeatSession) LightEvent_Signal(&currS->waitEvent);
			}
			if (R_SUCCEEDED(initres)) httpcPatchExit();
			currS.reset();
		}

		LightEvent_Signal(&threadFinishEvent);
	}

	u64 NetHandler::GetConsoleUniqueHash()
	{
		return ConsoleUniqueHash;
	}

	u64 NetHandler::GetConsoleSecureHash(int part)
	{
		return ConsoleSecureHash[part & 1];
	}

	std::string NetHandler::GetConsoleUniquePassword()
	{
		return ConsoleUniquePassword;
	}

	std::string NetHandler::GetUserUniqueName()
	{
		return UserUniqueName;
	}

	void NetHandler::InitColsoleUniqueHash()
	{
	}

	NetHandler::RequestHandler::RequestHandler() : session( std::make_unique<Session>(NetHandler::mainServerURL)) { }

	template<class Tin>
	void NetHandler::RequestHandler::AddRequest(RequestType type, const Tin& value)
	{
		Lock l(requestMutex);
		minibson::document reqDoc;
		reqDoc.set<Tin>("value", value);

		doc.set(reqStr[(int)type], reqDoc);
		addedRequests.push_back(type);
	}
	template void NetHandler::RequestHandler::AddRequest<int>(RequestType type, const int& value);

	void NetHandler::RequestHandler::AddRequest(RequestType type, const minibson::document& value)
	{
		Lock l(requestMutex);
		minibson::document reqDoc;
		reqDoc.set("value", value);

		doc.set(reqStr[(int)type], std::move(reqDoc));
		addedRequests.push_back(type);
	}

    void NetHandler::RequestHandler::AddRequest(RequestType type, minibson::document &&value)
    {
		Lock l(requestMutex);
		minibson::document reqDoc;
		reqDoc.set("value", std::move(value));

		doc.set(reqStr[(int)type], std::move(reqDoc));
		addedRequests.push_back(type);
    }

    void NetHandler::RequestHandler::Cleanup()
	{
		Lock l(requestMutex);
		session->Cleanup();
		doc.clear();
		addedRequests.clear();
	}

	void NetHandler::RequestHandler::Start()
	{
		Lock l(requestMutex);
		session->SetData(std::move(doc));
			
		session->PrepareStart();
		{
			Lock lock(Session::pendingSessionsMutex);
			Session::pendingSessions.push_back(session);
		}
		
		LightEvent_Signal(&Session::threadEvent);
	}

	void NetHandler::RequestHandler::Wait()
	{
		session->Wait();
	}

	void NetHandler::RequestHandler::WaitTimeout(const Time& time)
	{
		session->WaitTimeout(time);
	}

	Result NetHandler::RequestHandler::GetLastResult()
	{
		return session->lastRes;
	}

	NetHandler::Session::Status NetHandler::RequestHandler::GetStatus()
	{
		return session->GetStatus();
	}

	bool NetHandler::RequestHandler::HasFinished()
	{
		return session->HasFinished();
	}

	void NetHandler::RequestHandler::SetFinishedCallback(bool(*callback)(RequestHandler*))
	{
		session->setFinishedCallback(reinterpret_cast<bool(*)(void*)>(callback), this);
	}

	template<class Tout>
	int NetHandler::RequestHandler::GetResult(RequestType type, Tout* value)
	{
		int res = ErrResultNotPresent;
		if (session->GetStatus() != Session::Status::SUCCESS)
			return session->lastRes;

		minibson::document& out = session->GetData();

		auto it = out.find(reqStr[(int)type]);
		if (it != out.end() && it->second->get_node_code() == minibson::bson_node_type::document_node) {
			minibson::document* replyDoc = static_cast<minibson::document*>(it->second.get());
			res = replyDoc->get<int>("res", ErrResultNotPresent);
			if (res >= 0)
				*value = replyDoc->get<Tout>("value", *value);
			out.erase(it);
		}

		return res;
	}
	template int NetHandler::RequestHandler::GetResult<int>(RequestType type, int* value);

	int NetHandler::RequestHandler::GetResult(RequestType type, minibson::document* value)
	{
		int res = ErrResultNotPresent;
		if (session->GetStatus() != Session::Status::SUCCESS)
			return session->lastRes;

		minibson::document& out = session->GetData();

		auto it = out.find(reqStr[(int)type]);
		if (it != out.end() && it->second->get_node_code() == minibson::bson_node_type::document_node) {
			minibson::document* replyDoc = static_cast<minibson::document*>(it->second.get());
			res = replyDoc->get<int>("res", ErrResultNotPresent);
			if (res >= 0) {
				auto it2 = replyDoc->find("value");
				if (it2 != replyDoc->end() && it2->second->get_node_code() == minibson::bson_node_type::document_node)
					*value = std::move(*static_cast<minibson::document*>(it2->second.get()));
			}
			out.erase(it);
		}

		return res;
	}

	bool NetHandler::RequestHandler::Contains(RequestType type)
	{
		return std::find(addedRequests.begin(), addedRequests.end(), type) != addedRequests.end();
	}
}
