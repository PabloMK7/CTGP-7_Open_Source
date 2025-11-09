/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Stresser.hpp
Open source lines: 75/75 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "DataStructures.hpp"

namespace CTRPluginFramework {
    enum class StressMode {
        NONE,
        PRESS_A,
        MOVE_PRESS_A,
        RACE,
    };

    class Stresser {
public:

    static Stresser& getInstance() {
        return stresser;
    }

    void setMode(StressMode mode) {
        currMode = mode;
        modeTimer = 0;
    }

    void Init();
    void Calc(PadData* padData);

    u32 Rnd() {
        return random.Get();
    }

    u32 Rnd(u32 min, u32 max) { // [min, max)
        return random.Get(min, max);
    }

    u32 LastPad() {
        return lastPad;
    }

private:
    StressMode currMode = StressMode::NONE;
    SeadRandom random;
    u32 lastPad = 0;
    u32 timerAccel = 0;
    u32 accelMode = 0;
    u32 timerTurn = 0;
    u32 turnMode = 0;
    u32 timerDrift = 0;
    u32 driftMode = 0;
    s16 newAnalogY = 0;
    u32 waitPressA = 0;

    u32 modeTimer = 0;

    static Stresser stresser;
    
};

void g_StresserInit();
void g_StresserCalc(PadData* padData);
u32 g_StresserRnd(u32 min, u32 max);
u32 g_StresserLastPad();
void g_StresserRandomizeSettings();
void g_StresserUpdateMode(StressMode mode);
u32 g_StresserGetTitleMenuTimer(u32 origTimer);
}

