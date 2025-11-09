/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: LED_Control.cpp
Open source lines: 155/155 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "LED_Control.hpp"
#include "3DS.h"
#include "math.h"
#include "AsyncRunner.hpp"

#define M_PIF 3.1415926f

namespace CTRPluginFramework {

	static Time currSecureTime = Time::Zero;
	static bool SecureTimeLock = false;
	static Handle ptmsysmHandle = 0;
	static Clock timer;
	static bool isTimeZero = false;

	static void SecureCallback() {
		if (timer.HasTimePassed(currSecureTime) && !isTimeZero) {
			LED::StopLEDPattern();
			AsyncRunner::StopAsync(SecureCallback);
			SecureTimeLock = false;
			currSecureTime = Time::Zero;
		}
	}

	void LED::SetSecureTime(Time secureTime) {
		if (SecureTimeLock) return;
		currSecureTime = secureTime;
		timer.Restart();
		if (secureTime == Time::Zero) isTimeZero = true;
		AsyncRunner::StartAsync(SecureCallback);
		SecureTimeLock = true;
		return;
	}

	bool LED::IsPatternPlaying() {
		return SecureTimeLock;
	}

	Result LED::Play(RGBLedPattern& pat) {
		u32* ipc = getThreadCommandBuffer();
	    ipc[0] = 0x8010640;
	    memcpy(&ipc[1], &pat, 0x64);
	    Result ret = svcSendSyncRequest(ptmsysmHandle);
	    if(ret < 0) return ret;
	    return ipc[1];
	}

	Result LED::Init() {
		if (ptmsysmHandle != 0) {
			return 0;
		}
		Result res = srvGetServiceHandle(&ptmsysmHandle, "ptm:sysm");
		return res;
	}

	s32 PatternPlayerTaskfunc(void* arg) {
		RGBLedPattern& pattern = *(RGBLedPattern*)arg;
		if (LED::Init() < 0) return false;
		if (LED::Play(pattern) < 0) return false;
		return 0;
	}

	static Task ledPatternPlayer(PatternPlayerTaskfunc, nullptr, Task::Affinity::AppCore);

	bool LED::PlayLEDPattern(RGBLedPattern& pattern, Time playtime) {
		if (IsPatternPlaying()) return false;
		ledPatternPlayer.Start(&pattern);
		SetSecureTime(playtime);
		return true;
	}

	static RGBLedPattern g_patEmpty = {0};
	bool LED::StopLEDPattern() {
		if (!IsPatternPlaying()) return false;
		if (isTimeZero) {
			isTimeZero = false;
			return true;
		}
		ledPatternPlayer.Start(&g_patEmpty);
		return true;
	}

	void LED::GeneratePattern(RGBLedPattern& pat, Color color, LED_PatType type, Time delay_time, Time loop_delay, u8 smooth, float r_shift, float g_shift, float b_shift) {

		pat.unknown1 = 0;

		float tempVal = delay_time.AsSeconds() * 0x10;
		if (tempVal >= 0x100) tempVal = 0xFF;
		pat.delay = (u8) tempVal;

		pat.smooth = (u8) smooth;

		tempVal = loop_delay.AsSeconds() * 0x10;
		if (tempVal >= 0x100) tempVal = 0xFF;
		pat.loop_delay = (u8)tempVal;

		float x, xr, xg, xb, dummy;
		switch(type) {
			case (LED_PatType::WAVE_DESC):			
				for(int i = 0; i < 32; i++) {
					x = ((float)i / 31) * 2 * M_PIF;
					pat.r[i] = ((cos(x + (2 * M_PIF * modff(r_shift, &dummy))) + 1) / 2) * color.r;
					pat.g[i] = ((cos(x + (2 * M_PIF * modff(g_shift, &dummy))) + 1) / 2) * color.g;
					pat.b[i] = ((cos(x + (2 * M_PIF * modff(b_shift, &dummy))) + 1) / 2) * color.b;
				}
				break;
			case (LED_PatType::WAVE_ASC):
				for(int i = 0; i < 32; i++) {
					x = ((float)i / 31) * 2 * M_PIF;
					pat.r[i] = ((cos(x + (2 * M_PIF * modff(r_shift, &dummy)) + M_PIF) + 1) / 2) * color.r;
					pat.g[i] = ((cos(x + (2 * M_PIF * modff(g_shift, &dummy)) + M_PIF) + 1) / 2) * color.g;
					pat.b[i] = ((cos(x + (2 * M_PIF * modff(b_shift, &dummy)) + M_PIF) + 1) / 2) * color.b;
				}
				break;
			case (LED_PatType::ASCENDENT):
				for(int i = 0; i < 32; i++) {
					xr = modff((float)i / 32 + r_shift, &dummy);
					xg = modff((float)i / 32 + g_shift, &dummy);
					xb = modff((float)i / 32 + b_shift, &dummy);
					pat.r[i] = (u8)(xr * color.r);
					pat.g[i] = (u8)(xg * color.g);
					pat.b[i] = (u8)(xb * color.b);
				}
				break;
			case (LED_PatType::DESCENDENT):
				for(int i = 0; i < 32; i++) {
					xr = 1 - modff((float)i / 32 + r_shift, &dummy);
					xg = 1 - modff((float)i / 32 + g_shift, &dummy);
					xb = 1 - modff((float)i / 32 + b_shift, &dummy);
					pat.r[i] = (u8)(xr * color.r);
					pat.g[i] = (u8)(xg * color.g);
					pat.b[i] = (u8)(xb * color.b);
				}
				break;
			case (LED_PatType::CONSTANT):
			default:
				for(int i = 0; i < 32; i++) {
					pat.r[i] = color.r;
					pat.g[i] = color.g;
					pat.b[i] = color.b;
				}
				break;
		}
	}
}