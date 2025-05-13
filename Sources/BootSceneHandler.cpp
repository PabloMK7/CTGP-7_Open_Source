/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BootSceneHandler.cpp
Open source lines: 354/355 (99.72%)
*****************************************************/

#include "BootSceneHandler.hpp"
#include "main.hpp"
#include "GameAlloc.hpp"
#include "Math.hpp"

namespace CTRPluginFramework {
    RT_HOOK BootSceneHandler::bootRenderThreadCalcHook = { 0 };
    RT_HOOK BootSceneHandler::costCheckerStartHook = { 0 };
    bool BootSceneHandler::drawAdded = false;
    u8* BootSceneHandler::bootrenderthread = nullptr;
    BCLIM* BootSceneHandler::logo_l;
    BCLIM* BootSceneHandler::logo_r;
    BCLIM* BootSceneHandler::logo_v;
    BCLIM* BootSceneHandler::logo_k;
    void* BootSceneHandler::framebuffer = nullptr;
    float BootSceneHandler::logoBlendFactor = 0.f;
    float BootSceneHandler::versionBlendFactor = 0.f;
    float BootSceneHandler::kartBlendFactor = 0.f;
    int BootSceneHandler::globalTimer = 0;
    int BootSceneHandler::disappearingTimer = 0;
    int BootSceneHandler::state4Timer = 0;
    float BootSceneHandler::progbarProgress = 0.0f;
    float BootSceneHandler::progbarTargetProgress = 0.0f;
    bool BootSceneHandler::renderProgbar;
    std::vector<BootSceneHandler::Progressor> BootSceneHandler::progressors;
    BootSceneHandler::ProgressHandle BootSceneHandler::currhandle = 0;
    BootSceneHandler::ProgressHandle BootSceneHandler::gameProgressHandle;
    Mutex BootSceneHandler::progressMutex;
    bool BootSceneHandler::progressChanged = false;

    void BootSceneHandler::Initialize() {
        constexpr int gameProgressSteps = 18;
        gameProgressHandle = RegisterProgress(gameProgressSteps, ProgressGroup::PROGRESS_GAME);
    }

    BootSceneHandler::ProgressHandle BootSceneHandler::RegisterProgress(int amount, ProgressGroup group) {
        ProgressHandle ret = currhandle++;
        progressors.push_back(Progressor{.group = group, .current = 0, .total = amount});
        return ret;
    }

    void BootSceneHandler::Progress(ProgressHandle handle, int progressAmount) {
        Lock lock(progressMutex);
        progressChanged = true;
        Progressor& prog = progressors.at(handle);
        
        prog.current += progressAmount;
        if (prog.current > prog.total) prog.current = prog.total;
    }

    void BootSceneHandler::UpdateProgress() {
        if (!progressChanged) return;
        Lock lock(progressMutex);
        progressChanged = false;
        int currProgress[(int)ProgressGroup::PROGRESS_SIZE] = {0};
        int totalProgress[(int)ProgressGroup::PROGRESS_SIZE] = {0};

        for (auto it = progressors.begin(); it != progressors.end(); it++) {
            currProgress[(int)it->group] += it->current;
            totalProgress[(int)it->group] += it->total;
        }
        constexpr float completionFactor = 1.f / (int)ProgressGroup::PROGRESS_SIZE;
        float prevProg = progbarTargetProgress;
        progbarTargetProgress = 0.f;
        for (int i = 0; i < (int)ProgressGroup::PROGRESS_SIZE; i++) {
            if (totalProgress[i] == 0) {
                totalProgress[i] = 1;
                currProgress[i] = 1;
            }
            progbarTargetProgress += ((float)currProgress[i] / (float)totalProgress[i]) * completionFactor;
        }
        if (prevProg > progbarTargetProgress) progbarTargetProgress = prevProg;
    }

