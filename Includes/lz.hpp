/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: lz.hpp
Open source lines: 37/37 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    struct LZCompressArg {
        void* inputAddr;
        size_t inputSize;
    };

    struct LZCompressResult {
        void* outputAddr;
        size_t outputSize;
    };

    // 1st call compress
    void LZ77Compress(const LZCompressArg& input);
    // 2nd optionally wait
    void LZ77CompressWait();
    // 3rd get result (check outputSize != 0)
    const LZCompressResult& LZ77CompressResult();
    // 4th cleanup
    void LZ77Cleanup();
}





