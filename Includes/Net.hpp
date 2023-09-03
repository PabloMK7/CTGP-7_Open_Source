/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Net.hpp
Open source lines: 155/159 (97.48%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "cheats.hpp"
#include "NetHandler.hpp"
#include "array"
#include "rt.hpp"

namespace CTRPluginFramework {
	class Net
	{
	public:

		enum class CTWWLoginStatus
		{
			SUCCESS = 0,
			NOTLOGGED = 1,
			PROCESSING = 2,
			FAILED = 3,
			VERMISMATCH = 4,
			MESSAGE = 5,
			MESSAGEKICK = 6,
		};
		static CTWWLoginStatus GetLoginStatus();
		static const std::string& GetServerMessage();
		static void AcknowledgeMessage();
		static u32 GetLastErrorCode();
		static void SetLatestCourseID(u32 courseID);


#ifdef BETA_BUILD
		enum class BetaState
		{
			NONE = 0,
			GETTING = 1,
			YES = 2,
			NO = 3,
			NODISCORD = 4,
			FAILED = 5
		};
		static void StartBetaRequest();
		static BetaState GetBetaState();
		static void SetBetaState(BetaState state);
#endif // BETA_BUILD

		//
		enum class PlayerNameMode
		{
			HIDDEN = 0,
			SHOW = 1,
			CUSTOM = 2
		};
		//

		enum class OnlineStateMachine
		{
			OFFLINE = 0,
			IDLE = 1,
			SEARCHING = 2,
			WATCHING = 3,
			PREPARING = 4,
			RACING = 5,
			RACE_FINISHED = 6,
		};

		static void UpdateOnlineStateMahine(OnlineStateMachine mode, bool titleScreenLogin = false);
		static void WaitOnlineStateMachine();
		static void Initialize();

		static bool IsRunningPretendo();

		struct GameAuthenticationData {
			s32 result{};
			s32 http_status_code{};
			std::array<char, 32> server_address{};
			u16 server_port{};
			u16 padding1{};
			u32 unused{};
			std::array<char, 256> auth_token{};
			u64 server_time{};
		};
		struct CustomGameAuthentication {
			bool populated{false};
			Handle eventHandle{};
			s32 result{};
			std::string server_address{};
			u16 server_port{};
			std::string auth_token{};
			u64 server_time{};
		};
		static CustomGameAuthentication customAuthData;
		static RT_HOOK GetMyPasswordHook;
		static Result OnGetMyPassword(char* passwordOut, u32 maxPasswordSize);
		static RT_HOOK RequestGameAuthenticationDataHook;
		static Result OnRequestGameAuthenticationData(Handle event, u32 serverID, u16* arg2, u8 arg3, u8 arg4);
		static RT_HOOK GetGameAuthenticationDataHook;
		static Result OnGetGameAuthenticationData(GameAuthenticationData* data);

		class DiscordInfo
		{
		private:
		public:
			bool success = false;
			bool isLinked = false;
			u32 linkCode = 0;
			std::string userName;
			std::string userDiscrim;
			std::string userNick;
			bool canBeta = false;
		};
		
		static DiscordInfo GetDiscordInfo(bool requestLink);
		static void DiscordLinkMenu();

		static int lastPressedOnlineModeButtonID;
		static u32 lastRoomVRMean;
		static u32 vrPositions[2];
		static float ctwwCPURubberBandMultiplier;
		static float ctwwCPURubberBandOffset;
		static StarGrade myGrade;
		static std::string trackHistory;

	private:
	#if CITRA_MODE == 0
		static NetHandler::RequestHandler netRequests;
	#endif
		static CTWWLoginStatus lastLoginStatus;
		static OnlineStateMachine currState;
		static std::string lastServerMessage;
		static u64 currLoginToken;
		static u32 lastErrorMessage;
		static u32 latestCourseID;
		static u32 pretendoState;

		static void applyBlockedTrackList();

#if CITRA_MODE == 0
#ifdef BETA_BUILD
		static BetaState betaState;
		static NetHandler::RequestHandler betaRequests;
		static bool OnBetaRequestFinishCallback(NetHandler::RequestHandler* req);
#endif // 

		static bool OnRequestFinishCallback(NetHandler::RequestHandler* req);
#endif
	};
}