    void BootSceneHandler::OnBootRenderThreadCalc(u8* brt) {
        bootrenderthread = brt;
        u32* bootrenderthread32 = (u32*)bootrenderthread;
        s32& mystatus = ((s32*)bootrenderthread32)[0xC4/4];
        s32& requeststatus = ((s32*)bootrenderthread32)[0xC8/4];
        bool& finished = ((bool*)bootrenderthread)[0xD4];
        if (requeststatus >= 0) {
            mystatus = requeststatus;
            requeststatus = -1;
            if (mystatus == 2) {
                bootrenderthread[0xE4] = bootrenderthread[0xE5] = bootrenderthread[0xE6] = 1;
            }
            if (mystatus == 5) {
                bootrenderthread[0xE4] = bootrenderthread[0xE5] = bootrenderthread[0xE6] = -1;
            }
        }
        if (!drawAdded) {
            #if CITRA_MODE == 1
            svcKernelSetState(0x20000, 0);
            #endif

            drawAdded = true;
            std::string dirbase = "/CTGP-7/gamefs/UI/no_archive/boot/";
            u32* heap = (u32*)(((u32*)bootrenderthread32)[0xCC/4]);

            auto loadBclim = [](BCLIM** out, u32* heap, const std::string& file) {
                File f(file);
                if (!f.IsOpen())
                    panic();
                u32 size = (u32)f.GetSize();
                void* data = GameAlloc::game_operator_new(size, heap, 4);
                if (!data)
                    panic();
                f.Read(data, size);
                *out = new BCLIM(data, size);
            };
            framebuffer = GameAlloc::game_operator_new(240 * 400 * 2, heap, 4);
            loadBclim(&logo_l, heap, dirbase + "clogo_l.bclim");
            loadBclim(&logo_r, heap, dirbase + "clogo_r.bclim");
            loadBclim(&logo_k, heap, dirbase + "clogo_k.bclim");
            loadBclim(&logo_v, heap, dirbase + "clogo_v.bclim");
            if (!framebuffer) panic();
        }
        if (mystatus < 2) {
            ((void(*)(u8*))bootRenderThreadCalcHook.callCode)(bootrenderthread);
            return;
        }
        if (mystatus == 5) {
            requeststatus = 0;
            finished = true;
            GameAlloc::game_operator_delete(logo_v->data);
            GameAlloc::game_operator_delete(logo_k->data);
            GameAlloc::game_operator_delete(logo_r->data);
            GameAlloc::game_operator_delete(logo_l->data);
            delete logo_v;
            delete logo_k;
            delete logo_r;
            delete logo_l;
            GameAlloc::game_operator_delete(framebuffer);

            #if CITRA_MODE == 1
            svcKernelSetState(0x20000, 0xFFFF);
            #endif
        } else {
            Draw();
        }
        //NOXTRACE("sdfdsf", "0xC4: 0x%d, 0xC8: 0x%d, 0xD0: 0x%d, 0xD4: 0x%d", bootrenderthread32[0xC4/4], bootrenderthread32[0xC8/4], bootrenderthread32[0xD0/4], bootrenderthread32[0xD4/4]);
    }

    void BootSceneHandler::OnCostCheckerStart(void* costChecker) {
        Progress(gameProgressHandle);
        ((void(*)(void*))costCheckerStartHook.callCode)(costChecker);
    }

    Color BootSceneHandler::LogoBlend(const Color& dst, const Color& src) {
        Vector3 d = Vector3::FromColor(dst);
        d.Lerp(Vector3(), 1.f - logoBlendFactor);
        Color ret = Vector3::ToColor(d);
        ret.a = 0;
        return ret;
    }

    Color BootSceneHandler::VersionBlend(const Color& dst, const Color& src) {
        Vector3 dstColor = Vector3::FromColor(dst);
        Vector3 srcColor = Vector3::FromColor(src);
        float dstAlpha = (dst.a / 255.f) * versionBlendFactor;
        Vector3 finalColor = srcColor * (1 - dstAlpha) + dstColor * dstAlpha;
        Color ret = Vector3::ToColor(finalColor);
        ret.a = 0;
        return ret;
    }

