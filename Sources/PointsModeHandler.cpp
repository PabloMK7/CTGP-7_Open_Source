/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: PointsModeHandler.cpp
Open source lines: 1762/1762 (100.00%)
*****************************************************/

#include "PointsModeHandler.hpp"
#include "ExtraResource.hpp"
#include "Sound.hpp"
#include "CwavReplace.hpp"
#include "MarioKartFramework.hpp"
#include "main.hpp"
#include "SequenceHandler.hpp"
#include "CustomTextEntries.hpp"
#include "VersusHandler.hpp"
#include "MissionHandler.hpp"
#include "MenuPage.hpp"
#include "CourseManager.hpp"
#include "TextFileParser.hpp"
#include "CharacterHandler.hpp"
#include "Unicode.h"
#include "str16utils.hpp"
#include "Lang.hpp"
#include "MK7Memory.hpp"
#include "AsyncRunner.hpp"
#include "MenuPage.hpp"
#include "StatsHandler.hpp"
#include "SaveBackupHandler.hpp"

namespace CTRPluginFramework {
    bool PointsModeHandler::isPointsMode = false;
    bool PointsModeHandler::comesFromRace = false;
    const PointsModeDisplayController::CongratState PointsModeHandler::comboToCongrat[10] = {
        PointsModeDisplayController::CongratState::NONE,
        PointsModeDisplayController::CongratState::NONE,
        PointsModeDisplayController::CongratState::NONE,
        PointsModeDisplayController::CongratState::NICE,
        PointsModeDisplayController::CongratState::NICE,
        PointsModeDisplayController::CongratState::GREAT,
        PointsModeDisplayController::CongratState::GREAT,
        PointsModeDisplayController::CongratState::EXCELLENT,
        PointsModeDisplayController::CongratState::EXCELLENT,
        PointsModeDisplayController::CongratState::FANTASTIC,
    };
    const std::unordered_map<PointsModeHandler::PointAction, PointsModeHandler::ActionInfo> PointsModeHandler::pointActionInfo = {
        {PointsModeHandler::PointAction::MUSHROOM, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::BOOST_PAD, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::MINITURBO, PointsModeHandler::ActionInfo(5, PointsModeHandler::ActionFlags::NON_ACCUMULATIVE)},
        {PointsModeHandler::PointAction::SUPERMINITURBO, PointsModeHandler::ActionInfo(15)},
        {PointsModeHandler::PointAction::START_SMALL, PointsModeHandler::ActionInfo(10, PointsModeHandler::ActionFlags::IGNORE_POS_MULTIPLIER)},
        {PointsModeHandler::PointAction::START_MEDIUM, PointsModeHandler::ActionInfo(20, PointsModeHandler::ActionFlags::IGNORE_POS_MULTIPLIER)},
        {PointsModeHandler::PointAction::START_PERFECT, PointsModeHandler::ActionInfo(30, PointsModeHandler::ActionFlags::IGNORE_POS_MULTIPLIER)},
        {PointsModeHandler::PointAction::TRICK, PointsModeHandler::ActionInfo(20)},
        {PointsModeHandler::PointAction::IMPROVED_TRICK, PointsModeHandler::ActionInfo(30)},
        {PointsModeHandler::PointAction::SLIPSTREAM, PointsModeHandler::ActionInfo(30)},
        {PointsModeHandler::PointAction::GOLD_MUSHROOM, PointsModeHandler::ActionInfo(5)},
        {PointsModeHandler::PointAction::COIN, PointsModeHandler::ActionInfo(5)},
        {PointsModeHandler::PointAction::STAR_RING, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_PIPE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_GOOMBA, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_SWOOP, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_PYLON, PointsModeHandler::ActionInfo(1)},
        {PointsModeHandler::PointAction::HIT_BOULDER, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_CHEEP, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_SHELLFISH, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_CRAB, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_GOAT, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_POT, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_FLYSHYGUY, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_BARREL, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_TIKITAK, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_FROGOON, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_CAR, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_BOX, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_PENGUIN, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_ICICLE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_THWOMP, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_FAKEBUSH, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_FAKEGOOMBA, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_PIRANHAPLANT, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_CHAINCHOMP, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_FISHBONE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_ANCHOR, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_MUSICNOTE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_LEAVES, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_WIGGLER, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_KILLER, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_ROCKYWRENCH, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_SNOWMAN, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_SNOWBALL, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_IRONBALL, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_TABLE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_CACTUS, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_TRAIN, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::HIT_BEE, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::GLIDE_TIME, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::ACCU_KOURAG, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_KOURAR, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_BANANA, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_KOURAB, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_THUNDER, PointsModeHandler::ActionInfo(5)},
        {PointsModeHandler::PointAction::ACCU_FAKEBOX, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_BOMB, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_FLOWER, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_TAIL, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_GESSO, PointsModeHandler::ActionInfo(10)},
        {PointsModeHandler::PointAction::ACCU_STAR, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_MEGAMUSH, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::ACCU_BULLET, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::BULLET_OVERTAKE, PointsModeHandler::ActionInfo(25)},
        {PointsModeHandler::PointAction::PAYBACK, PointsModeHandler::ActionInfo(50)},
        {PointsModeHandler::PointAction::SMASH, PointsModeHandler::ActionInfo(50)},
        {PointsModeHandler::PointAction::ITEM_DESTROYED, PointsModeHandler::ActionInfo(5)},
        {PointsModeHandler::PointAction::ITEM_DEFLECTED, PointsModeHandler::ActionInfo(5)},
        {PointsModeHandler::PointAction::BLUE_DODGED, PointsModeHandler::ActionInfo(100)},
        {PointsModeHandler::PointAction::NEXT_LAP, PointsModeHandler::ActionInfo(0, PointsModeHandler::ActionFlags::FINISH_CROSS | PointsModeHandler::ActionFlags::IGNORE_POS_MULTIPLIER)},
        {PointsModeHandler::PointAction::NEXT_SECTION, PointsModeHandler::ActionInfo(0, PointsModeHandler::ActionFlags::FINISH_CROSS | PointsModeHandler::ActionFlags::IGNORE_POS_MULTIPLIER)},
        {PointsModeHandler::PointAction::RACE_FINISH_FRENZY, PointsModeHandler::ActionInfo(100)},
        {PointsModeHandler::PointAction::RACE_FINISH, PointsModeHandler::ActionInfo(0, PointsModeHandler::ActionFlags::RACE_FINISH | PointsModeHandler::ActionFlags::RESET_COMBO)},
    };
    const std::unordered_map<u16, std::pair<u32, PointsModeHandler::PointAction>> PointsModeHandler::gobjCommonHitInfo = {
        {0x12E, {0x18C, PointAction::HIT_PIPE}}, // Pipe
        {0xC9, {0x201, PointAction::HIT_GOOMBA}}, // Goomba
        {0xD8, {0x201, PointAction::HIT_GOOMBA}}, // Blue Goomba
        {0xCA, {0x20E, PointAction::HIT_SWOOP}}, // Swoop
        {0x15C, {0x79, PointAction::HIT_PYLON}}, // Red Traffic Cone
        {0x15D, {0x79, PointAction::HIT_PYLON}}, // Blue Traffic Cone
        {0x15E, {0x79, PointAction::HIT_PYLON}}, // Yellow Traffic Cone
        {0x66, {0x1F4, PointAction::HIT_BOULDER}}, // Boulder
        {0x79, {0x1F4, PointAction::HIT_BOULDER}}, // Boulder
        {0xD5, {0x19C, PointAction::HIT_SHELLFISH}}, // Shellfish
        {0xE0, {0x1FC, PointAction::HIT_GOAT}}, // Goat
        {0xDA, {0x24D, PointAction::HIT_POT}}, // Pot
        {0xDB, {0x24D, PointAction::HIT_POT}}, // Moving pot
        {0xD9, {0x18E, PointAction::HIT_FLYSHYGUY}}, // Fly shy guy
        {0x17D, {0x24D, PointAction::HIT_BARREL}}, // DK Barrel
        {0x77, {0x214, PointAction::HIT_CAR}}, // Moving Car
        {0x7, {0x24D, PointAction::HIT_BOX}}, // Box
        {0x19F, {0x204, PointAction::HIT_CAR}}, // Static Car
        {0xCF, {0x2BC, PointAction::HIT_PENGUIN}}, // Penguin
        {0x17A, {0x198, PointAction::HIT_ICICLE}}, // Icicle
        {0xCB, {0x1BC, PointAction::HIT_THWOMP}}, // Thwomp
        {0xCE, {0x22C, PointAction::HIT_THWOMP}}, // Star Thwomp
        {0x18D, {0x1F4, PointAction::HIT_FAKEGOOMBA}}, // Fake Goomba
        {0x18E, {0x1F4, PointAction::HIT_FAKEBUSH}}, // Fake Bush
        {0x7C, {0x284, PointAction::HIT_CHAINCHOMP}}, // Fake Goomba
        {0xDF, {0x18E, PointAction::HIT_FISHBONE}}, // Fish Bone
        {0xD, {0x24D, PointAction::HIT_BARREL}}, // DK Barrel
        {0xDD, {0x261, PointAction::HIT_MUSICNOTE}}, // Music Note
        {0x6E, {0x265, PointAction::HIT_CAR}}, // Moving Car
        {0x6, {0x24C, PointAction::HIT_LEAVES}}, // Leaves
        {0xD2, {0x211, PointAction::HIT_KILLER}}, // Bullet bill
        {0xD3, {0x18E, PointAction::HIT_ROCKYWRENCH}}, // Rocky Wrench
        {0x159, {0x1F4, PointAction::HIT_SNOWMAN}}, // Snowman
        {0x6D, {0x1F4, PointAction::HIT_SNOWBALL}}, // Snowball
        {0x6B, {0x260, PointAction::HIT_IRONBALL}}, // Pin ball
        {0x71, {0x194, PointAction::HIT_TABLE}}, // Table
        {0x13D, {0x190, PointAction::HIT_CACTUS}}, // Cactus
        {0x68, {0x258, PointAction::HIT_TRAIN}}, // Train
    };
    const std::unordered_map<u16, std::pair<u32, PointsModeHandler::PointAction>> PointsModeHandler::gobjObjectCrashHitInfo = {
        {0xD1, {0x188, PointAction::HIT_CHEEP}}, // Cheep Cheep
        {0xCD, {0x188, PointAction::HIT_CRAB}}, // Crab
        {0xD6, {0x188, PointAction::HIT_TIKITAK}}, // Tiki Goon
        {0xD7, {0x1F4, PointAction::HIT_FROGOON}}, // Frogoon
        {0x1B, {0x188, PointAction::HIT_ANCHOR}}, // Anchor
        {0xDE, {0x188, PointAction::HIT_BEE}}, // Bee
    };
    ModeManagerData* PointsModeHandler::modeManagerData = nullptr;
    CRaceInfo* PointsModeHandler::cRaceInfo = nullptr;
    u8* PointsModeHandler::pointBcwavs[6] = {0};
    std::queue<std::pair<PointsModeHandler::PointAction, float>> PointsModeHandler::pendingActions;
    float PointsModeHandler::multiplyFinalBonus = 0;
    PointsModeDisplayController* PointsModeHandler::pointsDisplay = nullptr;
    u32 PointsModeHandler::visualComboAmount = 0;
    MarioKartTimer PointsModeHandler::timer = 0;
    MarioKartTimer PointsModeHandler::lastPoppedActionTimer = 0;
    MarioKartTimer PointsModeHandler::lastBoostPadTime = 0;
    MarioKartTimer PointsModeHandler::lastStarRingTime = 0;
    MarioKartTimer PointsModeHandler::lastGliderTime = 0;
    MarioKartTimer PointsModeHandler::lastGoldenMushTime = 0;
    MarioKartTimer PointsModeHandler::comboTimer = 0;
    MarioKartTimer PointsModeHandler::nonAccComboTimer = 0;
    u32 PointsModeHandler::comboAmount = 0;
    u32 PointsModeHandler::finishCrossPoints = 0;
    PointsModeDisplayController::CongratState PointsModeHandler::congratKind = PointsModeDisplayController::CongratState::NONE;
    u32 PointsModeHandler::myPoints = 0;
    PointsModeHandler::ItemSlotInfo PointsModeHandler::itemSlotInfos[MAX_PLAYER_AMOUNT]{};
    std::optional<std::pair<const std::u16string*, int>> PointsModeHandler::visualReason = {};
    u32 PointsModeHandler::visualPointsAmount = 0;
    PointsModeDisplayController::CongratState PointsModeHandler::visualCongratKind = PointsModeDisplayController::CongratState::NONE;
    u32 PointsModeHandler::lastPlayerLap = 1;
    int PointsModeHandler::lastHitByPlayer = -1;
    bool PointsModeHandler::raceStarted = false;
    bool PointsModeHandler::raceFinished = false;
    bool PointsModeHandler::slipStreamPerformed = false;
    bool PointsModeHandler::kartMovementPenalize = false;
    bool PointsModeHandler::kartMovementPenalizeLoop = false;
    bool PointsModeHandler::visualKartMovementPenalize = false;
    MarioKartTimer PointsModeHandler::masterWasInLoop = 0;
    int PointsModeHandler::bulletPosition = -1;
    float PointsModeHandler::prevMaxMasterRaceProgress = 0.f, PointsModeHandler::masterRaceProgress = 0.f, PointsModeHandler::masterMaxRaceProgress = 0.f;
    MarioKartTimer PointsModeHandler::masterNotAdvancingTimer = 0;
    u32 PointsModeHandler::selectedCourse = 0;
    bool PointsModeHandler::isPlayingWeeklyChallenge = false;
    minibson::document PointsModeHandler::weeklyConfig;
    std::pair<std::vector<PointsModeHandler::RankInfo>, std::vector<PointsModeHandler::RankInfo>> PointsModeHandler::weeklyLeaderBoard;
    u64 PointsModeHandler::weeklyConfigFetchTime;
    minibson::document PointsModeHandler::SaveData::save;

