/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MissionHandler.cpp
Open source lines: 1564/1575 (99.30%)
*****************************************************/

#include "MissionHandler.hpp"
#include "MarioKartFramework.hpp"
#include "SequenceHandler.hpp"
#include "VersusHandler.hpp"
#include "Lang.hpp"
#include "main.hpp"
#include "Sound.hpp"
#include "ExtraResource.hpp"
#include "CourseManager.hpp"
#include "CwavReplace.hpp"
#include "StatsHandler.hpp"
#include "str16utils.hpp"
#include "Unicode.h"
#include "ExtraUIElements.hpp"
#include "ItemHandler.hpp"
#include "CustomTextEntries.hpp"
#include "MenuPage.hpp"
#include "Stresser.hpp"
#include "AsyncRunner.hpp"

extern "C" u32 * g_altGameModeplayerStatPointer;

namespace CTRPluginFramework {
    bool MissionHandler::isMissionMode = false;
    char16_t MissionHandler::cupNameFormatReplace[5] = { 0x0E, 0x01, 0x03, 0x00, 0x00 };
    MissionHandler::MissionParameters* MissionHandler::missionParam = nullptr;
    MissionHandler::MissionParameters* MissionHandler::temporaryParam[4] = {nullptr};
    bool MissionHandler::lastLoadedCupAccessible = false;
    u32 MissionHandler::lastLoadedCup = -1;
    void* MissionHandler::gameBuffer = nullptr;
    int MissionHandler::AddPoints = 0;
    std::pair<std::string, std::string> MissionHandler::pointsStr;
    int MissionHandler::AddPointsQueue = 0;
    MarioKartTimer MissionHandler::finalTime = MarioKartTimer(0);
    u32 MissionHandler::finalPoints = 0;
	ExtraResource::SARC* MissionHandler::extraSarc = nullptr;
    ExtraResource::SARC* MissionHandler::gameCourseSarc = nullptr;
    ExtraResource::StreamedSarc* MissionHandler::replacementSarc = nullptr;
    std::vector<u32> MissionHandler::replacedSarcFiles;
    std::map<u32, MusicSlotMngr::MusicSlotEntry> MissionHandler::customMusic;
    int MissionHandler::currMissionWorld = -1;
	int MissionHandler::currMissionLevel = -1;
	bool MissionHandler::raceBeenFinished = true;
    bool MissionHandler::missionConditionFailed = false;
    int MissionHandler::failMissionFrames = -1;
    bool MissionHandler::coinSoundReplaced = false;
	bool MissionHandler::cmsnCalculatedChecksum = false;
	bool MissionHandler::sarcCalculatedChecksum = false;
    u32 MissionHandler::calculatedChecksum = 0;
	MarioKartTimer MissionHandler::lastMainTimer;
    MissionHandler::MissionResult MissionHandler::lastMissionResult;
    std::vector<u32*> MissionHandler::missionGates;

    const std::string MissionHandler::GetMissionDirPath(u32 world) {
        const char* missionDirNames[2] = {"/CTGP-7/gamefs/Missions", "/CTGP-7/MyStuff/Missions"};
        return missionDirNames[world < 9 ? 0 : 1];
    }

    void MissionHandler::InitializeText()
    {
        Language::MsbtHandler::SetString(CustomTextEntries::mission, NAME("ms_miss"));
        Language::MsbtHandler::SetString(CustomTextEntries::missionDesc, NAME("ms_desc"));
        std::string startOver = Language::MsbtHandler::GetString(9007);
        // Next menu
        Language::MsbtHandler::SetString(9012, startOver, true); // View replay
        Language::MsbtHandler::SetTextEnabled(9012, false);
        // Pause menu
        Language::MsbtHandler::SetText(9013, Language::MsbtHandler::GetText(9003), true);
        Language::MsbtHandler::SetString(9014, startOver, true); // View replay
        Language::MsbtHandler::SetTextEnabled(9013, false);
        Language::MsbtHandler::SetTextEnabled(9014, false);
        // 1st race (in order to create linked list)
        Language::MsbtHandler::SetString(1831, "", true);
        Language::MsbtHandler::SetTextEnabled(1831, false);

        for (int i = 1821; i < 1826; i++) {
            Language::MsbtHandler::SetText(i, cupNameFormatReplace, true);
            Language::MsbtHandler::SetTextEnabled(i, false);
        }
        const Language::MsbtHandler::ControlString* ptsControl = Language::MsbtHandler::FindControlString(Language::MsbtHandler::GetText(9830), 1, 6);
        if (ptsControl) {
            char16_t buff[0x20];
            ptsControl->GetChoice(buff, 0);
            std::string singlePts = Language::MsbtHandler::GetString(buff);
            std::string pluralPts;
            if (ptsControl->GetChoice(buff, 1))
                pluralPts = Language::MsbtHandler::GetString(buff);
            else
                pluralPts = singlePts;
            pointsStr = std::make_pair(singlePts, pluralPts);
        } else
        {
            std::string pts = Language::MsbtHandler::GetString(9830);
            pointsStr = std::make_pair(pts, pts);
        }       
    }

    void MissionHandler::setupExtraResource()
    {
        if (!gameBuffer)
        {
            gameBuffer = GameAlloc::MemAlloc(gameBufferSize, 0x80);
        }
    }

