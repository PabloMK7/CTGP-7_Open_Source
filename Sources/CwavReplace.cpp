/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CwavReplace.cpp
Open source lines: 98/98 (100.00%)
*****************************************************/

#include "CwavReplace.hpp"
#include "cheats.hpp"

namespace CTRPluginFramework {
    std::vector<std::tuple<u32, u32, u8*, u8, Time>> CwavReplace::replacements;
    Mutex CwavReplace::replacementsMutex;

    //KnownIDs
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::coinGetSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::bananaDropSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::starThemeSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::konohaStartSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::konohaEndSE;
    //

    void CwavReplace::SetReplacementCwav(u32 archiveID, u32 fileID, void* cwavFile, int times, float validForFrames) {
        Lock l(replacementsMutex);
        Time validUntil = Time::Zero;
        if (validForFrames)
            validUntil = Ticks(svcGetSystemTick()) + Seconds(validForFrames / 60.f);
        for (auto it = replacements.begin(); it != replacements.end(); it++) {
            if (std::get<0>(*it) == archiveID && std::get<1>(*it) == fileID)
            {
                if (cwavFile) {
                    bool isValid = std::get<4>(*it) == Time::Zero || std::get<4>(*it) >= Ticks(svcGetSystemTick());
                    u32 prevTimes = isValid ? std::get<3>(*it) : 0;
                    *it = std::make_tuple(std::get<0>(*it), std::get<1>(*it), (u8*)cwavFile, (prevTimes + times), validUntil);
                }
                else replacements.erase(it);
                return;
            }
        }
        if (cwavFile) replacements.push_back(std::make_tuple(archiveID, fileID, (u8*)cwavFile, times, validUntil));
    }

    u8* CwavReplace::GetReplacementCwav(u8* originalCwav, u32 archiveID, u32 fileID) {
        Lock l(replacementsMutex);
        for (auto it = replacements.begin(); it != replacements.end();) {
            if (std::get<0>(*it) == archiveID && std::get<1>(*it) == fileID)
            {
                u8* ret = std::get<2>(*it);
                bool isValid = std::get<4>(*it) == Time::Zero || std::get<4>(*it) >= Ticks(svcGetSystemTick());
                if (!isValid) {
                    it = replacements.erase(it);
                    continue;
                }

                u8 times = std::get<3>(*it);
                if (times) {
                    if (times == 1) it = replacements.erase(it);
                    else *it = std::make_tuple(std::get<0>(*it), std::get<1>(*it), ret, times - 1, std::get<4>(*it));
                }

                return ret;
            }
            ++it;
        }
        
        /*const u8 tempData[] = {0x5B ,0x00 ,0x0F ,0x0F ,0x0F ,0x32 ,0x10 ,0x5D ,0x5B ,0xC6 ,0xD2 ,0xD3 ,0x0D ,0x24 ,0xDE ,0x12};
        const u32 offsetIntoFile = 0xE0;

        if (originalCwav && !memcmp(originalCwav + offsetIntoFile, tempData, sizeof(tempData))) {
            NOXTRACE("asdas", "0x%08X, 0x%08X, %d", archiveID, fileID, counter++);
        }*/
        /*
        if (!waslognext && logNext) 
            ids.clear();
        if (logNext) {
            if (std::find(ids.begin(), ids.end(), fileID) == ids.end())
                ids.push_back(fileID);   
        }
        if (!logNext && ids.size()) {
            std::string log = "";
            for (int i = 0; i < ids.size(); i++)
                log += std::to_string(ids[i]) + " ";
            NOXTRACE("sdfsdfsdfsd", log.c_str());
        }
        waslognext = logNext;
        */
        
        return originalCwav;
    }
    void CwavReplace::LockState() {
        replacementsMutex.Lock();
    }
    void CwavReplace::UnlockState() {
        replacementsMutex.Unlock();
    }
}