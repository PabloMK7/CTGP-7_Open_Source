/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MicActivity.cpp
Open source lines: 112/112 (100.00%)
*****************************************************/

#include "MicActivity.hpp"
#include <malloc.h>

namespace CTRPluginFramework {
    MicActivityDetector::~MicActivityDetector() {
        End();
    }

    bool MicActivityDetector::Start() {
        if (running)
            return true;

        buffer = (s8*)memalign(0x1000, bufferSize);
        if (!buffer)
            return false;

        memset(buffer, 0, bufferSize);

        if (R_FAILED(micInit((u8*)buffer, bufferSize))) {
            free(buffer);
            buffer = nullptr;
            return false;
        }

        dataSize = micGetSampleDataSize();

        readPos = micGetLastSampleOffset();
        smoothed = 0.0f;
        level = 0;

        Result res = MICU_StartSampling(
            MICU_ENCODING_PCM8_SIGNED,
            MICU_SAMPLE_RATE_8180,
            0,
            dataSize,
            true
        );

        if (R_FAILED(res)) {
            micExit();
            free(buffer);
            buffer = nullptr;
            return false;
        }            

        running = true;
        return true;
    }

    void MicActivityDetector::End() {
        if (!running)
            return;

        MICU_StopSampling();
        level = 0;
        smoothed = 0.0f;

        micExit();
        free(buffer);
        buffer = nullptr;
        
        running = false;
    }

    void MicActivityDetector::Tick() {
        if (!running)
            return;

        u32 writePos = micGetLastSampleOffset();
        if (writePos == readPos)
            return; // Nothing to read

        u32 count = 0;
        u32 sumAbs = 0;
        u8 peak = 0;

        while (readPos != writePos) {
            int a = abs((int)buffer[readPos]);
            sumAbs += (u32)a;
            if ((u8)a > peak) {
                // Peak value helps more in average calculation
                peak = (u8)a;
            }

            count++;
            readPos = (readPos + 1) % dataSize;
        }

        if (count == 0)
            return;

        float avg = (float)sumAbs / (float)count;
        float norm = avg / 128.0f;
        norm = norm * 0.75f + ((float)peak / 128.0f) * 0.25f;
        smoothed = smoothed * 0.75f + norm * 0.25f;

        if (smoothed < 0.02f)       level = 0;
        else if (smoothed < 0.04f)  level = 1;
        else if (smoothed < 0.08f)  level = 2;
        else if (smoothed < 0.16f)  level = 3;
        else                        level = 4;
    }
}