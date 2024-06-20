/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BlueCoinChallenge.cpp
Open source lines: 252/373 (67.56%)
*****************************************************/

#include "BlueCoinChallenge.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "MissionHandler.hpp"
#include "UserCTHandler.hpp"
#include "main.hpp"
#include "CwavReplace.hpp"
#include "CustomTextEntries.hpp"

namespace CTRPluginFramework {

    namespace CoinLocationInit {
        struct CoinInfo {
            u16 courseID;
            struct {
                Vector3 position;
                u16 version;
            } coinData;
        };

        static constexpr const CoinInfo coinInfo[] {
        };
    }
    void BlueCoinChallenge::InitCoinLocations() {
        for (int i = 0; i < sizeof(CoinLocationInit::coinInfo) / sizeof(CoinLocationInit::CoinInfo); i++) {
            coinLocations.insert(coinLocations.end(), std::make_pair(CoinLocationInit::coinInfo[i].courseID, std::make_pair(CoinLocationInit::coinInfo[i].coinData.position, CoinLocationInit::coinInfo[i].coinData.version)));
        }
    }

    std::map<u32, std::pair<Vector3, u16>> BlueCoinChallenge::coinLocations;
    string16 BlueCoinChallenge::blueCoinCollectedStr;
    bool BlueCoinChallenge::coinSpawned = false;
    bool BlueCoinChallenge::coinWasSpawned = false;
    bool BlueCoinChallenge::coinDisabledCCSelector = false;
    u32 BlueCoinChallenge::GameAddr::coinCreateTrigEffect = 0;
    u32 BlueCoinChallenge::rotateProgress = 0;
    u32 BlueCoinChallenge::coinObjBase = 0;
    u8* BlueCoinChallenge::blueCoinSound = nullptr;
    u32 BlueCoinChallenge::coinCollectedDialogFrames = 0;

    bool BlueCoinChallenge::IsCoinCollected(u32 courseID) {
        for (auto it = SaveHandler::saveData.collectedBlueCoins.begin(); it != SaveHandler::saveData.collectedBlueCoins.end(); it++) {
            u16 currCourseID = *it >> 16;
            if (currCourseID == courseID)
                return true;
        }
        return false;
    }

    u32 BlueCoinChallenge::GetCollectedCoinCount() {
        return SaveHandler::saveData.collectedBlueCoins.size();
    }

    u32 BlueCoinChallenge::GetTotalCoinCount() {
        return coinLocations.size();
    }

    void BlueCoinChallenge::SetCoinCollected(u32 courseID) {
        // Assume the coin was not collected previously
        auto it = coinLocations.find(courseID);
        if (it == coinLocations.end() || it->second.second == 0xFFFF)
            return;
        u32 val = (courseID << 16) | (it->second.second & 0xFFFF);
        SaveHandler::saveData.collectedBlueCoins.push_back(val);
        SaveHandler::SaveSettingsAll();
    }

    void BlueCoinChallenge::Initialize() {
        InitCoinLocations();

        ExtraResource::SARC::FileInfo fInfo;
        blueCoinSound = ExtraResource::mainSarc->GetFile("RaceCommon.szs/blueCoin/blueCoin.bcwav", &fInfo);

        // Clear version mismatch coins
        for (auto it = SaveHandler::saveData.collectedBlueCoins.begin(); it != SaveHandler::saveData.collectedBlueCoins.end();) {
            u32 val = *it;
            u16 courseID = val >> 16;
            u16 version = val & 0xFFFF;
            auto it2 = coinLocations.find(courseID);
            if (it2 == coinLocations.end() || version != it2->second.second)
                it = SaveHandler::saveData.collectedBlueCoins.erase(it);
            else
                it++;
        }
    }

