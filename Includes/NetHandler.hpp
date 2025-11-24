/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: NetHandler.hpp
Open source lines: 167/168 (99.40%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Minibson.hpp"
#include "MiscUtils.hpp"

#define NET_ERROR_CODE(x) (INT32_MIN + x)

namespace CTRPluginFramework {
	class NetHandler
	{
	public:
		inline static constexpr int ErrInitHTTP = NET_ERROR_CODE(1);
		inline static constexpr int ErrComDis = NET_ERROR_CODE(2);
		inline static constexpr int ErrNoInput = NET_ERROR_CODE(3);
		inline static constexpr int ErrInvalidSize = NET_ERROR_CODE(4);
		inline static constexpr int ErrInvalidDownload = NET_ERROR_CODE(5);
		inline static constexpr int ErrResultNotPresent = NET_ERROR_CODE(6);
		inline static constexpr int ErrInvalidServerResponse = NET_ERROR_CODE(7);


		class RequestHandler;
		class Session
		{
		public:
			enum class Status
			{
				IDLE,
				PROCESSING,
				SUCCESS,
				FAILURE
			};

			Session(const std::string& url);
			~Session();
			void SetData(minibson::document&& data);
			void setFinishedCallback(bool(*callback)(void*), void* arg = nullptr);
			void ClearInputData();
			void Cleanup();
			minibson::document& GetData();
			void PrepareStart();
			void Wait();
			void WaitTimeout(const Time& time);
			Status GetStatus();
			bool HasFinished();
			Result lastRes;
			LightEvent waitEvent;
			static std::vector<std::shared_ptr<Session>> pendingSessions;

			static void Initialize();
			static void Terminate();
			static ThreadEx* sessionThread;
		private:
			friend class RequestHandler;
			
			void Initialize(bool wait);
			const std::string& remoteUrl;
			MiscUtils::Buffer rawencbson;
			Status status;
			bool isFinished;
			minibson::document outDocument;
			bool(*finishedCallback)(void*);
			void* finishedCallbackData;
			
			void bsontopayload(minibson::document&& data);
			s32 payloadtobson(u8* data, u32 size);
			static void sessionFunc(void* arg);
			
			static Mutex pendingSessionsMutex;
			static LightEvent threadEvent;
			static LightEvent threadFinishEvent;
			static bool runThread;
		};

		// NOTE: Can be safely destroyed after starting, no need to wait
		class RequestHandler
		{
		public:
			enum class RequestType
			{
				BETA_VER,
				GENERAL_STATS,
				LOGIN,
				LOGOUT,
				ONLINE_SEARCH,
				ONLINE_PREPARING,
				ONLINE_RACING,
				ONLINE_WATCHING,
				ONLINE_LEAVEROOM,
				HEARTBEAT,
				ONLINE_RACEFINISH,
				DISCORD_INFO,
				UPLOAD_MII,
				ONLINETOKEN,
				UNIQUEPID,
				ROOM_CHAR_IDS,
				MESSAGE,
				ULTRASHORTCUT,
				BADGES,
				POINTS_WEEKLY_CONFIG,
				POINTS_LEADER_BOARD,
				POINTS_WEEKLY_SCORE,
				SAVE_BACKUP_PUT,
				SAVE_BACKUP_GET,
			};
			RequestHandler();

			template <class Tin>
			void AddRequest(RequestType type, const Tin& value);
			void AddRequest(RequestType type, const minibson::document& value);
			void AddRequest(RequestType type, minibson::document&& value);

			void Cleanup();
			void Start();
			void Wait();
			void WaitTimeout(const Time& time);
			Result GetLastResult();
			Session::Status GetStatus();
			bool HasFinished();
			void SetFinishedCallback(bool(*callback)(RequestHandler*));

			template <class Tout>
			int GetResult(RequestType type, Tout* value);
			int GetResult(RequestType type, minibson::document* value);
			bool Contains(RequestType type);

		private:
			std::shared_ptr<Session> session;
			minibson::document doc;
			std::vector<RequestType> addedRequests;
			Mutex requestMutex;

			static const char* reqStr[];
		};

		static const std::string mainServerURL;
		static const std::string mainServerHost;

		static u64 GetConsoleUniqueHash();
		static u64 GetConsoleSecureHash(int part);
		static std::string GetConsoleUniquePassword();
		static std::string GetUserUniqueName();
		static void InitColsoleUniqueHash();

		static void SetHttpcStolenMemory(void* addr, size_t size) {
			httpcStolenMemory = {addr, size};
		}
		static const std::pair<void*, size_t>& GetHttpcStolenMemory() {
			return httpcStolenMemory;
		}

	private:
		static u64 ConsoleUniqueHash;
		static u64 ConsoleSecureHash[2];
		static std::string ConsoleUniquePassword;
		static std::string UserUniqueName;
		static std::pair<void*, size_t> httpcStolenMemory;
	};
}