/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: LED_Control.hpp
Open source lines: 62/62 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
	typedef struct
	{
		u8 delay;
		u8 smooth;
		u8 loop_delay;
		u8 unknown1;	
	    u8 r[32];
	    u8 g[32];
	    u8 b[32];
	} RGBLedPattern;

	enum class LED_PatType
	{
		CONSTANT = 0, // Constant
		DESCENDENT = 1, // linear up
		ASCENDENT = 2, // linear down
		WAVE_ASC = 3, // sine wave up
		WAVE_DESC = 4, // sine wave down
	};
	
	class LED {
		private:
			friend s32 PatternPlayerTaskfunc(void* arg);
			static Result Play(RGBLedPattern& pattern);
			static void SetSecureTime(Time secureTime);
		public:
			static Result Init();
			static bool IsPatternPlaying();
			static bool PlayLEDPattern(RGBLedPattern& pattern, Time playtime);
			static bool StopLEDPattern();
			static void GeneratePattern(RGBLedPattern& pat, Color color, LED_PatType type, Time delay_time, Time loop_delay, u8 smooth = 0, float r_shift = 0, float g_shift = 0, float b_shift = 0);
	};
}

/* Example
static RGBLedPattern pattern1 = LED::GeneratePattern(Color::Magenta, LED_PatType::WAVE_ASC, Seconds(1), Seconds(0), 10);
static RGBLedPattern pattern2 = LED::GeneratePattern(Color::Yellow, LED_PatType::WAVE_DESC, Seconds(2), Seconds(2), 10);
if (Controller::IsKeyPressed(Key::DPadDown)) {
	LED::PlayLEDPattern(pattern1, Seconds(5)); -> Magenta, Sinus ascendent wave repeating each second with no delay between loops, loops during 5 seconds.
}
if (Controller::IsKeyPressed(Key::DPadUp)) {
	LED::PlayLEDPattern(pattern2, Seconds(9)); -> Yellow, Sinus descendent wave repeating each two seconds with two second delay between loops, loops during 9 seconds.
}

Since the time is stored in one byte, the minimum amount of time is 1/16 seconds.

Infinite delay time (only the first sample is played): delay_time = 0 seconds
loop only once: loop_delay > 16 seconds
*/