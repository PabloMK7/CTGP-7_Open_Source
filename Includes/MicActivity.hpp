/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MicActivity.hpp
Open source lines: 51/51 (100.00%)
*****************************************************/

#pragma once

#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class MicActivityDetector
    {
    public:
        MicActivityDetector() = default;

        ~MicActivityDetector();

        bool Start();

        void End();

        void Tick();

        bool IsRunning() const {
            return running;
        }

        int GetLevel() const {
            return level;
        }

        float GetRawLevel() const {
            return smoothed;
        }

    private:
        s8*   buffer = nullptr;
        u32   bufferSize = 0x1000;
        u32   dataSize{};
        u32   readPos{};

        bool  running{};

        int   level{};
        float smoothed{};
    };
}