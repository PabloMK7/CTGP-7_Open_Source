/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BootSceneHandler.hpp
Open source lines: 79/79 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "rt.hpp"
#include "BCLIM.hpp"
#include "map"

namespace CTRPluginFramework {
    class BootSceneHandler {
    public:
        enum class ProgressGroup {
            PROGRESS_GAME,
            PROGRESS_MOD,

            PROGRESS_SIZE
        };
        using ProgressHandle = int;
        static ProgressHandle RegisterProgress(int amount, ProgressGroup group = ProgressGroup::PROGRESS_MOD);
        static void Progress(ProgressHandle handle, int progressAmount = 1);

        static void Initialize();

        static RT_HOOK bootRenderThreadCalcHook;
        static RT_HOOK costCheckerStartHook;
        static void OnBootRenderThreadCalc(u8* bootrenderthread);
        static void OnCostCheckerStart(void* costChecker);

    private:
        class Progressor {
        public:
            ProgressGroup group;
            int current;
            int total;
        };
        static std::vector<Progressor> progressors;
        static ProgressHandle currhandle;
        static Mutex progressMutex;
        static bool progressChanged;
        static void UpdateProgress();

        static std::pair<void*, BCLIM::RenderBackend> BootInterface(void* fb) {
            return std::make_pair(fb, BootLogoBackend);
        }
        static Color ReadPixel(void* fb, int posX, int posY);
        static void WritePixel(void* fb, int posX, int posY, const Color& c);
        static void ClearRect(void* fb, const Rect<int>& rect, u16 color);
        static void FadeRect(void* fb, const Rect<int>& rect, u8 amount);
        static void BootLogoBackend(void* usrData, bool isRead, Color* c, int posX, int posY);
        static void Draw();
        static bool drawAdded;
        static u8* bootrenderthread;
        static BCLIM* logo_l;
        static BCLIM* logo_r;
        static BCLIM* logo_v;
        static BCLIM* logo_k;
        static void* framebuffer;
        static int globalTimer;
        static int disappearingTimer;
        static int state4Timer;
        static float logoBlendFactor;
        static float versionBlendFactor;
        static float kartBlendFactor;
        static float progbarProgress;
        static float progbarTargetProgress;
        static bool renderProgbar;
        static Color LogoBlend(const Color& dst, const Color& src);
        static Color VersionBlend(const Color& dst, const Color& src);
        static ProgressHandle gameProgressHandle;
        BootSceneHandler() = delete;
    };
}