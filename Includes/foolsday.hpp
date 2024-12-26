/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: foolsday.hpp
Open source lines: 22/22 (100.00%)
*****************************************************/

#include <3ds.h>
#include "rt.hpp"

namespace CTRPluginFramework {
    bool checkFoolsDay();
    void setFoolsSeed();
    const char16_t* getFoolsText();
    void applyRaceJoke();
    void loadJokeResources();
    void playSirenJoke();
    void playMemEraseJoke();
    extern bool g_isFoolActive;
}