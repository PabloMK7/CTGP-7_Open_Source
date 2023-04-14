/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Net.hpp
Open source lines: 121/125 (96.80%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "cheats.hpp"
#include "NetHandler.hpp"

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
			OFFLINE,
			IDLE,
			SEARCHING,
			WATCHING,
			PREPARING,
			RACING,
			RACE_FINISHED
		};

		static void UpdateOnlineStateMahine(OnlineStateMachine mode, bool titleScreenLogin = false);
		static void WaitOnlineStateMachine();
		static void Initialize();

		static bool IsRunningPretendo();

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

		static u32 lastRoomVRMean;
		static u32 vrPositions[2];
		static float ctwwCPURubberBandMultiplier;
		static float ctwwCPURubberBandOffset;

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