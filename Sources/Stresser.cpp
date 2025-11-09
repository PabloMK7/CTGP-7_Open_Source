/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Stresser.cpp
Open source lines: 223/223 (100.00%)
*****************************************************/

#include "Stresser.hpp"
#include "main.hpp"
#include "SaveHandler.hpp"
#include "rt.hpp"
#include "stdio.h"
#include "csvc.h"
#include "plgldr.h"

namespace CTRPluginFramework {

    #if STRESS_MODE == 1
    Stresser Stresser::stresser{};
    #endif

    void Stresser::Init()
    {
        currMode = StressMode::NONE;
        random.Init((u32)svcGetSystemTick());
    }

    void Stresser::Calc(PadData* padData)
    {
        u32& pad = padData->currentPad;
        u32 rnd;
        modeTimer++;

        switch (currMode)
        {
        case StressMode::PRESS_A:
            if (modeTimer == 1) {
                waitPressA = random.Get(0, 1 * 60 * 60 + 30 * 60);
            }
            if (modeTimer >= waitPressA) { 
                if ((random.Get() % 2) == 0) pad |= Key::A;
            }                
            break;
        case StressMode::MOVE_PRESS_A:
            rnd = random.Get(0, 101);
            if (rnd < 5) {
                pad |= Key::A;
            } else if (rnd == 5) {
                pad |= Key::B;
            } else if (rnd >= 6 && rnd <= 40) {
                pad |= Key::DPadDown;
            } else if (rnd >= 41 && rnd <= 70) {
                pad |= Key::DPadRight;
            } else if (rnd >= 71 && rnd <= 100) {
                pad |= Key::R;
            }
            break;
        case StressMode::RACE:
            if (timerAccel == 0) {
                accelMode = random.Get(0, 16);
                timerAccel = random.Get(0, 2 * 60);
            } else {
                timerAccel--;
            }
            if (timerDrift == 0) {
                driftMode = random.Get(0, 2);
                timerDrift = random.Get(0, 2 * 60);
            } else {
                timerDrift--;
            }
            if (timerTurn == 0) {
                turnMode = random.Get(0, 8);
                timerTurn = random.Get(0, 1 * 60);
                newAnalogY = ((s16)random.Get(0, 0x9C * 2)) - 0x9C;
            } else {
                timerTurn--;
            }

            if (accelMode < 13) {
                pad |= Key::A;
            } else if (accelMode == 13) {
                pad |= Key::B;
            }

            if (driftMode == 0) {
                pad |= Key::R;
            }
            
            if (turnMode == 0) {
                padData->analogX = -0x9C;
            } else if (turnMode == 1) {
                padData->analogX = 0x9C;
            }
            padData->analogY = newAnalogY;
            if ((random.Get() & 0x1F) == 0) pad |= Key::L;

            if (modeTimer >= 5 * 60 * 60) {
                if ((random.Get() & 0x1F) == 0) {
                    pad |= Key::Start;
                }
                if ((random.Get() & 0xF) == 0) {
                    pad |= Key::A;
                }
            }

            break;
        default:
            break;
        }
        lastPad = pad;
    }

    static RT_HOOK fopenhook = {0};
    static void* fopencallback(const char* file, const char* mode) {

        std::string path(file);
        if (path.starts_with("D:")) {
            static bool isFirst = true;
            if (isFirst) {
                //Reinit services
                srvInit();
                fsInit();
                archiveMountSdmc();
                isFirst = false;
            }

            size_t pos = path.find_last_of("/\\");
            if (pos != std::string::npos) {
                path = path.substr(pos + 1);
            }
            path = "sdmc:/CTGP-7/profile/" + path;
            return ((void*(*)(const char*, const char*))fopenhook.callCode)(path.c_str(), mode);
        }

        return ((void*(*)(const char*, const char*))fopenhook.callCode)(file, mode);
    }

    void g_StresserInit()
    {
    #if STRESS_MODE == 1
        Directory::Remove("/CTGP-7/profile");
        Directory::Create("/CTGP-7/profile");
        PluginHeaderNew* header = reinterpret_cast<PluginHeaderNew*>(0x07000000);
        header->waitForReplyTimeout = 60000000000ULL;
        Stresser::getInstance().Init();
        rtInitHook(&fopenhook, (u32)fopen, (u32)fopencallback);
        rtEnableHook(&fopenhook);
        svcFlushEntireDataCache();
        svcInvalidateEntireInstructionCache();
    #endif
    }

    void g_StresserCalc(PadData *padData)
    {
    #if STRESS_MODE == 1
        Stresser::getInstance().Calc(padData);
    #endif
    }
    u32 g_StresserRnd(u32 min, u32 max)
    {
    #if STRESS_MODE == 1
        return Stresser::getInstance().Rnd(min, max);
    #else
        return min;
    #endif
    }
    u32 g_StresserLastPad() {
    #if STRESS_MODE == 1
        return Stresser::getInstance().LastPad();
    #else
        return 0;
    #endif
    }
    void g_StresserRandomizeSettings()
    {
    #if STRESS_MODE == 1
        Stresser& s = Stresser::getInstance();
        auto& d = SaveHandler::saveData;
        d.cc_settings.enabled = (s.Rnd() & 3) == 0;
        if (d.cc_settings.enabled) {
            d.cc_settings.value = (float)s.Rnd(100, 400);
        }
        d.flags1.backCamEnabled = s.Rnd() % 2;
        d.flags1.warnItemEnabled = s.Rnd() % 2;
        d.flags1.improvedRoulette = s.Rnd() % 2;
        d.flags1.improvedTricks = s.Rnd() % 2;
        d.flags1.autoacceleration = s.Rnd() % 2;
        d.flags1.brakedrift = s.Rnd() % 2;
        d.flags1.automaticDelayDrift = s.Rnd() % 2;
        d.flags1.improvedHonk = s.Rnd() % 2;
        d.speedometer.enabled = (s.Rnd() & 3) != 0;
        if (d.speedometer.enabled) {
            d.speedometer.unit = s.Rnd() % 2;
            d.speedometer.mode = s.Rnd(0, 3);
        }
        SaveHandler::ApplySettings();
    #endif
    }
    void g_StresserUpdateMode(StressMode mode)
    {
    #if STRESS_MODE == 1
        Stresser::getInstance().setMode(mode);
    #endif
    }

    u32 g_StresserGetTitleMenuTimer(u32 origTimer)
    {
    #ifdef STRESS_TEST_DEMO_TITLE
        return 10000;
    #endif
    #ifdef RELEASE_BUILD
         return origTimer;
    #else
    #if STRESS_MODE == 1
        return origTimer;
    #else
        return 0;
    #endif
    #endif // RELEASE_BUILD
    }
}