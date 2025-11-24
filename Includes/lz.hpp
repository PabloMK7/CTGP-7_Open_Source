/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: lz.hpp
Open source lines: 46/46 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "MiscUtils.hpp"

namespace CTRPluginFramework {
    struct LZResult {
        bool good;
        // NOTE: Buffer size does not match output size!
        MiscUtils::Buffer outputBuffer;
        size_t outputSize;
    };
    
    struct LZArg {
        bool isCompress;
        const void* inputAddr;
        size_t inputSize;
        void (*onCompressFinish)(const LZResult&);
    };

    // 1st lock
    void LZ77Lock();
    // 2st call compress
    void LZ77Perform(const LZArg& input);
    // 3nd optionally wait
    void LZ77Wait();
    // 4rd get result (check outputSize != 0)
    const LZResult& LZ77Result();
    // 5th cleanup
    void LZ77Cleanup();
    // 6th unlock
    void LZ77Unlock();
}





