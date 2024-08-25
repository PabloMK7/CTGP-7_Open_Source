/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: main.hpp
Open source lines: 111/120 (92.50%)
*****************************************************/

#pragma once

//#define GOTO_TOADCIRCUIT
//#define LOAD_MINIMUM_RESOURCES
//#define IGNORE_STATS
//#define BETA_BUILD
//#define BETA_PATTERN_FILES
//#define DISCORD_BETA
//#define NOMUSIC
//#define LOG_ONIONFS_FILES
//#define DISABLE_CT_HASH
//#define DEBUG_ONLINE_TRACKS
//#define NO_DISCONNECT_ONLINE
//#define BETA_SARC_FILE
//#define BETA_GHOST_FILE
//#define USE_HOKAKU
//#define DISABLE_ONLINE_ACCESS
//#define ALL_ACHIEVEMENTS
//#define STRESS_TEST_DEMO_TITLE
//#define ALLOW_SAVES_FROM_OTHER_CID
//#define DISABLE_BLUE_COINS
//#define NO_HUD_RACE
//#define SETUP_BLUE_COINS
//#define REPORT_RACE_PROGRESS

#include "CTRPluginFramework.hpp"
#include "Lang.hpp"
#ifndef RELEASE_BUILD
	#include "OSDManager.hpp"
#endif

#define THREADVARS_MAGIC  0x21545624 // !TV$

extern u8 g_isOnlineMode;

namespace CTRPluginFramework
{
	#ifndef RELEASE_BUILD
		#define TRACE  { OSDManager["trace"].SetScreen(true).SetPos(10,50) = std::string(__FUNCTION__) << ":" << __LINE__; Sleep(Seconds(2)); }
		#define XTRACE(id, posx, posy, str, ...) { OSDManager[id].SetScreen(true).SetPos(posx,posy) = std::string(__FUNCTION__) << ": " << __LINE__ << Utils::Format(str, ##__VA_ARGS__); Sleep(Seconds(0.04f)); }
		#define NOXTRACEPOS(id, posx, posy, str, ...) { OSDManager[id].SetScreen(true).SetPos(posx,posy) = Utils::Format(str, ##__VA_ARGS__); /*Sleep(Seconds(0.04f));*/ }
		#define NOXTRACE(id, str, ...) { OSDManager[id].SetScreen(true).SetPos(10,50) = Utils::Format(str, ##__VA_ARGS__); /*Sleep(Seconds(0.04f));*/ }
	#else
		#define TRACE
		#define XTRACE
		#define NOXTRACE
		#define NOXTRACEPOS
	#endif

	struct LaunchSettings {
	};

	#define ISGAMEONLINE (g_isOnlineMode & 0x01)

	void 	InitMainClasses(void);

	void	menucallback();

	void	warnItemUse_apply(bool enabled);
	void 	warnItemUse(MenuEntry *entry);

	void	speedometer_apply();
	void 	speedometersettings(MenuEntry *entry);

	void    ccselector_apply(MenuEntry *entry);
	void    ccselector(MenuEntry *entry);
	void    ccselectorsettings(MenuEntry *entry);

	void backwardscam(MenuEntry *entry);
	void backwardscam_apply(bool active);

	void onMenuChangeCallback(u32 menuID);

	void changeRoundNumber(MenuEntry *entry);

	void courseOrder(MenuEntry* entry);
	void courseOrder_apply(bool isAlphabetical);

	void renderImprovements(MenuEntry* entry);
	void renderImprovements_apply(bool isRenderOptimization);

	void improvedTricks_apply(bool enabled);
	void improvedTricks(MenuEntry* entry);

	void autoAccel_apply(bool enabled);
	void autoAccelSetting(MenuEntry* entry);

	void brakedrift_apply(bool enabled);
	void brakeDrift(MenuEntry* entry);

	void automaticdelaydrift_apply(bool enabled);
	void automaticdelaydrift_entryfunc(MenuEntry* entry);

	void serverEntryHandler(MenuEntry* entry);

	void useCTGP7server_apply(bool useCTGP7);

	void achievementsEntryHandler(MenuEntry* entry);

	void bluecoin_apply(bool enabled);
	void bluecoin_entryfunc(MenuEntry* entry);
}
