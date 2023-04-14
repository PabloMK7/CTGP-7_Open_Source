/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: GameAlloc.hpp
Open source lines: 28/28 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"


namespace CTRPluginFramework {
    class GameAlloc {
    static constexpr u32 EXTRAHEAPSIZE = 0x200000;
    public:
        static void* (*game_operator_new)(u32 size, u32* heap, u32 unk);
        static void* (*game_operator_new_autoheap)(u32 size);
        static void (*game_operator_delete)(void* data);
        static u32 gameHeap;
        static u32 allocated;
        //static u8* buffer;
        //static u8* currPos;
        static void* MemAlloc(u32 size, u32 align = 4);
    };

}
