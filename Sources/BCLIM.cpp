/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BCLIM.cpp
Open source lines: 287/287 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "BCLIM.hpp"

namespace CTRPluginFramework {

    static inline int ColorClamp(int Color)
    {
        if (Color > 255) Color = 255;
        if (Color < 0) Color = 0;
        return Color;
    }

    const u8 BCLIM::textureTileOrder[] =
	{
		0,  1,   4,  5,
		2,  3,   6,  7,

		8,  9,   12, 13,
		10, 11,  14, 15
	};

    const u8 BCLIM::etc1Modifiers[][2] =
    {
        { 2, 8 },
        { 5, 17 },
        { 9, 29 },
        { 13, 42 },
        { 18, 60 },
        { 24, 80 },
        { 33, 106 },
        { 47, 183 }
    };

    Color BCLIM::TransparentBlendFunc(const Color& dst, const Color& src) {
        return src.Blend(dst, Color::BlendMode::Alpha);
    }

    Color BCLIM::OpaqueBlendFunc(const Color& dst, const Color& src) {
        return dst;
    }

    void BCLIM::RenderInterfaceBackend(void* usrData, bool isRead, Color* c, int posX, int posY) {
        Render::Interface* interface = (Render::Interface*)usrData;
        if (isRead) {
            interface->ReadPixel(posX, posY, *c);
        } else {
            interface->DrawPixel(posX, posY, *c);
        }
    }

    void BCLIM::ScreenBackend(void* usrData, bool isRead, Color* c, int posX, int posY) {
        Screen* screen = (Screen*)usrData;
        if (isRead) {
            screen->ReadPixel(posX, posY, *c);
        } else {
            screen->DrawPixel(posX, posY, *c);
        }
    }

    static bool FastContains(const Rect<int>& rect, Vector<int> point) {
        return point.x >= rect.leftTop.x && point.x < (rect.leftTop.x + rect.size.x) &&
            point.y >= rect.leftTop.y && point.y < (rect.leftTop.y + rect.size.y);
    } 