    void MissionHandler::onModeMissionEnter()
    {
        if (isMissionMode) return;
        isMissionMode = true;
        MarioKartFramework::resultBarHideRank = true;
        MarioKartFramework::forcedResultBarAmount = 4;
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2264, 0x16, 0x1); // Return from game to GP cup
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2274, 0x16, 0x1); // single to GP cup
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x235C, 0x4, 0x2); // GP cup to single

        std::string selectMission = NAME("ms_selmiss");
        Language::MsbtHandler::SetString(9009, selectMission, false); // Next Course
        Language::MsbtHandler::SetTextEnabled(9012, true);
        Language::MsbtHandler::SetTextEnabled(9013, true);
        Language::MsbtHandler::SetTextEnabled(9014, true);
        for (int i = 1821; i < 1826; i++) {
            Language::MsbtHandler::SetTextEnabled(i, true);
        }
        ItemHandler::extraKouraB = 2;
    }

    void MissionHandler::onModeMissionExit()
    {
        if (!isMissionMode) return;
        isMissionMode = false;
        MarioKartFramework::resultBarHideRank = false;
        MarioKartFramework::forcedResultBarAmount = -1;
        MarioKartFramework::setSkipGPCoursePreview(false);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2264, 0xB, 0x3);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2274, 0xD, 0x1);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x235C, 0x15, 0x2);

        Language::MsbtHandler::SetTextEnabled(9009, false);
        Language::MsbtHandler::SetTextEnabled(9012, false);
        Language::MsbtHandler::SetTextEnabled(9013, false);
        Language::MsbtHandler::SetTextEnabled(9014, false);
        for (int i = 1821; i < 1826; i++) {
            Language::MsbtHandler::SetTextEnabled(i, false);
        }
        Language::MsbtHandler::SetTextEnabled(1831, false);
        for (int i = 0; i < 4; i++) {
            if (temporaryParam[i])
                delete temporaryParam[i];
            temporaryParam[i] = nullptr;
        }
        lastLoadedCup = -1;
        missionParam = nullptr;
        cmsnCalculatedChecksum = false;
        sarcCalculatedChecksum = false;
        calculatedChecksum = 0;        
        customMusic.clear();
        ItemHandler::extraKouraB = 0;
    }

    void MissionHandler::OnRaceEnter() {
        if (!isMissionMode) return;
    }
    
    void MissionHandler::OnRaceExit() {
        if (!isMissionMode) return;
        if (extraSarc) delete extraSarc;
        extraSarc = nullptr;
        if (replacementSarc) delete replacementSarc;
        replacementSarc = nullptr;
        if (gameCourseSarc) delete gameCourseSarc;
        gameCourseSarc = nullptr;
        if (coinSoundReplaced) {
            coinSoundReplaced = false;
            CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::coinGetSE, nullptr);
        }
        replacedSarcFiles.clear();
    }

    void MissionHandler::OnRaceFinish() {
        if (!isMissionMode) return;
        
        calculateResult();
    }

    void MissionHandler::calculateCompleteCondition() {
        if (missionConditionFailed) return;
        u8* playerPositions = MarioKartFramework::getModeManagerData()->driverPositions;

        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::SCOREISYELLOW)) {
            missionConditionFailed = !pointControlShouldBeYellow(counterUIGet(true));
        }
        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::SCOREISNOTYELLOW)) {
            missionConditionFailed = pointControlShouldBeYellow(counterUIGet(true));
        }
        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER0FIRST)) {
            missionConditionFailed = playerPositions[0] != 1;
        }
        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER1FIRST)) {
            missionConditionFailed = playerPositions[1] != 1;
        }
        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER2FIRST)) {
            missionConditionFailed = playerPositions[2] != 1;
        }
    }

    void MissionHandler::calculateResult() {
        if (lastMissionResult.isCalculated) return;
        float grade = 0.f;
        float slope = 0.f;

        lastMissionResult.isCalculated = true;
        lastMissionResult.hasBestGrade = true;

        if (!raceBeenFinished) {
            raceBeenFinished = true;
            int totalAdd = AddPoints + AddPointsQueue;
            counterUIAdd(totalAdd);
            AddPoints = AddPointsQueue = 0;
            finalTime = lastMainTimer;
            finalPoints = counterUIGet();
        }

        calculateCompleteCondition();

        switch (missionParam->MissionFlags->calcType)
        {
        case MissionParameters::CMSNMissionFlagsSection::CalculationType::TIMER:
            {
                Linear l(Vector2(missionParam->MissionFlags->GetMinGradeTimer().GetFrames(), 0.499f), Vector2(missionParam->MissionFlags->GetMaxGradeTimer().GetFrames(), 9.5f));
                grade = l(finalTime.GetFrames());
                slope = l.Slope();
            }
            
            break;
        case MissionParameters::CMSNMissionFlagsSection::CalculationType::POINTS:
            {
                Linear l(Vector2(missionParam->MissionFlags->minGrade, 0.5f), Vector2(missionParam->MissionFlags->maxGrade, 9.5f));
                grade = l(finalPoints);
                slope = l.Slope();
            }
            break;
        default:
            break;
        }

        if (missionConditionFailed)
            grade = 0.f;
        
        if (grade < 0.5f) {
            lastMissionResult.points = 0;
            lastMissionResult.resultBarOrder = 1;
        } else if (grade >= 9.5f) {
            lastMissionResult.points = 10;
            lastMissionResult.resultBarOrder = 0;
        } else {
            lastMissionResult.points = roundf(grade);
            lastMissionResult.resultBarOrder = 2;
        }

        SaveData::SaveEntry saveEntry(
            missionParam->MissionFlags->calcType,
            missionParam->InfoSection->saveIteration
        );
        SaveData::GetMissionSave(missionParam->InfoSection->uniqueMissionID, saveEntry);
        bool overridePrevChecksum = false;

        if (!saveEntry.hasData) {
            if (!missionConditionFailed) {
                saveEntry.hasData = true;
                saveEntry.SetChecksumValid(true);
                saveEntry.isDirty = true;
                saveEntry.grade = lastMissionResult.points;
                switch (missionParam->MissionFlags->calcType)
                {
                case MissionParameters::CMSNMissionFlagsSection::CalculationType::TIMER:
                    saveEntry.SetTime(finalTime);
                    lastMissionResult.bestScore = finalTime.GetFrames();
                    break;
                case MissionParameters::CMSNMissionFlagsSection::CalculationType::POINTS:
                    saveEntry.SetScore(finalPoints);
                    lastMissionResult.bestScore = finalPoints;
                    break;
                }
                lastMissionResult.bestGrade = saveEntry.grade;
            } else {
                lastMissionResult.hasBestGrade = false;
            }
        } else {
            switch (missionParam->MissionFlags->calcType)
            {
            case MissionParameters::CMSNMissionFlagsSection::CalculationType::TIMER:
                if (((slope > 0.f && finalTime > saveEntry.GetTime()) || (slope < 0.f && finalTime < saveEntry.GetTime())) && !missionConditionFailed) {
                    saveEntry.SetTime(finalTime);
                    saveEntry.grade = lastMissionResult.points;
                    saveEntry.isDirty = true;
                    overridePrevChecksum = true;
                }
                lastMissionResult.bestScore = saveEntry.GetTime().GetFrames();
                break;
            case MissionParameters::CMSNMissionFlagsSection::CalculationType::POINTS:
                if (((slope > 0.f && finalPoints > saveEntry.GetScore()) || (slope < 0.f && finalPoints < saveEntry.GetScore())) && !missionConditionFailed) {
                    saveEntry.SetScore(finalPoints);
                    saveEntry.grade = lastMissionResult.points;
                    saveEntry.isDirty = true;
                    overridePrevChecksum = true;
                }
                lastMissionResult.bestScore = saveEntry.GetScore();
                break;
            }
            lastMissionResult.bestGrade = saveEntry.grade;
        }

        bool checksumValid = (saveEntry.IsChecksumValid() || overridePrevChecksum) && calculatedChecksum != 0 &&
                                missionParam->InfoSection->checksum == calculatedChecksum && 
                                missionParam->InfoSection->missionWorld == currMissionWorld &&
                                missionParam->InfoSection->missionLevel == currMissionLevel;
        
        if (saveEntry.hasData) saveEntry.SetChecksumValid(checksumValid);
        SaveData::SetMissionSave(missionParam->InfoSection->uniqueMissionID, saveEntry);
        #if CITRA_MODE == 0
        if (checksumValid && lastMissionResult.points == 10) SaveData::SetFullGradeFlag(currMissionWorld, currMissionLevel);
        #else
        if (currMissionWorld <= 4 && lastMissionResult.points == 10) SaveData::SetFullGradeFlag(currMissionWorld, currMissionLevel);
        #endif
        StatsHandler::OnMissionFinish(lastMissionResult.points, checksumValid, currMissionWorld);
    }

    void MissionHandler::resultBarAmountSetName(u32 baseresultbar, u32 message) {
        
    }

    static float g_MissionResultBarPos[3];
    float* MissionHandler::setResultBarPosition(u32 index, float* original) {        
        u32* ordering;
        u32 posibleOrders[3][4] = { { 0, 1, 2, 4 } , { 2, 0, 1, 4 } , { 1, 0, 2, 4 } };

        ordering = posibleOrders[lastMissionResult.resultBarOrder];
        
        g_MissionResultBarPos[0] = -60;
        g_MissionResultBarPos[1] = 75.f - 25.f * ordering[index];
        g_MissionResultBarPos[2] = 0;

        return g_MissionResultBarPos;
    }
    void MissionHandler::updateResultBars(u32 racePage)
    {
        std::string bestEver;
        std::string maxGrade;
        std::string minGrade;
        std::string playerGrade;

        switch (missionParam->MissionFlags->calcType)
        {
        case MissionParameters::CMSNMissionFlagsSection::CalculationType::TIMER:
            bestEver = NAME("ms_rcrd") + ": " + MarioKartTimer(lastMissionResult.bestScore).Format();
            maxGrade = NAME("ms_gradelim") + ": " + missionParam->MissionFlags->GetMaxGradeTimer().Format();
            minGrade = NOTE("ms_gradelim") + ": " + missionParam->MissionFlags->GetMinGradeTimer().Format();
            if (missionConditionFailed)
                playerGrade = NAME("ms_failed");
            else
                playerGrade = NAME("ms_timescor") + ": " + finalTime.Format();
            break;
        case MissionParameters::CMSNMissionFlagsSection::CalculationType::POINTS:
            bestEver = NAME("ms_rcrd") + ": " + std::to_string(lastMissionResult.bestScore);
            maxGrade = NAME("ms_gradelim") + ": " + std::to_string(missionParam->MissionFlags->maxGrade);
            minGrade = NOTE("ms_gradelim") + ": " + std::to_string(missionParam->MissionFlags->minGrade);
            if (missionConditionFailed)
                playerGrade = NAME("ms_failed");
            else
                playerGrade = NOTE("ms_timescor") + ": "  + std::to_string(finalPoints);
            break;
        default:
            break;
        }

        if (!lastMissionResult.hasBestGrade)
            bestEver = NAME("ms_rcrd") + ": " + NAME("ms_nodata");

        MarioKartFramework::BaseResultBar playerBar = MarioKartFramework::resultBarArray[0];
        MarioKartFramework::BaseResultBar maxBar = MarioKartFramework::resultBarArray[1];
        MarioKartFramework::BaseResultBar minBar = MarioKartFramework::resultBarArray[2];
        MarioKartFramework::BaseResultBar bestBar = MarioKartFramework::resultBarArray[3];
        
        std::u16string utf16;
        u16* utf16ptr;

        Utils::ConvertUTF8ToUTF16(utf16, playerGrade);
        utf16ptr = (u16*)utf16.c_str();
        MarioKartFramework::BaseResultBar_setName(playerBar, &utf16ptr);
        utf16.clear();

        Utils::ConvertUTF8ToUTF16(utf16, maxGrade);
        utf16ptr = (u16*)utf16.c_str();
        MarioKartFramework::BaseResultBar_setName(maxBar, &utf16ptr);
        utf16.clear();

        Utils::ConvertUTF8ToUTF16(utf16, minGrade);
        utf16ptr = (u16*)utf16.c_str();
        MarioKartFramework::BaseResultBar_setName(minBar, &utf16ptr);
        utf16.clear();

        Utils::ConvertUTF8ToUTF16(utf16, bestEver);
        utf16ptr = (u16*)utf16.c_str();
        MarioKartFramework::BaseResultBar_setName(bestBar, &utf16ptr);

        MarioKartFramework::BaseResultBar_setPoint(maxBar, 10);
        MarioKartFramework::BaseResultBar_setPoint(minBar, 0);        
        MarioKartFramework::BaseResultBar_setPoint(bestBar, lastMissionResult.hasBestGrade ? lastMissionResult.bestGrade : 0);
        MarioKartFramework::BaseResultBar_addPoint(playerBar, lastMissionResult.points);
    }

    MissionHandler::MissionParameters::CMSNDriverOptionsSection::OptionsEntry MissionHandler::GetFixedDriverOptions(MissionParameters::CMSNDriverOptionsSection::OptionsEntry& original)
    {
        if (original.driverID == EDriverID::DRIVER_RECOMMENDED) return MissionParameters::CMSNDriverOptionsSection::OptionsEntry(EDriverID::DRIVER_RECOMMENDED, original.bodyID, original.tireID, original.wingID);
        u32 driverID = 0, bodyID = 0, tireID = 0, wingID = 0;

        if (original.driverID == EDriverID::DRIVER_RANDOM) {
            driverID = Utils::Random(EDriverID::DRIVER_BOWSER, EDriverID::DRIVER_YOSHI - 2);
            if (driverID >= EDriverID::DRIVER_MIIM) driverID += 2; // Skip MII
        } else {
            driverID = original.driverID;
            if (driverID >= EDriverID::DRIVER_SHYGUYCOLORSTART && driverID <= EDriverID::DRIVER_SHYGUYCOLOREND) {
                driverID = EDriverID::DRIVER_SHYGUY;
            }
        }

        if (original.bodyID == EBodyID::BODY_RANDOM) {
            bodyID = Utils::Random(EBodyID::BODY_STD, EBodyID::BODY_GOLD);
        } else bodyID = original.bodyID;

        if (original.tireID == ETireID::TIRE_RANDOM) {
            tireID = Utils::Random(ETireID::TIRE_STD, ETireID::TIRE_MUSH);
        } else tireID = original.tireID;

        if (original.wingID == EWingID::WING_RANDOM) {
            wingID = Utils::Random(EWingID::WING_STD, EWingID::WING_GOLD);
        } else wingID = original.wingID;
        return MissionParameters::CMSNDriverOptionsSection::OptionsEntry(driverID, bodyID, tireID, wingID);
    }

    static int g_lastSelectedIndex = 0;
    static bool g_canDeleteData = false;
    void MissionHandler::OnKbdEvent(Keyboard& kbd, KeyboardEvent &event) {
        bool reDraw = false;
        if (event.type == KeyboardEvent::EventType::KeyPressed && event.affectedKey == Key::X && g_canDeleteData) {
            if (!temporaryParam[g_lastSelectedIndex])
                return;
            bool confirm = MessageBox(NOTE("ms_delsave"), DialogType::DialogYesNo, ClearScreen::Top)();
            if (confirm) {
                reDraw = true;
                SaveData::ClearMissionSave(temporaryParam[g_lastSelectedIndex]->InfoSection->uniqueMissionID);
                event.selectedIndex = g_lastSelectedIndex;
            }
        }
        if (reDraw || event.type == KeyboardEvent::EventType::SelectionChanged) {
            if (!temporaryParam[event.selectedIndex])
                return;
            g_lastSelectedIndex = event.selectedIndex;
            if ((int)event.selectedIndex < 0)
            {
                kbd.GetMessage() = "";
            } else {
                const char* newLines = "\n\n\n\n";
                MissionParameters::CMSNTextSection::TextEntry* currentry = temporaryParam[event.selectedIndex]->TextString->GetLangEntry(Language::GetCurrLangID());
                MissionParameters::CMSNMissionFlagsSection* missionFlags = temporaryParam[event.selectedIndex]->MissionFlags;
                kbd.GetMessage() = std::to_string(lastLoadedCup + 1) + "-" + std::to_string(event.selectedIndex + 1) + " (" + currentry->GetTitle() + ")";
                kbd.GetMessage() += HorizontalSeparator();
                kbd.GetMessage() += currentry->GetDescription();
                kbd.GetMessage() += newLines + currentry->descriptionNewlines + 1;
                kbd.GetMessage() += HorizontalSeparator();
                kbd.GetMessage() += "\n";
                kbd.GetMessage() += "\n";
                SaveData::SaveEntry saveEntry(
                    temporaryParam[event.selectedIndex]->MissionFlags->calcType,
                    temporaryParam[event.selectedIndex]->InfoSection->saveIteration
                );
                SaveData::GetMissionSave(temporaryParam[event.selectedIndex]->InfoSection->uniqueMissionID, saveEntry);
                g_canDeleteData = saveEntry.hasData;
                switch (missionFlags->calcType)
                {
                case MissionParameters::CMSNMissionFlagsSection::CalculationType::TIMER:
                    kbd.GetMessage() += NAME("ms_gradelim") + ": " + missionFlags->GetMaxGradeTimer().Format() + "\n";
                    kbd.GetMessage() += NOTE("ms_gradelim") + ": " + missionFlags->GetMinGradeTimer().Format() + "\n";
                    if (saveEntry.hasData) {
                        kbd.GetMessage() += NAME("ms_rcrd") + ": " + saveEntry.GetTime().Format() + " (" + std::to_string(saveEntry.grade) + ")";
                        #if CITRA_MODE == 0
                        if (!saveEntry.IsChecksumValid()) kbd.GetMessage() += " (U)";
                        #endif
                        kbd.GetMessage() += RightAlign(Color::Yellow << NAME("ms_delsave") << ResetColor(), 35, 367);
                    }
                    break;
                case MissionParameters::CMSNMissionFlagsSection::CalculationType::POINTS:
                    kbd.GetMessage() += NAME("ms_gradelim") + ": " + std::to_string(missionFlags->maxGrade) + GetPtsString(missionFlags->maxGrade) + "\n";
                    kbd.GetMessage() += NOTE("ms_gradelim") + ": " + std::to_string(missionFlags->minGrade) + GetPtsString(missionFlags->minGrade) + "\n";
                    if (saveEntry.hasData) {
                        kbd.GetMessage() += NAME("ms_rcrd") + ": " + std::to_string(saveEntry.GetScore()) + " (" + std::to_string(saveEntry.grade) + ")";
                        #if CITRA_MODE == 0
                        if (!saveEntry.IsChecksumValid()) kbd.GetMessage() += " (U)";
                        #endif
                        kbd.GetMessage() += RightAlign(Color::Yellow << NAME("ms_delsave") << ResetColor(), 35, 367);
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }

    std::string& MissionHandler::GetPtsString(u32 num) {
        if (num == 1)
            return pointsStr.first;
        else
            return pointsStr.second;
    }

    void MissionHandler::OpenKbd() {
        Process::Pause();
        Keyboard kbd(" ");
        kbd.CanAbort(true);
        kbd.OnKeyboardEvent(OnKbdEvent);
        std::vector<std::string> opts;
        int firstEntry = -1;
        for (int i = 0; i < 4; i++) {
            if (!temporaryParam[i])
                opts.push_back("");
            else
            {
                if (firstEntry < 0) firstEntry = i;
                opts.push_back(std::to_string(lastLoadedCup + 1) + "-" + std::to_string(i+1) + " (" + temporaryParam[i]->TextString->GetLangEntry(Language::GetCurrLangID())->GetTitle() + ")");
            }
        }
        if (firstEntry < 0) panic("The selected world is empty.");
        kbd.Populate(opts);
        kbd.ChangeSelectedEntry(firstEntry);
        KeyboardEvent e;
        e.type = KeyboardEvent::EventType::SelectionChanged;
        e.selectedIndex = firstEntry;
        OnKbdEvent(kbd, e);
        int choose = -1;
        do {
#if STRESS_MODE == 1
            choose = g_StresserRnd(0, 4);
#else
            choose = kbd.Open();
#endif
            if (choose < 0) {
                choose = kbd.GetLastSelectedEntry();
                if (choose == -1 || !temporaryParam[choose]) choose = 0;
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->cancelCupSelect = true;
            }
        } while (choose < 0 || !temporaryParam[choose]);

        missionParam = temporaryParam[choose];
        currMissionLevel = choose + 1;
        if (extraSarc) delete extraSarc;
        extraSarc = nullptr;
        cmsnCalculatedChecksum = false;
        sarcCalculatedChecksum = false;
        calculatedChecksum = 0;
        if (replacementSarc) delete replacementSarc;
        replacementSarc = nullptr;
        if (gameCourseSarc) delete gameCourseSarc;
        gameCourseSarc = nullptr;

        Process::Play();
        AsyncRunner::StopAsync(OpenKbd);

        Language::MsbtHandler::SetString(1831, missionParam->TextString->GetLangEntry(Language::GetCurrLangID())->GetTitle());
        MissionParameters::CMSNDriverOptionsSection::OptionsEntry& userEntry = missionParam->DriverOptions->entries[0];
        u32 playerType = EPlayerType::TYPE_USER;
        u32 screwID = 0;
        MissionParameters::CMSNDriverOptionsSection::OptionsEntry realEntry = GetFixedDriverOptions(userEntry);
        MarioKartFramework::BasePage_SetDriver(0, &realEntry.driverID, &playerType);
        MarioKartFramework::BasePage_SetParts(0, &realEntry.bodyID, &realEntry.tireID, &realEntry.wingID, &screwID);
        MarioKartFramework::setSkipGPCoursePreview(!missionParam->MissionFlags->flags.displayCourseIntro);
    }

    u32 MissionHandler::OnGetCupText(u32 cup, u32 track)
    {
        if (track != 0)
            return CustomTextEntries::courseDisplay + track;
        if (coinSoundReplaced) {
            coinSoundReplaced = false;
            CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::coinGetSE, nullptr);
        }
        if (cup != lastLoadedCup)
        {
            lastLoadedCupAccessible = false;
            for (int i = 0; i < 8; i++) {
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->SetButtonEnabledState(i, false);
            }
            currMissionWorld = cup + 1;
            for (int i = 0; i < 4; i++)
                if (temporaryParam[i]) delete temporaryParam[i];
            
            for (int i = 0; i < 4; i++)
            {
                char tmpbuf[100];
                sprintf(tmpbuf, (GetMissionDirPath(currMissionWorld) + "/%d_%d/settings.cmsn").c_str(), currMissionWorld, i + 1);
                temporaryParam[i] = new MissionParameters(tmpbuf);
                if (!temporaryParam[i]->Loaded)
                {
                    delete temporaryParam[i];
                    temporaryParam[i] = nullptr;
                    Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + i, "");
                } else {
                    SaveData::SaveEntry saveEntry(
                        temporaryParam[i]->MissionFlags->calcType,
                        temporaryParam[i]->InfoSection->saveIteration
                    );
                    SaveData::GetMissionSave(temporaryParam[i]->InfoSection->uniqueMissionID, saveEntry);
                    std::string text(temporaryParam[i]->TextString->GetLangEntry(Language::GetCurrLangID())->GetTitle());
                    if (saveEntry.hasData) {
                        text += " (" + std::to_string(saveEntry.grade) + ")";
                    }
                    std::u16string finalText;
                    Utils::ConvertUTF8ToUTF16(finalText, text);
                    #if CITRA_MODE == 0
                    if (!saveEntry.IsChecksumValid())
                        finalText = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::BLUE) + finalText;
                    else
                    #endif
                    if (SaveData::GetFullGradeFlag(currMissionWorld, i + 1))
                        finalText = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 165, 0)) + finalText;

                    lastLoadedCupAccessible = true;
                    Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + i, finalText);
                    MenuPageHandler::MenuSingleCupGPPage::GetInstace()->SetButtonEnabledState(MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon, true);
                }
            }
            lastLoadedCup = cup;
        } else {
            for (int i = 0; i < 8; i++) {
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->SetButtonEnabledState(i, false);
            }
            if (lastLoadedCupAccessible)
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->SetButtonEnabledState(MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon, true);
        }
        return CustomTextEntries::courseDisplay + track;
    }

    void MissionHandler::GetCurrentCupGrade(SaveHandler::CupRankSave::GrandPrixData* out) {
        bool notFinished = false;
        u32 totalGrade = 0;
        u32 currentGrade = 0;
        for (int i = 0; i < 4; i++) {
            if (!temporaryParam[i]) continue;
            SaveData::SaveEntry saveEntry(
                temporaryParam[i]->MissionFlags->calcType,
                temporaryParam[i]->InfoSection->saveIteration
            );
            SaveData::GetMissionSave(temporaryParam[i]->InfoSection->uniqueMissionID, saveEntry);
            if (!saveEntry.hasData) {
                notFinished = true;
                break;
            } else {
                totalGrade += 10;
                currentGrade += saveEntry.grade;
            }
        }
        if (notFinished || totalGrade == 0) {
            out->isCompleted = false;
            out->starRank = 0;
            out->trophyType = 0;
        } else {
            float progress = (float)currentGrade / (float)totalGrade;
            out->isCompleted = true;
            if (progress == 0.f) {
                out->starRank = 0;
                out->trophyType = 0;
            } else if (progress < 0.25f) {
                out->starRank = 0;
                out->trophyType = 1;
            } else if (progress < 0.5f) {
                out->starRank = 0;
                out->trophyType = 2;
            } else if (progress < 0.65f) {
                out->starRank = 0;
                out->trophyType = 3;
            } else if (progress < 0.8f) {
                out->starRank = 4;
                out->trophyType = 3;
            } else if (progress < 1.f) {
                out->starRank = 5;
                out->trophyType = 3;
            } else if (progress == 1.f) {
                out->starRank = 6;
                out->trophyType = 3;
            }
        }
    }

    void MissionHandler::OnGetGrandPrixData(SaveHandler::CupRankSave::GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror) {
        u32 cup = FROMBUTTONTOCUP(*GPID);
        if (lastLoadedCup != cup) panic("Grand prix data unexpected cup.");
        GetCurrentCupGrade(out);
    }

    void MissionHandler::OnCupSelect(u32 retCup)
    {
        AsyncRunner::StartAsync(OpenKbd);
    }

    void MissionHandler::OnCPUSetupParts(u32 slotID)
    {
        if (!MissionHandler::isMissionMode || slotID == 0 || slotID >= missionParam->DriverOptions->playerAmount) return;
        MissionParameters::CMSNDriverOptionsSection::OptionsEntry driverEntry = GetFixedDriverOptions(missionParam->DriverOptions->entries[slotID]);
        if (driverEntry.driverID == EDriverID::DRIVER_RECOMMENDED) return;
        u32 playerType = EPlayerType::TYPE_CPU;
        u32 screwID = 0;
        MarioKartFramework::BasePage_SetDriver(slotID, &driverEntry.driverID, &playerType);
        MarioKartFramework::BasePage_SetParts(slotID, &driverEntry.bodyID, &driverEntry.tireID, &driverEntry.wingID, &screwID);
    }

    void MissionHandler::OnApplySettingsGP() {
        if (!isMissionMode) return;
        VersusHandler::ApplySpecifiedSetting(VersusHandler::VSSettings::CC, missionParam->MissionFlags->ccClass);
        VersusHandler::ApplySpecifiedSetting(VersusHandler::VSSettings::CPU, missionParam->MissionFlags->cpuDifficulty);
        u32 itemSettings = missionParam->ItemOptions->itemMode;
        if (itemSettings == MissionParameters::CMSNItemOptionsSection::ItemMode::Custom) itemSettings = MissionParameters::CMSNItemOptionsSection::ItemMode::All;
        VersusHandler::ApplySpecifiedSetting(VersusHandler::VSSettings::ITEMS, itemSettings);
    }

    void MissionHandler::OnRacePageGenGP(void* racePage) {
        if (!isMissionMode) return;
        // Original
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::NAME](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::TEXTURE](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::EFFECT](racePage);
        if (isScoreEnabled() && isScoreVisible()) MarioKartFramework::RacePageInitFunctions[RacePageInitID::POINT](racePage);
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0 && ((u32*)racePage)[0x26C/4] != 4) {
			MarioKartFramework::RacePageInitFunctions[RacePageInitID::ITEM_SLOT](racePage);
			if (missionParam->MissionFlags->flags.rankVisible) MarioKartFramework::RacePageInitFunctions[RacePageInitID::RANK](racePage);
		}
        if (unkFlag == 0) SpeedometerController::Load();
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::MAP](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::MAP_ICON](racePage);
		if (unkFlag == 0) {
			MarioKartFramework::RacePageInitFunctions[RacePageInitID::RANK_BOARD](racePage);
			if (missionParam->MissionFlags->flags.lapCounterVisible) MarioKartFramework::RacePageInitFunctions[RacePageInitID::LAP](racePage);
			if (!missionParam->MissionFlags->flags.coinCounterHidden) MarioKartFramework::RacePageInitFunctions[RacePageInitID::COIN](racePage);
            //if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::WIPE](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::TEXT](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::CAPTION](racePage);
		MarioKartFramework::RacePageInitFunctions[RacePageInitID::TIMER](racePage);
    }

    void MissionHandler::LoadCoursePreview(char16_t* dst) {
        if (!isMissionMode) return;
        std::string previewPath = Utils::Format((GetMissionDirPath(currMissionWorld) + "/%d_%d/preview.bcstm").c_str(), currMissionWorld, currMissionLevel);
        if (File::Exists(previewPath)) {
            strcpy16(dst, "ram:");
            strcat16n(dst, previewPath.c_str(), 0x100);
        }
    }

    bool MissionHandler::LoadCustomMusic(char16_t* dst, bool isFinalLap) {
        if (!isMissionMode) return false;
        auto it = customMusic.find(missionParam->MissionFlags->courseID);
		if (it != customMusic.end()) {
			std::string musicPath = "ram:" + GetMissionDirPath(currMissionWorld) + "/Music/STRM_C";
            strcpy16(dst, musicPath.c_str());
            strcat16n(dst, (*it).second.musicFileName.data(), 0x100);
            if (isFinalLap) {
                strcat16n(dst, u"_F.bcstm", 0x100);
			}
			else {
				strcat16n(dst, u"_N.bcstm", 0x100);
			}
            return true;
		}
        return false;
    }

    bool MissionHandler::LoadCustomMusicData(u32* out, u32 dataMode, bool isNormalLap) {
        if (!isMissionMode) return false;
        auto it = customMusic.find(missionParam->MissionFlags->courseID);
		if (it != customMusic.end()) {
            if (dataMode == 0) {
                if (isNormalLap) {
				    *out = (*it).second.normalBeatBPM;
                }
                else {
                    *out = (*it).second.fastBeatBPM;
                }
            } else if (dataMode == 1) {
                if (isNormalLap) {
				    *out = (*it).second.normalBeatOffset;
                }
                else {
                    *out = (*it).second.fastBeatOffset;
                }
            } else {
                *out = (*it).second.mode;
            }
			return true;
		}
        return false;
    }

    bool MissionHandler::LoadCustomMusicNameAuthor(std::string& name, std::string& author) {
        if (!isMissionMode) return false;
        auto it = customMusic.find(missionParam->MissionFlags->courseID);
		if (it != customMusic.end() && !it->second.musicName.empty()) {
            name = it->second.musicName;
            author = it->second.GetAuthorString();
            return true;
        }
        return false;
    }

    void MissionHandler::OnRaceDirectorCreateBeforeStructure()
    {
        if (!isMissionMode) return;
        lastMissionResult.isCalculated = false;
        AddPoints = AddPointsQueue = 0;
        raceBeenFinished = false;
        missionConditionFailed = false;
        failMissionFrames = -1;
        missionGates.clear();
        if (!cmsnCalculatedChecksum) {
            cmsnCalculatedChecksum = true;
            calculatedChecksum += missionParam->GetCheckSum();
        }
        ExtraResource::SARC::FileInfo fInfo;
        u8* coinSound = extraSarc->GetFile("coin_sound.bcwav", &fInfo);
        if (coinSound) {
            coinSoundReplaced = true;
            CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::coinGetSE, coinSound);
        }
        customMusic.clear();
        std::string previewPath = Utils::Format((GetMissionDirPath(currMissionWorld) + "/%d_%d/music.ini").c_str(), currMissionWorld, currMissionLevel);
        MusicSlotMngr::initMusicSlotData(previewPath, currMissionWorld > 8, customMusic);
        return;
    }

    void MissionHandler::OnGPKartFinishRace(u32* finishPosition, u32* halfDriverAmount, u32 playerID) {
        if (*finishPosition == 1 && playerID != MarioKartFramework::masterPlayerID && 
        missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER0FIRST))
        {
            missionConditionFailed = true;
            failMissionFrames = 0;
        }

        if (*finishPosition == 1 && (
            (playerID != 1 && missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER1FIRST)) ||
            (playerID != 2 && missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::PLAYER2FIRST))
            ))
        {
            missionConditionFailed = true;
            failMissionFrames = 0;
        }

        if (playerID == MarioKartFramework::masterPlayerID) {
            calculateResult();
            *halfDriverAmount = 4;
            if (lastMissionResult.points <= 0)
                *finishPosition = 8;
            else if (lastMissionResult.points >= 10)
                *finishPosition = 1;
            else
                *finishPosition = 3;
        }
    }

    void MissionHandler::tryOverwriteFile(SafeStringBase* file, ExtraResource::SARC::FileInfo& info) {
        u32 fileHash = gameCourseSarc->GetHash(file->data);
        if (std::find(replacedSarcFiles.begin(), replacedSarcFiles.end(), fileHash) != replacedSarcFiles.end())
            return;
        replacedSarcFiles.push_back(fileHash);
        u8* gameFilePtr = gameCourseSarc->GetFile(fileHash, &info);
        replacementSarc->ReadFile(gameFilePtr, info, *file);
    }

    u8* MissionHandler::GetReplacementFile(SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo, u8* courseSarc) {
        if (!isMissionMode) return nullptr;
        if (!extraSarc) {
            std::string sarcNameBase = Utils::Format((GetMissionDirPath(currMissionWorld) + "/%d_%d/").c_str(), currMissionWorld, currMissionLevel);
            File sarc(sarcNameBase + "data.sarc");
            if (sarc.IsOpen() && sarc.GetSize() < gameBufferSize) {
                sarc.Read(gameBuffer, sarc.GetSize());
            } else {
                memset(gameBuffer, 0, 0x100);
            }
            extraSarc = new ExtraResource::SARC((u8*)gameBuffer, false);
            replacementSarc = new ExtraResource::StreamedSarc(sarcNameBase + "replacement.sarc");
            gameCourseSarc = new ExtraResource::SARC(courseSarc, false);
            if (!sarcCalculatedChecksum) {
                sarcCalculatedChecksum = true;
                calculatedChecksum += extraSarc->GetDataChecksum(missionParam->InfoSection->key);
                calculatedChecksum += replacementSarc->GetDataChecksum(missionParam->InfoSection->key, 0x1000);
            }
        }
        ExtraResource::SARC::FileInfo tempFInfo;
        if (!fileInfo) fileInfo = &tempFInfo;
        if (missionParam->MissionFlags->missionType == MissionParameters::CMSNMissionFlagsSection::MissionType::GATES && !strcmp(file->data, "MpBoard/MpBoard_NoClide.kcl")) {
            SafeStringBase replaceFile("MpBoard/MpBoard.kcl");
            return extraSarc->GetFile(replaceFile, fileInfo);
        }
        u8* ret = extraSarc->GetFile(*file, fileInfo);
        if (!ret) tryOverwriteFile(file, *fileInfo);
        return ret;
    }

    bool MissionHandler::isScoreEnabled() {
        return (isMissionMode && missionParam->MissionFlags->initialScore != 0xFF);
    }

    bool MissionHandler::isScoreVisible() {
        return (isMissionMode && !missionParam->MissionFlags->flags.scoreHidden);
    }

    bool MissionHandler::forceBackwards() {
        return(isMissionMode && MarioKartFramework::isRaceState && MarioKartFramework::allowBackwardsCamera() && missionParam->MissionFlags->flags.forceBackwards);
    }

    bool MissionHandler::blockLapJingle() {
        return (isMissionMode && missionParam->MissionFlags->flags.finishOnSection);
    }

    bool MissionHandler::blockCrashCoinSpawn() {
        return (isMissionMode && missionParam->MissionFlags->flags.blockCrashCoinSpawn);
    }

    u8* MissionHandler::GetReplacementScoreIcon(ExtraResource::SARC::FileInfo* fileInfo) {
        return extraSarc->GetFile("score_icon.bclim", fileInfo);
    }

    u8* MissionHandler::GetReplacementItemBox(ExtraResource::SARC::FileInfo* fileInfo) {
        return extraSarc->GetFile("itemBox/itemBox.bcmdl", fileInfo);
    }

    u8* MissionHandler::GetReplacementCoin(ExtraResource::SARC::FileInfo* fileInfo, bool isLod) {
        return extraSarc->GetFile(isLod ? "Coin_lod/Coin_lod.bcmdl" : "Coin_detail/Coin_detail.bcmdl", fileInfo);
    }

    static char g_gateModelName[0x10];
    const char* MissionHandler::OnObjBaseSetupMdlGetName(u32* objectBase, const char* origName) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::GATES) return origName;
        GOBJEntry* entry = (GOBJEntry*)(((u32**)objectBase)[2][0]);
        if (entry->objID == 0x1AA) { // Music Park glide board
            missionGates.push_back(objectBase);
            u32 id = entry->settings[2];
            if (id == 0) return origName;
            if (id > 10) id = 10;
            sprintf(g_gateModelName, "MpBoard.%d", id);
            return g_gateModelName;
        }
        return origName;
    }

    void MissionHandler::OnGateThrough(u32* objectMpBoard, int gateID) {
        if ((missionParam->MissionFlags->SubTypeGate() == MissionParameters::CMSNMissionFlagsSection::SubTypes::Gate::ORDER && (gateID - 1 == counterUIGet())) ||
            (missionParam->MissionFlags->SubTypeGate() == MissionParameters::CMSNMissionFlagsSection::SubTypes::Gate::ALL)) {
            bool* isGateActive = (bool*)objectMpBoard + 0x17B;
            *isGateActive = true;
            MarioKartFramework::ObjModelBaseChangeAnimation((u32)objectMpBoard, 0, 0.f);
            GivePoint();
        }
    }

    void MissionHandler::OnMpBoardObjectInit(u32* objectMpBoard) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::GATES) return;
        u32* state = objectMpBoard + (0x1B4 / 4);
        *state = 1;
        float* skew = (float*)objectMpBoard + (0x190 / 4);
        *skew = 1.0f;
        u32* skew2 = objectMpBoard + (0x1F4 / 4);
        *skew2 = 0;
        bool* isGateActive = (bool*)objectMpBoard + 0x17B;
        *isGateActive = false;
    }

    bool MissionHandler::tcBoardAllowPlayAnim(u32* objectTcBoard) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::GATES) return true;
        GOBJEntry* entry = (GOBJEntry*)(((u32**)objectTcBoard)[2][0]);
        return entry->objID != 0x1AA;
    }

    void MissionHandler::OnObjectBaseSetScale(u32* objectBase, Vector3& copyTo, const Vector3& copyFrom) {
        GOBJEntry* entry = (GOBJEntry*)(((u32**)objectBase)[2][0]);
        if (entry->objID == 0x1C) { // Melting Ice
            // No need to do anything anymore
        }
        copyTo = copyFrom;
    }

    void MissionHandler::OnObjectMeltIceInitObj(u32* objectBase) {
        if (!isMissionMode) return;
        GOBJEntry* entry = (GOBJEntry*)(((u32**)objectBase)[2][0]);
        
        if (entry->settings[0] == 0) { // Only visual
            MarioKartFramework::KDGndColBlock_remove(objectBase[0x19C/4]);
        } else if (entry->settings[0] == 1) { // Only collision
            ((u8*)objectBase)[0x7C] = 0;
        }
    }

    bool MissionHandler::OnKartEffectTrigger(u32 vehicleMove, u32 effectType) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::GATES || effectType != 7) return true;
        Vector3& kartPos = *(Vector3*)(vehicleMove + 0x24);
        float closestGateDist = HUGE_VALF;
        int closestGate = -1;
        for (int i = 0; i < missionGates.size(); i++) {
            GOBJEntry* entry = (GOBJEntry*)(((u32**)missionGates[i])[2][0]);
            float dist = DistanceNoSqrt(kartPos, entry->position);
            if (dist < closestGateDist) {
                closestGateDist = dist;
                closestGate = i;
            }
        }
        if (closestGate != -1)
        {
            bool* isGateActive = (bool*)missionGates[closestGate] + 0x17B;
            if (!*isGateActive) {
                GOBJEntry* entry = (GOBJEntry*)(((u32**)missionGates[closestGate])[2][0]);
                OnGateThrough(missionGates[closestGate], entry->settings[2]);
            }
        }
        
        return false;
    }

    u32 MissionHandler::onKartItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 reactObject, u32 objCallFuncPtr, int mode)
    {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::OBJECTS)
            return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);

        u32 vehicleReact = 0;
        if (mode == 1)
            // 0x158 -> infoproxy
            vehicleReact = ((u32***)(reactObject))[0][0x158 / 4][0];
        else if (mode == 0)
            vehicleReact = reactObject;
        else if (mode == 2)
            vehicleReact = MarioKartFramework::getVehicle(ItemHandler::thunderPlayerID);

        int playerID = ((u32*)vehicleReact)[0x84 / 4];

        if (playerID != MarioKartFramework::masterPlayerID)
            return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);

        u16 objID = ((GOBJEntry***)object)[2][0]->objID;

        switch (missionParam->MissionFlags->SubTypeObjects())
        {
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Objects::ITEMBOX:
            if ((objID == 0x4 || objID == 0x65) && eObjectReactType == OBJECTREACTTYPE_DESTRUCTVE) {
                bool wasBroken = (((u32**)object)[0x84 / 4][0x38 / 4] & 0x100000) != 0;
                u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
                bool isBroken = (((u32**)object)[0x84 / 4][0x38 / 4] & 0x100000) != 0;
                if (!wasBroken && isBroken) GivePoint();
                return ret;
            }
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Objects::ROCKYWRENCH:
            if (objID == 0xD3 && eObjectReactType == OBJECTREACTTYPE_DESTRUCTVE) {
                bool wasHit = ((u8*)object)[0x18E] != 0;
                u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
                bool isHit = ((u8*)object)[0x18E] != 0;
                if (!wasHit && isHit) GivePoint();
                return ret;
            }
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Objects::CRAB:
            if (objID == 0xCD && eObjectReactType == OBJECTREACTTYPE_DESTRUCTVE) {
                bool wasHit = ((u8*)object)[0x188 + 0x10] != 0 || ((u32*)object)[(0x188 + 0x40) / 4] != 0;
                u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
                bool isHit = ((u8*)object)[0x188 + 0x10] != 0;
                if (!wasHit && isHit) GivePoint();
                return ret;
            }
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Objects::FROGOON:
            if (objID == 0xD7 && eObjectReactType == OBJECTREACTTYPE_DESTRUCTVE) {
                bool wasHit = ((u8*)object)[0x1F4 + 0x10] != 0 || ((u32*)object)[(0x1F4 + 0x40) / 4] != 0;
                u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
                bool isHit = ((u8*)object)[0x1F4 + 0x10] != 0;
                if (!wasHit && isHit) GivePoint();
                return ret;
            }
        default:
            break;
        }

        return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
    }

    void MissionHandler::onKartHitCoin(int playerID) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::OBJECTS
        || missionParam->MissionFlags->SubTypeObjects() != MissionParameters::CMSNMissionFlagsSection::SubTypes::Objects::COIN
        || playerID != MarioKartFramework::masterPlayerID) return;

        GivePoint();
    }
    
    void MissionHandler::OnKartDash(u32* vehicleMove, EDashType dash, bool isImprovedTrick) {
        if (!isMissionMode || missionParam->MissionFlags->missionType != MissionParameters::CMSNMissionFlagsSection::MissionType::BOOSTS || vehicleMove[0x21] != MarioKartFramework::masterPlayerID) return;
        MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts subType = missionParam->MissionFlags->SubTypeBoosts();
        switch (subType)
        {
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::BOOSTPAD:
            if (dash == EDashType::BOOST_PAD) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::MINITURBO:
            if (dash == EDashType::MINITURBO || dash == EDashType::SUPERMINITURBO) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::SUPERMINITURBO:
            if (dash == EDashType::SUPERMINITURBO) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::ROCKET_START:
            if (dash >= EDashType::START_VERYSMALL && dash <= EDashType::START_PERFECT) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::TRICK:
            if (dash >= EDashType::TRICK_GROUND && dash <= EDashType::TRICK_WATER) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::IMPROVED_TRICK:
            if (dash >= EDashType::TRICK_GROUND && dash <= EDashType::TRICK_WATER && isImprovedTrick) GivePoint();
            break;
        case MissionParameters::CMSNMissionFlagsSection::SubTypes::Boosts::STAR_RING:
            if (dash == EDashType::STAR_RING) GivePoint();
            break;
        default:
            break;
        }
    }

    void MissionHandler::OnKartAccident(u32* vehicleReact, ECrashType* type) {
        if (!isMissionMode || vehicleReact[0x21] != MarioKartFramework::masterPlayerID || *type == ECrashType::CRASHTYPE_FALSESTART) return;
        if (missionParam->MissionFlags->flags.pointWhenAccident)
            GivePoint();
        if (!raceBeenFinished && missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::NOTGETHIT)) {
            missionConditionFailed = true;
            failMissionFrames = 0;
        }
    }

    u32 MissionHandler::GetCoinRespawnFrames() {
        if (!isMissionMode || missionParam->MissionFlags->respawnCoinTimer == 0xFFFF) return MarioKartTimer::ToFrames(0, 5, 0);
        else if (missionParam->MissionFlags->respawnCoinTimer == 0) return 0x7FFFFFFF;
        else return missionParam->MissionFlags->respawnCoinTimer * 20;
    }

    u32 MissionHandler::GetItemBoxRespawnFrames() {
        if (!isMissionMode || missionParam->ItemOptions->respawnItemBoxTimer == 0xFFFF) return MarioKartTimer::ToFrames(0, 2, 0);
        else if (missionParam->ItemOptions->respawnItemBoxTimer == 0) return 0x7FFFFFFF;
        return missionParam->ItemOptions->respawnItemBoxTimer;
    }

    float MissionHandler::CalculateSpeedMod(float baseSpeedMod) {
        bool useCustomCC = missionParam->MissionFlags->ccSelectorSpeed != 0;
        float ccvalaue = useCustomCC ? (missionParam->MissionFlags->ccSelectorSpeed * CC_SINGLE) : 1.f;

        if (useCustomCC && missionParam->MissionFlags->ccSelectorSpeed < 150) {
            baseSpeedMod = ccvalaue * baseSpeedMod;
        } else {
            if (ccvalaue > baseSpeedMod) {
                baseSpeedMod = ccvalaue;
            }
        }
        return baseSpeedMod;
    }
    bool MissionHandler::AllowImprovedTricks() {
        return !isMissionMode || (missionParam->MissionFlags->improvedTricks != MissionParameters::CMSNMissionFlagsSection::ImproveTricksMode::DISABLED); 
    }
	bool MissionHandler::ForceImprovedTricks() {
        return isMissionMode && (missionParam->MissionFlags->improvedTricks == MissionParameters::CMSNMissionFlagsSection::ImproveTricksMode::ENABLED);
    }

    bool MissionHandler::neeedsCustomItemHandling() 
    {
        if (!isMissionMode) return false;
        return (missionParam->ItemOptions->itemMode == MissionParameters::CMSNItemOptionsSection::ItemMode::Custom);
    }

    u16 MissionHandler::handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag)
    {
        MissionParameters::CMSNItemOptionsSection::ConfigEntry* entry = missionParam->ItemOptions->GetConfig(MarioKartFramework::startedItemSlot == MarioKartFramework::masterPlayerID);
        u32 row = 0;

        if (entry->configMode == MissionParameters::CMSNItemOptionsSection::ConfigEntry::ConfigMode::Rank) {
            row = rowIndex;
        } else if (entry->configMode == MissionParameters::CMSNItemOptionsSection::ConfigEntry::ConfigMode::DriverID) {
            row = MarioKartFramework::startedItemSlot;
        } else if (entry->configMode == MissionParameters::CMSNItemOptionsSection::ConfigEntry::ConfigMode::BoxID) {
            row = rowIndex + 1;
        } else if (entry->configMode == MissionParameters::CMSNItemOptionsSection::ConfigEntry::ConfigMode::TrueRank) {
            row = MarioKartFramework::getModeManagerData()->driverPositions[MarioKartFramework::startedItemSlot];
        }

        u16 totalProb = 0;
        for (int i = 0; i < EItemSlot::ITEM_SIZE; i++) {
            if ((blockedBitFlag & (1 << i)) == 0 || entry->disableCooldown) {
                u16 currProb = entry->probabilities[row][i];
                totalProb += currProb;
            }
            dstArray[i] = totalProb;
            MarioKartFramework::storeImprovedRouletteItemProbability(i, totalProb);
        }
        return totalProb;
    }

    u32 MissionHandler::calcItemSlotTimer(u32 timer, u32 driverID)
    {
        if (MissionHandler::isMissionMode && missionParam->ItemOptions->itemMode == MissionParameters::CMSNItemOptionsSection::ItemMode::Custom) 
        {
            MissionParameters::CMSNItemOptionsSection::ConfigEntry* entry = missionParam->ItemOptions->GetConfig(driverID == MarioKartFramework::masterPlayerID);
            if (entry->rouletteSpeed != 0 && timer >= entry->rouletteSpeed)
                return 180;
        }
        return timer + 1;
    }

    void MissionHandler::GivePoint() {
        if (missionParam->MissionFlags->countDirection.scoreDirectionUp) ++AddPoints;
        else --AddPoints;

        if (missionParam->MissionFlags->HasCompleteCondition(MissionParameters::CMSNMissionFlagsSection::CompleteCondition::SCOREISNOTYELLOW) &&
        pointControlShouldBeYellow(counterUIGet(true))) failMissionFrames = 0;
    }

    static MarioKartTimer g_givePlayerTimer;
    static MarioKartTimer g_giveCPUTimer;
    static float* g_animEval;
    static MarioKartTimer g_scoreUpTimer;

    void MissionHandler::onFrame(bool raceFinished) {
        if (!raceFinished) {
            if (g_givePlayerTimer.GetFrames() != 0xFFFFFFFF) {
                if (g_givePlayerTimer.GetFrames() == 0) {
                    g_givePlayerTimer.SetFrames(missionParam->ItemOptions->GetConfig(true)->giveItemEach);

                    MarioKartFramework::startItemSlot(MarioKartFramework::masterPlayerID, 0);
                }
                --g_givePlayerTimer;
            }
            if (g_giveCPUTimer.GetFrames() != 0xFFFFFFFF) {
                if (g_giveCPUTimer.GetFrames() == 0) {
                    g_giveCPUTimer.SetFrames(missionParam->ItemOptions->GetConfig(false)->giveItemEach);

                    for (int i = 0; i < missionParam->DriverOptions->playerAmount; i++) {
                        if (i == MarioKartFramework::masterPlayerID) continue;
                        MarioKartFramework::startItemSlot(i, 0);
                    }
                }
                --g_giveCPUTimer;
            }
            if (AddPoints && !AddPointsQueue) {
                AddPointsQueue = AddPoints;
                AddPoints = 0;
            }
            if (AddPointsQueue != 0) {
                if (g_scoreUpTimer > MarioKartTimer(MarioKartTimer::ToFrames(0, 0, 100))) {
                    g_scoreUpTimer = MarioKartTimer(0);
                    counterUIAdd(((AddPointsQueue < 0) ? -1 : 1));
                    AddPointsQueue += ((AddPointsQueue < 0) ? 1 : -1);
                };
            }
        }
        g_scoreUpTimer++;
    }

    bool MissionHandler::shouldFinishRace(MarioKartTimer& raceTimer) {
        if (missionParam->MissionFlags->finishRaceTimer != 0xFFFFFFFF) {
            if((missionParam->MissionFlags->countDirection.timerDirectionUp && raceTimer >= missionParam->MissionFlags->GetFinishRaceTimer())
            || (!missionParam->MissionFlags->countDirection.timerDirectionUp && raceTimer <= missionParam->MissionFlags->GetFinishRaceTimer()))
            return true;
        }
        if (missionParam->MissionFlags->finishRaceScore != 0xFF) {
            if((missionParam->MissionFlags->countDirection.scoreDirectionUp && counterUIGet(true) >= missionParam->MissionFlags->finishRaceScore)
            || (!missionParam->MissionFlags->countDirection.scoreDirectionUp && counterUIGet(true) <= missionParam->MissionFlags->finishRaceScore))
            return true;
        }
        if (missionParam->MissionFlags->flags.finishOnSection && missionParam->MissionFlags->lapAmount == 1) {
            KartLapInfo* info = MarioKartFramework::getKartLapInfo(MarioKartFramework::masterPlayerID);
            if (info->currentLap == 2) {
                return true;
            }
        }
        return (failMissionFrames >= 0) ? (failMissionFrames-- == 0) : false;
    }

    void MissionHandler::timerHandler(MarioKartTimer& timerObject)
    {
        bool raceFinish = shouldFinishRace(timerObject);
        if (raceFinish) {
            MarioKartFramework::forceFinishRace = true;
        }
        else {
            if (missionParam->MissionFlags->countDirection.timerDirectionUp)
                ++timerObject;
            else if (timerObject.GetFrames() > 0)
                --timerObject;
            lastMainTimer = timerObject;
        }
        onFrame(raceFinish || raceBeenFinished);
    }

    static bool g_setInitialCounterValue = false;
    void MissionHandler::setInitialTimer(MarioKartTimer& timerObject)
    {
        if (missionParam->ItemOptions->itemMode == MissionParameters::CMSNItemOptionsSection::ItemMode::Custom)
        {
            MissionParameters::CMSNItemOptionsSection::ConfigEntry* currEntry = missionParam->ItemOptions->GetConfig(true);
            if (currEntry->giveItemEach != 0)
                g_givePlayerTimer.SetFrames(currEntry->giveItemOffset);
            else
                g_givePlayerTimer.SetFrames(0xFFFFFFFF);
            
            currEntry = missionParam->ItemOptions->GetConfig(false);
            if (currEntry->giveItemEach != 0)
                g_giveCPUTimer.SetFrames(currEntry->giveItemOffset);
            else
                g_giveCPUTimer.SetFrames(0xFFFFFFFF);
        } else {
            g_givePlayerTimer.SetFrames(0xFFFFFFFF);
            g_giveCPUTimer.SetFrames(0xFFFFFFFF);
        }

        timerObject = missionParam->MissionFlags->GetInitialTimer();
        g_setInitialCounterValue = false;
    }

    void MissionHandler::setInitialCounter()
    {
        if (!g_setInitialCounterValue) {
            g_setInitialCounterValue = true;
            counterUISet(missionParam->MissionFlags->initialScore);
        }
    }

    bool MissionHandler::isRankVisible() {
        return !isMissionMode || missionParam->MissionFlags->flags.rankVisible;
    }

    bool MissionHandler::isLakituVisible() {
        return !isMissionMode || missionParam->MissionFlags->flags.lakituVisible;
    }

    u8 MissionHandler::getLapAmount() {
        return missionParam->MissionFlags->lapAmount;
    }

    u32 MissionHandler::getInitialMissionTime() {
        return missionParam->MissionFlags->GetInitialTimer().GetFrames();
    }

    bool MissionHandler::pointControlShouldBeYellow(u32 val)
    {
        if (missionParam->MissionFlags->yellowScore == 0xFF) return false;
        return (missionParam->MissionFlags->countDirection.scoreDirectionUp && val >= missionParam->MissionFlags->yellowScore)
            || (!missionParam->MissionFlags->countDirection.scoreDirectionUp && val <= missionParam->MissionFlags->yellowScore);
    }

    u32 MissionHandler::isItemBoxAllowed(u32 itemMode)
    {
        if (!isMissionMode) return itemMode;
        if (missionParam->ItemOptions->spawnItemBoxes) return itemMode;
        else return EItemMode::ITEMMODE_NONE;
    }

    void MissionHandler::counterUIAdd(int value, bool playSound)
    {
        (g_altGameModeplayerStatPointer - 0x9)[0x2F5] += value;
        if (playSound && value != 0 && isScoreVisible()) {
            if (pointControlShouldBeYellow(counterUIGet())) Snd::PlayMenu(missionParam->MissionFlags->flags.scoreNegative ? Snd::CANCEL_L : Snd::PAUSE_TO_NEXT);
            else Snd::PlayMenu(missionParam->MissionFlags->flags.scoreNegative ? Snd::CANCEL_M : Snd::PAUSE_OFF);
        }
    }

    void MissionHandler::counterUISet(u32 value, bool playSound)
    {
        (g_altGameModeplayerStatPointer - 0x9)[0x2F5] = value;
        if (playSound && value != 0 && isScoreVisible()) Snd::PlayMenu(missionParam->MissionFlags->flags.scoreNegative ? Snd::CANCEL_M : Snd::PAUSE_OFF);
    }

    u32 MissionHandler::counterUIGet(bool includeQueue)
    {
        return (g_altGameModeplayerStatPointer - 0x9)[0x2F5] + (includeQueue ? (AddPoints + AddPointsQueue) : 0);
    }

    MissionHandler::MissionParameters::MissionParameters(const std::string& fileName)
    {
        File missionFile(fileName, File::READ);
        if (!missionFile.IsOpen() || (fileSize = missionFile.GetSize()) < sizeof(CMSNHeader)) { Loaded = false; return;}
        CMSNHeader tmpHead;
        missionFile.Read(&tmpHead, sizeof(CMSNHeader));
        if (tmpHead.signature != CMSNSignature || tmpHead.version > CMSNSupportedVersion) { Loaded = false; return; }
        missionFile.Seek(0, File::SeekPos::SET);
        fileData = new u8[tmpHead.fileSize];
        missionFile.Read(fileData, tmpHead.fileSize);
        Loaded = ProcessFileData();
    }

    MissionHandler::MissionParameters::MissionParameters(const u8* fData, u32 fSize)
    {
        CMSNHeader* tmpHead = (CMSNHeader*)fData;
        fileSize = fSize;
        if (tmpHead->signature != CMSNSignature || tmpHead->version > CMSNSupportedVersion) { Loaded = false; return; }
        fileData = (u8*)::operator new(tmpHead->fileSize);
        memcpy(fileData, fData, tmpHead->fileSize);
        Loaded = ProcessFileData();
    }

    MissionHandler::MissionParameters::~MissionParameters()
    {
        if (fileData) operator delete(fileData);
    }

    u32  MissionHandler::MissionParameters::GetCheckSum() {
    }

    bool MissionHandler::MissionParameters::ProcessFileData()
    {
        if (!fileData) return false;
        Header = (CMSNHeader*)fileData;
        CMSNSectionTable* table = (CMSNSectionTable*)(fileData + Header->sectionTableOffset);
        for (int i = 0; i < table->amount; i++) {
            void* dataPtr = (fileData + Header->sectionsOffset + table->entries[i].offset);
            switch (table->entries[i].type)
            {
            case CMSNSectionType::SectionDriverOptions:
                if (DriverOptions == nullptr) DriverOptions = (CMSNDriverOptionsSection*)dataPtr;
                break;
            case CMSNSectionType::SectionMissionFlags:
                if (MissionFlags == nullptr) MissionFlags = (CMSNMissionFlagsSection*)dataPtr;
                break;
            case CMSNSectionType::SectionItemOptions:
                if (ItemOptions == nullptr) ItemOptions = (CMSNItemOptionsSection*)dataPtr;
                break;
            case CMSNSectionType::SectionText:
                if (TextString == nullptr) TextString = (CMSNTextSection*)dataPtr;
                break;
            case CMSNSectionType::SectionInformation:
                if (InfoSection == nullptr) InfoSection = (CMSNInformationSection*) dataPtr;
            default:
                break;
            }
        }
        if (MissionFlags != nullptr && DriverOptions != nullptr && ItemOptions != nullptr) return true;
        return false;
    }
    
    minibson::encdocument MissionHandler::SaveData::save;

    void MissionHandler::SaveData::Load() {
        SaveHandler::SaveFile::LoadStatus status;
        save = SaveHandler::SaveFile::Load(SaveHandler::SaveFile::SaveType::MISSION, status);
        u64 scID = save.get<u64>("cID", 0ULL);
        if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS && (scID == NetHandler::GetConsoleUniqueHash()
        #if CITRA_MODE == 1
        || scID == 0x5AFF5AFF5AFF5AFF
        #endif
        #ifdef ALLOW_SAVES_FROM_OTHER_CID
        || true
        #endif
        )) {
            ;
        } else {
            save.clear();
            save.set<u64>("cID", NetHandler::GetConsoleUniqueHash());
            save.set("missionSave", minibson::document());
            save.set("missionFullGrade", minibson::document());
        }
        if (!save.contains("missionFullGrade"))
            save.set("missionFullGrade", minibson::document());
    }
	void MissionHandler::SaveData::Save() {
        save.set<u64>("cID", NetHandler::GetConsoleUniqueHash());
        SaveHandler::SaveFile::Save(SaveHandler::SaveFile::SaveType::MISSION, save);
    }

    void MissionHandler::SaveData::GetMissionSave(const char missionID[0xC], MissionHandler::SaveData::SaveEntry& entry) {
        entry.hasData = false;
        entry.isDirty = false;
        minibson::document empty;
        const minibson::document& data = save.get("missionSave", empty).get(missionID, empty);
        if (data.empty())
            return;
        u32 saveIteration = data.get("s", (int)0);
        if (saveIteration != entry.saveIteration)
            return;
        PackedSavedInfo info;
        info.raw = data.get("i", (int)-1);
        if (info.calcType != (u32)entry.calcType)
            return;
        
        entry.isChecksumValid = data.get("c", false);
        entry.score = data.get("b", (int)0);
        entry.grade = info.grade;
        entry.hasData = true;
    }

    void MissionHandler::SaveData::SetMissionSave(const char missionID[0xC], MissionHandler::SaveData::SaveEntry& entry) {
        if (!entry.hasData) return;
        minibson::document data;
        minibson::document empty;

        PackedSavedInfo info;
        info.grade = entry.grade;
        info.calcType = (u32)entry.calcType;
        data.set("i", (int)info.raw);
        data.set("b", (int)entry.score);

        data.set("c", entry.isChecksumValid);
        data.set("s", (int)entry.saveIteration);

        save.get_noconst("missionSave", empty).set(missionID, data);

        if (entry.isDirty)
        {
            entry.isDirty = false;
            SaveData::Save();
        }
    }

    void MissionHandler::SaveData::ClearMissionSave(const char missionID[0xC]) {
        minibson::document empty;
        save.get_noconst("missionSave", empty).remove(missionID);
        SaveData::Save();
    }

    void MissionHandler::SaveData::SetFullGradeFlag(u32 world, u32 level) {
        if (level == 0 || level > 4 || world == 0) return;
        minibson::document empty;
        u32 entry = (world - 1) / 8;
        u32 data = save.get("missionFullGrade", empty).get(std::to_string(entry).c_str(), (int)0);
        data |= (1 << (((world - 1) & 7) * 4 + (level - 1)));
        save.get_noconst("missionFullGrade", empty).set(std::to_string(entry).c_str(), (int)data);
        SaveData::Save();
    }

	bool MissionHandler::SaveData::GetFullGradeFlag(u32 world, u32 level) {
        if (level == 0 || level > 4 || world == 0) return false;
        u32 entry = (world - 1) / 8;
        minibson::document empty;
        u32 data = save.get("missionFullGrade", empty).get(std::to_string(entry).c_str(), (int)0);
        return data & (1 << (((world - 1) & 7) * 4 + (level - 1)));
    }

    std::pair<int, int> MissionHandler::SaveData::GetAllFullGradeFlag() {
        int total = 0, curr = 0;
        for (int i = 1; i <= 4; i++) {
            for (int j = 1; j <= 4; j++) {
                if (GetFullGradeFlag(i, j))
                    curr++;
                total++;
            }
        }
        return std::make_pair(total, curr);
    }
}