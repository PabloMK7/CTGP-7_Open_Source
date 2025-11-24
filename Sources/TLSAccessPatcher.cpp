/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: TLSAccessPatcher.cpp
Open source lines: 75/75 (100.00%)
*****************************************************/

#include "TLSAccessPatcher.hpp"
#include "array"
#include "tuple"
#include "main.hpp"
#include <sys/reent.h>
#include "csvc.h"


namespace CTRPluginFramework {

    extern "C" {
        void __gcov_indirect_call_profiler_v4_atomic (long long int, void*);
        void* __getThreadLocalStorage(void);
    }

    std::vector<TLSAccessPatcher::Snippet> TLSAccessPatcher::snippets;

    static void GenerateBranch(void* source, void* dest, bool link) {
        uint32_t* instr = reinterpret_cast<uint32_t*>(source);
        uintptr_t srcAddr = reinterpret_cast<uintptr_t>(source);
        uintptr_t dstAddr = reinterpret_cast<uintptr_t>(dest);

        // Calculate offset: ((dest - (source + 8)) >> 2)
        int32_t offset = static_cast<int32_t>(dstAddr - (srcAddr + 8)) >> 2;

        // Select opcode for B or BL
        uint32_t opcode = link ? 0xEB000000 : 0xEA000000;

        // Encode instruction
        uint32_t branchInstr = opcode | (offset & 0x00FFFFFF);

        // Write branch instruction
        std::memcpy(instr, &branchInstr, sizeof(branchInstr));
    }

    void TLSAccessPatcher::PatchPlugin()
    {
    #if STRESS_MODE == 1  
        u32 profiler = (u32)__gcov_indirect_call_profiler_v4_atomic;
        u32 profiler_body = decodeARMBranch((u32*)(profiler + 4));

        u32 start_addr = profiler_body;
        u32 end_addr = start_addr + 0x60;
        while(start_addr != end_addr) {
            u32 content = *(vu32*)start_addr;
            if ((content & ~0xF000) == 0xEE1D0F70) {
                Process(&snippets.emplace_back(), start_addr);
            }
            start_addr++;
        }
        svcFlushEntireDataCache();
        svcInvalidateEntireInstructionCache();
    #endif
    }

    void TLSAccessPatcher::Process(Snippet *snippet, u32 addr)
    {
        u32* source = reinterpret_cast<u32*>(addr);
        u32 origInst = *source;
        u32 origRegister = (origInst & 0xF000) >> 12;
        snippet->pop_orig_reg = 0xE49D0004 | (origRegister << 12);
        snippet->backaddr = addr + 0x4;
        GenerateBranch(&snippet->blToGetThreadLocalStorage, (void*)__getThreadLocalStorage, true);
        GenerateBranch(source, snippet, false);
    }
}