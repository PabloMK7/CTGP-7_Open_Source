/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: rt.cpp
Open source lines: 77/77 (100.00%)
*****************************************************/

#include <cstring>
#include "3ds.h"
#include "rt.hpp"
#include <vector>

Handle hCurrentProcess = 0;
u32 currentPid = 0;

u32 getCurrentProcessId() {
	svcGetProcessId(&currentPid, 0xffff8001);
	return currentPid;
}

u32 getCurrentProcessHandle() {
	u32 handle = 0;
	u32 ret;

	if (hCurrentProcess != 0) {
		return hCurrentProcess;
	}
	svcGetProcessId(&currentPid, 0xffff8001);
	ret = svcOpenProcess(&handle, currentPid);
	if (ret != 0) {
		return 0;
	}
	hCurrentProcess = handle;
	return hCurrentProcess;
}

u32 rtGetPageOfAddress(u32 addr) {
	return (addr / 0x1000) * 0x1000;
}

u32 rtGenerateJumpCode(u32 dst, u32* buf) {
	buf[0] = 0xe51ff004;
	buf[1] = dst;
	return 8;
}

void rtInitHook(RT_HOOK* hook, u32 funcAddr, u32 callbackAddr) {
	hook->isEnabled = 0;
	hook->funcAddr = funcAddr;

	memcpy(hook->bakCode, (void*) funcAddr, 8);
	rtGenerateJumpCode(callbackAddr, hook->jmpCode);
	memcpy(hook->callCode, (void*) funcAddr, 8);
	rtGenerateJumpCode(funcAddr + 8, &(hook->callCode[2]));
}

void rtEnableHook(RT_HOOK* hook) {
	if (hook->isEnabled) {
		return;
	}
	u32* dst = (u32*)hook->funcAddr;
	dst[0] = hook->jmpCode[0];
	dst[1] = hook->jmpCode[1];
	hook->isEnabled = 1;
}

void rtDisableHook(RT_HOOK* hook) {
	if (!hook->isEnabled) {
		return;
	}
	u32* dst = (u32*)hook->funcAddr;
	dst[0] = hook->bakCode[0];
	dst[1] = hook->bakCode[1];
	hook->isEnabled = 0;
}