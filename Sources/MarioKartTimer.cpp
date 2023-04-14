/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartTimer.cpp
Open source lines: 18/18 (100.00%)
*****************************************************/

#include "MarioKartTimer.hpp"
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    std::string MarioKartTimer::Format()
    {
        return Utils::Format("%02d:%02d:%03d", GetMinutes(), GetSeconds(), GetMilliseconds());
    }
}