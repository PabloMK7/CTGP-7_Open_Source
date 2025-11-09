/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: mallocDebug.cpp
Open source lines: 200/201 (99.50%)
*****************************************************/

#include "mallocDebug.hpp"

#ifdef DEBUG_MALLOC

extern "C" char* __text_end__;

namespace CTRPluginFramework {

    void*(*MallocDebug::getreent)(void);
    u32(*MallocDebug::malloc_r)(void* reent, u32 size);
    void(*MallocDebug::free_r)(void* reent, u32 ptr);
    u32(*MallocDebug::realloc_r)(void* reent, u32 ptr, u32 size);
    u32(*MallocDebug::calloc_r)(void* reent, u32 elcnt, u32 elsz);
    u32(*MallocDebug::memalign_r)(void* reent, u32 align, u32 size);

    RT_HOOK MallocDebug::mallocHook = { 0 };
    RT_HOOK MallocDebug::freeHook = { 0 };
    RT_HOOK MallocDebug::reallocHook = { 0 };
    RT_HOOK MallocDebug::callocHook = { 0 };
    RT_HOOK MallocDebug::memalignHook = { 0 };

    Mutex MallocDebug::allocateMutex;

    MallocDebug::AllocationInfo MallocDebug::allocInfo[8192];
    u32 MallocDebug::totalAllocationSize = 0;

    u32 MallocDebug::mallocThunk(u32 size)
    {
        u32 addr = malloc_r(getreent(), size + 0x8);
        AllocateAddr(addr + 0x4, size);
        return addr + 0x4;
    }

    void MallocDebug::freeThunk(u32 ptr)
    {
        u32 minusSz = FreeAddr(ptr);
        free_r(getreent(), ptr - minusSz);
    }

    u32 MallocDebug::reallocThunk(u32 ptr, u32 size)
    {
        FreeAddr(ptr);
        u32 addr = realloc_r(getreent(), ptr - 0x4, size + 0x8);
        AllocateAddr(addr + 0x4, size);
        return addr + 0x4;
    }

    u32 MallocDebug::callocThunk(u32 elcnt, u32 elsz)
    {
        u32 size = elcnt * elsz;
        u32 addr = malloc_r(getreent(), size + 0x8);
        memset((u8*)(addr + 0x4), 0, size);
        AllocateAddr(addr + 0x4, size);
        return addr + 0x4;
    }

    u32 MallocDebug::memalignThunk(u32 align, u32 size)
    {
        u32 addr = memalign_r(getreent(), align, size);
        AllocateAddr(addr, 0xFFEEFFEE);
        return addr;
    }

    extern "C" void* memalign(u32, u32);

    void MallocDebug::Initialize()
    {
        u32 mallocAddr = (u32)malloc;
        u32 freeAddr = (u32)free;
        u32 reallocAddr = (u32)realloc;
        u32 callocAddr = (u32)calloc;
        u32 memalignAddr = (u32)memalign;

        getreent = (void*(*)(void))decodeARMBranch((u32*)(mallocAddr + 0xC));

        // Very implementation dependent, please check the values are still right after compiling
        malloc_r = (u32(*)(void*,u32))decodeARMBranch((u32*)(mallocAddr + 0x1C));
        free_r = (void(*)(void*,u32))decodeARMBranch((u32*)(freeAddr + 0x1C));
        realloc_r = (u32(*)(void*,u32,u32))decodeARMBranch((u32*)(reallocAddr + 0x24));
        calloc_r = (u32(*)(void*,u32,u32))decodeARMBranch((u32*)(callocAddr + 0x24));
        memalign_r = (u32(*)(void*,u32,u32))decodeARMBranch((u32*)(memalignAddr + 0x24));

        rtInitHook(&mallocHook, mallocAddr, (u32)mallocThunk);
        rtEnableHook(&mallocHook);
        rtInitHook(&freeHook, freeAddr, (u32)freeThunk);
        rtEnableHook(&freeHook);
        rtInitHook(&reallocHook, reallocAddr, (u32)reallocThunk);
        rtEnableHook(&reallocHook);
        rtInitHook(&callocHook, callocAddr, (u32)callocThunk);
        rtEnableHook(&callocHook);
        rtInitHook(&memalignHook, memalignAddr, (u32)memalignThunk);
        rtEnableHook(&memalignHook);
    }