    Color BootSceneHandler::ReadPixel(void* fb, int posX, int posY) {
        constexpr u32 _bytesPerPixel = 2;
        constexpr u32 _rowSize = 240;
        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;
        union
        {
            u16     u;
            u8      b[2];
        }           half;
        half.u = *reinterpret_cast<u16 *>((u32)fb + offset);
        Color c;
        c.r = (half.u >> 8) & 0xF8;
        c.g = (half.u >> 3) & 0xFC;
        c.b = (half.u << 3) & 0xF8;
        c.a = 255;
        return c;
    }
    void BootSceneHandler::WritePixel(void* fb, int posX, int posY, const Color& c) {
        constexpr u32 _bytesPerPixel = 2;
        constexpr u32 _rowSize = 240;
        u32 offset = (_rowSize - 1 - posY + posX * _rowSize) * _bytesPerPixel;
        union
        {
            u16     u;
            u8      b[2];
        }           half;
        half.u  = (c.r & 0xF8) << 8;
        half.u |= (c.g & 0xFC) << 3;
        half.u |= (c.b & 0xF8) >> 3;
        *reinterpret_cast<u16 *>((u32)fb + offset) = half.u;
    }

    
    void BootSceneHandler::ClearRect(void* fb, const Rect<int>& rect, u16 color) {
        constexpr u32 _bytesPerPixel = 2;
        constexpr u32 _rowSize = 240;
        for (int x = rect.leftTop.x; x < rect.leftTop.x + rect.size.x; x++) {
            for (int y = rect.leftTop.y; y < rect.leftTop.y + rect.size.y; y++) {
                u32 offset = (_rowSize - 1 - y + x * _rowSize) * _bytesPerPixel;
                *reinterpret_cast<u16 *>((u32)fb + offset) = color;
            }
        }
    }

    void BootSceneHandler::BootLogoBackend(void* usrData, bool isRead, Color* c, int posX, int posY) {
        if (isRead) {
            *c = ReadPixel(usrData, posX, posY);
        } else {
            if (renderProgbar) {
                Vector1 d(c->r / 255.f);
                if (posX == 136 || posX == 136 + 127 || posY == 158 || posY == 158 + 15) {
                    // Draw prog bar rectangle
                    d = 1.f;
                } else {
                    // Fade the image in the borders
                    if (posX >= 136 && posX < 136 + 16) {
                        d.Lerp(Vector1(), 1.f - ((posX - 136) / 16.f));
                    } else if (posX >= 248 && posX < 248 + 16) {
                        d.Lerp(Vector1(), ((posX - 248) / 16.f));
                    }
                    
                    // Check the X limit of the prog bar
                    int progbarLimit = (136 - 8) + (progbarProgress * (128 + 15));

                    // Draw the arrow part
                    bool isBlack = false;
                    if (posX < progbarLimit + 8 && posX > progbarLimit - 8) {
                        int offset;
                        if (posY < 166) {
                            progbarLimit += (posY - 158);
                        } else {
                            progbarLimit += 7 - (posY - 166);
                        }
                        const u8 blackMarks[] = {2, 3, 6, 7};
                        for (int i = 0; i < sizeof(blackMarks) / sizeof(u8); i++)
                            isBlack |= (posX == progbarLimit - blackMarks[i]);
                    }

                    // Invert the color if below the prog bar limit
                    if (isBlack)
                        d = std::max(Vector1(0.75f).x - d.x, 0.f);
                    else if (posX <= progbarLimit)
                        d = Vector1(1.f) - d;
                }
                
                d.Lerp(Vector1(), 1.f - kartBlendFactor);
                *c = Vector3::ToColor(Vector3(d.x,d.x,d.x));
            }
            WritePixel(usrData, posX, posY, *c);
        }
    }
    