    void BCLIM::Render(const Rect<int>& position, std::pair<void*, RenderBackend> backend, const Rect<int>& crop, const Rect<int>& limits, std::pair<bool, ColorBlendCallback> colorBlender) {
        auto mappingScale = [](const Rect<int>& position, int x, int y, int w, int h) {
            int posX = position.leftTop.x;
            int posY = position.leftTop.y;
            float progX = x/(float)w;
            float progY = y/(float)h;

            return Vector<int>(posX + position.size.x * progX, posY + position.size.y * progY);
        };
        auto mappingDirect = [](const Rect<int>& position, int x, int y, int w, int h) {
            return Vector<int>(position.leftTop.x + x, position.leftTop.y + y);
        };
        Vector<int>(*mapping)(const Rect<int>& position, int x, int y, int w, int h);
		Color current;

        if (position.size.x == header->imag.width && position.size.y == header->imag.height)
            mapping = mappingDirect;
        else
            mapping = mappingScale;

        switch (header->imag.format) {
            case TextureFormat::L8:
            case TextureFormat::A8:
            case TextureFormat::LA4:
            case TextureFormat::LA8:
            case TextureFormat::HILO8:
            case TextureFormat::RGB8:
            case TextureFormat::RGBA5551:
            case TextureFormat::RGBA8:
            case TextureFormat::ETC1:
            case TextureFormat::L4:
            {
                int offs = 0;
                Vector<int> prevPos(-1, -1);
                for (int y = 0; y < header->imag.height; y+=8) {
                    for (int x = 0; x < header->imag.width; x+=8) {
                        for (int i = 0; i < 64; i++) {
                            int x2 = i % 8;
                            if (x + x2 >= crop.size.x || x + x2 >= header->imag.width) continue;
                            int y2 = i / 8;
                            if (y + y2 >= crop.size.y || y + y2 >= header->imag.height) continue;
                            auto drawPos = mapping(position, x + x2, y + y2, header->imag.width, header->imag.height);
                            if (!FastContains(limits, drawPos)) continue;
                            if (drawPos.x != prevPos.x || drawPos.y != prevPos.y) {
                                prevPos = drawPos;
                                int pos = textureTileOrder[x2 % 4 + y2 % 4 * 4] + 16 * (x2 / 4) + 32 * (y2 / 4);
                                int shift = (pos & 1) * 4;
                                u8 l = ((GetDataAt<u8>(offs + pos / 2) >> shift) & 0xF) * 0x11;
                                if (colorBlender.first)
                                    backend.second(backend.first, true, &current, drawPos.x, drawPos.y);
                                Color finalcolor = colorBlender.second(Color(l, l, l), current);
                                backend.second(backend.first, false, &finalcolor, drawPos.x, drawPos.y);
                            }
                        }
                        offs += 64 / 2;
                    }
                }
            }
            break;
            case TextureFormat::A4:
                break;
            case TextureFormat::RGB565:
            {
                int offs = 0;
                Vector<int> prevPos(-1, -1);
                for (int y = 0; y < header->imag.height; y+=8) {
                    for (int x = 0; x < header->imag.width; x+=8) {
                        for (int i = 0; i < 64; i++) {
                            int x2 = i % 8;
                            if (x + x2 >= crop.size.x || x + x2 >= header->imag.width) continue;
                            int y2 = i / 8;
                            if (y + y2 >= crop.size.y || y + y2 >= header->imag.height) continue;
                            auto drawPos = mapping(position, x + x2, y + y2, header->imag.width, header->imag.height);
                            if (!FastContains(limits, drawPos)) continue;
                            if (drawPos.x != prevPos.x || drawPos.y != prevPos.y) {
                                prevPos = drawPos;
                                int pos = textureTileOrder[x2 % 4 + y2 % 4 * 4] + 16 * (x2 / 4) + 32 * (y2 / 4);
                                u16 pixel = GetDataAt<u16>(offs + pos * 2);
                                u8 b = (u8)((pixel & 0x1F) << 3);
                                u8 g = (u8)((pixel & 0x7E0) >> 3);
                                u8 r = (u8)((pixel & 0xF800) >> 8);
                                if (colorBlender.first)
                                    backend.second(backend.first, true, &current, drawPos.x, drawPos.y);
                                Color finalcolor = colorBlender.second(Color(r, g, b), current);
                                backend.second(backend.first, false, &finalcolor, drawPos.x, drawPos.y);
                            }
                        }
                        offs += 64 * 2;
                    }
                }
            }
            break;
            case TextureFormat::RGBA4:
            {
				int offs = 0;
                Vector<int> prevPos(-1, -1);
				for (int y = 0; y < header->imag.height; y+=8) {
                    for (int x = 0; x < header->imag.width; x+=8) {
                        for (int i = 0; i < 64; i++) {
							int x2 = i % 8;
                            if (x + x2 >= crop.size.x || x + x2 >= header->imag.width) continue;
                            int y2 = i / 8;
                            if (y + y2 >= crop.size.y || y + y2 >= header->imag.height) continue;
                            auto drawPos = mapping(position, x + x2, y + y2, header->imag.width, header->imag.height);
                            if (!FastContains(limits, drawPos)) continue;
                            if (drawPos.x != prevPos.x || drawPos.y != prevPos.y) {
                                prevPos = drawPos;
                                int pos = textureTileOrder[x2 % 4 + y2 % 4 * 4] + 16 * (x2 / 4) + 32 * (y2 / 4);
                                u16 pixel = GetDataAt<u16>(offs + pos * 2);
                                u8 r = ((pixel >> 12) & 0xF) * 16;
                                u8 g = ((pixel >> 8) & 0xF) * 16;
                                u8 b = ((pixel >> 4) & 0xF) * 16;
                                u8 a = ((pixel >> 0) & 0xF) * 16;
                                if (colorBlender.first)
                                    backend.second(backend.first, true, &current, drawPos.x, drawPos.y);
                                Color finalcolor = colorBlender.second(Color(r, g, b, a), current);
                                backend.second(backend.first, false, &finalcolor, drawPos.x, drawPos.y);
                            }
						}
						offs += 64 * 2;
					}
				}
            }
            break;
            case TextureFormat::ETC1A4:
            {
                int offs = 0;
                Vector<int> prevPos(-1, -1);
                for (int y = 0; y < header->imag.height; y += 8)
                {
                    for (int x = 0; x < header->imag.width; x += 8)
                    {
                        for (int i = 0; i < 8; i += 4)
                        {
                            for (int j = 0; j < 8; j += 4)
                            {
                                u64 alpha = GetDataAt<u64>(offs);
                                offs += 8;
                                u64 data = GetDataAt<u64>(offs);
                                bool diffbit = ((data >> 33) & 1) == 1;
                                bool flipbit = ((data >> 32) & 1) == 1; //0: |||, 1: |-|
                                int r1, r2, g1, g2, b1, b2;
                                if (diffbit) //'differential' mode
                                {
                                    int r = (int)((data >> 59) & 0x1F);
                                    int g = (int)((data >> 51) & 0x1F);
                                    int b = (int)((data >> 43) & 0x1F);
                                    r1 = (r << 3) | ((r & 0x1C) >> 2);
                                    g1 = (g << 3) | ((g & 0x1C) >> 2);
                                    b1 = (b << 3) | ((b & 0x1C) >> 2);
                                    r += (int)((data >> 56) & 0x7) << 29 >> 29;
                                    g += (int)((data >> 48) & 0x7) << 29 >> 29;
                                    b += (int)((data >> 40) & 0x7) << 29 >> 29;
                                    r2 = (r << 3) | ((r & 0x1C) >> 2);
                                    g2 = (g << 3) | ((g & 0x1C) >> 2);
                                    b2 = (b << 3) | ((b & 0x1C) >> 2);
                                }
                                else //'individual' mode
                                {
                                    r1 = (int)((data >> 60) & 0xF) * 0x11;
                                    g1 = (int)((data >> 52) & 0xF) * 0x11;
                                    b1 = (int)((data >> 44) & 0xF) * 0x11;
                                    r2 = (int)((data >> 56) & 0xF) * 0x11;
                                    g2 = (int)((data >> 48) & 0xF) * 0x11;
                                    b2 = (int)((data >> 40) & 0xF) * 0x11;
                                }
                                int Table1 = (int)((data >> 37) & 0x7);
                                int Table2 = (int)((data >> 34) & 0x7);
                                for (int y3 = 0; y3 < 4; y3++)
                                {
                                    for (int x3 = 0; x3 < 4; x3++)
                                    {
                                        if (x + j + x3 >= crop.size.x) continue;
                                        if (y + i + y3 >= crop.size.y) continue;
                                        auto drawPos = mapping(position, x + j + x3, y + i + y3, header->imag.width, header->imag.height);
                                        if (!FastContains(limits, drawPos)) continue;
                                        if (drawPos.x != prevPos.x || drawPos.y != prevPos.y) {
                                            prevPos  = drawPos;
                                            int val = (int)((data >> (x3 * 4 + y3)) & 0x1);
                                            bool neg = ((data >> (x3 * 4 + y3 + 16)) & 0x1) == 1;
                                            Color c;
                                            if ((flipbit && y3 < 2) || (!flipbit && x3 < 2))
                                            {
                                                int add = etc1Modifiers[Table1][val] * (neg ? -1 : 1);
                                                c.a = (u8)(((alpha >> ((x3 * 4 + y3) * 4)) & 0xF) * 0x11);
                                                c.r = (u8)ColorClamp(r1 + add);
                                                c.g = (u8)ColorClamp(g1 + add);
                                                c.b = (u8)ColorClamp(b1 + add);
                                            }
                                            else
                                            {
                                                int add = etc1Modifiers[Table2][val] * (neg ? -1 : 1);
                                                c.a = (u8)(((alpha >> ((x3 * 4 + y3) * 4)) & 0xF) * 0x11);
                                                c.r = (u8)ColorClamp(r2 + add);
                                                c.g = (u8)ColorClamp(g2 + add);
                                                c.b = (u8)ColorClamp(b2 + add);
                                            }
                                            if (colorBlender.first)
                                                backend.second(backend.first, true, &current, drawPos.x, drawPos.y);
                                            Color finalcolor = colorBlender.second(c, current);
                                            backend.second(backend.first, false, &finalcolor, drawPos.x, drawPos.y);
                                        }
                                    }
                                }
                                offs += 8;
                            }
                        }
                    }
                }
            }
            break;
        }
    }
}