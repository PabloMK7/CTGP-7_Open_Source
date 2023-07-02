/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: UserCTHandler.cpp
Open source lines: 412/413 (99.76%)
*****************************************************/

#include "UserCTHandler.hpp"
#include "TextFileParser.hpp"
#include "CourseManager.hpp"
#include "Lang.hpp"
#include "cheats.hpp"
#include "MarioKartFramework.hpp"
#include "str16utils.hpp"
#include "CustomTextEntries.hpp"
#include "SequenceHandler.hpp"
#include "TextFileParser.hpp"
#include "MenuPage.hpp"

u32 USERTRACKID = 0xFF;

namespace CTRPluginFramework {
    
    std::vector<UserCTHandler::CustomCup> UserCTHandler::customCups;
    bool UserCTHandler::usingCustomCup = false;
    u32 UserCTHandler::selectedCustomCup = -1;
    u32 UserCTHandler::currentTrack = 0;
    ExtraResource::StreamedSarc* UserCTHandler::textureSarc;
    u32 UserCTHandler::lastLastLoadedTrack = INVALIDTRACK;
    std::vector<u32> UserCTHandler::replacedSarcFiles;

    UserCTHandler::SkipToCourseConfig UserCTHandler::skipConfig;
    u32 UserCTHandler::BaseMenuPage_applySetting_TitleDemo_Race = 0;
    u32 UserCTHandler::BaseMenuPage_applySetting_GP = 0;

    UserCTHandler::CustomCourse::CustomCourse() : lapAmount(0) {}
    UserCTHandler::CustomCourse::CustomCourse(const std::string& iName, const std::string& cName, const std::string& fName, u32 oSlot, u32 lapAm) :
        internalName(iName), courseName(cName), fileName(fName), originalSlot(oSlot), lapAmount(lapAm) {}

    UserCTHandler::CustomCup::CustomCup() : cupName("") {}
    UserCTHandler::CustomCup::CustomCup(const std::string& cName) : cupName(cName) {}

    void UserCTHandler::Initialize() {
        populateCups();
        CourseManager::initGlobalCupTranslateTable();
        initSkipConfig();
    }

    void UserCTHandler::populateCups() {
        Directory rootDir("/CTGP-7/MyStuff/Courses");
        if (!rootDir.IsOpen())
            return;
        std::vector<std::string> foundDir;
        rootDir.ListDirectories(foundDir);
        for (auto it = foundDir.cbegin(); it != foundDir.cend(); it++) {
            Directory subDir("/CTGP-7/MyStuff/Courses/" + *it);
            if (!subDir.IsOpen())
                continue;
            std::vector<std::string> foundSubFiles;
            subDir.ListFiles(foundSubFiles, ".szs");
            if (foundSubFiles.empty())
                continue;
            CustomCup addCup = populateCourses(foundSubFiles, *it);
            if (addCup.GetCourse(0).lapAmount == 0)
                continue;
            customCups.push_back(addCup);
        }
        if (customCups.size() & 1)
            customCups.push_back(CustomCup(""));
    }