    void PointsModeHandler::InitializeText()
    {
        Language::MsbtHandler::SetString(CustomTextEntries::points, NAME("score_attack"));
        Language::MsbtHandler::SetString(CustomTextEntries::pointsDesc, NOTE("score_attack"));

        Language::MsbtHandler::SetString(CustomTextEntries::customCupStart + POINTSRANDOMCUPID, NAME("sc_at_spc_cup"));
        Language::MsbtHandler::SetString(CustomTextEntries::customCupStart + POINTSWEEKLYCHALLENGECUPID, NOTE("sc_at_spc_cup"));

        for (auto it = pointActionInfo.begin(); it != pointActionInfo.end(); ++it) {
            std::string key = Utils::Format("pts_action_%d", (int)it->first);
            std::string value = Language::GetName(key);
            Language::RemoveEntry(key);
            Utils::ConvertUTF8ToUTF16(it->second.name, value);
        }
        
        PointsModeDisplayController::InitializeText();
    }

    void PointsModeHandler::InitializePerTrackText(ExtraResource::SARC *sarc)
    {
        for (auto it = pointActionInfo.begin(); it != pointActionInfo.end(); ++it) {
            it->second.per_track_name.clear();
        }
        
        u8* fileData = sarc->GetFile("points.txt", nullptr);
        if (!fileData) return;
        TextFileParser parser;
        parser.ParseLines((char*)fileData);

        for (auto it = pointActionInfo.begin(); it != pointActionInfo.end(); ++it) {
            std::string key = Utils::Format("pts_action_%d_%s", (int)it->first, Language::GetCurrLangID());
            std::string value = parser.getEntry(key);
            if (value.empty()) {
                key = Utils::Format("pts_action_%d_%s", (int)it->first, "ENG");
                value = parser.getEntry(key);
                if (value.empty()) continue;
            }
            Utils::ConvertUTF8ToUTF16(it->second.per_track_name, value);
        }
    }

    void PointsModeHandler::OnRaceDirectorCreateBeforeStructure()
    {
        if (!isPointsMode) return;
        while (!pendingActions.empty()) pendingActions.pop();
        modeManagerData = MarioKartFramework::getModeManagerData();
        cRaceInfo = MarioKartFramework::getRaceInfo(true);
        comesFromRace = true;
        timer = 0;
        lastPoppedActionTimer = 0;
        lastBoostPadTime = 0;
        lastStarRingTime = 0;
        lastGliderTime = 0;
        lastGoldenMushTime = 0;
        comboTimer = 0;
        nonAccComboTimer = 0;
        comboAmount = 0;
        finishCrossPoints = 0;
        myPoints = 0;
        visualPointsAmount = 0;
        visualComboAmount = 0;
        lastPlayerLap = 1;
        lastHitByPlayer = -1;
        raceStarted = false;
        raceFinished = false;
        slipStreamPerformed = false;
        kartMovementPenalize = false;
        kartMovementPenalizeLoop = false;
        visualKartMovementPenalize = false;
        masterWasInLoop = 0;
        masterRaceProgress = 0;
        prevMaxMasterRaceProgress = 0;
        masterMaxRaceProgress = 0;
        masterNotAdvancingTimer = 0;
        bulletPosition = -1;
        congratKind = PointsModeDisplayController::CongratState::NONE;
        visualCongratKind = PointsModeDisplayController::CongratState::NONE;
        for (int i = 0; i < MAX_PLAYER_AMOUNT; i++) itemSlotInfos[i].Reset(true);
        CalculateBonuses();
    }

    void PointsModeHandler::OnRaceStart(bool startCountdown)
    {
        if (!isPointsMode) return;
        if (startCountdown) {
            pointsDisplay->OnRaceStart();
        } else {
            raceStarted = true;
        }
    }

    void PointsModeHandler::DoPointAction(PointAction action)
    {
        if (!isPointsMode || raceFinished || (kartMovementPenalize && action != PointAction::RACE_FINISH)) return;
        u32 position = modeManagerData->driverPositions[MarioKartFramework::masterPlayerID] - 1;
        u32 total = cRaceInfo->playerAmount - 1;
        if (total == 0) total = 1;
        float multiplier = 1.5f + (1.f - ((float)position / total)) * 1.5f;
        if (position == total) multiplier = 0.25f;
        pendingActions.push({action, multiplier});
    }

    void PointsModeHandler::OnVisualElementCalc()
    {
        
        if (visualReason.has_value()) {
            pointsDisplay->SetReason(*visualReason->first, visualReason->second);
            visualReason.reset();
        }

        if (visualComboAmount != comboAmount) {
            visualComboAmount = comboAmount;
            pointsDisplay->SetCombo(comboAmount > 1 ? comboAmount : 0);
        }

        if (visualPointsAmount != myPoints) {
            visualPointsAmount = myPoints;
            pointsDisplay->GotoNumber(myPoints);
        }

        if (visualCongratKind != congratKind) {
            visualCongratKind = congratKind;
            pointsDisplay->SetCongrat(congratKind);
        }

        if (visualKartMovementPenalize != kartMovementPenalize) {
            visualKartMovementPenalize = kartMovementPenalize;
            pointsDisplay->SetNumberDisabled(kartMovementPenalize);
        }
    }

