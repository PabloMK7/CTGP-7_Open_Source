/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: GameAlloc.cpp
Open source lines: 42/42 (100.00%)
*****************************************************/

#include "GameAlloc.hpp"

namespace CTRPluginFramework {
    void* (*GameAlloc::game_operator_new)(u32, u32*, u32) = nullptr;
	void* (*GameAlloc::game_operator_new_autoheap)(u32 size) = nullptr;
	void (*GameAlloc::game_operator_delete)(void* data) = nullptr; // 0x0052A448
	//u8* GameAlloc::buffer = nullptr;
	//u8* GameAlloc::currPos = nullptr;
	u32 GameAlloc::allocated = 0;
	u32 GameAlloc::gameHeap = 0;

	void* GameAlloc::MemAlloc(u32 size, u32 align)
	{
		if (!game_operator_new) return nullptr;
		if (allocated + size > EXTRAHEAPSIZE) return nullptr;
		allocated += size;
		return (u8*)game_operator_new(size, (u32*)gameHeap, align);
		/*
		if (!game_operator_new) return nullptr;
		if (!buffer) {
			buffer = currPos = (u8*)game_operator_new(EXTRAHEAPSIZE, (u32*)gameHeap, 4);
		}
		u32 clearbits = 0;
		if (!align) return nullptr;
		while (!(align & 1)) { align >>= 1; clearbits++;}
		u32 newAddr = (u32)currPos;
		newAddr = (((newAddr - 1) >> clearbits) + 1) << clearbits;
		currPos = (u8*)((u32)currPos + size);
		if (currPos > buffer + EXTRAHEAPSIZE) return nullptr;
		return (void*)newAddr;
		*/
	}
}