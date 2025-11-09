/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: NetHandler.hpp
Open source lines: 147/148 (99.32%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Minibson.hpp"

namespace CTRPluginFramework {
	class NetHandler
	{
	public:
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
			void SetData(const minibson::document& data);
			void setFinishedCallback(bool(*callback)(void*), void* arg = nullptr);
			void ClearInputData();
			void Cleanup();
			const minibson::document& GetData();
			void PrepareStart();
			void Wait();
			void WaitTimeout(const Time& time);
			Status GetStatus();
			bool HasFinished();
			Result lastRes;
			static const minibson::document defaultDoc;
			LightEvent waitEvent;
			static std::vector<std::shared_ptr<Session>> pendingSessions;

			static void Initialize();
			static void Terminate();
			static ThreadEx* sessionThread;
		private:
			friend class RequestHandler;
			
			void Init();
			void Reset();
			const std::string& remoteUrl;
			u8* rawbsondata;
			u32 rawbsonsize;
			Status status;
			bool isFinished;
			minibson::encdocument* outputData;
			bool(*finishedCallback)(void*);
			void* finishedCallbackData;
			
			void bsontopayload(const minibson::document& data);
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
			};
			RequestHandler();

			template <class Tin>
			void AddRequest(RequestType type, const Tin& value);
			void AddRequest(RequestType type, const minibson::document& value);

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

	private:
		static u64 ConsoleUniqueHash;
		static u64 ConsoleSecureHash[2];
		static std::string ConsoleUniquePassword;
		static std::string UserUniqueName;
	};
}