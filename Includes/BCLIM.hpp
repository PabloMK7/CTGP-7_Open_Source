/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BCLIM.hpp
Open source lines: 88/88 (100.00%)
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


        using ColorBlendCallback = Color(*)(const Color &, const Color &);
        static std::pair<bool, ColorBlendCallback> TransparentBlend() {
            return std::make_pair(true, TransparentBlendFunc);
        }
        static std::pair<bool, ColorBlendCallback> OpaqueBlend() {
            return std::make_pair(false, OpaqueBlendFunc);
        }

        using RenderBackend = void(*)(void*, bool, Color*, int posX, int posY);
        static std::pair<void*, RenderBackend> RenderInterface(Render::Interface& renderer) {
            return std::make_pair(&renderer, RenderInterfaceBackend);
        }
        static std::pair<void*, RenderBackend> ScreenInterface(const Screen& screen) {
            return std::make_pair((void*)&screen, ScreenBackend);
        }

        void Render(const Rect<int>& position, std::pair<void*, RenderBackend> backend, const Rect<int>& crop = Rect<int>(0, 0, INT32_MAX, INT32_MAX), const Rect<int>& limits = Rect<int>(0, 0, 400, 240), std::pair<bool, ColorBlendCallback> colorBlend = TransparentBlend());

        void* data;
    private:
        Header* header;
        static const u8 textureTileOrder[16];
        static const u8 etc1Modifiers[8][2];

        template<typename T>
        inline T GetDataAt(int offset) {
            return *(T*)(((u32)data) + offset);
        }

        static void RenderInterfaceBackend(void* usrData, bool isRead, Color* c, int posX, int posY);
        static void ScreenBackend(void* usrData, bool isRead, Color* c, int posX, int posY);

        
        static Color TransparentBlendFunc(const Color& dst, const Color& src);
        static Color OpaqueBlendFunc(const Color& dst, const Color& src);
    };
}