/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveHandlerInfos.hpp
Open source lines: 143/143 (100.00%)
*****************************************************/

#pragma once
#include "types.h"

namespace CTRPluginFramework {

	static constexpr const char* g_saveCodes[] = {
		"0",	// CC_SELECTOR_VALUE
		"1",	// CC_SELECTOR_ENABLED

		"2",	// SPEED_ENABLED
		"3",	// SPEED_GRAPHICAL
		"4",	// SPEED_UNIT

		"55",	// VS_CPU_OPTION
		"6",	// VS_CPU_AMOUNT
		"7",	// VS_ITEM_OPTION
		"8",	// VS_ROUND_AMOUNT
		"9",	// VS_COURSE_OPTION
		"7_1",  // VS_CUSTOM_ITEMS_RANDOM
		"7_2",  // VS_CUSTOM_ITEMS_CUSTOM 

		"a",	// SCREENSHOT_HOTKEY
		"b",	// BACKCAM_ENABLED
		"c",	// WARNITEM_ENABLED
		"d",	// FIRST_OPENING
		"e",	// LOCALZOOMMAP_ENABLED
		"f",	// READY_TO_FOOL
		"g",	// CTWW_ACTIVATED
		"h",	// ALPHABETICAL_ENABLED
		"i",	// SCREENSHOT_ENABLED
		"j",	// IMPROVEDROULETTE_ENABLED
		"k",	// NUMBER_OF_ROUNDS
		"l",	// SERVER_UPLOAD_STATS
		"m",    // SERVER_NAME_MODE
		"n",    // SERVER_NAME_STR
		"enye",	// IMPROVED_TRICKS
		"o",    // CUSTOM_TRACKS_VR
		"p",    // COUNTDOWN_VR
		"q",    // RENDER_OPTIMIZATION
		"r",    // AUTO_ACCELERATION
		"s",    // AUTO_ACCELERATION_USESA
		"t",    // BRAKE_DRIFT
		"u",	// CREDITS_WATCHED
		"vv",	// PENDING_ACHIEVEMENTS
		"w", 	// ACHIEVEMENTS
		"x",    // AUTOMATIC_DELAY_DRIFT
		"y",	// USE_CTGP7_SERVER
		"z",	// PRINCIPAL_ID
		"_0",   // CHAR_HANDLER_DRIVER_CHOICES
		"_1",   // CHAR_HANDLER_DISABLED
		"_2",   // COLLECTED_BLUE_COINS
		"_3",   // PENDING_SPECIAL_ACHIEVEMENTS
		"_4",   // SPECIAL_ACHIEVEMENTS
		"_5",   // CUSTOM_KARTS_ENABLED
		"_6",   // BLUE_COINS_ENABLED
	};

	class CTGP7SaveInfo
	{
	public:
		enum CTGP7SaveCode
		{
			CC_SELECTOR_VALUE = 0,
			CC_SELECTOR_ENABLED,

			SPEED_ENABLED,
			SPEED_MODE,
			SPEED_UNIT,

			VS_CPU_OPTION,
			VS_CPU_AMOUNT,
			VS_ITEM_OPTION,
			VS_ROUND_AMOUNT,
			VS_COURSE_OPTION,
			VS_CUSTOM_ITEMS_RANDOM,
			VS_CUSTOM_ITEMS_CUSTOM,

			SCREENSHOT_HOTKEY,
			BACKCAM_ENABLED,
			WARNITEM_ENABLED,
			FIRST_OPENING,
			LOCALZOOMMAP_ENABLED,
			READY_TO_FOOL,
			CTWW_ACTIVATED,
			ALPHABETICAL_ENABLED,
			SCREENSHOT_ENABLED,
			IMPROVEDROULETTE_ENABLED,
			NUMBER_OF_ROUNDS,

			SERVER_UPLOAD_STATS,
			SERVER_NAME_MODE,
			SERVER_NAME_STR,

			IMPROVED_TRICKS,

			CUSTOM_TRACKS_VR,
			COUNTDOWN_VR,

			RENDER_OPTIMIZATION,

			AUTO_ACCELERATION,
			AUTO_ACCELERATION_USESA,

			BRAKE_DRIFT,

			CREDITS_WATCHED,
			PENDING_ACHIEVEMENTS,
			ACHIEVEMENTS,

			AUTOMATIC_DELAY_DRIFT,

			USE_CTGP7_SERVER,
			PRINCIPAL_ID,

			CHAR_HANDLER_DRIVER_CHOICES,
			CHAR_HANDLER_DISABLED,

			COLLECTED_BLUE_COINS,

			PENDING_SPECIAL_ACHIEVEMENTS,
			SPECIAL_ACHIEVEMENTS,

			CUSTOM_KARTS_ENABLED,
			
			BLUE_COINS_ENABLED,
		};

		static constexpr const char* getSaveCode(CTGP7SaveCode saveCode) {
			return g_saveCodes[(u32)saveCode];
		};
	private:
		CTGP7SaveInfo();
	};
}