    void BootSceneHandler::Draw() {
        s32& mystatus = ((s32*)bootrenderthread)[0xC4/4];
        s32& requeststatus = ((s32*)bootrenderthread)[0xC8/4];
        s32& timer = ((s32*)bootrenderthread)[0xD0/4];
        void* finalfb = (void*)((u32*)bootrenderthread)[0xEC/4];
        bool draw = false;
        bool appearing = true;
        Rect<int> nocrop = Rect<int>(0, 0, INT32_MAX, INT32_MAX);
        Rect<int> limits = Rect<int>(0, 0, 400, 240);
        if (mystatus == 2) {
            requeststatus = 3;
        }
        if (mystatus == 3) {
            draw = true;
        }
        if (mystatus == 4) {
            draw = true;
            state4Timer++;
            // Failsafe in case something didn't load properly
            if (state4Timer > 120 && progbarTargetProgress < 1.f) {
                progbarTargetProgress = 1.f;
            }
            if (progbarProgress >= 1.f) {
                appearing = false;
                if (timer)
                    timer--;
                else
                    requeststatus = 5;
            }
        }
        if (draw) {
            UpdateProgress();
            if (!(globalTimer & 1)) {            
                if (globalTimer >= (45 + 45 + 30) && progbarProgress < progbarTargetProgress) {
                    progbarProgress += (2.f / 128.f);
                    if (progbarProgress > progbarTargetProgress) progbarProgress = progbarTargetProgress;
                }
                // Logo
                if (globalTimer < (45 + 30) || disappearingTimer > 44) {
                    float versionMoveFactor;
                    if (appearing) {
                        logoBlendFactor = std::clamp(globalTimer / 45.f, 0.f, 1.f);
                        versionMoveFactor = std::clamp((globalTimer - 30) / 30.f, 0.f, 1.f);
                        versionMoveFactor = 1.f - sqrtf(1.f - ((versionMoveFactor - 1.f) * (versionMoveFactor - 1.f)));
                        versionBlendFactor = 1.f - versionMoveFactor;
                    } else {
                        versionBlendFactor = logoBlendFactor = std::clamp((29 - (disappearingTimer - 44)) / 30.f, 0.f, 1.f);
                        versionMoveFactor = 0.f;
                    }
                    ClearRect(framebuffer, Rect<int>(230, 98, 106, 32), 0);
                    logo_l->Render(Rect<int>(104, 62, 128, 64), BootInterface(framebuffer), nocrop, limits, std::make_pair(false, LogoBlend));
                    logo_r->Render(Rect<int>(232, 62, 64, 64), BootInterface(framebuffer), nocrop, limits, std::make_pair(false, LogoBlend));

                    int posX = 230 + versionMoveFactor * (270 - 230);
                    logo_v->Render(Rect<int>(posX, 98, 64, 32), BootInterface(framebuffer), nocrop, limits, std::make_pair(true, VersionBlend));
                }
                if (globalTimer >= (45 + 45) && disappearingTimer <= 30) {
                    float kartProgress = globalTimer / 180.f;
                    int kartProgressInt = kartProgress;
                    kartProgress -= kartProgressInt;
                    if (appearing) {
                        kartBlendFactor = std::clamp((globalTimer - (45 + 45)) / 30.f, 0.f, 1.f);
                    } else {
                        kartBlendFactor = std::clamp((29 - disappearingTimer) / 30.f, 0.f, 1.f);
                    }
                    Rect<int> kartLimits(136, 158, 128, 16);
                    renderProgbar = true;
                    logo_k->Render(Rect<int>(136 - (128 * kartProgress), 158, 128, 16), BootInterface(framebuffer), nocrop, kartLimits, BCLIM::OpaqueBlend());
                    logo_k->Render(Rect<int>(136 - (128 * kartProgress) + 128, 158, 128, 16), BootInterface(framebuffer), nocrop, kartLimits, BCLIM::OpaqueBlend());
                    renderProgbar = false;
                }
                {
                    constexpr u32 _bytesPerPixel = 2;
                    constexpr u32 _rowSize = 240;
                    constexpr u32 drawStartX = 104;
                    constexpr u32 drawEndX = 336;
                    u32 offsetstart = (_rowSize - 1 - 240 + drawStartX * _rowSize) * _bytesPerPixel;
                    u32 offsetend = (_rowSize - 1 - 0 + drawEndX * _rowSize) * _bytesPerPixel;
                    memcpy((u8*)finalfb + offsetstart, (u8*)framebuffer + offsetstart, offsetend - offsetstart);
                }
            }
            globalTimer++;
            if (!appearing) disappearingTimer++;
        }
    }

}