    void PointsModeHandler::OnVehicleCalc(u32 *vehicle)
    {
        if (!isPointsMode) return;
        int playerID = vehicle[0x21];
        
        if (playerID == MarioKartFramework::masterPlayerID) {
            timer++;
            if (KartFlags::GetFromVehicle((u32)vehicle).isWingOpened && (((timer - lastGliderTime) > MarioKartTimer(0, 1, 500)))) {
                lastGliderTime = timer;
                DoPointAction(PointAction::GLIDE_TIME);
            }
            KartLapInfo* info = MarioKartFramework::getKartLapInfo(playerID);
            if (info->currentLap > lastPlayerLap) {
                lastPlayerLap = info->currentLap;
                u32 totalLaps = MarioKartFramework::getRaceInfo(false)->lapAmount;
                bool isSection = totalLaps == 1;
                if (isSection) totalLaps = 3;
                u32 prizesAmount = totalLaps - 1;
                constexpr u32 pointsByPosition[8] = {200, 160, 120, 80, 60, 40, 20, 0};
                u32 position = modeManagerData->driverPositions[playerID] - 1;
                finishCrossPoints = pointsByPosition[position] / prizesAmount;
                if (finishCrossPoints) {
                    DoPointAction(isSection ? PointAction::NEXT_SECTION : PointAction::NEXT_LAP);
                }
            }
            u32 slipStreamTimer = vehicle[0xFD8/4];
            if (slipStreamTimer) {
                if (!slipStreamPerformed) {
                    slipStreamPerformed = true;
                    DoPointAction(PointAction::SLIPSTREAM);
                }
            } else {
                slipStreamPerformed = false;
            }
            {
                KartButtonData button = KartButtonData::GetFromVehicle((u32)vehicle);
                KartFlags& flags = KartFlags::GetFromVehicle((u32)vehicle);
                kartMovementPenalize = false;
                if (!button.accel || button.brake) {
                    float speedFactor = *(float*)((u32)vehicle + 0x330 + 0xC00);
                    if (speedFactor < 0.25) {
                        kartMovementPenalize = true;
                    }
                }
                if (raceStarted && !raceFinished && masterRaceProgress <= prevMaxMasterRaceProgress) {
                    if (masterNotAdvancingTimer > MarioKartTimer(0, 1, 0)) {
                        kartMovementPenalize |= masterWasInLoop.GetFrames() ? kartMovementPenalizeLoop : true;
                    } else {
                        masterNotAdvancingTimer++;
                    }
                } else {
                    masterNotAdvancingTimer.DecrementIfNonZero();
                }
                bool isInLoop = MarioKartFramework::vehicleIsInLoopKCL((u32)vehicle) == 2;
                if (isInLoop && !masterWasInLoop.GetFrames()) {
                    kartMovementPenalizeLoop = kartMovementPenalize;
                }
                if (isInLoop) masterWasInLoop = MarioKartTimer(0, 1, 0);
                masterWasInLoop.DecrementIfNonZero();
            }
            bool isBullet = vehicle[(0xC30) / 4] & 0x400000;
            if (isBullet) {
                u32 position = modeManagerData->driverPositions[playerID];
                if (bulletPosition == -1) {
                    bulletPosition = position;
                } else if (position < bulletPosition) {
                    bulletPosition = position;
                    DoPointAction(PointAction::BULLET_OVERTAKE);
                }
            } else if (bulletPosition != -1) {
                bulletPosition = -1;
            }
        }
        if (itemSlotInfos[playerID].frenzyFrames) {
            u32 remaining = --itemSlotInfos[playerID].frenzyFrames;
            ItemHandler::ItemSlotStatus status = ItemHandler::GetItemSlotStatus(playerID);
            if (remaining) {
                if (status.mode == ItemHandler::ItemSlotStatus::MODE_EMPTY) {   
                    MarioKartFramework::nextForcedItem = itemSlotInfos[playerID].frenzySlot;
                    itemSlotInfos[playerID].isStarting = true;
                    MarioKartFramework::startItemSlot(playerID, 0);
                    itemSlotInfos[playerID].isStarting = false;
                }
            } else {
                ItemHandler::ClearItem(playerID);
            }

        } else if (itemSlotInfos[playerID].remainingStarts || itemSlotInfos[playerID].nextFrenzy) {
            ItemHandler::ItemSlotStatus status = ItemHandler::GetItemSlotStatus(playerID);
            if (itemSlotInfos[playerID].remainingStarts && status.mode == ItemHandler::ItemSlotStatus::MODE_EMPTY) {
                itemSlotInfos[playerID].remainingStarts--;
                if (playerID == MarioKartFramework::masterPlayerID) {
                    if (itemSlotInfos[playerID].remainingStarts == 1) {
                        ExtendedItemBoxController::SetNextAmountMode(ExtendedItemBoxController::AmountMode::AMOUNT_2X);
                    } else if (itemSlotInfos[playerID].remainingStarts == 0) {
                        ExtendedItemBoxController::SetNextAmountMode(ExtendedItemBoxController::AmountMode::AMOUNT_NONE);
                    }
                }                
                itemSlotInfos[playerID].isStarting = true;
                if (itemSlotInfos[playerID].x2x3ItemSlots[0] == itemSlotInfos[playerID].x2x3ItemSlots[1] &&
                    itemSlotInfos[playerID].x2x3ItemSlots[0] != EItemSlot::ITEM_SIZE) {
                        MarioKartFramework::nextForcedItemBlockedFlags = 1 << (u32)(itemSlotInfos[playerID].x2x3ItemSlots[0]);
                    }
                MarioKartFramework::startItemSlot(playerID, itemSlotInfos[playerID].lastItemBoxID);
                itemSlotInfos[playerID].isStarting = false;
            } else if (status.mode == ItemHandler::ItemSlotStatus::MODE_DECIDED) {
                if (itemSlotInfos[playerID].nextFrenzy) {
                    if (status.item == EItemSlot::ITEM_KILLER || status.item == EItemSlot::ITEM_KINOKOP ||
                        status.item == EItemSlot::ITEM_KONOHA || status.item == EItemSlot::ITEM_FLOWER ||
                        status.item == EItemSlot::ITEM_TEST4) 
                    {
                        // These items don't make sense to be frenzied, better to delay the frenzy to the next item instead
                        itemSlotInfos[playerID].nextFrenzy = false;
                        itemSlotInfos[playerID].forceNextFrenzy = true;
                    } else {
                        itemSlotInfos[playerID].nextFrenzy = false;
                        itemSlotInfos[playerID].totalFrenzies++;
                        u32 itemDirector = MarioKartFramework::getItemDirector();
                        SeadArrayPtr<u32*>& kartItems = *(SeadArrayPtr<u32*>*)(itemDirector + 0xC0);
                        ItemHandler::UseItem(ItemHandler::GetDirectorFromSlot(itemDirector, EItemSlot::ITEM_STAR), EItemSlot::ITEM_STAR, kartItems[playerID]);
                        itemSlotInfos[playerID].frenzyFrames = 450;
                        itemSlotInfos[playerID].frenzySlot = status.item;
                        if (playerID == MarioKartFramework::masterPlayerID) {
                            ExtendedItemBoxController::SetNextAmountMode(ExtendedItemBoxController::AmountMode::AMOUNT_FRENZY);
                        }
                        itemSlotInfos[playerID].remainingStarts = 0;
                    }
                } else if (itemSlotInfos[playerID].remainingStarts && itemSlotInfos[playerID].x2x3ItemSlots[itemSlotInfos[playerID].remainingStarts - 1] == EItemSlot::ITEM_SIZE) {
                    itemSlotInfos[playerID].x2x3ItemSlots[itemSlotInfos[playerID].remainingStarts - 1] = status.item;
                }
            }
            
        }
        if (playerID == MarioKartFramework::masterPlayerID) {
            HandlePendingActions(vehicle);
        }
    }

    void PointsModeHandler::HandlePendingActions(u32 *vehicle)
    {
        constexpr MarioKartTimer popInterval(0, 0, 150);
        if (pendingActions.size() && ((timer - lastPoppedActionTimer) >= popInterval)) {
            lastPoppedActionTimer = timer;

            
            auto [pointAction, pointMultiplier] = pendingActions.front();
            pendingActions.pop();
            const ActionInfo& action = GetActionInfo(pointAction);
            bool resetCombo = action.flags & ActionFlags::RESET_COMBO;

            comboAmount = resetCombo ? 1 : (comboAmount + 1);
            if (resetCombo) {
                comboTimer = 0;
            } else if (action.flags & ActionFlags::NON_ACCUMULATIVE) {
                if (nonAccComboTimer == 0) {
                    nonAccComboTimer = PointsToComboTime(action.points); 
                }
            } else {
                comboTimer = std::max(comboTimer, PointsToComboTime(action.points));
            }

            bool isFinishCross = action.flags & ActionFlags::FINISH_CROSS;
            bool isFinishRace = action.flags & ActionFlags::RACE_FINISH;
            bool ignorePosMultiplier = action.flags & ActionFlags::IGNORE_POS_MULTIPLIER;
            u32 basePoints;
            if (ignorePosMultiplier) {
                pointMultiplier = 3.f;
            }
            if (isFinishCross) {
                comboTimer = comboTimer + MarioKartTimer(0, 1, 0);
                basePoints = finishCrossPoints;
                finishCrossPoints = 0;
            } else if (isFinishRace) {
                basePoints = finishCrossPoints;
                finishCrossPoints = 0;
                pointMultiplier = 1.f;
            } else {
                basePoints = action.points;
            }

            u32 addPoints = ((basePoints + (std::min(16ul, comboAmount - 1) * 3)) * pointMultiplier) * multiplyFinalBonus;

            myPoints += addPoints;

            visualReason = std::make_pair<const std::u16string*, int>(&action.GetName(), addPoints);

            congratKind = comboAmount > 9 ? PointsModeDisplayController::CongratState::FANTASTIC : comboToCongrat[comboAmount];
            if (Snd::PlayMenu(Snd::SoundID::ITEM_DECIDE) != 0) {
                u32 sID = (u32)congratKind;
                if (sID == 4 && comboAmount > 9) sID = 5;
                CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::itemDecideSE, pointBcwavs[sID], 1);
            }
        }

