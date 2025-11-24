/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: GameAlloc.hpp
Open source lines: 65/65 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"


namespace CTRPluginFramework {
    class GameAlloc {
    static constexpr u32 EXTRAHEAPSIZE = 0x640000;
    public:
        static void* (*get_current_heap)();
        static void* (*game_operator_new)(u32 size, u32* heap, u32 unk);
        static void* (*game_operator_new_autoheap)(u32 size);
        static void (*game_operator_delete)(void* data);
        static u32 gameHeap;
        static u32 allocated;
        //static u8* buffer;
        //static u8* currPos;
        static void* MemAlloc(u32 size, u32 align = 4);

        class BasicHeap {
        public:
            BasicHeap() = default;
            BasicHeap(size_t size) {
                base_ptr = (u8*)MemAlloc(size, 0x80);
                current_ptr = current_ptr;
                total_size = size;
            }

            void Clear() {
                current_ptr = base_ptr;
            }

            size_t GetFreeSize() {
                return total_size - (current_ptr - base_ptr);
            }

            u8* Alloc(size_t size, u32 align = 4) {
                if (!base_ptr)
                    return nullptr;
                u32 mask = align - 1;
                current_ptr = (u8*)(((u32)current_ptr + mask) & ~mask);
                if (GetFreeSize() < size) {
                    return nullptr;
                }
                u8* ret = current_ptr;
                current_ptr += size;
                return ret;
            }

        private:
            u8* base_ptr = nullptr;
            u8* current_ptr = nullptr;
            size_t total_size = 0;
        };
    };

}
