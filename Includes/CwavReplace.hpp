/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CwavReplace.hpp
Open source lines: 51/51 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "vector"
#include "tuple"
#include "map"

namespace CTRPluginFramework {
    class CwavReplace
    {
    private:
        struct ReplacementCwavInfo {
            u32 times;
            Time validUntil;
            std::map<u32*, void*> cwavData;
        };
        static inline u32 ArchiveIDFileIDToKey(u32 archiveID, u32 fileID) {
            return (archiveID << 16) | (fileID & 0xFFFF);
        }

        static std::map<u32, ReplacementCwavInfo> replacements_map;
        static Mutex replacementsMutex;
    public:
        class KnownIDs {
        public:
            static const constexpr std::pair<u32, u32> coinGetSE = {0x0500002A, 0x0000004C};
            static const constexpr std::pair<u32, u32> bananaDropSE = {0x0500002A, 0x00000084};
            static const constexpr std::pair<u32, u32> starThemeSE = {0x0500002A, 0x00000086};
            static const constexpr std::pair<u32, u32> konohaStartSE = {0x0500002A, 0x000000A6};
            static const constexpr std::pair<u32, u32> konohaEndSE = {0x0500002A, 0x000000A9};
            static const constexpr std::pair<u32, u32> itemDecideSE = {0x0500002A, 0x0000003A};
        };
        static inline void SetReplacementCwav(const std::pair<u32, u32> sound, void* cwavFile, int times = 0, float validForFrames = 0, u32* sndHandle = 0, bool amend = false) {
            SetReplacementCwav(sound.first, sound.second, cwavFile, times, validForFrames, sndHandle, amend);
        }
        static void SetReplacementCwav(u32 archiveID, u32 fileID, void* cwavFile, int times = 0, float validForFrames = 0, u32* sndHandle = 0, bool amend = false);
        static u8* GetReplacementCwav(u8* originalCwav, u32 archiveID, u32 fileID, u32 soundPlayer, u32 trackPlayer, u32 lr);
        static void LockState();
        static void UnlockState();

        static u32 fromWaveLR;
    };    
}