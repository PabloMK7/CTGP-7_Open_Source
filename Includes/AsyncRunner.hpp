/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: AsyncRunner.hpp
Open source lines: 32/32 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "functional"

namespace CTRPluginFramework {
    class AsyncRunner {
    public:
        static void StartAsync(void(*func)(void)) {
            (*PluginMenu::GetRunningInstance()) += func;
            if (AtomicPostIncrement(&counter) == 0) {
                (*PluginMenu::GetRunningInstance()).UpdateEveryOtherFrame(false);
            }            
        }

        static void StopAsync(void(*func)(void)) {
            (*PluginMenu::GetRunningInstance()) -= func;
            if (AtomicDecrement(&counter) == 0) {
                (*PluginMenu::GetRunningInstance()).UpdateEveryOtherFrame(true);
            }
        }
    private:
        static u32 counter;
    };
}