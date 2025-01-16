/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Net.hpp
Open source lines: 208/212 (98.11%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "main.hpp"
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
		static Clock heartBeatClock;
		static void HeartBeat();

		static void UpdateOnlineStateMahine(OnlineStateMachine mode, bool titleScreenLogin = false);
		static void WaitOnlineStateMachine();
		static void Initialize();

		static bool IsOnCTGP7Network();
		static bool IsPrivateNetwork() {
			return privateRoomID != 0;
		}

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
			
			// Input
			Handle eventHandle{};
			u32 serverID;
			std::u16string screenName;
			u8 sdkMajor;
			u8 sdkMinor;

			// Output
			s32 result{};
			std::string server_address{};
			u16 server_port{};
			std::string auth_token{};
			u64 server_time{};
		};
		static bool temporaryRedirectCTGP7;
		static bool friendsLoggedInCTGP7;
		static u64 privateRoomID;
		static CustomGameAuthentication customAuthData;
		static RT_HOOK friendsLoginHook;
		static int OnFriendsLogin(Handle handle);
		static RT_HOOK friendsGetLastResponseResultHook;
		static int OnFriendsGetLastResponseResult();
		static RT_HOOK friendsIsLoggedInHook;
		static int OnFriendsIsLoggedIn(bool* logged);
		static RT_HOOK friendsLogoutHook;
		static int OnFriendsLogout();
		static RT_HOOK GetMyPrincipalIDHook;
		static int OnGetMyPrincipalID();
		static RT_HOOK GetMyPasswordHook;
		static Result OnGetMyPassword(char* passwordOut, u32 maxPasswordSize);
		static RT_HOOK RequestGameAuthenticationDataHook;
		static Result OnRequestGameAuthenticationData(Handle event, u32 serverID, char16_t* arg2, u8 arg3, u8 arg4);
		static RT_HOOK GetGameAuthenticationDataHook;
		static Result OnGetGameAuthenticationData(GameAuthenticationData* data);
		static RT_HOOK wrapMiiDataHook;
		static void OnWrapMiiData(u8* output, u8* input);
		static RT_HOOK unWrapMiiDataHook;
		static void OnUnWrapMiiData(u8* output, u8* input);
		static RT_HOOK setPrimaryNATCheckServerHook;
		static void OnSetPrimaryNATCheckServer(u32 rootTransport, const char16_t* server, u16 startPort, u16 endPort);
		static RT_HOOK setSecondaryNATCheckServerHook;
		static void OnSetSecondaryNATCheckServer(u32 rootTransport, const char16_t* server, u16 startPort, u16 endPort);
		static RT_HOOK onProcessNotificationEventHook;
		static void OnProcessNotificationEvent(u32 own, u32* notificationEvent);

		static Task checkMessageAndKickTask;
		static s32 checkMessageAndKickTaskfunc(void* arg);

		static Task ultraShortcutTask;
		static void ReportUltraShortcut() {
			ultraShortcutTask.Start();
		}

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
		static void UnlinkDiscord();
		static void DiscordLinkMenu();

		static int lastPressedOnlineModeButtonID;
		static u32 lastRoomVRMean;
		static u32 vrPositions[2];
		static float ctwwCPURubberBandMultiplier;
		static float ctwwCPURubberBandOffset;
		static StarGrade myGrade;
		static std::string trackHistory;
		static std::string allowedTracks;
		static std::string allowedItems;
		static float vrMultiplier;
		static std::vector<u64> whiteListedCharacters;
		static std::string myServerName;
		static std::string othersServerNames[MAX_PLAYER_AMOUNT];
		static u64 othersBadgeIDs[MAX_PLAYER_AMOUNT];

		static bool IsCharacterBlocked(EDriverID driverID, u64 characterID);

	private:
		static NetHandler::RequestHandler netRequests;
		static CTWWLoginStatus lastLoginStatus;
		static OnlineStateMachine currState;
		static std::string lastServerMessage;
		static u64 currLoginToken;
		static u32 lastErrorMessage;
		static u32 latestCourseID;
		static u32 pretendoState;

		static void applyAllowedItems();
		static void applyBlockedTrackList();

		static s32 reportUltraShortcutTaskfunc(void* arg);

#ifdef BETA_BUILD
		static BetaState betaState;
		static NetHandler::RequestHandler betaRequests;
		static bool OnBetaRequestFinishCallback(NetHandler::RequestHandler* req);
#endif // 

		static bool OnRequestFinishCallback(NetHandler::RequestHandler* req);
	};
}