    void MallocDebug::AllocateAddr(u32 addr, u32 sz) {
        Lock  lock(allocateMutex);

        if (addr == 0)
            DumpStackTrace(1, addr);

        u32 insertPos = 0xFFFFFFFF;
        u32 existingPos = 0xFFFFFFFF;
        for (int i = 0; i < allocSize; i++) {
            if (insertPos == 0xFFFFFFFF && allocInfo[i].type == AllocationInfo::NOTUSED)
                insertPos = i;
            
            if (allocInfo[i].affectedAddr == addr) {
                if (allocInfo[i].type == AllocationInfo::NOTFREE)
                    DumpStackTrace(2, addr);
                else
                    existingPos = i;
            }
        }
        if (insertPos == 0xFFFFFFFF && existingPos == 0xFFFFFFFF)
            DumpStackTrace(3, addr);
        if (existingPos != 0xFFFFFFFF)
            insertPos = existingPos;
        allocInfo[insertPos].type = AllocationInfo::NOTFREE;
        allocInfo[insertPos].affectedAddr = addr;
        allocInfo[insertPos].affectedSize = sz;
        if (sz != 0xFFEEFFEE) {
            ((u32*)addr)[-1] = 0xBABECACA;
            *(u32*)(addr + sz) = 0xBABECACB;
            totalAllocationSize += sz;
        }
    }
    u32 MallocDebug::FreeAddr(u32 addr) {
        Lock  lock(allocateMutex);

        if (addr == 0)
            DumpStackTrace(4, addr);

        u32 existingPos = 0xFFFFFFFF;
        for (int i = 0; i < allocSize; i++) {
            if (allocInfo[i].affectedAddr == addr) {
                if (allocInfo[i].type == AllocationInfo::FREE)
                    DumpStackTrace(5, addr);
                else
                    existingPos = i;
            }
        }
        if (existingPos == 0xFFFFFFFF)
            DumpStackTrace(6, addr);

        allocInfo[existingPos].type = AllocationInfo::FREE;
        allocInfo[existingPos].affectedAddr = addr;
        u32 sz = allocInfo[existingPos].affectedSize;

        if (sz == 0xFFEEFFEE) {
            return 0;
        }
        else {
            if (((u32*)addr)[-1] != 0xBABECACA)
                DumpStackTrace(7, addr);
            else if (*(u32*)(addr + sz) != 0xBABECACB)
                DumpStackTrace(8, addr);
            totalAllocationSize -= sz;
            u8* ptr = (u8*)addr;
            u8* endPtr = (u8*)(addr + sz);
            while (ptr != endPtr) *ptr++ = 0x4E;
            return 4;
        }
    }

    static u32 SaveStackPointer() {
        u32 sp_val;
        __asm__ volatile("mov %0, sp" : "=r"(sp_val));
        return sp_val;
    }

    void MallocDebug::DumpStackTrace(u32 reason, u32 addr)
    {
        u32* sp = (u32*)SaveStackPointer();
        volatile u32 stackAddr[0x26] = { 0 };
        u32 stackAddrCnt = 2;
        u32 stackOffset = 0;
        u32 pluginTextStart = 0x07000100;
        u32 pluginTextEnd = (u32)&__text_end__;

        while (stackOffset < 0x2000 / 4 && stackAddrCnt < 0x26) {
            u32 value = sp[stackOffset++];
            if ((value >= pluginTextStart && value <= pluginTextEnd)) {
                stackAddr[stackAddrCnt++] = value;
            }
        }
        stackAddr[0] = 0xCACA0000 | reason;
        stackAddr[1] = addr;
        *(u32*)nullptr = 0;
    }
}
#endif // DEBUG_MALLOC