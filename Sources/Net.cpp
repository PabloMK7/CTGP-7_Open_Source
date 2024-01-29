/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Net.cpp
Open source lines: 713/745 (95.70%)
*****************************************************/

#include "Net.hpp"
#include "CourseManager.hpp"
#include "Lang.hpp"
#include "SaveHandler.hpp"
#include "foolsday.hpp"
#include "ExtraResource.hpp"
#include "lz.hpp"
#include "acta.h"
#include "MenuPage.hpp"

extern "C" u32 g_regionID;

namespace CTRPluginFramework {
	int Net::lastPressedOnlineModeButtonID = -1;
	u32 Net::lastRoomVRMean = 1000;
	u32 Net::vrPositions[2] = { 0 };
	float Net::ctwwCPURubberBandMultiplier = 1.f;
	float Net::ctwwCPURubberBandOffset = 0.f;
	StarGrade Net::myGrade = StarGrade::NONE;
	std::string Net::trackHistory;
	NetHandler::RequestHandler Net::netRequests;
	Net::CTWWLoginStatus Net::lastLoginStatus = Net::CTWWLoginStatus::NOTLOGGED;
	Net::OnlineStateMachine Net::currState = Net::OnlineStateMachine::OFFLINE;
	std::string Net::lastServerMessage;
	u64 Net::currLoginToken = 0;
	u32 Net::lastErrorMessage = 0;
	u32 Net::latestCourseID = INVALIDTRACK;
	u32 Net::pretendoState = 2;
	RT_HOOK Net::GetGameAuthenticationDataHook = { 0 };
	RT_HOOK Net::RequestGameAuthenticationDataHook = { 0 };
	RT_HOOK Net::GetMyPrincipalIDHook = { 0 };
	RT_HOOK Net::GetMyPasswordHook = { 0 };
	RT_HOOK Net::wrapMiiDataHook = { 0 };
	RT_HOOK Net::unWrapMiiDataHook = { 0 };
	bool Net::temporaryRedirectCTGP7 = false;
	Net::CustomGameAuthentication Net::customAuthData;

	Net::CTWWLoginStatus Net::GetLoginStatus()
	{
		return lastLoginStatus;
	}

	const std::string& Net::GetServerMessage()
	{
		return lastServerMessage;
	}

	void Net::AcknowledgeMessage()
	{
		if (lastLoginStatus == CTWWLoginStatus::MESSAGE)
			lastLoginStatus = CTWWLoginStatus::SUCCESS;
	}
	u32 Net::GetLastErrorCode()
	{
		return lastErrorMessage;
	}
	void Net::SetLatestCourseID(u32 courseID)
	{
		latestCourseID = courseID;
	}

#ifdef BETA_BUILD
	NetHandler::RequestHandler Net::betaRequests;
	Net::BetaState Net::betaState = Net::BetaState::NONE;

#ifdef DISCORD_BETA
	static Task g_getDiscordInfoTask{[](void* args) {
		Net::DiscordInfo info = Net::GetDiscordInfo(false);
		if (info.success && info.isLinked && info.canBeta) {
			Net::SetBetaState(Net::BetaState::YES);
		} else {
			Net::SetBetaState(Net::BetaState::NODISCORD);
		}
		return (s32)0;
	}};
#endif

	void Net::StartBetaRequest()
	{
		if (betaState != BetaState::NONE)
			return;
		betaState = BetaState::GETTING;
		betaRequests.SetFinishedCallback(OnBetaRequestFinishCallback);
		betaRequests.AddRequest<int>(NetHandler::RequestHandler::RequestType::BETA_VER, lastBetaVersion);
		betaRequests.Start();
	}

	Net::BetaState Net::GetBetaState()
	{
		return betaState;
	}

	void Net::SetBetaState(BetaState state) {
		betaState = state;
	}

