/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CustomTextEntries.hpp
Open source lines: 30/30 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class CustomTextEntries {
    public:
        // Range entries
        static constexpr u32 customCupStart = 700000;
        static constexpr u32 customTrackStart = 710000;

        // Individual entries
        static constexpr u32 dialog = 720000;
        static constexpr u32 mission = 720001;
        static constexpr u32 missionDesc = 720002;
        static constexpr u32 courseDisplay = 720003; // 6 of them
        static constexpr u32 demoRaceTextTop = 720009;
        static constexpr u32 demoRaceTextBot = 720010;
        static constexpr u32 options = 720011;
        static constexpr u32 optionsDesc = 720012;
    };
}