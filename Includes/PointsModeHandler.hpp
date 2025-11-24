/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: PointsModeHandler.hpp
Open source lines: 325/325 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "ExtraUIElements.hpp"
#include "DataStructures.hpp"
#include "MarioKartTimer.hpp"
#include "Minibson.hpp"
#include "SaveHandler.hpp"
#include "map"
#include "queue"
#include "optional"

#define POINTSCUPID 66
#define POINTSRANDOMCUPID 67
#define POINTSWEEKLYCHALLENGECUPID 68

namespace CTRPluginFramework {
    class PointsModeHandler {
    public:
        enum class PointAction : u32 {
            MUSHROOM,
            BOOST_PAD,
            MINITURBO,
            SUPERMINITURBO,
            START_SMALL,
            START_MEDIUM,
            START_PERFECT,
            TRICK,
            IMPROVED_TRICK,
            SLIPSTREAM,
            GOLD_MUSHROOM,
            
            COIN,
            STAR_RING,

            HIT_PIPE,
            HIT_GOOMBA,
            HIT_SWOOP,
            HIT_PYLON,
            HIT_BOULDER,
            HIT_CHEEP,
            HIT_SHELLFISH,
            HIT_CRAB,
            HIT_GOAT,
            HIT_POT,
            HIT_FLYSHYGUY,
            HIT_BARREL,
            HIT_TIKITAK,
            HIT_FROGOON,
            HIT_CAR,
            HIT_BOX,
            HIT_PENGUIN,
            HIT_ICICLE,
            HIT_THWOMP,
            HIT_FAKEBUSH,
            HIT_FAKEGOOMBA,
            HIT_PIRANHAPLANT,
            HIT_CHAINCHOMP,
            HIT_FISHBONE,
            HIT_ANCHOR,
            HIT_MUSICNOTE,
            HIT_LEAVES,
            HIT_WIGGLER,
            HIT_KILLER,
            HIT_ROCKYWRENCH,
            HIT_SNOWMAN,
            HIT_SNOWBALL,
            HIT_IRONBALL,
            HIT_TABLE,
            HIT_CACTUS,
            HIT_TRAIN,
            HIT_BEE,
            
            GLIDE_TIME,

            ACCU_KOURAG,
            ACCU_KOURAR,
            ACCU_BANANA,
            ACCU_KOURAB,
            ACCU_THUNDER,
            ACCU_FAKEBOX,
            ACCU_BOMB,
            ACCU_FLOWER,
            ACCU_TAIL,
            ACCU_GESSO,
            ACCU_STAR,
            ACCU_MEGAMUSH,
            ACCU_BULLET,
            BULLET_OVERTAKE,
            PAYBACK,
            SMASH,

            ITEM_DESTROYED,
            ITEM_DEFLECTED,
            BLUE_DODGED,

            NEXT_LAP,
            NEXT_SECTION,
            RACE_FINISH_FRENZY,
            RACE_FINISH,
        };

        class RankInfo {
        public:
            int rank;
            u32 score;
            std::string name;

        static std::vector<RankInfo> FromString(const std::string& str);
        };

        static bool isPointsMode;
        static bool comesFromRace;

        static void InitializeText();
        static void InitializePerTrackText(ExtraResource::SARC* sarc);

        static void InitDisplayController() {
            if (isPointsMode && !pointsDisplay) {
                pointsDisplay = new PointsModeDisplayController();
            } 
        }
        static void DestroyDisplayController() {
            if (isPointsMode && pointsDisplay) {
                delete pointsDisplay;
                pointsDisplay = nullptr;
            }
        }

        static void OnRaceDirectorCreateBeforeStructure();
        static void OnRaceStart(bool startCountdown);

        static void DoPointAction(PointAction action);
        static void OnVisualElementCalc();
        static void OnVehicleCalc(u32* vehicle);
        static void HandlePendingActions(u32* vehicle);

        static void DoItemHitKart(u32* srcVehicle, u32* dstVehicle, EItemType iType);
        static void OnGessoAfterInitUse(u32* itemObjGesso);
        static void OnClearItem(int playerID);

        static void ForceNextFrenzy(int playerID);

        static void OnKartDash(u32* vehicleMove, EDashType dash);
        static u32 OnKartItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 reactObject, u32 objCallFuncPtr, int mode); // mode: 0 kart, 1 item, 2 thunder
        static void OnItemDirectorDoReaction(u32* director, u32* itemObj1, u32* itemObj2, EItemReact* react1, EItemReact* react2, Vector3* pos1, Vector3* pos2);
        static void OnKartDodgedBlueShell(int playerID);
        static void OnKartPress(u32* srcVehicle, u32* dstVehicle);

        static void OnItemDirectorStartSlot(int playerID, u32 boxID);
        static u32 OnCalcItemSlotTimer(u32 timer);

        static bool OnVehicleDecideSurfaceTrickable(u32 *vehicle, bool isTrickable);

        static void OnLapRankCheckerCalc(u32* laprankchecker);
        static void OnPlayerGoal(int playerID);

        static void SetupExtraResource();

        static void onPointsModeEnter();
        static void onPointsModeExit();

