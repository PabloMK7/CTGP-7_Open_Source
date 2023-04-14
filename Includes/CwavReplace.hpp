/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CwavReplace.hpp
Open source lines: 40/40 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "vector"
#include "tuple"


namespace CTRPluginFramework {
    class CwavReplace
    {
    private:
        
    public:
        class KnownIDs {
        public:
            static const constexpr std::pair<u32, u32> coinGetSE = {0x0500002A, 0x0000004C};
            static const constexpr std::pair<u32, u32> bananaDropSE = {0x0500002A, 0x00000084};
            static const constexpr std::pair<u32, u32> starThemeSE = {0x0500002A, 0x00000086};
            static const constexpr std::pair<u32, u32> konohaStartSE = {0x0500002A, 0x000000A6};
            static const constexpr std::pair<u32, u32> konohaEndSE = {0x0500002A, 0x000000A9};
        };
        static inline void SetReplacementCwav(const std::pair<u32, u32> sound, void* cwavFile, int times = 0, float validForFrames = 0) {
            SetReplacementCwav(sound.first, sound.second, cwavFile, times, validForFrames);
        }
        static void SetReplacementCwav(u32 archiveID, u32 fileID, void* cwavFile, int times = 0, float validForFrames = 0);
        static u8* GetReplacementCwav(u8* originalCwav, u32 archiveID, u32 fileID);
        static void LockState();
        static void UnlockState();
        static std::vector<std::tuple<u32, u32, u8*, u8, Time>> replacements;
        static Mutex replacementsMutex;
    };    
}