        bool comboZero = false;
        if (comboTimer > 0) {
            comboTimer--;
            comboZero = comboTimer == 0 && nonAccComboTimer == 0;
        }
        if (nonAccComboTimer > 0) {
            nonAccComboTimer--;
            comboZero = comboTimer == 0 && nonAccComboTimer == 0;
            
        }
        if (comboZero) {
            comboAmount = 0;
            congratKind = PointsModeDisplayController::CongratState::NONE;
        }
    }

    void PointsModeHandler::DoItemHitKart(u32 *srcVehicle, u32 *dstVehicle, EItemType iType)
    {
        if (!isPointsMode) return;
        int srcPlayerID = srcVehicle[0x21];
        int dstPlayerID = dstVehicle[0x21];
        if (dstPlayerID == MarioKartFramework::masterPlayerID && srcPlayerID != dstPlayerID) {
            lastHitByPlayer = srcPlayerID;
        }
        if (srcPlayerID == MarioKartFramework::masterPlayerID && srcPlayerID != dstPlayerID) {
            if (lastHitByPlayer == dstPlayerID) {
                DoPointAction(PointAction::PAYBACK);
                lastHitByPlayer = -1;
            } else
            switch (iType)
            {
            case EItemType::ITYPE_KOURAG:
                DoPointAction(PointAction::ACCU_KOURAG);
                break;
            case EItemType::ITYPE_KOURAR:
                DoPointAction(PointAction::ACCU_KOURAR);
                break;
            case EItemType::ITYPE_BANANA:
                DoPointAction(PointAction::ACCU_BANANA);
                break;
            case EItemType::ITYPE_STAR:
                DoPointAction(PointAction::ACCU_STAR);
                break;
            case EItemType::ITYPE_KOURAB:
                DoPointAction(PointAction::ACCU_KOURAB);
                break;
            case EItemType::ITYPE_THUNDER:
                DoPointAction(PointAction::ACCU_THUNDER);
                break;
            case EItemType::ITYPE_FAKEBOX:
                DoPointAction(PointAction::ACCU_FAKEBOX);
                break;
            case EItemType::ITYPE_BOMB:
                DoPointAction(PointAction::ACCU_BOMB);
                break;
            case EItemType::ITYPE_BIGKINOKO:
                DoPointAction(PointAction::ACCU_MEGAMUSH);
                break;
            case EItemType::ITYPE_KILLER:
                DoPointAction(PointAction::ACCU_BULLET);
                break;
            case EItemType::ITYPE_FLOWER:
                DoPointAction(PointAction::ACCU_FLOWER);
                break;
            case EItemType::ITYPE_TAIL:
                DoPointAction(PointAction::ACCU_TAIL);
                break;
            default:
                break;
            }
        }
    }

    void PointsModeHandler::OnGessoAfterInitUse(u32 *itemObjGesso)
    {
        if (!isPointsMode) return;
        int playerID = ((u32***)itemObjGesso)[0x158/4][0][0x21];
        if (playerID != MarioKartFramework::masterPlayerID) return;

        FixedArrayPtr<bool>& hitPlayers = *(FixedArrayPtr<bool>*)(((u32)itemObjGesso) + 0x23C);
        if (!hitPlayers[playerID]) {
            for (int i = 0; i < hitPlayers.size; i++) {
                if (hitPlayers[i])
                    DoPointAction(PointAction::ACCU_GESSO);
            }
        }
    }

    void PointsModeHandler::OnClearItem(int playerID)
    {
        if (!isPointsMode) return;
        itemSlotInfos[playerID].Reset(false);
    }

    void PointsModeHandler::ForceNextFrenzy(int playerID)
    {
        itemSlotInfos[playerID].forceNextFrenzy = true;
    }

    void PointsModeHandler::OnKartDash(u32 *vehicleMove, EDashType dash)
    {
        if (!isPointsMode) return;
        int playerID = vehicleMove[0x21];
        if (playerID != MarioKartFramework::masterPlayerID) return;

        if (dash & EDashType::MUSHROOM) {
            ItemHandler::ItemSlotStatus status = ItemHandler::GetItemSlotStatus(MarioKartFramework::masterPlayerID);
            if (status.mode == ItemHandler::ItemSlotStatus::MODE_DECIDED && status.item == EItemSlot::ITEM_KINOKOP && status.goldenTimer >= 0) {
                if (timer - lastGoldenMushTime > MarioKartTimer(0, 1, 0)) {
                    lastGoldenMushTime = timer;
                    DoPointAction(PointAction::GOLD_MUSHROOM);
                }
            } else {
                DoPointAction(PointAction::MUSHROOM);
            }
        }
        if (dash & EDashType::BOOST_PAD) {
            float vehicleSpeed = *(float*)((u32)vehicleMove + 0x32C + 0xC00);
            if (vehicleSpeed >= 4.5f && timer - lastBoostPadTime > MarioKartTimer(0, 0, 500)) {
                lastBoostPadTime = timer;
                DoPointAction(PointAction::BOOST_PAD);
            }
        }
        if (dash & EDashType::MINITURBO) DoPointAction(PointAction::MINITURBO);
        if (dash & EDashType::SUPERMINITURBO) DoPointAction(PointAction::SUPERMINITURBO);
        if ((dash & EDashType::START_VERYSMALL) || (dash & EDashType::START_SMALL)) DoPointAction(PointAction::START_SMALL);
        if ((dash & EDashType::START_MEDIUM) || (dash & EDashType::START_BIG)) DoPointAction(PointAction::START_MEDIUM);
        if (dash & EDashType::START_PERFECT) DoPointAction(PointAction::START_PERFECT);

        if (dash & EDashType::COIN_GRAB) DoPointAction(PointAction::COIN);
        if (dash & EDashType::STAR_RING) {
            if (timer - lastStarRingTime > MarioKartTimer(0, 0, 300)) {
                lastStarRingTime = timer;
                DoPointAction(PointAction::STAR_RING);
            }
        }
    }

    u32 PointsModeHandler::OnKartItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 reactObject, u32 objCallFuncPtr, int mode)
    {
        if (!isPointsMode)
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
        
        u32 ret = 0;

        // Common
        auto it = gobjCommonHitInfo.find(objID);
        if (it != gobjCommonHitInfo.end() && eObjectReactType == EObjectReactType::OBJECTREACTTYPE_DESTRUCTVE) {
            bool wasHit = ((u8*)object)[it->second.first] != 0;
            ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
            bool isHit = ((u8*)object)[it->second.first] != 0;
            if (!wasHit && isHit) {
                DoPointAction(it->second.second);
            }
        }

        // Object crash
        else if (((it = gobjObjectCrashHitInfo.find(objID)) != gobjObjectCrashHitInfo.end()) && eObjectReactType == EObjectReactType::OBJECTREACTTYPE_DESTRUCTVE) {
            bool wasHit = ((u8*)object)[it->second.first + 0x10] != 0 || ((u32*)object)[(it->second.first + 0x40) / 4] != 0;
            ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
            bool isHit = ((u8*)object)[it->second.first] != 0;
            if (!wasHit && isHit) {
                DoPointAction(it->second.second);
            }
        }

        // Piranha plant
        else if (objID == 0xDC && eObjectReactType == EObjectReactType::OBJECTREACTTYPE_DESTRUCTVE) {
			u8* state2 = ((u8*)object) + 0x1A5;
			u8 prevState2 = *state2;
			u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
			if (prevState2 != *state2 && *state2 == 5) {
				PointsModeHandler::DoPointAction(PointsModeHandler::PointAction::HIT_PIRANHAPLANT);
			}
			return ret;
		}

        // Music piranha plant
        else if (objID == 0xE1 && eObjectReactType == EObjectReactType::OBJECTREACTTYPE_DESTRUCTVE) {
			u8 prevState2 = *(((u8*)object) + 0x1A5);
            bool wasHit = ((u8*)object)[0x234] != 0;
			u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
            bool isHit = ((u8*)object)[0x234] != 0;
			if (!wasHit && isHit && prevState2 != 5) {
				PointsModeHandler::DoPointAction(PointsModeHandler::PointAction::HIT_PIRANHAPLANT);
			}
			return ret;
		}

        // Wiggler
        else if (objID == 0x72 && eObjectReactType == EObjectReactType::OBJECTREACTTYPE_DESTRUCTVE) {
            u32 objWhole = object;
            if (((u32*)object)[0x32C/4] >= 0x14000000 && ((u32*)object)[0x32C/4] < 0x1A000000) {
                // Wiggler part
                objWhole = ((u32*)object)[0x32C/4];
            }
            bool wasHit = ((u8*)objWhole)[0x4414] != 0 || ((u8*)objWhole)[0x4415] != 0;
            u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
            bool isHit = ((u8*)objWhole)[0x4414] != 0;
            if (!wasHit && isHit) {
				PointsModeHandler::DoPointAction(PointsModeHandler::PointAction::HIT_WIGGLER);
			}
			return ret;
		}

        // Unhandled
        else {
            ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, reactObject);
        }
        return ret;
    }

    void PointsModeHandler::OnItemDirectorDoReaction(u32 *director, u32 *itemObj1, u32 *itemObj2, EItemReact *react1, EItemReact *react2, Vector3 *pos1, Vector3 *pos2)
    {
        if (!isPointsMode) return;
        u32* vehicle1 = (u32*)(itemObj1[0x158/4] ? (((u32**)itemObj1)[0x158/4][0] ? ((u32**)itemObj1)[0x158/4][0] : 0) : 0);
        u32* vehicle2 = (u32*)(itemObj2[0x158/4] ? (((u32**)itemObj2)[0x158/4][0] ? ((u32**)itemObj2)[0x158/4][0] : 0) : 0);
        int playerID1 = vehicle1 ? vehicle1[0x21] : -1;
        int playerID2 = vehicle2 ? vehicle2[0x21] : -1;

        // When a green shell is deflected by the tail, its playerID becomes the same as the tail user
        if (playerID1 == playerID2 && *react1 != EItemReact::REACT_TAILDEFLECT && *react2 != EItemReact::REACT_TAILDEFLECT) return;
        
        EItemReact res = EItemReact::REACT_NONE;
        if (playerID1 == MarioKartFramework::masterPlayerID) {
            res = *react2;
            if ((EItemType)((u8*)itemObj2)[0x156] == EItemType::ITYPE_BOMB && itemObj2[0x168/4] >= 0)
                return;
        } else if (playerID2 == MarioKartFramework::masterPlayerID) {
            res = *react1;
            if ((EItemType)((u8*)itemObj1)[0x156] == EItemType::ITYPE_BOMB && itemObj1[0x168/4] >= 0)
                return;
        } else {
            return;
        }

        switch (res)
        {
        case EItemReact::REACT_DESTROY:
        case EItemReact::REACT_TAILDESTROY:
            DoPointAction(PointAction::ITEM_DESTROYED);
            break;
        case EItemReact::REACT_TAILDEFLECT:
            DoPointAction(PointAction::ITEM_DEFLECTED);
            break;
        default:
            break;
        }
    }

    void PointsModeHandler::OnKartDodgedBlueShell(int playerID)
    {
        if (!isPointsMode || playerID != MarioKartFramework::masterPlayerID) return;
        
        DoPointAction(PointAction::BLUE_DODGED);
    }

    void PointsModeHandler::OnKartPress(u32 *srcVehicle, u32 *dstVehicle)
    {
        if (!isPointsMode) return;
        int srcPlayerID = srcVehicle[0x21];
        if (srcPlayerID == MarioKartFramework::masterPlayerID) {
            DoPointAction(PointAction::SMASH);
        }
    }

    void PointsModeHandler::OnItemDirectorStartSlot(int playerID, u32 boxID)
    {
        if (!isPointsMode || itemSlotInfos[playerID].isStarting || ItemHandler::GetItemSlotStatus(playerID).mode != ItemHandler::ItemSlotStatus::MODE_EMPTY) return;
        itemSlotInfos[playerID].lastItemBoxID = boxID;
        itemSlotInfos[playerID].remainingStarts = 2;
        itemSlotInfos[playerID].x2x3ItemSlots[0] = itemSlotInfos[playerID].x2x3ItemSlots[1] = EItemSlot::ITEM_SIZE;
        auto getTotalFrenzies = []() {
            int tot = 0;
            u32 totPlayer = cRaceInfo->playerAmount;
            for (int i = 0; i < totPlayer; i++) {
                u32 vehicle = MarioKartFramework::getVehicle(i);
                if (vehicle && ((u32*)vehicle)[0xFF4/4]) tot++;
            }
            return tot;
        };
        u32 frenzy_probability;
        if (playerID == MarioKartFramework::masterPlayerID) {
            frenzy_probability = isPlayingWeeklyChallenge ? 2 : 1; // Weekly challenge 30%, otherwise 20%
        } else {
            frenzy_probability = 0; // 10% for CPUs
        }
        if (itemSlotInfos[playerID].forceNextFrenzy || (itemSlotInfos[playerID].totalFrenzies < 3 && ItemHandler::GetItemRandom().Get(0, 10) <= frenzy_probability && getTotalFrenzies() < 2)) {
            itemSlotInfos[playerID].forceNextFrenzy = false;
            itemSlotInfos[playerID].nextFrenzy = true;
        }
        if (playerID == MarioKartFramework::masterPlayerID) {
            ExtendedItemBoxController::SetNextAmountMode(ExtendedItemBoxController::AmountMode::AMOUNT_3X);
        }
    }

    u32 PointsModeHandler::OnCalcItemSlotTimer(u32 timer)
    {
        if (!isPointsMode) return timer + 1;
        if (timer >= 45) return 180;
        return timer + 1;
    }

    bool PointsModeHandler::OnVehicleDecideSurfaceTrickable(u32 *vehicle, bool isTrickable)
    {
        int playerID = vehicle[0x21];
        if (itemSlotInfos[playerID].frenzyFrames) return true;
        return isTrickable;
    }

    void PointsModeHandler::OnLapRankCheckerCalc(u32 *laprankchecker)
    {
        if (!isPointsMode) return;
        FixedArrayPtr<LapRankCheckerKartInfo>& lapKartInfos = *(FixedArrayPtr<LapRankCheckerKartInfo>*)((u32)laprankchecker + 0x28);
        LapRankCheckerKartInfo& masterInfo = lapKartInfos[MarioKartFramework::masterPlayerID];
        masterRaceProgress = masterInfo.currRaceProgress;
        prevMaxMasterRaceProgress = masterMaxRaceProgress;
        masterMaxRaceProgress = masterInfo.maxRaceProgress;
    }

    void PointsModeHandler::OnPlayerGoal(int playerID)
    {
        if (!isPointsMode || playerID != MarioKartFramework::masterPlayerID) return;
        constexpr float pointsByPositionMultiplier[8] = {1.f, 0.85f, 0.75f, 0.55f, 0.45f, 0.25f, 0.5f, 0};
        constexpr u32 finishPointsBase = 3000;
        // TODO: This may not be accurate
        u32 position = modeManagerData->driverPositions[MarioKartFramework::masterPlayerID] - 1;
        finishCrossPoints = finishPointsBase * pointsByPositionMultiplier[position];
        if (finishCrossPoints) {
            if (itemSlotInfos[playerID].frenzyFrames) {
                DoPointAction(PointAction::RACE_FINISH_FRENZY);
            }
            DoPointAction(PointAction::RACE_FINISH);
        }

        raceFinished = true;
    }

    void PointsModeHandler::SetupExtraResource()
    {
        for (int i = 0; i < 6; i++) {
            ExtraResource::SARC::FileInfo fInfo;
            pointBcwavs[i] = ExtraResource::mainSarc->GetFile(Utils::Format("RaceCommon.szs/pointsMode/point_sound_%d.bcwav", i), &fInfo);
        }
    }

    void PointsModeHandler::onPointsModeEnter()
    {
        if (isPointsMode) return;
        isPointsMode = true;
        MarioKartFramework::resultBarHideRank = true;
        MarioKartFramework::forcedResultBarAmount = 3;
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2264, 0x16, 0x1); // Return from game to GP cup
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2274, 0x12, 0x1); // single to chara
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2324, 0x4, 0x2); // chara to single

        std::string changeCourse = Language::MsbtHandler::GetString(9005);
        Language::MsbtHandler::SetString(9009, changeCourse, false); // Next Course
        Language::MsbtHandler::SetTextEnabled(9013, true);
        Language::MsbtHandler::SetTextEnabled(9014, true);

        Language::MsbtHandler::SetString(1831, NAME("sc_at_race_start"));
        ItemHandler::extraKouraB = 7;

        MarioKartFramework::BasePageSetCC(EEngineLevel::ENGINELEVEL_150CC);
        MarioKartFramework::BasePageSetMirror(false);
    }

    void PointsModeHandler::onPointsModeExit()
    {
        if (!isPointsMode) return;
        isPointsMode = false;
        comesFromRace = false;
        isPlayingWeeklyChallenge = false;
        weeklyLeaderBoard.first.clear();
        weeklyLeaderBoard.second.clear();
        MarioKartFramework::resultBarHideRank = false;
        MarioKartFramework::forcedResultBarAmount = -1;
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2264, 0xB, 0x3);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2274, 0xD, 0x1);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x2324, 0xD, 0x2);
        SequenceHandler::addFlowPatch(0x5B3E8654, 0x235C, 0x15, 0x2);

        Language::MsbtHandler::SetTextEnabled(9009, false);
        Language::MsbtHandler::SetTextEnabled(9013, false);
        Language::MsbtHandler::SetTextEnabled(9014, false);

        Language::MsbtHandler::SetTextEnabled(1831, false);
        ItemHandler::extraKouraB = 0;
    }

    bool PointsModeHandler::IsWeeklyConfigValid() {
        if (weeklyConfigFetchTime == 0 || weeklyConfig.get_numerical("uid") == 0)
            return false;
        int courseID = CourseManager::getCourseIDFromName(weeklyConfig.get("trackSzs", ""));
        if (courseID < 0)
            return false;
        u64 currTime = osGetTime() / 1000;
        u64 remaining = weeklyConfig.get_numerical("remaining");
        return currTime < (weeklyConfigFetchTime + remaining);
    }

    bool PointsModeHandler::IsWeeklyLdrBoardValid()
    {
        return !weeklyLeaderBoard.first.empty();
    }

    int PointsModeHandler::FetchLatestWeeklyData(bool config, bool leaderboard) {
        NetHandler::RequestHandler requester;

        if (config) {
            minibson::document reqDoc;
            #ifndef DISABLE_POINTS_WEEKLY
                        reqDoc.set<int>("localVer", Net::lastOnlineVersion);
            #else
                        reqDoc.set<int>("localVer", 0);
            #endif
            requester.AddRequest(NetHandler::RequestHandler::RequestType::POINTS_WEEKLY_CONFIG, reqDoc);
        }
        if (leaderboard) {
            minibson::document reqDoc;
            requester.AddRequest(NetHandler::RequestHandler::RequestType::POINTS_LEADER_BOARD, reqDoc);
        }
		
		requester.Start();
		requester.Wait();

        if (config) {
            minibson::document resDoc;
            int res = requester.GetResult(NetHandler::RequestHandler::RequestType::POINTS_WEEKLY_CONFIG, &resDoc);
            if (res == 0) {
                weeklyConfigFetchTime = osGetTime() / 1000;
                weeklyConfig = std::move(resDoc);
            } else {
                return res;
            }
            if (!IsWeeklyConfigValid()) {
                return 100;
            }
        }
        if (leaderboard) {
            minibson::document resDoc;
            int res = requester.GetResult(NetHandler::RequestHandler::RequestType::POINTS_LEADER_BOARD, &resDoc);
            if (res == 0 && weeklyConfig.get_numerical("uid") == resDoc.get_numerical("uid")) {
                weeklyLeaderBoard.first = RankInfo::FromString(resDoc.get("top", ""));
                weeklyLeaderBoard.second = RankInfo::FromString(resDoc.get("around", ""));
            } else {
                weeklyLeaderBoard.first.clear();
                weeklyLeaderBoard.second.clear();
                return res;
            }
        }        

        return 0;
    }

    int PointsModeHandler::UploadWeeklyScore(u32 myScore, std::string& messageOut)
    {
        NetHandler::RequestHandler requester;

        minibson::document reqDoc;
        #ifndef DISABLE_POINTS_WEEKLY
        reqDoc.set<int>("localVer", Net::lastOnlineVersion);
        #else
        reqDoc.set<int>("localVer", 0);
        #endif
        reqDoc.set<u64>("uid", weeklyConfig.get_numerical("uid"));
        reqDoc.set<int>("score", (int)myScore);
        requester.AddRequest(NetHandler::RequestHandler::RequestType::POINTS_WEEKLY_SCORE, reqDoc);

		requester.Start();
		requester.Wait();

        minibson::document resDoc;
        int res = requester.GetResult(NetHandler::RequestHandler::RequestType::POINTS_WEEKLY_SCORE, &resDoc);
        if (res == 0 && weeklyConfig.get_numerical("uid") == resDoc.get_numerical("uid")) {
            weeklyLeaderBoard.first = RankInfo::FromString(resDoc.get("top", ""));
            weeklyLeaderBoard.second = RankInfo::FromString(resDoc.get("around", ""));
        } else {
            if (res == (int)Net::CTWWLoginStatus::MESSAGEKICK) {
                messageOut = resDoc.get("loginMessage", "");
            } else if (res == 100) {
                messageOut = NAME("sc_wc_ended");
            }
            return res;
        }

        return 0;
    }

    std::array<bool, 4> PointsModeHandler::HasPlayerRecommendedParts(int playerID)
    {
        std::array<bool, 4> ret = {0};
        CKartInfo& playerInfo = MarioKartFramework::getRaceInfo(true)->kartInfos[playerID];
        auto recDrivers = GetRecommendedParts(0);
        auto recBodies = GetRecommendedParts(1);
        auto recTires = GetRecommendedParts(2);
        auto recWings = GetRecommendedParts(3);

        for (auto it = recDrivers.begin(); it != recDrivers.end(); it++) {
            if (playerInfo.driverID == *it) {
                ret[0] = true;
                break;
            }
        }
        for (auto it = recBodies.begin(); it != recBodies.end(); it++) {
            if (playerInfo.bodyID == *it) {
                ret[1] = true;
                break;
            }
        }
        for (auto it = recTires.begin(); it != recTires.end(); it++) {
            if (playerInfo.tireID == *it) {
                ret[2] = true;
                break;
            }
        }
        for (auto it = recWings.begin(); it != recWings.end(); it++) {
            if (playerInfo.wingID == *it) {
                ret[3] = true;
                break;
            }
        }
        return ret;
    }

    std::vector<u32> PointsModeHandler::GetRecommendedParts(int mode)
    {
        static const char* names[] = {
            "recDrivers",
            "recBodies",
            "recTires",
            "recWings",
        };

        auto elem = TextFileParser::Split(weeklyConfig.get(names[mode], ""));
        std::vector<u32> ret;
        for (auto el = elem.begin(); el != elem.end(); el++) {
            u32 id = std::strtoul(el->c_str(), NULL, 10);
            ret.push_back(id);
        }
        return ret;
    }

    std::vector<u32> PointsModeHandler::GetBadgeLimits()
    {
        auto elem = TextFileParser::Split(weeklyConfig.get("limits", ""), ":");
        std::vector<u32> ret;
        for (auto el = elem.begin(); el != elem.end(); el++) {
            u32 id = std::strtoul(el->c_str(), NULL, 10);
            ret.push_back(id);
        }
        if (ret.size() != 3) {
            ret.resize(3);
            memset(ret.data(), 0, ret.size() * sizeof(u32));
        }
        return ret;
    }

    std::pair<std::string, std::string> PointsModeHandler::GenerateWeeklyConfigPage(int page)
    {
        std::string ret;
        std::string title;
        auto secondsToDisplay = [](u32 seconds) -> std::string {
            u32 days = seconds / (60 * 60 * 24);
            seconds %= (60 * 60 * 24);

            u32 hours = seconds / (60 * 60);
            seconds %= (60 * 60);

            u32 minutes = seconds / 60;
            seconds %= 60;

            if (days) {
                return Utils::Format("%dd, %dh", days, hours);
            } else if (hours) {
                return Utils::Format("%dh, %dm", hours, minutes);
            } else {
                return Utils::Format("%dm, %ds", minutes, seconds);
            }
        };

        if (page == 0) {
            title = NOTE("sc_at_spc_cup");
            std::string trackName;
            int courseID = CourseManager::getCourseIDFromName(weeklyConfig.get("trackSzs", ""));
            CourseManager::getCourseText(trackName, courseID, true);
            auto recDrivers = GetRecommendedParts(0);
            auto recBodies = GetRecommendedParts(1);
            auto recTires = GetRecommendedParts(2);
            auto recWings = GetRecommendedParts(3);
            auto partsBonus = HasPlayerRecommendedParts(MarioKartFramework::masterPlayerID);
            u32 bonuses = 0;
            for (int i = 0; i < partsBonus.size(); i++) if (partsBonus[i]) bonuses++;
            float multiplyBonus = 1.f + (bonuses * (0.5f/4));
            CKartInfo& playerInfo = MarioKartFramework::getRaceInfo(true)->kartInfos[MarioKartFramework::masterPlayerID];
            u64 endTime = weeklyConfigFetchTime + weeklyConfig.get_numerical("remaining");
            u64 nowTime = osGetTime() / 1000;
            u32 remaining = 0;
            if (nowTime < endTime)
                remaining = static_cast<u32>(endTime - nowTime);

            ret += NAME("sc_wc_curr") + "\n";
            ret += "    " + trackName + "\n";
            ret += "    " + NAME("sc_wc_time") + " " + secondsToDisplay(remaining) + "\n\n";
            ret += NAME("sc_wc_bonus") + "\n";
            ret += (partsBonus[0] ? Color::LimeGreen : ResetColor()) << "    ";
            for (auto it = recDrivers.begin(); it != recDrivers.end(); it++) {
                if (it != recDrivers.begin())
                    ret += ", ";
                ret += CharacterHandler::GetDriverName(static_cast<EDriverID>(*it));
            }
            ret += "\n";
            ret += (partsBonus[1] ? Color::LimeGreen : ResetColor()) << "    ";
            for (auto it = recBodies.begin(); it != recBodies.end(); it++) {
                if (it != recBodies.begin())
                    ret += ", ";
                ret += CharacterHandler::GetKartBodyName(static_cast<EBodyID>(*it));
            }
            ret += "\n";    
            ret += (partsBonus[2] ? Color::LimeGreen : ResetColor()) << "    ";
            for (auto it = recTires.begin(); it != recTires.end(); it++) {
                if (it != recTires.begin())
                    ret += ", ";
                ret += CharacterHandler::GetKartTireName(static_cast<ETireID>(*it));
            }
            ret += "\n";    
            ret += (partsBonus[3] ? Color::LimeGreen : ResetColor()) << "    ";
            for (auto it = recWings.begin(); it != recWings.end(); it++) {
                if (it != recWings.begin())
                    ret += ", ";
                ret += CharacterHandler::GetKartWingName(static_cast<EWingID>(*it));
            }
            ret += "\n";
            ret += Utils::Format((NOTE("sc_wc_bonus") + " (x%.3f)\n").c_str(), multiplyBonus);
        } else if (page == 1 || page == 2) {
            title = (page == 1) ? NAME("sc_wc_board") : NOTE("sc_wc_board");
            auto& rankVec = (page == 1) ? weeklyLeaderBoard.first : weeklyLeaderBoard.second;
            if (rankVec.empty()) {
                ret += "\n" + CenterAlign(NOTE("sc_wc_ended"));
            } else {
                auto badgeLimits = GetBadgeLimits();

                auto calcColor = [](const std::vector<u32> limits, const RankInfo& info) -> std::string {
                    if (info.rank < 0) {
                        return Color::LimeGreen;
                    }
                    if (info.rank <= limits[0]) {
                        return Color(0xEF, 0xBF, 0x04);
                    }
                    if (info.rank <= limits[1]) {
                        return Color(0xBB, 0xD2, 0xFF);
                    }
                    if (info.rank <= limits[2]) {
                        return Color(0xCE, 0x89, 0x49);
                    }
                    return ResetColor();
                };
                for (auto it = rankVec.begin(); it != rankVec.end(); it++) {
                    ret += calcColor(badgeLimits, *it) + Utils::Format("%d. ", std::abs(it->rank)) + limitUtf8(it->name, 22) + SkipToPixel(240) + Utils::Format("%d\n", it->score) + ResetColor();
                }
            }
        }
        return {ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + title + ToggleDrawMode(Render::FontDrawMode::UNDERLINE), ret};
    }

    static u32 g_selectedCupID;
    static u32 g_selectedCourseID;
    static u32 g_selectedIndex;
    static int g_keyboardkey = -1;
    static int g_fetchInfoRes = 0;
    static bool g_fetchCfg = false, g_fetchBrd = false, g_canDeleteSave = false;

    std::string PointsModeHandler::GenerateCupTopMessage(u32 courseID)
    {
        u32 clear_score = CourseManager::getCourseData(courseID)->pointsModeClearScore;
        u32 previous_score = SaveData::GetTrackScore(courseID);
        g_canDeleteSave = previous_score != 0;

        std::string res = ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + CenterAlign(NAME("score_attack")) + ToggleDrawMode(Render::FontDrawMode::UNDERLINE);
        res += "\n";
        std::string course; CourseManager::getCourseText(course, courseID, true);
        res += CenterAlign(course) + HorizontalSeparator();
        res += CenterAlign((previous_score >= clear_score) ? (Color::Lime << NAME("sc_at_clear")) : (Color::Red << NOTE("sc_at_clear"))) + ResetColor() + "\n\n";
        res += NAME("sc_at_score") + ": " + RightAlign(previous_score ? std::to_string(previous_score) : NOTE("sc_at_save"), 30, 320) + "\n";
        res += NOTE("sc_at_score") + ": " + RightAlign(std::to_string(clear_score), 30, 320) + "\n";
        res +=  HorizontalSeparator();
        res += "\n\n";
        if (g_canDeleteSave)
            res += RightAlign(Color::Yellow << NAME("sc_at_save") << ResetColor(), 35, 367);
        return res;
    }

    void PointsModeHandler::OnCourseKeyboardEvent(Keyboard& kbd, KeyboardEvent &event) {
        bool reDraw = false;
        if (event.type == KeyboardEvent::EventType::KeyPressed && event.affectedKey == Key::X && g_canDeleteSave) {
            bool confirm = MessageBox(NAME("sc_at_delete"), DialogType::DialogYesNo, ClearScreen::Top)();
            if (confirm) {
                reDraw = true;
                SaveData::SetTrackScore(g_selectedCourseID, 0);
                event.selectedIndex = g_selectedIndex;
            }
        }
        if (reDraw || event.type == KeyboardEvent::EventType::SelectionChanged) {
            if (event.selectedIndex >= 0 && event.selectedIndex <= 4) {
                g_selectedIndex = event.selectedIndex;
                g_selectedCourseID = globalCupData[FROMBUTTONTOCUP(g_selectedCupID)][event.selectedIndex];
                kbd.GetMessage() = PointsModeHandler::GenerateCupTopMessage(g_selectedCourseID);
            }
        }
    }

    void PointsModeHandler::OnCupSelectCallback()
    {
        Process::Pause();
        if (g_selectedCupID == POINTSWEEKLYCHALLENGECUPID) {
            if (!SaveHandler::CheckAndShowServerCommunicationDisabled()) {
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->cancelCupSelect = true;
                goto exit;
            }
            bool cfgvalid = IsWeeklyConfigValid();
            bool brdvalid = cfgvalid && IsWeeklyLdrBoardValid();
            if (!cfgvalid || !brdvalid) {
                g_fetchCfg = !cfgvalid;
                g_fetchBrd = !brdvalid;
                Keyboard kbdTemp(NAME("sc_wc_getting"));
                g_fetchInfoRes = 0;
                kbdTemp.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                    if (event.type == KeyboardEvent::EventType::FrameTop) {
                        // Pls don't kill me for this...
                        if (g_fetchInfoRes == -5) {
                            g_fetchInfoRes = FetchLatestWeeklyData(g_fetchCfg, g_fetchBrd);
                            k.Close(); 
                        } else {
                            g_fetchInfoRes--;
                        }
                    }
                });
                kbdTemp.Populate({std::string("")});
                kbdTemp.Open();
                if (g_fetchInfoRes != 0) {
                    kbdTemp.GetMessage() = g_fetchInfoRes == (int)Net::CTWWLoginStatus::VERMISMATCH ? NOTE("update_check") : Utils::Format(NOTE("sc_wc_getting").c_str(), g_fetchInfoRes);
                    kbdTemp.OnKeyboardEvent(nullptr);
                    kbdTemp.Populate({NAME("exit")});
                    kbdTemp.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    kbdTemp.ChangeSelectedEntry(0);
#if STRESS_MODE == 0
                    kbdTemp.Open();
#endif
                    MenuPageHandler::MenuSingleCupGPPage::GetInstace()->cancelCupSelect = true;
                    goto exit;
                }
            }
            int currMenu = 0;
		    int totalMenu = 3;
            int opt = 0;
            Keyboard kbd("dummy");
            kbd.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                if (event.type == KeyboardEvent::EventType::KeyPressed) {
                    if (event.affectedKey == Key::R) {
                        g_keyboardkey = 100;
                        SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
                        k.Close();
                    }
                    else if (event.affectedKey == Key::L) {
                        g_keyboardkey = 200;
                        SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
                        k.Close();
                    }
                }
            });
            do {
                kbd.Populate({Language::MsbtHandler::GetString(5010), Language::MsbtHandler::GetString(2005) });
                kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
                auto content = GenerateWeeklyConfigPage(currMenu);
                std::string topStr = CenterAlign(content.first + "\n");
                std::string fmtStr = std::string(FONT_L " ") + NAME("page") + " (%02d/%02d) " FONT_R;
                topStr += ToggleDrawMode(Render::UNDERLINE) + " " + CenterAlign(Utils::Format(fmtStr.c_str(), currMenu + 1, totalMenu)) + RightAlign(" ", 30, 365) + ToggleDrawMode(Render::UNDERLINE);
                topStr += ToggleDrawMode(Render::LINEDOTTED);
                topStr += content.second;
                kbd.GetMessage() = topStr;
#if STRESS_MODE == 0
                opt = kbd.Open();
#else
                opt = 0;
#endif
                if (g_keyboardkey != -1) opt = g_keyboardkey;
                g_keyboardkey = -1;
                switch (opt)
                {
                case 100:
                    currMenu++;
                    if (currMenu >= totalMenu)
                        currMenu = 0;
                    break;
                case 200:
                    currMenu--;
                    if (currMenu < 0)
                        currMenu = totalMenu - 1;
                    break;
                case 0:
                    selectedCourse = CourseManager::getCourseIDFromName(weeklyConfig.get("trackSzs", ""));
                    isPlayingWeeklyChallenge = true;
                    break;
                default:
                    opt = -1;
                    MenuPageHandler::MenuSingleCupGPPage::GetInstace()->cancelCupSelect = true;
                    break;
                }
            } while (opt != 0 && opt != -1);
        } else {
            g_selectedCourseID = globalCupData[FROMBUTTONTOCUP(g_selectedCupID)][0];
            selectedCourse = VersusHandler::OpenCourseKeyboard(g_selectedCupID, true, GenerateCupTopMessage(g_selectedCourseID), OnCourseKeyboardEvent);
            if (selectedCourse == INVALIDTRACK) {
                MenuPageHandler::MenuSingleCupGPPage::GetInstace()->cancelCupSelect = true;
            }
        }
    exit:
        AsyncRunner::StopAsync(OnCupSelectCallback);
		Process::Play();
    }

    void PointsModeHandler::CalculateBonuses()
    {
        multiplyFinalBonus = 1.f;
        if (isPlayingWeeklyChallenge) {
            
            auto partsRec = HasPlayerRecommendedParts(MarioKartFramework::masterPlayerID);
            u32 bonuses = 0;
            for (int i = 0; i < partsRec.size(); i++) if (partsRec[i]) bonuses++;
            multiplyFinalBonus += (bonuses * (0.5f/4));
        }
    }

    static std::string g_uploadResStr;
    void PointsModeHandler::WeeklyChallengeEndKbd()
    {
#ifndef DISABLE_CT_HASH
        bool score_invalid = (!ExtraResource::lastTrackFileValid && !(CourseManager::lastLoadedCourseID == 0x7 || CourseManager::lastLoadedCourseID == 0x8 || 
                                                     CourseManager::lastLoadedCourseID == 0x9 || CourseManager::lastLoadedCourseID == 0x1D));
#else
        bool score_invalid = false;
#endif

        AsyncRunner::StopAsync(WeeklyChallengeEndKbd);
        Process::Pause();
        u32 newScore = myPoints;
        u32 oldScore = 0;
        for (auto it = weeklyLeaderBoard.second.begin(); it != weeklyLeaderBoard.second.end(); it++) {
            if (it->rank < 0) {
                oldScore = it->score;
                break;
            }
        }
        
        Keyboard kbd("dummy");
        std::string msg = "";
        msg += CenterAlign(NAME("sc_wc_results"));
        msg += HorizontalSeparator();
        if (oldScore) {
            msg += NAME("sc_wc_score") + SkipToPixel(240) + Utils::Format("%d", oldScore) + "\n";
        }
        if (newScore > oldScore) { msg += ToggleDrawMode(Render::FontDrawMode::BOLD); }
        msg += NOTE("sc_wc_score") + SkipToPixel(240) + Utils::Format("%d", newScore) + "\n";
        if (score_invalid) {
            msg += std::string("\n") << Color::Red << "Invalid Checksum." + ResetColor();
        } else if (newScore > oldScore) { 
            msg += ToggleDrawMode(Render::FontDrawMode::BOLD);
            msg += "\n" + NAME("sc_wc_upload");
        }
        kbd.GetMessage() = msg;
        
        if (newScore > oldScore && !score_invalid) {
            kbd.Populate({NOTE("sc_wc_upload"), Language::MsbtHandler::GetString(2005)});
            kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
            kbd.CanAbort(false);
        } else {
            kbd.Populate({NAME("exit")});
            kbd.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
        }

#if STRESS_MODE == 0
        int opt = kbd.Open();
#else
        int opt = -1;
#endif
        if (opt == 0 && newScore > oldScore) {
            do {
                Keyboard kbdTemp(NAME("sc_wc_uploading"));
                g_fetchInfoRes = 0;
                g_uploadResStr.clear();
                kbdTemp.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                    if (event.type == KeyboardEvent::EventType::FrameTop) {
                        // Pls don't kill me for this...
                        if (g_fetchInfoRes == -5) {
                            g_fetchInfoRes = UploadWeeklyScore(myPoints, g_uploadResStr);
                            k.Close(); 
                        } else {
                            g_fetchInfoRes--;
                        }
                    }
                });
                kbdTemp.Populate({std::string("")});
                kbdTemp.Open();
                kbdTemp.OnKeyboardEvent(nullptr);
                if (g_fetchInfoRes == 0) {
                    auto content = GenerateWeeklyConfigPage(2);
                    kbdTemp.GetMessage() = CenterAlign(content.first) + HorizontalSeparator() + content.second;
                    kbdTemp.Populate({NAME("exit")});
                    kbdTemp.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    kbdTemp.ChangeSelectedEntry(0);
                    kbdTemp.Open();
                } else {
                    bool allowRetry = g_fetchInfoRes < 0;
                    kbdTemp.GetMessage() = NOTE("sc_wc_uploading") + "\n\n" + (g_uploadResStr.empty() ? Utils::Format("0x%08X", g_fetchInfoRes) : g_uploadResStr);
                    if  (allowRetry) {
                        kbdTemp.GetMessage() += std::string("\n\n") + NAME("sc_wc_retry");
                        kbdTemp.Populate({Language::MsbtHandler::GetString(2003), NAME("exit")});
                        kbdTemp.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
                    } else {
                        kbdTemp.Populate({NAME("exit")});
                        kbdTemp.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    }
                    kbdTemp.ChangeSelectedEntry(0);
                    int opt = kbdTemp.Open();
                    if (allowRetry && opt == 0) {
                        continue;
                    }
                }
                break;
            } while (true);
        }
        Process::Play();
    }

    void PointsModeHandler::OnNextTrackLoad() {
        if (!isPointsMode || !isPlayingWeeklyChallenge)
            return;

        AsyncRunner::StartAsync(WeeklyChallengeEndKbd);
    }

    void PointsModeHandler::OnCupSelect(u32 cupID)
    {
        isPlayingWeeklyChallenge = false;
        if (cupID == POINTSRANDOMCUPID) {
            CourseManager::getRandomCourse(&selectedCourse, true);
        } else {
            g_selectedCupID = cupID;
            AsyncRunner::StartAsync(OnCupSelectCallback);
        }
    }

    void PointsModeHandler::updateResultBars(u32 racePage)
    {
        MarioKartFramework::BaseResultBar playerBar = MarioKartFramework::resultBarArray[0];
        MarioKartFramework::BaseResultBar targetBar = MarioKartFramework::resultBarArray[2];
        MarioKartFramework::BaseResultBar bestBar = MarioKartFramework::resultBarArray[1];
        
        MK7::Sequence::BaseRacePage* page = (MK7::Sequence::BaseRacePage*)racePage;

        u32 totalTime = MarioKartTimer::ToFrames(0, 1, 0) / page->m_result_bar_point_interval;
        page->m_result_bar_point_increment = std::max(1ul, myPoints / totalTime);
        
        *((u8*)playerBar + 0x7B) = 0; // Set pts. mode
        *((u8*)targetBar + 0x7B) = 0; // Set pts. mode
        *((u8*)bestBar + 0x7B) = 0; // Set pts. mode
        
        u16* utf16ptr;

        MarioKartFramework::BaseResultBar_setPoint(playerBar, 0);
        MarioKartFramework::BaseResultBar_addPoint(playerBar, myPoints);
        MarioKartFramework::BaseResultBar_setCountryVisible(playerBar, false);
        FixedStringBase<char16_t, 0x20>* playerNames = MarioKartFramework::getPlayerNames();
        utf16ptr = (u16*)&playerNames[0].strData;
        MarioKartFramework::BaseResultBar_setName(playerBar, &utf16ptr);

#ifndef DISABLE_CT_HASH
        bool score_invalid = (!ExtraResource::lastTrackFileValid && !(CourseManager::lastLoadedCourseID == 0x7 || CourseManager::lastLoadedCourseID == 0x8 || 
                                                     CourseManager::lastLoadedCourseID == 0x9 || CourseManager::lastLoadedCourseID == 0x1D));
#else
        bool score_invalid = false;
#endif

        if (!score_invalid) StatsHandler::OnScoreAttackFinish(isPlayingWeeklyChallenge, myPoints, CourseManager::lastLoadedCourseID);

        if (isPlayingWeeklyChallenge)
            return;

        u32 clear_score = CourseManager::getCourseData(CourseManager::lastLoadedCourseID)->pointsModeClearScore;
        u32 previous_score = SaveData::GetTrackScore(CourseManager::lastLoadedCourseID);

        std::u16string utf16;

        MarioKartFramework::BaseResultBar_setPoint(targetBar, clear_score);
        MarioKartFramework::BaseResultBar_addPoint(targetBar, 0);

        utf16.clear();
        if (myPoints > previous_score && !score_invalid) {
            MarioKartFramework::BaseResultBar_setPoint(bestBar, previous_score);
             MarioKartFramework::BaseResultBar_addPoint(bestBar, myPoints - previous_score);
            Utils::ConvertUTF8ToUTF16(utf16, NAME("sc_at_record"));
            utf16ptr = (u16*)utf16.c_str();
            MarioKartFramework::BaseResultBar_setName(bestBar, &utf16ptr);
            MarioKartFramework::BaseResultBar_SetBarColor(bestBar, 4);
            SaveData::SetTrackScore(CourseManager::lastLoadedCourseID, myPoints);
            previous_score = myPoints;
        } else {
            MarioKartFramework::BaseResultBar_setPoint(bestBar, previous_score);
             MarioKartFramework::BaseResultBar_addPoint(bestBar, 0);
             MarioKartFramework::BaseResultBar_SetBarColor(bestBar, 0);
            Utils::ConvertUTF8ToUTF16(utf16, NAME("sc_at_score"));
            utf16ptr = (u16*)utf16.c_str();
            MarioKartFramework::BaseResultBar_setName(bestBar, &utf16ptr);
        }
        MarioKartFramework::BaseResultBar_setCountryVisible(bestBar, false);

        utf16.clear();
        if (score_invalid) {
            Utils::ConvertUTF8ToUTF16(utf16, "Invalid Sum");
            utf16ptr = (u16*)utf16.c_str();
            MarioKartFramework::BaseResultBar_setName(targetBar, &utf16ptr);
            MarioKartFramework::BaseResultBar_SetBarColor(targetBar, 2);      
        } else {
            
            if (previous_score >= clear_score) {
                Utils::ConvertUTF8ToUTF16(utf16, NAME("sc_at_clear_small"));
                MarioKartFramework::BaseResultBar_SetBarColor(targetBar, 3);
            } else {
                Utils::ConvertUTF8ToUTF16(utf16, NOTE("sc_at_clear_small"));
                MarioKartFramework::BaseResultBar_SetBarColor(targetBar, 2);
            }
            utf16ptr = (u16*)utf16.c_str();
            MarioKartFramework::BaseResultBar_setName(targetBar, &utf16ptr);
        }
        MarioKartFramework::BaseResultBar_setCountryVisible(targetBar, false);
    }

    static float g_PointsResultBarPos[3];
    float *PointsModeHandler::setResultBarPosition(u32 index, float *original)
    {       
        g_PointsResultBarPos[0] = (index == 0) ? 0 : (isPlayingWeeklyChallenge ? -1000 : -60);
        g_PointsResultBarPos[1] = 75.f - 25.f * ((index == 0) ? 0 : (index + 1)) ;
        g_PointsResultBarPos[2] = 0;

        return g_PointsResultBarPos;
    }

    u32 PointsModeHandler::OnGetCupText(u32 cup, u32 track) {
        if (track != 0)
            return CustomTextEntries::courseDisplay + track;

        for (u32 i = 0; i < 4; i++) {
            u32 courseID = globalCupData[cup][i];
            std::string out;
            std::u16string out16;
            CourseManager::getCourseText(out, courseID, false);
            Utils::ConvertUTF8ToUTF16(out16, out);
            if (SaveData::GetTrackScore(courseID) >= CourseManager::getCourseData(courseID)->pointsModeClearScore) {
                out16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(32, 255, 32)) + out16;
            }
            Language::MsbtHandler::SetString(CustomTextEntries::courseDisplay + i, out16);
        }

        return CustomTextEntries::courseDisplay + track;
    }

    void PointsModeHandler::OnGetGrandPrixData(SaveHandler::CupRankSave::GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror) {
        if (*GPID == POINTSRANDOMCUPID || *GPID == POINTSWEEKLYCHALLENGECUPID) {
            out->isCompleted = false;
            out->starRank = 0;
            out->trophyType = 0;
        }
        u32 cup = FROMBUTTONTOCUP(*GPID);
        u32 total = 0;
        for (int i = 0; i < 4; i++) {
            u32 courseID = globalCupData[cup][i];
            if (SaveData::GetTrackScore(courseID) >= CourseManager::getCourseData(courseID)->pointsModeClearScore)
                total++;
        }
        if (total == 0) {out->isCompleted = true; out->starRank = 0; out->trophyType = 0;}
        else if (total == 1) {out->isCompleted = true; out->starRank = 0; out->trophyType = 1;}
        else if (total == 2) {out->isCompleted = true; out->starRank = 0; out->trophyType = 2;}
        else if (total == 3) {out->isCompleted = true; out->starRank = 0; out->trophyType = 3;}
        else {out->isCompleted = true; out->starRank = 6; out->trophyType = 3;}
    }

    const PointsModeHandler::ActionInfo &PointsModeHandler::GetActionInfo(PointAction action)
    {
        // Should never fail
        auto it = pointActionInfo.find(action);
        return it->second;
    }
    MarioKartTimer PointsModeHandler::PointsToComboTime(int points)
    {
        constexpr float multiplier = 1.825f;
        MarioKartTimer ret = 0;
        if (points < 10) {
            ret = MarioKartTimer(0, 1, 400);
        } else if (points < 15) {
            ret = MarioKartTimer(0, 1, 800);
        } else if (points < 20) {
            ret = MarioKartTimer(0, 1, 950);
        } else if (points < 25) {
            ret = MarioKartTimer(0, 2, 100);
        } else if (points < 30) {
            ret = MarioKartTimer(0, 2, 250);
        } else {
            ret = MarioKartTimer(0, 2, 400);
        }

        return ret * multiplier;
    }

    std::vector<PointsModeHandler::RankInfo> PointsModeHandler::RankInfo::FromString(const std::string& str)
    {
        std::vector<RankInfo> out;
        auto split = TextFileParser::Split(str, "\\");
        if (split.size() % 3 != 0)
            return out;
        for (size_t i = 0; i < split.size(); i+=3) {
            RankInfo r;
            r.rank = strtol(split[i].c_str(), NULL, 10);
            r.name = split[i+1];
            r.score = strtoul(split[i+2].c_str(), NULL, 10);
            out.push_back(r);
        }
        return out;
    }

    void PointsModeHandler::SaveData::Load() {
        SaveBackupHandler::AddDoBackupHandler("point", SaveBackup);
        SaveBackupHandler::AddRestoreBackupHandler("point", RestoreBackup);

        SaveHandler::SaveFile::LoadStatus status;
        bool resetSave = false;
        minibson::document savedoc = SaveHandler::SaveFile::Load(SaveHandler::SaveFile::SaveType::POINT, status);
        if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS) {
            save = std::move(savedoc);
        } else {
            SaveHandler::SaveFile::HandleError(SaveHandler::SaveFile::SaveType::POINT, status);
            save.set("pointsSave", minibson::document());
        }        
    }

	void PointsModeHandler::SaveData::Save() {
        save.set<u64>("_cID", NetHandler::GetConsoleUniqueHash());
        save.set<s64>("sID0", SaveHandler::saveData.saveID[0]);
		save.set<s64>("sID1", SaveHandler::saveData.saveID[1]);
        SaveHandler::SaveFile::Save(SaveHandler::SaveFile::SaveType::POINT, save);
    }

    minibson::document PointsModeHandler::SaveData::SaveBackup() {
        minibson::document copy = save;
        copy.remove("_cID");
        copy.remove("sID0");
        copy.remove("sID1");
        return copy;
    }

    bool PointsModeHandler::SaveData::RestoreBackup(const minibson::document &doc)
    {
        save = doc;
        return true;
    }

    void PointsModeHandler::SaveData::SetTrackScore(u32 courseID, u32 score)
    {
        minibson::document empty;
        std::string id = std::string(CourseManager::getCourseData(courseID)->name);

        if (score == 0) {
            save.get_noconst("pointsSave", empty).remove(id);
        } else {
            save.get_noconst("pointsSave", empty).set<int>(id, (int)score);
        }
        
        SaveHandler::SaveSettingsAll();
    }

    u32 PointsModeHandler::SaveData::GetTrackScore(u32 courseID)
    {
        minibson::document empty;
        return (u32)save.get("pointsSave", empty).get<int>(std::string(CourseManager::getCourseData(courseID)->name), 0);
    }

    std::pair<u32, u32> PointsModeHandler::GetCompletedTracks()
    {
        u32 total = 0;
        u32 completed = 0;
        for (int i = ORIGINALTRACKLOWER; i <= ORIGINALTRACKUPPER; i++) {
            if (SaveData::GetTrackScore(i) >= CourseManager::getCourseData(i)->pointsModeClearScore)
                completed++;
            total++;
        }
        for (int i = CUSTOMTRACKLOWER; i <= CUSTOMTRACKUPPER; i++) {
            if (SaveData::GetTrackScore(i) >= CourseManager::getCourseData(i)->pointsModeClearScore)
                completed++;
            total++;
        }
        return {completed, total};
    }

    bool PointsModeHandler::HasCompletedAllTracks()
    {
        auto comp = GetCompletedTracks();
        return comp.first == comp.second;
    }
}