    void BlueCoinChallenge::InitializeLanguage() {
        const std::string& col = NAME("blue_coin_obtain");
        auto dollarStart = col.find('$');
        auto dollarEnd = col.find('$', dollarStart + 1);
        if (dollarStart != col.npos && dollarEnd != col.npos) {
            std::string start = col.substr(0, dollarStart);
            std::string coin = col.substr(dollarStart + 1, dollarEnd - (dollarStart + 1));
            std::string end = col.substr(dollarEnd + 1);
            blueCoinCollectedStr.clear();
            Utils::ConvertUTF8ToUTF16(blueCoinCollectedStr, start);
            blueCoinCollectedStr.append(Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(0, 0, 255)));
            Utils::ConvertUTF8ToUTF16(blueCoinCollectedStr, coin);
            blueCoinCollectedStr.append(Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::RESET));
            Utils::ConvertUTF8ToUTF16(blueCoinCollectedStr, end);
        } else {
            Utils::ConvertUTF8ToUTF16(blueCoinCollectedStr, col);
        }
    }

    bool BlueCoinChallenge::HasChallenge() {
        CRaceInfo* raceInfo = MarioKartFramework::getRaceInfo(true);
        u32 lastTrack = CourseManager::lastLoadedCourseID;

        CCSettings* ccset = static_cast<CCSettings*>(ccselectorentry->GetArg());

        coinSpawned = false;
        // If...
        if (
            // blue coins disabled
            !SaveHandler::saveData.flags1.blueCoinsEnabled ||
            // or it's not SP GP or SP TT...
            (raceInfo->raceMode.type != 0 || raceInfo->raceMode.mode > 2) ||
            // or is mission mode...
            MissionHandler::isMissionMode ||
            // or is a custom cup
            lastTrack == USERTRACKID ||
            // or is battle mode
            (lastTrack >= BATTLETRACKLOWER && lastTrack <= BATTLETRACKUPPER) ||
            // or checksum invalid
            (!ExtraResource::lastTrackFileValid && !(lastTrack == 0x7 || lastTrack == 0x8 || lastTrack == 0x9 || lastTrack == 0x1D)) ||
            // or track does not have coin
            coinLocations.find(lastTrack) == coinLocations.end()
        )
            return false;

        #ifdef DISABLE_BLUE_COINS
        return false;
        #endif

        coinDisabledCCSelector = ccset->enabled;
        coinWasSpawned = coinSpawned = true;
        return true;
    }

    GOBJEntry BlueCoinChallenge::GetBlueCoinGOBJ() {
        GOBJEntry ret{};
        ret.objID = 0x7D;

        u32 lastTrack = CourseManager::lastLoadedCourseID;
        if (lastTrack == USERTRACKID)
            lastTrack = UserCTHandler::GetCurrentCourseOrigSlot();

        auto it = coinLocations.find(lastTrack);
        if (it == coinLocations.end())
            panic();
        ret.position = it->second.first + Vector3(0.f, 15.f, 0.f);
        ret.rotation = Vector3(M_PI / 2.f, 0.f, 0.f);
        return ret;
    }
    
    void BlueCoinChallenge::CalcObj(u32 objBase) {
        coinObjBase = objBase;
        rotateProgress += (0xFF258C00 << 1);
		MarioKartFramework::ObjModelBaseRotate(coinObjBase, rotateProgress);
    }

    static string16 g_coinCollectedText;
    bool BlueCoinChallenge::closeCoinCollectedDialog(const Screen &s) {
        if (s.IsTop && BlueCoinChallenge::coinCollectedDialogFrames) {
            if (Controller::IsKeyDown(Key::Start) && BlueCoinChallenge::coinCollectedDialogFrames > 46)
                BlueCoinChallenge::coinCollectedDialogFrames = 46;
            if (--BlueCoinChallenge::coinCollectedDialogFrames == 45) {
                MarioKartFramework::closeDialog();
            }
            if (!BlueCoinChallenge::coinCollectedDialogFrames) {
                MarioKartFramework::isPauseBlocked = false;
                OSD::Stop(closeCoinCollectedDialog);
                g_coinCollectedText.clear();
            }
        }
        return false;
    }

    void BlueCoinChallenge::DespawnCoin() {
        coinSpawned = false;
        ((u8*)coinObjBase)[0x7C] = 0;
    }

    void BlueCoinChallenge::OnKartHitCoin(u32 vehicleMove) {
        if (coinSpawned) {
            bool coinWasCollected = IsCoinCollected(CourseManager::lastLoadedCourseID);
            u32 coinsRemaining = coinLocations.size() - GetCollectedCoinCount();
            DespawnCoin();
            if (!coinWasCollected && !coinDisabledCCSelector) SetCoinCollected(CourseManager::lastLoadedCourseID);

		    int playerID = ((u32*)vehicleMove)[0x21];
            void(*createCoinGrabEffect)(u32 effectDirector, u32 playerID, Vector3* position) = (void(*)(u32, u32, Vector3*))GameAddr::coinCreateTrigEffect;
            Vector3& fromPoint = *(Vector3*)(vehicleMove + 0x24);
            createCoinGrabEffect(MarioKartFramework::getEffectDirector(), playerID, &fromPoint);

            u32 soundPlayer = ((u32*)vehicleMove)[0xDC/4];
            u32*(*sndactorkart_startsound)(u32 soundPlayer, u32 soundID, u32* soundHandle) = (decltype(sndactorkart_startsound))(((u32**)soundPlayer)[0][0x70/4]);
            CwavReplace::LockState();
            u32* retHandle = sndactorkart_startsound(soundPlayer, Snd::SoundID::COIN_GET, nullptr);
            if (retHandle)
                CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::coinGetSE, blueCoinSound, 1, 5.f);
            CwavReplace::UnlockState();

            if (!coinWasCollected) {
                if (!coinDisabledCCSelector) {
                    g_coinCollectedText = blueCoinCollectedStr + (u16*)u"\n\n";
                    std::string rem_text = (coinsRemaining - 1 == 0) ? NAME("blue_coin_all") : Utils::Format(NOTE("blue_coin_obtain").c_str(), coinsRemaining - 1);
                    Utils::ConvertUTF8ToUTF16(g_coinCollectedText, rem_text);
                    Language::MsbtHandler::SetString(CustomTextEntries::dialog, g_coinCollectedText);
                } else {
                    Language::MsbtHandler::SetString(CustomTextEntries::dialog, NAME("blue_coin_nocc"));
                }
                MarioKartFramework::isPauseBlocked = true;
                MarioKartFramework::openDialog(DialogFlags(DialogFlags::Mode::NOBUTTON), "", nullptr, true);
                OSD::Run(closeCoinCollectedDialog);
                coinCollectedDialogFrames = 255;
            }
        }
    }

    void BlueCoinChallenge::SaveCoinLoc() {
    #if false
        u32 vehicle = MarioKartFramework::getVehicle(0);
        Vector3 fromPoint = *(Vector3*)(vehicle + 0x24);
        fromPoint -= Vector3(0.f, 5.220f, 0.f); // To compensate yoshi + red wheels combo
        coinLocations[CourseManager::lastLoadedCourseID] = std::make_pair(fromPoint, 0xFFFF);
        OSD::Notify(Utils::Format("Saved at: X: %.3f, %.3f, %.3f", fromPoint.x, fromPoint.y, fromPoint.z));
    #endif
    }

    void BlueCoinChallenge::DumpCoinLocsToFile() {
    #if false
        File dumpLoc("/bluecoins.txt", File::RWC);
        std::string dump;
        for (auto it = coinLocations.begin(); it != coinLocations.end(); it++) {
            dump += Utils::Format("{0x%X, {{%.3ff, %.3ff, %.3ff}, %d}},\n", it->first, it->second.first.x, it->second.first.y, it->second.first.z, it->second.second);
        }
        dumpLoc.Write(dump.data(), dump.size());
        OSD::Notify("Written to file");
    #endif
    }
}