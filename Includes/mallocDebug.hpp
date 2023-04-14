/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: mallocDebug.hpp
Open source lines: 69/69 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "rt.hpp"

//#define DEBUG_MALLOC

#ifdef DEBUG_MALLOC
namespace CTRPluginFramework {
	class MallocDebug
	{
	public:
		static u32 mallocThunk(u32 size);
		static void freeThunk(u32 ptr);
		static u32 reallocThunk(u32 ptr, u32 size);
		static u32 callocThunk(u32 elcnt, u32 elsz);
		static u32 memalignThunk(u32 align, u32 size);
		static void Initialize();

		static void AllocateAddr(u32 addr, u32 size);
		static u32 FreeAddr(u32 addr);
		static void DumpStackTrace(u32 reason, u32 addr);

		static void*(*getreent)(void);

		static u32(*malloc_r)(void* reent, u32 size);
		static void(*free_r)(void* reent, u32 ptr);
		static u32(*realloc_r)(void* reent, u32 ptr, u32 size);
		static u32(*calloc_r)(void* reent, u32 elcnt, u32 elsz);
		static u32(*memalign_r)(void* reent, u32 align, u32 size);

		static RT_HOOK mallocHook;
		static RT_HOOK freeHook;
		static RT_HOOK reallocHook;
		static RT_HOOK callocHook;
		static RT_HOOK memalignHook;

		static Mutex allocateMutex;

		struct AllocationInfo
		{
			enum AllocationType
			{
				NOTUSED = 0,
				NOTFREE = 1,
				FREE = 2,
			};
			AllocationType type = NOTUSED;
			u32 affectedAddr = 0;
			u32 affectedSize = 0;
		};
		static AllocationInfo allocInfo[8192];
		static constexpr u32 allocSize = sizeof(allocInfo) / sizeof(AllocationInfo);

		static u32 totalAllocationSize;

	private:
		MallocDebug();
	};
}
#endif // DEBUG_MALLOC