	bool Net::OnBetaRequestFinishCallback(NetHandler::RequestHandler* req)
	{
		int def = 0;
		int res = req->GetResult(NetHandler::RequestHandler::RequestType::BETA_VER, &def);
		bool failed = R_FAILED(req->GetLastResult());
		req->Cleanup();
#ifdef DISCORD_BETA
		if (failed)
			betaState = BetaState::FAILED;
		else if (res == 0)
			g_getDiscordInfoTask.Start();
		else
			betaState = BetaState::NO;
#else
		if (failed)
			betaState = BetaState::FAILED;
		else if (res == 0)
			betaState = BetaState::YES;
		else
			betaState = BetaState::NO;
#endif		
		return false;
	}
#endif
	bool Net::OnRequestFinishCallback(NetHandler::RequestHandler* req) {
		int res = -1;
		minibson::document reqDoc;
		lastErrorMessage = req->GetLastResult();
		if (req->Contains(NetHandler::RequestHandler::RequestType::LOGIN)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::LOGIN, &reqDoc);
			if (res < 0)
				lastLoginStatus = CTWWLoginStatus::FAILED;
			else {
				CTWWLoginStatus stat = static_cast<CTWWLoginStatus>(res);
				currLoginToken = reqDoc.get_numerical("token", 0);
				switch (stat)
				{
				case CTWWLoginStatus::NOTLOGGED:
				case CTWWLoginStatus::PROCESSING:
				case CTWWLoginStatus::FAILED:
					stat = CTWWLoginStatus::FAILED;
					break;
				case CTWWLoginStatus::SUCCESS:
				case CTWWLoginStatus::VERMISMATCH:
					break;
				case CTWWLoginStatus::MESSAGE:
					lastServerMessage = reqDoc.get("loginMessage", "Failed to get\nmessage.");
					break;
				case CTWWLoginStatus::MESSAGEKICK:
					lastServerMessage = reqDoc.get("loginMessage", "Failed to get\nkick message");
					break;
				default:
					break;
				}
				if (stat == CTWWLoginStatus::SUCCESS || stat == CTWWLoginStatus::MESSAGE)
				{
					SaveHandler::saveData.ctVR = reqDoc.get("ctvr", 1000);
					SaveHandler::saveData.cdVR = reqDoc.get("cdvr", 1000);
					vrPositions[0] = reqDoc.get("ctvrPos", 0);
					vrPositions[1] = reqDoc.get("cdvrPos", 0);
					g_regionID = reqDoc.get("regionID", 3);
					myGrade = (StarGrade)reqDoc.get("myStarGrade", 0);
				}
				if (g_isFoolActive && (stat == CTWWLoginStatus::SUCCESS || stat == CTWWLoginStatus::MESSAGE))
					stat = CTWWLoginStatus::FAILED;
				lastLoginStatus = stat;
			}
			if (reqDoc.get("needsMiiUpload", false)) {
				LZCompressArg arg;
				arg.inputAddr = MarioKartFramework::GetSelfMiiIcon();
				if (!arg.inputAddr) {
					req->Cleanup();
					return false;
				}
				arg.inputSize = 64 * 64 * 2;
				LZ77Compress(arg);
				LZ77CompressWait();
				LZCompressResult res = LZ77CompressResult();
				minibson::document newDoc;
				newDoc.set("miiIcon", res.outputAddr, res.outputSize);
				newDoc.set<int>("miiIconChecksum", MarioKartFramework::GetSelfMiiIconChecksum() & 0x7FFFFFFF);
				LZ77Cleanup();
				req->Cleanup();
				req->AddRequest(NetHandler::RequestHandler::RequestType::UPLOAD_MII, newDoc);
				req->Start(false);
				return true;
			}
		}
		else if (req->Contains(NetHandler::RequestHandler::RequestType::ONLINE_SEARCH)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_SEARCH, &reqDoc);
			if (static_cast<CTWWLoginStatus>(res) != CTWWLoginStatus::SUCCESS) {
				if (static_cast<CTWWLoginStatus>(res) == CTWWLoginStatus::NOTLOGGED)
				{
					minibson::document newDoc;
					PlayerNameMode currPlayerNameMode = (PlayerNameMode)SaveHandler::saveData.serverDisplayNameMode;
					MarioKartFramework::SavePlayerData sv;
					MarioKartFramework::getMyPlayerData(&sv);
					std::string miiName = sv.miiData.GetName();
					newDoc.set<int>("nameMode", (int)currPlayerNameMode);
					newDoc.set("miiName", (const char*)miiName.c_str());
					newDoc.set<bool>("reLogin", true);
					newDoc.set<bool>("ctgp7network", SaveHandler::saveData.flags1.useCTGP7Server);
					//newDoc.set<bool>("pretendo", Net::IsRunningPretendo());
					if (currPlayerNameMode == PlayerNameMode::SHOW || currPlayerNameMode == PlayerNameMode::CUSTOM) {
						if (sv.miiData.flags.profanity) {
							newDoc.set<int>("nameMode", (int)PlayerNameMode::HIDDEN);
						}
						else {
							if (currPlayerNameMode == PlayerNameMode::SHOW)
								newDoc.set("nameValue", (const char*)miiName.c_str());
							else if (currPlayerNameMode == PlayerNameMode::CUSTOM)
								newDoc.set("nameValue", (std::string(SaveHandler::saveData.serverDisplayCustomName) + " [" + miiName + "]").c_str());
						}
					}
					req->Cleanup();
					req->AddRequest(NetHandler::RequestHandler::RequestType::RELOGIN, newDoc);
					req->Start(false);
					return true;
				}
				else
					MarioKartFramework::dialogBlackOut();
			}
			else {
				res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_SEARCH, &reqDoc);
				if (res < 0)
					MarioKartFramework::dialogBlackOut();
				if (g_getCTModeVal != CTMode::ONLINE_NOCTWW) {
					SaveHandler::saveData.ctVR = reqDoc.get("ctvr", 1000);
					SaveHandler::saveData.cdVR = reqDoc.get("cdvr", 1000);
					vrPositions[0] = reqDoc.get("ctvrPos", 0);
					vrPositions[1] = reqDoc.get("cdvrPos", 0);
				}
				trackHistory = reqDoc.get("trackHistory", "");
			}
		}
		else if (req->Contains(NetHandler::RequestHandler::RequestType::ONLINE_PREPARING)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_PREPARING, &reqDoc);
			if (static_cast<CTWWLoginStatus>(res) != CTWWLoginStatus::SUCCESS)
			{
				if (static_cast<CTWWLoginStatus>(res) == CTWWLoginStatus::MESSAGEKICK) {
					MarioKartFramework::dialogBlackOut(reqDoc.get("loginMessage", "Failed to get\nkick message"));
				}
				else if (static_cast<CTWWLoginStatus>(res) == CTWWLoginStatus::VERMISMATCH) {
					MarioKartFramework::dialogBlackOut(NOTE("update_check"));
				} 
				else {
					MarioKartFramework::dialogBlackOut();
				}
			}
			if (g_getCTModeVal != CTMode::ONLINE_NOCTWW) {
				MarioKartFramework::onlineBotPlayerIDs.clear();
				MarioKartFramework::resetdriverchoices();
				MarioKartFramework::neededCPU = reqDoc.get("neededPlayerAmount", (int)0);
				if (MarioKartFramework::neededCPU) MarioKartFramework::neededCPUCurrentPlayers = MarioKartFramework::GetValidStationIDAmount();
				MarioKartFramework::cpuRandomSeed = reqDoc.get("cpuRandomSeed", (int)1);
				lastRoomVRMean = reqDoc.get("vrMean", (int)1000);
				ctwwCPURubberBandMultiplier = reqDoc.get("rubberBMult", 1.);
				ctwwCPURubberBandOffset = reqDoc.get("rubberBOffset", 0.);
			}
		}
		else if (req->Contains(NetHandler::RequestHandler::RequestType::ONLINE_RACING)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_RACING, &reqDoc);
			if (static_cast<CTWWLoginStatus>(res) != CTWWLoginStatus::SUCCESS)
				MarioKartFramework::dialogBlackOut();
		} else if (req->Contains(NetHandler::RequestHandler::RequestType::ONLINE_RACEFINISH)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_RACEFINISH, &reqDoc);
			trackHistory = reqDoc.get("trackHistory", "");
		} else if (req->Contains(NetHandler::RequestHandler::RequestType::ONLINE_WATCHING)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::ONLINE_WATCHING, &reqDoc);
			if (static_cast<CTWWLoginStatus>(res) != CTWWLoginStatus::SUCCESS)
				MarioKartFramework::dialogBlackOut();
			MarioKartFramework::resetdriverchoices();
			// In watch mode, the player stationIDs may be in another order, so it doesn't really matter as it will be wrong anyways.
			MarioKartFramework::cpuRandomSeed = Utils::Random();
		}
		else if (req->Contains(NetHandler::RequestHandler::RequestType::RELOGIN)) {
			res = req->GetResult(NetHandler::RequestHandler::RequestType::RELOGIN, &reqDoc);
			if (static_cast<CTWWLoginStatus>(res) != CTWWLoginStatus::SUCCESS) {
				MarioKartFramework::dialogBlackOut();
			}
			else {
				currLoginToken = reqDoc.get_numerical("token", 0);
				if (MarioKartFramework::currGatheringID) {
					minibson::document newDoc;
					newDoc.set<u64>("token", currLoginToken);
					newDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
					if (g_getCTModeVal == CTMode::ONLINE_NOCTWW)
						reqDoc.set<int>("gameMode", (lastPressedOnlineModeButtonID == 1) ? 3 : 2);
					else
						reqDoc.set<int>("gameMode", (g_getCTModeVal == CTMode::ONLINE_CTWW) ? 0 : 1);
					req->Cleanup();
					req->AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_SEARCH, newDoc);
					req->Start(false);
					return true;
				}					
			}
		}
		req->Cleanup();
		return false;
	}

	void Net::UpdateOnlineStateMahine(OnlineStateMachine mode, bool titleScreenLogin)
	{	
		if (mode == currState)
			return;

		WaitOnlineStateMachine();

		if (currState == OnlineStateMachine::OFFLINE && mode == OnlineStateMachine::IDLE) // Login
		{
			minibson::document loginDoc;
			PlayerNameMode currPlayerNameMode = (PlayerNameMode)SaveHandler::saveData.serverDisplayNameMode;
			MarioKartFramework::SavePlayerData sv;
			MarioKartFramework::getMyPlayerData(&sv);
			std::string miiName = sv.miiData.GetName();
			loginDoc.set<int>("nameMode", (int)currPlayerNameMode);
			loginDoc.set("miiName", miiName.c_str());
			loginDoc.set<bool>("reLogin", false);
			loginDoc.set<bool>("ctgp7network", SaveHandler::saveData.flags1.useCTGP7Server);
			//loginDoc.set<bool>("pretendo", Net::IsRunningPretendo());
			if (currPlayerNameMode == PlayerNameMode::SHOW || currPlayerNameMode == PlayerNameMode::CUSTOM) {
				if (sv.miiData.flags.profanity) {
					loginDoc.set<int>("nameMode", (int)PlayerNameMode::HIDDEN);
				}
				else {
					if (currPlayerNameMode == PlayerNameMode::SHOW)
						loginDoc.set("nameValue", (const char*)miiName.c_str());
					else if (currPlayerNameMode == PlayerNameMode::CUSTOM)
						loginDoc.set("nameValue", (std::string(SaveHandler::saveData.serverDisplayCustomName) + " [" + miiName + "]").c_str());
				}
			}
			if (titleScreenLogin) loginDoc.set<int>("miiIconChecksum", MarioKartFramework::GetSelfMiiIconChecksum() & 0x7FFFFFFF);
			lastLoginStatus = CTWWLoginStatus::PROCESSING;
			netRequests.AddRequest(NetHandler::RequestHandler::RequestType::LOGIN, loginDoc);
			netRequests.Start();
		}
		else if (mode == OnlineStateMachine::SEARCHING) { // Joining room
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || (g_getCTModeVal == CTMode::ONLINE_NOCTWW && Net::IsOnCTGP7Network())) {
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
				if (g_getCTModeVal == CTMode::ONLINE_NOCTWW)
					reqDoc.set<int>("gameMode", (lastPressedOnlineModeButtonID == 1) ? 3 : 2);
				else
					reqDoc.set<int>("gameMode", (g_getCTModeVal == CTMode::ONLINE_CTWW) ? 0 : 1);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_SEARCH, reqDoc);
				netRequests.Start();
			}
		}
		else if (mode == OnlineStateMachine::PREPARING) { // Preparing room
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || (g_getCTModeVal == CTMode::ONLINE_NOCTWW && Net::IsOnCTGP7Network())) {
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
				reqDoc.set<bool>("imHost", MarioKartFramework::imRoomHost);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_PREPARING, reqDoc);
				netRequests.Start();
				if (g_getCTModeVal != CTMode::ONLINE_NOCTWW)
					applyBlockedTrackList();
			}
			else if (g_getCTModeVal == CTMode::ONLINE_COM || g_getCTModeVal == CTMode::ONLINE_NOCTWW)
			{
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::HEARTBEAT, reqDoc);
				netRequests.Start();
			}
			trackHistory = "";
		}
		else if (mode == OnlineStateMachine::RACING) { // Room start racing
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || (g_getCTModeVal == CTMode::ONLINE_NOCTWW && Net::IsOnCTGP7Network())) {
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
				reqDoc.set("courseSzsID", globalNameData.entries[latestCourseID].name);
				reqDoc.set("ctvr", (int)SaveHandler::saveData.ctVR);
				reqDoc.set("cdvr", (int)SaveHandler::saveData.cdVR);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_RACING, reqDoc);
				netRequests.Start();
			}
		}
		else if (mode == OnlineStateMachine::RACE_FINISHED) { // Room start racing
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || (g_getCTModeVal == CTMode::ONLINE_NOCTWW && Net::IsOnCTGP7Network())) {
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
				reqDoc.set("ctvr", (int)SaveHandler::saveData.ctVR);
				reqDoc.set("cdvr", (int)SaveHandler::saveData.cdVR);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_RACEFINISH, reqDoc);
				netRequests.Start();
			}
		}
		else if (mode == OnlineStateMachine::WATCHING) { // Room start racing
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || (g_getCTModeVal == CTMode::ONLINE_NOCTWW && Net::IsOnCTGP7Network())) {
				minibson::document reqDoc;
				reqDoc.set<u64>("token", currLoginToken);
				reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);
				netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_WATCHING, reqDoc);
				netRequests.Start();
			}
		}
		else if ((currState == OnlineStateMachine::SEARCHING || currState == OnlineStateMachine::PREPARING
			|| currState == OnlineStateMachine::WATCHING || currState == OnlineStateMachine::RACING || currState == OnlineStateMachine::RACE_FINISHED) && mode == OnlineStateMachine::IDLE) {
			minibson::document reqDoc;
			reqDoc.set<u64>("token", currLoginToken);
			netRequests.AddRequest(NetHandler::RequestHandler::RequestType::ONLINE_LEAVEROOM, reqDoc);
			netRequests.Start();
		}
		else if (mode == OnlineStateMachine::OFFLINE) { // Logout
			netRequests.AddRequest(NetHandler::RequestHandler::RequestType::LOGOUT, 0);
			netRequests.Start();
			lastLoginStatus = CTWWLoginStatus::NOTLOGGED;
			lastServerMessage.clear();
			myGrade = StarGrade::NONE;
			trackHistory = "";
		}

		currState = mode;
	}
	void Net::WaitOnlineStateMachine()
	{
		while (!netRequests.HasFinished()) {
			netRequests.WaitTimeout(Seconds(0.1f));
		}
	}
	void Net::Initialize()
	{
		NetHandler::Session::Initialize();
		netRequests.SetFinishedCallback(OnRequestFinishCallback);
	}

	bool Net::IsRunningPretendo() {
		#if CITRA_MODE == 0
		if (pretendoState != 2) return pretendoState == 1;

		u32 act_account_index = 0;
		u32 current_persistent_id = 0;
		u32 pretendo_persistent_id = 0;
		Result initres;
		if (R_SUCCEEDED(initres = actAInit()) &&
		R_SUCCEEDED(ACTA_GetAccountIndexOfFriendAccountId(&act_account_index, 2)) &&
		R_SUCCEEDED(ACTA_GetPersistentId(&current_persistent_id, ACT_CURRENT_ACCOUNT)) &&
		R_SUCCEEDED(ACTA_GetPersistentId(&pretendo_persistent_id, act_account_index))) {
			pretendoState = (current_persistent_id == pretendo_persistent_id) ? 1 : 0;
		}
		if (R_SUCCEEDED(initres)) actAExit();

		return pretendoState == 1;
		#else
		return false;
		#endif	
	}

	bool Net::IsOnCTGP7Network() {
		return SaveHandler::saveData.flags1.useCTGP7Server && !temporaryRedirectCTGP7;
	}

	int Net::OnGetMyPrincipalID() {
		#if CITRA_MODE == 0
		return ((int(*)())GetMyPrincipalIDHook.callCode)();
		#else
		if (SaveHandler::saveData.principalID) {
			return SaveHandler::saveData.principalID;
		} else {
			NetHandler::RequestHandler principalIDHandler;
			minibson::document reqDoc;
			principalIDHandler.AddRequest(NetHandler::RequestHandler::RequestType::UNIQUEPID, reqDoc);
			principalIDHandler.Start();
			principalIDHandler.Wait();
			minibson::document resDoc;
			int res = principalIDHandler.GetResult(NetHandler::RequestHandler::RequestType::UNIQUEPID, &resDoc);
			if (res >= 0) {
				SaveHandler::saveData.principalID = resDoc.get_numerical("unique_pid", 0);
				SaveHandler::SaveSettingsAll();
				return SaveHandler::saveData.principalID;
			} else {
				return 0;
			}
		}
		#endif
	}

	Result Net::OnGetMyPassword(char* passwordOut, u32 maxPasswordSize) {
		if (Net::IsOnCTGP7Network()) {
			strncpy(passwordOut, NetHandler::GetConsoleUniquePassword().c_str(), maxPasswordSize);
			return 0;
		} else {
			return ((Result(*)(char*, u32))GetMyPasswordHook.callCode)(passwordOut, maxPasswordSize);
		}
	}

	static s32 requestOnlineTokenTaskFunc(void* arg) {
		NetHandler::RequestHandler onlineTokenHandler;
		{
			minibson::document reqDoc;
			reqDoc.set("password", NetHandler::GetConsoleUniquePassword().c_str());
			onlineTokenHandler.AddRequest(NetHandler::RequestHandler::RequestType::ONLINETOKEN, reqDoc);
		}

		onlineTokenHandler.Start();
		onlineTokenHandler.Wait();

		minibson::document resDoc;
		int res = onlineTokenHandler.GetResult(NetHandler::RequestHandler::RequestType::ONLINETOKEN, &resDoc);
		if (res >= 0) {
			Net::temporaryRedirectCTGP7 = !resDoc.get("available", false);
			if (Net::temporaryRedirectCTGP7) {
				#if CITRA_MODE == 0
				OSD::Notify("CTGP-7 Network is under maintenance.");
				OSD::Notify("Falling back to Nintendo Network...");
				useCTGP7server_apply(false);
				return ((Result(*)(Handle, u32, const u16*, u8, u8))Net::RequestGameAuthenticationDataHook.callCode)(
					Net::customAuthData.eventHandle, Net::customAuthData.serverID, 
					Net::customAuthData.screenName.c_str(),
					Net::customAuthData.sdkMajor,
					Net::customAuthData.sdkMinor	
				);
				#else
				MarioKartFramework::dialogBlackOut("CTGP-7 Network is under maintenance.");
				return 0;
				#endif
			} else {
				Net::customAuthData.server_address = resDoc.get("address", "");
				Net::customAuthData.server_port = (u16)resDoc.get_numerical("port", 0);
				Net::customAuthData.auth_token = resDoc.get("online_token", "");
				Net::customAuthData.server_time = resDoc.get_numerical("server_time", 0);
				Net::customAuthData.populated = true;
				Net::customAuthData.result = 0;
			}
		} else {
			MarioKartFramework::dialogBlackOut(Utils::Format("%s\nErr: 0x%08X", NAME("fail_conn").c_str(), Net::GetLastErrorCode()));
			Net::temporaryRedirectCTGP7 = false;
			Net::customAuthData.populated = true;
			Net::customAuthData.result = res;
		}
		svcSignalEvent(Net::customAuthData.eventHandle);
		return 0;
	}

	static Task requestOnlineTokenTask{requestOnlineTokenTaskFunc, nullptr, Task::Affinity::AppCore};
	Result Net::OnRequestGameAuthenticationData(Handle event, u32 serverID, u16* arg2, u8 arg3, u8 arg4) {
		if (SaveHandler::saveData.flags1.useCTGP7Server) {
			customAuthData.eventHandle = event;
			customAuthData.serverID = serverID;
			customAuthData.screenName = string16(arg2);
			customAuthData.sdkMajor = arg3;
			customAuthData.sdkMinor = arg4;
			requestOnlineTokenTask.Start();
			return 0;
		} else {
			return ((Result(*)(Handle, u32, u16*, u8, u8))RequestGameAuthenticationDataHook.callCode)(event, serverID, arg2, arg3, arg4);
		}
	}

	Result Net::OnGetGameAuthenticationData(GameAuthenticationData* data) {
		if (IsOnCTGP7Network()) {
			memset(data, 0, sizeof(GameAuthenticationData));
			if (R_SUCCEEDED(customAuthData.result) && customAuthData.populated) {
				data->result = 1;
				data->http_status_code = 200;
				strncpy(data->server_address.data(), customAuthData.server_address.c_str(), data->server_address.size());
				data->server_port = customAuthData.server_port;
				strncpy(data->auth_token.data(), customAuthData.auth_token.c_str(), data->auth_token.size());
				data->server_time = customAuthData.server_time;
			}
			return customAuthData.result;
		} else {
			return ((Result(*)(GameAuthenticationData*))GetGameAuthenticationDataHook.callCode)(data);
		}
	}

	void Net::OnWrapMiiData(u8* output, u8* input) {
		#if CITRA_MODE == 1
		constexpr u32 nonce_offset = 0xA;
		constexpr u32 nonce_size = 0xC;
		constexpr u32 in_size = 0x60;
		
		memcpy(output, input + nonce_offset, nonce_size);
		memcpy(output + nonce_size, input, nonce_offset);
		memcpy(output + nonce_size + nonce_offset, input + nonce_size + nonce_offset, in_size - (nonce_offset + nonce_size));
		#else
		((void(*)(u8*, u8*))wrapMiiDataHook.callCode)(output, input);
		#endif
	}

	void Net::OnUnWrapMiiData(u8* output, u8* input) {
		#if CITRA_MODE == 1
		constexpr u32 nonce_offset = 0xA;
		constexpr u32 nonce_size = 0xC;
		constexpr u32 in_size = 0x70;

		memcpy(output, input + nonce_size, nonce_offset);
		memcpy(output + nonce_offset, input, nonce_size);
		memcpy(output + nonce_offset + nonce_size, input + nonce_offset + nonce_size, in_size - (nonce_offset + nonce_size));
		return;
		#else
		((void(*)(u8*, u8*))unWrapMiiDataHook.callCode)(output, input);
		#endif
	}

	Net::DiscordInfo Net::GetDiscordInfo(bool requestLink)
	{
		Net::DiscordInfo ret;
		#if CITRA_MODE == 0
		NetHandler::RequestHandler discordHandler;
		{
			minibson::document reqDoc;
			reqDoc.set<bool>("request", requestLink);
			discordHandler.AddRequest(NetHandler::RequestHandler::RequestType::DISCORD_INFO, reqDoc);
		}
		
		discordHandler.Start();
		discordHandler.Wait();

		minibson::document resDoc;
		int res = discordHandler.GetResult(NetHandler::RequestHandler::RequestType::DISCORD_INFO, &resDoc);
		if (res < 0)
			ret.success = false;
		else if (res == 1)
		{
			ret.success = true;
			ret.isLinked = false;
			if (requestLink)
			{
				u64 temp = 0;
				ret.linkCode = (u32)resDoc.get_numerical("code", temp);
			}
		} else {
			ret.success = true;
			ret.isLinked = true;
			ret.userName = resDoc.get("name", "");
			ret.userDiscrim = resDoc.get("discrim", "");
			ret.userNick = resDoc.get("nick", "");
			ret.canBeta = resDoc.get<bool>("canBeta", ret.canBeta);
		}
		#endif
		return ret;
	}

	static Net::DiscordInfo* g_keyboarddata;
	void Net::DiscordLinkMenu()
	{
		#if CITRA_MODE == 0
		DiscordInfo info;
		g_keyboarddata = &info;
		Keyboard kbd("dummy");
		info = GetDiscordInfo(false);
		if (!info.success) {
			kbd.GetMessage() = NAME("fail_conn");
			kbd.Populate({Language::MsbtHandler::GetString(2001)});
			kbd.CanAbort(true);
			kbd.Open();
			return;
		} else {
			if (!info.isLinked) {
				kbd.GetMessage() = NAME("disc_info") + "\n\n" + NAME("disc_notlink");
				kbd.Populate({Language::MsbtHandler::GetString(2003), Language::MsbtHandler::GetString(2004)});
				kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
				kbd.CanAbort(true);
				int opt = kbd.Open();
				if (opt == 0) {
					info = GetDiscordInfo(true);
					if (!info.success) {
						kbd.GetMessage() = NAME("fail_conn");
						kbd.Populate({Language::MsbtHandler::GetString(2001)});
						kbd.CanAbort(true);
						kbd.Open();
						return;
					} else if (!info.isLinked && info.linkCode) {
						kbd.GetMessage() = NAME("disc_linksteps1") + "\n\n";
						kbd.GetMessage() += NAME("disc_linksteps2") + "\n\n";
						kbd.GetMessage() += NAME("disc_linksteps3") + "\n\n";
						kbd.GetMessage() += NAME("disc_linksteps4") + "\n";
						kbd.GetMessage() += "@RedYoshiBot#2323 server link " + Utils::Format("%08X", info.linkCode);
						kbd.Populate({Language::MsbtHandler::GetString(2001)});
						kbd.CanAbort(true);
						kbd.Open();
						return;
					}
				}
			} else {
				kbd.GetMessage() = NAME("disc_info") + "\n\n";
				if (info.userDiscrim.empty())
					kbd.GetMessage() += NAME("disc_data") + Utils::Format("\n@%s\n\n", info.userName.c_str());
				else
					kbd.GetMessage() += NAME("disc_data") + Utils::Format("\n%s#%s\n\n", info.userName.c_str(), info.userDiscrim.c_str());
				kbd.GetMessage() += NOTE("disc_data") + Utils::Format("\n%s\n\n",info.userNick.c_str());
				kbd.GetMessage() += info.canBeta ? NAME("disc_beta") : NOTE("disc_beta");
				kbd.Populate({Language::MsbtHandler::GetString(2001)});
				kbd.CanAbort(true);
				kbd.Open();
				return;
			}
		}
		#endif
	}

	void Net::applyBlockedTrackList() {
		MenuPageHandler::MenuSingleCourseBasePage::blockedCourses.clear();
		std::vector<std::string> tracks = TextFileParser::Split(trackHistory);
		for (auto it = tracks.cbegin(); it != tracks.cend(); it++) {
			int courseID = CourseManager::getCourseIDFromName(*it);
			if (courseID >= 0)
				MenuPageHandler::MenuSingleCourseBasePage::blockedCourses.push_back(courseID);
		}
	}
}

