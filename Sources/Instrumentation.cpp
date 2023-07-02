/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Instrumentation.cpp
Open source lines: 58/58 (100.00%)
*****************************************************/

#ifdef INSTRUMENT_FUNCTIONS
#define NOTINSTRUMENT __attribute__((no_instrument_function))
#include "map"
#include "CTRPluginFramework.hpp"
namespace CTRPluginFramework {

    std::map<u32, u32> g_profileCount;
    RecursiveLock g_profileLock;
    bool g_disableProfile = true;

    extern "C" void NOTINSTRUMENT __cyg_profile_func_enter (void *this_fn, void *call_site) {
        if (g_disableProfile)
            return;
        RecursiveLock_Lock(&g_profileLock);
        g_disableProfile = true;
        g_profileCount[(u32)this_fn]++;
        g_disableProfile = false;
        RecursiveLock_Unlock(&g_profileLock);
    }

    extern "C" void NOTINSTRUMENT __cyg_profile_func_exit  (void *this_fn, void *call_site) {

    }

    void NOTINSTRUMENT init_instrumentation() {
        RecursiveLock_Init(&g_profileLock);
    }

    void NOTINSTRUMENT start_instrumentation() {
        OSD::Notify("Instrumentation started");
        g_disableProfile = false;
    }

    void NOTINSTRUMENT save_instrumentation() {
        RecursiveLock_Lock(&g_profileLock);
        g_disableProfile = true;
        File outFile("/instrumentation.out", File::RWC | File::TRUNCATE);
        for (auto it = g_profileCount.cbegin(); it != g_profileCount.cend(); it++) {
            struct {u32 addr; u32 count;} info;
            info.addr = it->first;
            info.count = it->second;
            outFile.Write(&info, sizeof(info));
        }
        g_profileCount.clear();
        OSD::Notify("Instrumentation written");
        RecursiveLock_Unlock(&g_profileLock);
    }
}
#endif