    static inline bool ends_with(std::string const & value, std::string const & ending)
    {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    UserCTHandler::CustomCup UserCTHandler::populateCourses(const std::vector<std::string>& szsFileNames, const std::string& cupName) {
        CustomCup ret(cupName);
        int i = 0;
        for (auto it = szsFileNames.cbegin(); it != szsFileNames.cend(); it++) {
            if (i >= 4)
                break;
            TextFileParser parser;
            if ((*it).length() < 5 || !ends_with(*it, ".szs") || !parser.ParseLine((*it).substr(0, (*it).length() - 4), "."))
                continue;
            auto parserData = parser.cbegin();
            if (parserData == parser.cend())
                continue;
            std::string courseName;
            u32 lapAmount;
            int slotID = CourseManager::getCourseIDFromName(parserData->first);
            if (slotID < 0) // Custom name for debugging
            {
                if (parserData->second.size() == 0)
                    continue;
                slotID = CourseManager::getCourseIDFromName(parserData->second[0]);
                if (slotID < 0)
                    continue;
                if (parserData->second.size() > 1) {
                    courseName = parserData->second[1];
                } else {
                    courseName = "";
                }
                if (parserData->second.size() > 1 && TextFileParser::IsNumerical(parserData->second[2], false)) {
                    u32 laps = std::stoi(parserData->second[2]);
                    if (laps > 0 && laps < 10)
                        lapAmount = laps;
                    else
                        lapAmount = CourseManager::getCourseData(slotID)->lapAmount;
                } else lapAmount = CourseManager::getCourseData(slotID)->lapAmount;
                ret.GetCourse(i) = CustomCourse(parserData->first, courseName, *it, slotID, lapAmount);
            } else {
                if (parserData->second.size() > 0) {
                    courseName = parserData->second[0];
                } else {
                    courseName = "";
                }
                if (parserData->second.size() > 1 && TextFileParser::IsNumerical(parserData->second[1], false)) {
                    u32 laps = std::stoi(parserData->second[1]);
                    if (laps > 0 && laps < 10)
                        lapAmount = laps;
                    else
                        lapAmount = CourseManager::getCourseData(slotID)->lapAmount;
                } else lapAmount = CourseManager::getCourseData(slotID)->lapAmount;
                ret.GetCourse(i) = CustomCourse("", courseName, *it, slotID, lapAmount);
            }
            
            i++;
        }
        return ret;
    }

    void UserCTHandler::FixNames() {
        for (auto it = customCups.begin(); it != customCups.end(); it++) {
            for (int i = 0; i < 4; i++) {
                if (it->GetCourse(i).lapAmount == 0 || it->GetCourse(i).courseName.length())
                    continue;
                CourseManager::getCourseText(it->GetCourse(i).courseName, it->GetCourse(i).originalSlot, false);
            }
        }
    }

    u32 UserCTHandler::GetCustomCupAmount() {
        return customCups.size();
    }

    u32 UserCTHandler::GetSelectedCustomCup() {
        u32 origSize = MAXCUPS;
        u32 customSize = CourseManager::finalGlobalCupTranslateTableSize;
        u32 ret;
        if (MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon < 4) {
            ret = (MenuPageHandler::MenuSingleCupBasePage::startingButtonID + MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon) % (customSize / 2);
            ret -= (origSize / 2);
            ret = ret * 2;
        } else {
            ret = ((MenuPageHandler::MenuSingleCupBasePage::startingButtonID + MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon - 4) % (customSize / 2)) + (customSize / 2);
            ret -= (customSize / 2 + origSize / 2);
            ret = ret * 2 + 1;
        }
        if (customCups[ret].GetCourse(0).lapAmount == 0)
            ret--;
        return ret;
    }

    void UserCTHandler::UpdateCurrentCustomCup(u32 cupID) {
        if (cupID == USERCUPID && MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon <= 7) {
            usingCustomCup = true;
            u32 newSelectedCup = GetSelectedCustomCup();
            if (newSelectedCup != selectedCustomCup) {
                selectedCustomCup = newSelectedCup;
                Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + 4, customCups[selectedCustomCup].cupName);
                for (int i = 0; i < 4; i++) {
                    if (customCups[selectedCustomCup].GetCourse(i).lapAmount)
                        Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + i, customCups[selectedCustomCup].GetCourse(i).courseName);
                    else
                        Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + i, "");
                }
                currentTrack = 0;
                UpdateCurrentCourseData();
            }
        } else {
            usingCustomCup = false;
        }
    }

    void UserCTHandler::UpdateCurrentCourseData() {
        if (!usingCustomCup) return;
        Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + 5, customCups[selectedCustomCup].GetCourse(currentTrack).courseName);
    }

    u32 UserCTHandler::GetCupTextID() {
        return CustomTextEntries::courseDisplay + 4;
    }

    u32 UserCTHandler::GetCourseTextID() {
        return CustomTextEntries::courseDisplay + 5;
    }

    static CTNameData::CTNameEntry customNameEntry = {
        &globalNameData.f,
    };

    u32 UserCTHandler::GetCourseInternalName() {
        if (skipConfig.enabled && skipConfig.courseID >= 0) return (u32)&globalNameData.entries[skipConfig.courseID]; 
        if (customCups[selectedCustomCup].GetCourse(currentTrack).internalName.empty())
        {
            u32 origSlot = customCups[selectedCustomCup].GetCourse(currentTrack).originalSlot;
            return (u32)&globalNameData.entries[origSlot];
        } else {
            customNameEntry.name = customCups[selectedCustomCup].GetCourse(currentTrack).internalName.c_str();
            return (u32)&customNameEntry;
        }
    }

    u32 UserCTHandler::GetCurrentCourseOrigSlot() {
        if (skipConfig.enabled && skipConfig.courseID >= 0) return skipConfig.courseID; 
        return globalNameData.entries[customCups[selectedCustomCup].GetCourse(currentTrack).originalSlot].originalSlot;
    }

    u32 UserCTHandler::GetCurrentCourseLapAmount() {
        if (skipConfig.enabled && skipConfig.courseID >= 0) return globalNameData.entries[skipConfig.courseID].lapAmount;
        return customCups[selectedCustomCup].GetCourse(currentTrack).lapAmount;
    }

    u32 UserCTHandler::GetCurrentTrackInCup() {
        return currentTrack;
    }

    u32 UserCTHandler::GetCurrentCupText(u32 track) {
        if (!skipConfig.enabled) UserCTHandler::UpdateCurrentCustomCup(USERCUPID);
        return CustomTextEntries::courseDisplay + track;
    }

    bool UserCTHandler::IsUsingCustomCup() {
        return usingCustomCup;
    }
    bool UserCTHandler::IsLastRace(u32 track) {
        if (skipConfig.enabled && skipConfig.courseID >= 0) return true;
        return track >= 3 || customCups[selectedCustomCup].GetCourse(track + 1).lapAmount == 0;
    }
    void UserCTHandler::OnNextTrackLoad() {
        if (MarioKartFramework::currentRaceMode.mode == 0) {
            if (usingCustomCup && skipConfig.enabled && skipConfig.courseID >= 0)
                usingCustomCup = false;
            else if (usingCustomCup && currentTrack < 3 && customCups[selectedCustomCup].GetCourse(currentTrack + 1).lapAmount != 0)
            {
                if (USERTRACKID == 0xFF)
                    USERTRACKID = 0xFE;
                else
                    USERTRACKID = 0xFF;
                currentTrack++;
                UpdateCurrentCourseData();
            } else {
                usingCustomCup = false;
            }
        }        
        CleanTextureSarc();
    }

    void UserCTHandler::CleanTextureSarc() {
        replacedSarcFiles.clear();
        if (textureSarc) delete textureSarc;
        textureSarc = nullptr;
    }

    void UserCTHandler::TimeTrialsSetTrack(u32 track) {
        if (usingCustomCup)
            for (int i = 0; i <= track; i++)
                if (customCups[selectedCustomCup].GetCourse(i).lapAmount != 0)
                    currentTrack = i;
        UpdateCurrentCourseData();
    }

    void UserCTHandler::GetCouseSZSPath(u16* dst, bool withLang) {
        if (skipConfig.enabled && skipConfig.courseID >= 0) return;
        std::string ret = "ram:/CTGP-7/MyStuff/Courses/" + customCups[selectedCustomCup].cupName + "/" + customCups[selectedCustomCup].GetCourse(currentTrack).fileName;
        if (withLang) ret += Language::GetLanguagePostfixChar();
        strcpy16n(dst, (u8*)ret.c_str(), 0x100 * sizeof(u16));
    }

    static bool g_loadTextureLastFileFound = false;
    static std::string g_loadTextureLastFileName;
    u8* UserCTHandler::LoadTextureFile(u32* archive, SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo) {
        if (!usingCustomCup && (CourseManager::lastLoadedCourseID >= ORIGINALTRACKLOWER && CourseManager::lastLoadedCourseID <= BATTLETRACKUPPER) && (!strstr(file->data, ".bcmdl") || !strstr(file->data, ".bclim") || !strstr(file->data, ".bcfog") || !strstr(file->data, ".bclgt"))) {
            if (lastLastLoadedTrack != CourseManager::lastLoadedCourseID) {
                g_loadTextureLastFileName = std::string("/CTGP-7/MyStuff/CourseTextures/") + globalNameData.entries[CourseManager::lastLoadedCourseID].name + ".sarc";
                g_loadTextureLastFileFound = File::Exists(g_loadTextureLastFileName);
                lastLastLoadedTrack = CourseManager::lastLoadedCourseID;
                CleanTextureSarc();
            }
            if (g_loadTextureLastFileFound) {
                if (!textureSarc) textureSarc = new ExtraResource::StreamedSarc(g_loadTextureLastFileName);
                u8* gameFilePtr;
                ExtraResource::SARC gameCourseSarc((u8*)(archive[0xE] - 0x14), false);
                ExtraResource::SARC::FileInfo defaultFInfo;
                if (!fileInfo) fileInfo = &defaultFInfo;
                u32 fileHash = gameCourseSarc.GetHash(file->data);
                gameFilePtr = gameCourseSarc.GetFile(fileHash, fileInfo);
                if (std::find(replacedSarcFiles.begin(), replacedSarcFiles.end(), fileHash) != replacedSarcFiles.end())
                    return gameFilePtr;
                replacedSarcFiles.push_back(fileHash);
                if (textureSarc->ReadFile(gameFilePtr, *fileInfo, *file))
                    return gameFilePtr;
                else
                    return nullptr;
            }
        }
        return nullptr;
    }

    void UserCTHandler::initSkipConfig() {
#ifdef GOTO_TOADCIRCUIT
        skipConfig.enabled = true;
        skipConfig.skipCoursePreview = true;
        skipConfig.cpuAmount = 8;
        skipConfig.driverID = EDriverID::DRIVER_YOSHI;
        skipConfig.bodyID = EBodyID::BODY_DSH;
        skipConfig.tireID = ETireID::TIRE_BIGRED;
        skipConfig.wingID = EWingID::WING_BASA;
        skipConfig.itemID = EItemSlot::ITEM_KILLER;
        skipConfig.useLeftToFinish = false;
        skipConfig.courseID = 0x4;
#else
        skipConfig.enabled = false;
        TextFileParser parser;
		if (!parser.Parse("/CTGP-7/MyStuff/debugSkipConfig.ini")) return;
        if (parser.getEntry("enabled", 0) == "true") {
            skipConfig.enabled = true;
            
            skipConfig.skipCoursePreview = parser.getEntry("skipPreview", 0) == "true";

            skipConfig.courseID = CourseManager::getCourseIDFromName(parser.getEntry("courseSZSID", 0));

            skipConfig.cpuAmount = 8;
            const std::string& playerAm = parser.getEntry("players", 0);
            if (!playerAm.empty() || TextFileParser::IsNumerical(playerAm, false)) {
                skipConfig.cpuAmount = std::stoi(playerAm);
                if (skipConfig.cpuAmount < 1) skipConfig.cpuAmount = 1;
                else if (skipConfig.cpuAmount > 8) skipConfig.cpuAmount = 8;
            }
            int ids[4] = {7, 0, 0, 0};
            for (int i = 0; i < 4; i++) {
                std::string part = parser.getEntry("driverParts", i);
                if (part.size() >= 2 && part.substr(0, 2) == "0x")
                    part.erase(0, 2);
                if (!part.empty() || TextFileParser::IsNumerical(part, true))
                    ids[i] = std::stoi(part, 0, 16);
            }
            skipConfig.driverID = ids[0];
            skipConfig.bodyID = ids[1];
            skipConfig.tireID = ids[2];
            skipConfig.wingID = ids[3];

            skipConfig.itemID = -1;
            std::string itemID = parser.getEntry("giveItem", 0);
            if (!itemID.empty() || TextFileParser::IsNumerical(itemID, false)) {
                if (itemID.size() >= 2 && itemID.substr(0, 2) == "0x")
                    itemID.erase(0, 2);
                if (!itemID.empty() || TextFileParser::IsNumerical(itemID, true)) {
                    skipConfig.itemID = std::stoi(itemID, 0, 16);
                    if (skipConfig.itemID < 0) skipConfig.itemID = -1;
                    else if (skipConfig.itemID > 0x13) skipConfig.itemID = -1;
                }
            }

            skipConfig.useLeftToFinish = parser.getEntry("forceFinish", 0) == "true";
        }
#endif
        
    }

    void UserCTHandler::ApplySkipToCourseConfig() {
        if (skipConfig.enabled && (skipConfig.courseID >= 0 || customCups.size() > 0)) {
            if (skipConfig.skipCoursePreview)
                SequenceHandler::addFlowPatch(SequenceHandler::rootSequenceID, 0x594, 0x03, 0x05); // Boot -> Race;
            else
                SequenceHandler::addFlowPatch(SequenceHandler::rootSequenceID, 0x594, 0x02, 0x01); // Boot -> Demo;

            //SequenceHandler::addFlowPatch(SequenceHandler::rootSequenceID, 0x594, 0x07, 0x01); //Credits

            MarioKartFramework::setSkipGPCoursePreview(skipConfig.skipCoursePreview);

            // Run titleDemo applyconfig to setup cpu settings
            ((void(*)())(BaseMenuPage_applySetting_TitleDemo_Race))(); 

            // Set race mode to GP
            MarioKartFramework::CRaceMode raceMode;
            raceMode.type = 0; raceMode.mode = 0; raceMode.submode = 2;
            MarioKartFramework::BasePage_SetRaceMode(&raceMode);

            // Set playertype back to user
			u32 playerType = EPlayerType::TYPE_USER;
            u32 screwID = 0;
			MarioKartFramework::BasePage_SetDriver(0, (s32*)&skipConfig.driverID, &playerType);
            MarioKartFramework::BasePage_SetParts(0, (s32*)&skipConfig.bodyID, (s32*)&skipConfig.tireID, (s32*)&skipConfig.wingID, &screwID);
            
            // Set cup to USERCUP
            MarioKartFramework::BasePageSetCup(USERCUPID);
            usingCustomCup = true;
            selectedCustomCup = 0;
            currentTrack = 0;

            // Run GP applyconfig to setup rest of settings
		    ((void(*)())(BaseMenuPage_applySetting_GP))(); // GP
        } else {
            skipConfig.enabled = false;
        }
    }
}