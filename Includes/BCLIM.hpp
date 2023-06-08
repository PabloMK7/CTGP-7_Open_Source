/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BCLIM.hpp
Open source lines: 64/64 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
    class BCLIM {
    public:
        enum class TextureFormat : u32 {
            L8 = 0,
            A8 = 1,
            LA4 = 2,
            LA8 = 3,
            HILO8 = 4,
            RGB565 = 5,
            RGB8 = 6,
            RGBA5551 = 7,
            RGBA4 = 8,
            RGBA8 = 9,
            ETC1 = 10,
            ETC1A4 = 11,
            L4 = 12,
            A4 = 13,
        };
        struct Header {
            u32 magic;
            u16 endian;
            u16 headerSize;
            u32 version;
            u32 fileSize;
            u32 blockCount;
            struct {
                u32 magic;
                u32 imagHeaderSize;
                u16 width;
                u16 height;
                TextureFormat format;
            } imag;
            u32 dataLength;
        };
        BCLIM(void* bclimData, u32 bclimSize) : BCLIM(bclimData, (Header*)((u32)bclimData + bclimSize - 0x28)) {}
        BCLIM(void* bclimData, Header* bclimHeader) : data(bclimData), header(bclimHeader) {}

        void Render(Rect<int> position, Render::Interface& renderer, Rect<int> crop = Rect<int>(0, 0, INT32_MAX, INT32_MAX));

    private:
        void* data;
        Header* header;
        static int textureTileOrder[16];
        static int etc1Modifiers[8][2];

        template<typename T>
        T GetDataAt(int offset) {
            return *(T*)(((u32)data) + offset);
        }
    };
}