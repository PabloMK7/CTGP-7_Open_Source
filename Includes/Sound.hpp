/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Sound.hpp
Open source lines: 59/59 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "3ds.h"

namespace CTRPluginFramework {
	
	class Snd {
	public:
		enum SoundID
		{
			NONE = 0,
			CANCEL_M = 0x010004C7,
			CANCEL_L = 0x010004C8,
			SETTING_CHANGE = 0x010004CC,
			SETTING_SELECT_BAR = 0x010004CD,
			BUTTON_INVALID = 0x010004D8,
			SLOT_STOP_TIRE = 0x010004E4, 
			SLOT_OK = 0x010004E6,
			SCROLL_LIST_STOP = 0x010004E8,
			RACE_OK = 0x010004EB,
			FLAG_OPEN = 0x01000502,
			PAUSE_OFF = 0x01000504,
			PAUSE_TO_NEXT = 0x01000509,
			KART_DASH = 0x0100052D,
			BANANA_STAND = 0x01000557,
			BREAK_ITEMBOX = 0x0100054B,
			BOMB_GND = 0x0100055D,
			STAR_THEME_PLAYER = 0x1000560,
			STAR_THEME_ENEMY = 0x1000561,
			TAIL_APPEAR = 0x01000576,
			TAIL_DISAPPEAR = 0x01000577,
			CANNON_MV = 0x010007DB,
			COIN_GET = 0x010007EF,
			CANNON_START_WALUIGI_PINBALL = 0x010008F6,
			KART_DASH_WALUIGI_PINBALL = 0x010008F7,
		};
		
		static void setupGamePlaySeFunc(u32 func);
		static void setupGameSoundObject(u32 object);
		static u32 PlayMenu(SoundID id);

		static void InitializeMenuSounds();

	private:
		static u32 soundObject;
		static u32(*playSysSe)(u32 soundObjet, SoundID id);
	};

}

