/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: rt.cpp
Open source lines: 84/84 (100.00%)
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

void rtSimpleHook(u32 funcAddr, u32 callbackAddr)
{
	rtGenerateJumpCode(callbackAddr, (u32*)funcAddr);
}

void rtInitHook(RT_HOOK *hook, u32 funcAddr, u32 callbackAddr)
{
    hook->funcAddr = funcAddr;
	hook->dstAddress = callbackAddr;

	memcpy(hook->callCode, (void*) funcAddr, 8);
	rtGenerateJumpCode(funcAddr + 8, &(hook->callCode[2]));
}

void rtEnableHook(RT_HOOK* hook) {
	if (rtHookEnabled(hook)) {
		return;
	}
	u32* src = (u32*)hook->funcAddr;
	rtGenerateJumpCode(hook->dstAddress, src);
}

void rtDisableHook(RT_HOOK* hook) {
	if (!rtHookEnabled(hook)) {
		return;
	}
	u32* src = (u32*)hook->funcAddr;
	src[0] = hook->callCode[0];
	src[1] = hook->callCode[1];
}

bool rtHookEnabled(RT_HOOK *hook)
{
	u32* src = (u32*)hook->funcAddr;
	return src[1] == hook->dstAddress;
}