        static bool isPlayingWeeklyChallenge;
        static minibson::document weeklyConfig;
        static std::pair<std::vector<RankInfo>, std::vector<RankInfo>> weeklyLeaderBoard;
        static u64 weeklyConfigFetchTime;
        static bool IsWeeklyConfigValid();
        static bool IsWeeklyLdrBoardValid();
        static int FetchLatestWeeklyData(bool config, bool leaderboard);
        static int UploadWeeklyScore(u32 myScore, std::string& messageOut);
        static std::array<bool, 4> HasPlayerRecommendedParts(int playerID);
        static std::vector<u32> GetRecommendedParts(int mode); // 0: Driver, 1: Body, 2: Tire, 3: Wing
        static std::vector<u32> GetBadgeLimits();
        static std::pair<std::string, std::string> GenerateWeeklyConfigPage(int page);
        static void CalculateBonuses();
        static void WeeklyChallengeEndKbd();
        static void OnNextTrackLoad();

        static u32 selectedCourse;
        static std::string GenerateCupTopMessage(u32 courseID);
        static void OnCourseKeyboardEvent(Keyboard& kbd, KeyboardEvent &event);
        static void OnCupSelectCallback();
        static void OnCupSelect(u32 cupID);
        static void updateResultBars(u32 racePage);
        static float* setResultBarPosition(u32 index, float* original);

        static u32 OnGetCupText(u32 cup, u32 track);
        static void OnGetGrandPrixData(SaveHandler::CupRankSave::GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror);

        static void LoadSaveData() {
            SaveData::Load();
        }
        static void SaveSaveData() {
            SaveData::Save();
        }

        static std::pair<u32, u32> GetCompletedTracks();
        static bool HasCompletedAllTracks();

    private:
        struct ActionFlags {
            enum : u32 {
                NON_ACCUMULATIVE = (1 << 0),
                FINISH_CROSS = (1 << 1),
                RACE_FINISH = (1 << 2),
                RESET_COMBO = (1 << 3),
                IGNORE_POS_MULTIPLIER = (1 << 4),
            };
        };
        struct ActionInfo {
            ActionInfo(int _points, u32 _flags = 0) : points(_points), flags(_flags), name(u"") {}

            int points;
            u32 flags;
            mutable std::u16string name;
            mutable std::u16string per_track_name;

            const std::u16string& GetName() const {
                if (!per_track_name.empty()) {
                    return per_track_name;
                }
                return name;
            }
        };
        static const ActionInfo& GetActionInfo(PointAction action);
        static MarioKartTimer PointsToComboTime(int points);
        static u8* pointBcwavs[6];

        static ModeManagerData* modeManagerData;
        static CRaceInfo* cRaceInfo;

        static const PointsModeDisplayController::CongratState comboToCongrat[10];
        static const std::unordered_map<PointAction, ActionInfo> pointActionInfo;
        static const std::unordered_map<u16, std::pair<u32, PointsModeHandler::PointAction>> gobjCommonHitInfo;
        static const std::unordered_map<u16, std::pair<u32, PointsModeHandler::PointAction>> gobjObjectCrashHitInfo;

        static MarioKartTimer timer;
        static MarioKartTimer lastPoppedActionTimer;
        static MarioKartTimer lastBoostPadTime;
        static MarioKartTimer lastStarRingTime;
        static MarioKartTimer lastGliderTime;
        static MarioKartTimer lastGoldenMushTime;
        static MarioKartTimer comboTimer;
        static MarioKartTimer nonAccComboTimer;
        static u32 comboAmount;
        static u32 finishCrossPoints;
        static PointsModeDisplayController::CongratState congratKind;
        static std::queue<std::pair<PointAction, float>> pendingActions;
        static float multiplyFinalBonus;

        static u32 myPoints;

        struct ItemSlotInfo {
            u32 lastItemBoxID{};
            u32 remainingStarts{};
            bool isStarting{};
            bool nextFrenzy{};
            bool forceNextFrenzy{};
            int totalFrenzies{};
            u32 frenzyFrames{};
            EItemSlot frenzySlot{};
            EItemSlot x2x3ItemSlots[2];
            void Reset(bool fullReset) {
                lastItemBoxID = 0;
                remainingStarts = 0;
                isStarting = false;
                nextFrenzy = false;
                if (fullReset) {
                    frenzyFrames = 0;
                    forceNextFrenzy = false;
                    totalFrenzies = 0;
                    frenzySlot = EItemSlot::ITEM_SIZE;
                    x2x3ItemSlots[0] = x2x3ItemSlots[1] = EItemSlot::ITEM_SIZE;
                } else {
                    if (frenzyFrames > 0) frenzyFrames = 1;
                }
            }
        };
        static ItemSlotInfo itemSlotInfos[MAX_PLAYER_AMOUNT];
        static u32 lastPlayerLap;
        static int lastHitByPlayer;
        static bool raceStarted;
        static bool raceFinished;
        static bool slipStreamPerformed;
        static bool kartMovementPenalize;
        static bool kartMovementPenalizeLoop;
        static MarioKartTimer masterWasInLoop;
        static int bulletPosition;
        
        static float prevMaxMasterRaceProgress, masterRaceProgress, masterMaxRaceProgress;
        static MarioKartTimer masterNotAdvancingTimer;

        static std::optional<std::pair<const std::u16string*, int>> visualReason;
        static u32 visualComboAmount;
        static u32 visualPointsAmount;
        static bool visualKartMovementPenalize;
        static PointsModeDisplayController::CongratState visualCongratKind;
        static PointsModeDisplayController* pointsDisplay;

        class SaveData {			
		public:

			static void Load();
			static void Save();
            static minibson::document SaveBackup();
            static bool RestoreBackup(const minibson::document& doc);

            static void SetTrackScore(u32 courseID, u32 score);
            static u32 GetTrackScore(u32 courseID);

		private:
			static minibson::document save;
		};

        friend class ItemHandler::AIItem;
    };
}