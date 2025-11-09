/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: rt.hpp
Open source lines: 40/40 (100.00%)
*****************************************************/

#pragma once
#include "3ds.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _RT_HOOK {
	u32 funcAddr;
	u32 dstAddress;
	u32 callCode[4];
} RT_HOOK;

void rtSimpleHook(u32 funcAddr, u32 callbackAddr);
void rtInitHook(RT_HOOK* hook, u32 funcAddr, u32 callbackAddr);
void rtEnableHook(RT_HOOK* hook);
void rtDisableHook(RT_HOOK* hook);
bool rtHookEnabled(RT_HOOK* hook);
u32 rtGenerateJumpCode(u32 dst, u32* buf);

inline __attribute__((always_inline)) u32   decodeARMBranch(const u32 *src)
{
	s32 off = (*src & 0xFFFFFF) << 2;
	off = (off << 6) >> 6; // sign extend

	return (u32)src + 8 + off;
}

#ifdef __cplusplus
}
#endif