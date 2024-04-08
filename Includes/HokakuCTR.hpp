/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: HokakuCTR.hpp
Open source lines: 19/19 (100.00%)
*****************************************************/

#pragma once
#include "main.hpp"
#ifdef USE_HOKAKU
#include "RMCLogger.hpp"
namespace CTRPluginFramework
{
    void    InitHokaku();
    extern RMCLogger* mainLogger;
}
#endif