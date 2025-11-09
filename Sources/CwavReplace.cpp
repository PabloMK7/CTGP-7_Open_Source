/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CwavReplace.cpp
Open source lines: 102/102 (100.00%)
*****************************************************/

#include "CwavReplace.hpp"
#include "main.hpp"
#include "CharacterHandler.hpp"

namespace CTRPluginFramework {
    std::map<u32, CwavReplace::ReplacementCwavInfo> CwavReplace::replacements_map;
    Mutex CwavReplace::replacementsMutex;
    u32 CwavReplace::fromWaveLR;

    //KnownIDs
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::coinGetSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::bananaDropSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::starThemeSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::konohaStartSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::konohaEndSE;
    const constexpr std::pair<u32, u32> CwavReplace::KnownIDs::itemDecideSE;
    //

    void CwavReplace::SetReplacementCwav(u32 archiveID, u32 fileID, void* cwavFile, int times, float validForFrames, u32* sndHandle, bool amend) {
        Lock l(replacementsMutex);
        Time validUntil = Time::Zero;
        u32 key = ArchiveIDFileIDToKey(archiveID, fileID);
        if (validForFrames)
            validUntil = Ticks(svcGetSystemTick()) + Seconds(validForFrames / 60.f);

        auto it = replacements_map.find(key);
        if (it != replacements_map.end()) {
            ReplacementCwavInfo& info = it->second;
            if (cwavFile) {
                bool isValid = info.validUntil == Time::Zero || info.validUntil >= Ticks(svcGetSystemTick());
                u32 prevTimes = isValid ? info.times : 0;
                if (!amend) {
                    info = ReplacementCwavInfo{.times = (prevTimes + times), .validUntil = validUntil};
                }
                info.cwavData[sndHandle] = cwavFile;
                return;
            } else {
                replacements_map.erase(it);
            }
        } else {
            if (cwavFile) {
                ReplacementCwavInfo& info = replacements_map[key];
                info = ReplacementCwavInfo{.times = (u32)times, .validUntil = validUntil};
                info.cwavData[sndHandle] = cwavFile;
            }
        }
    }

    static u32 g_incomingsndhandle;
    u8* CwavReplace::GetReplacementCwav(u8* originalCwav, u32 archiveID, u32 fileID, u32 soundPlayer, u32 trackPlayer, u32 lr) {
        Lock l(replacementsMutex);
        u32 key = ArchiveIDFileIDToKey(archiveID, fileID);

        auto it = replacements_map.find(key);
        if (it != replacements_map.end()) {
            ReplacementCwavInfo& info = it->second;
            auto soundIt = info.cwavData.begin();
            if (soundIt->first != nullptr) {
                if (lr == CwavReplace::fromWaveLR) {
                    g_incomingsndhandle = *(((u32*)soundPlayer) - 0xEC/4);
                } else {
                    g_incomingsndhandle = ((u32***)trackPlayer)[0x1C/4][0xC0/4][-0xEC/4];
                }
                soundIt = std::find_if(info.cwavData.begin(), info.cwavData.end(), [](const std::pair<u32*, void*> & t){
                    return g_incomingsndhandle == *t.first;
                });
                if (soundIt == info.cwavData.end())
                    return originalCwav;
            }
            u8* ret = (u8*)soundIt->second;
            bool isValid = info.validUntil == Time::Zero || info.validUntil >= Ticks(svcGetSystemTick());
            if (!isValid) {
                replacements_map.erase(it);
                return originalCwav;
            }
            if (info.times) {
                if (info.times == 1) replacements_map.erase(it);
                else {
                    info.times--; 
                }
            }
            return ret;
        }
        
        return originalCwav;
    }
    void CwavReplace::LockState() {
        replacementsMutex.Lock();
    }
    void CwavReplace::UnlockState() {
        replacementsMutex.Unlock();
    }
}