/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MissionHandler.hpp
Open source lines: 448/448 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "MarioKartFramework.hpp"
#include "MarioKartTimer.hpp"
#include "ExtraResource.hpp"
#include "Minibson.hpp"
#include "SaveHandler.hpp"
#include "MusicSlotMngr.hpp"
#define MISSIONCUPID 64

namespace CTRPluginFramework {
	class MissionHandler
	{
	public:
		class MissionParameters
		{
		public:
			MissionParameters(const std::string& fileName);
			MissionParameters(const u8* fileData, u32 fileSize);
			~MissionParameters();
			u32 GetCheckSum();
			
			struct CMSNDriverOptionsSection {
				struct OptionsEntry {
					s32 driverID;
					s32 bodyID;
					s32 tireID;
					s32 wingID;
					OptionsEntry(s32 dID, s32 bID, s32 tID, s32 wID) {
						driverID = dID;
						bodyID = bID;
						tireID = tID;
						wingID = wID;
					}
				};
				u32 playerAmount;
				OptionsEntry entries[8];
			};
			struct CMSNMissionFlagsSection {
				class SubTypes {
				public:
					enum class Gate {
						ORDER = 0,
						ALL = 1,
					};
					enum class Objects {
						ITEMBOX = 0,
						COIN = 1,
						ROCKYWRENCH = 2,
						CRAB = 3,
						FROGOON = 4
					};
					enum class Boosts {
						BOOSTPAD = 0,
						MINITURBO = 1,
						SUPERMINITURBO = 2,
						ROCKET_START = 3,
						TRICK = 4,
						IMPROVED_TRICK = 5,
						STAR_RING = 6,
					};
				};
				enum class MissionType : u8 {
					TIMER = 0,
					GATES = 1,
					OBJECTS = 2,
					BOOSTS = 3,
				};
				enum class CalculationType : u8 {
					TIMER = 0,
					POINTS = 1
				};
				enum class CompleteCondition : u8 {
					NONE = 0,
					SCOREISYELLOW = 1,
					SCOREISNOTYELLOW = 2,
					PLAYER0FIRST = 3,
					PLAYER1FIRST = 4,
					PLAYER2FIRST = 5,
					NOTGETHIT = 6,
				};
				enum class ImproveTricksMode : u8 {
					USER,
					DISABLED,
					ENABLED
				};
				u32 courseID;
				u8 ccClass;
				u8 cpuDifficulty;
				MissionType missionType;
				CalculationType calcType;
				struct {
					u32 rankVisible : 1;
					u32 lakituVisible : 1;
					u32 displayCourseIntro : 1;
					u32 scoreHidden : 1;
					u32 scoreNegative : 1;
					u32 forceBackwards : 1;
					u32 finishOnSection : 1;
					u32 coinCounterHidden : 1;
					u32 lapCounterVisible : 1;
					u32 pointWhenAccident : 1;
					u32 blockCrashCoinSpawn : 1;
				} flags;
				u32 initialTimer;
				u32 finishRaceTimer;
				u32 minGrade;
				u32 maxGrade;
				u8 initialScore;
				u8 finishRaceScore;
				u8 missionSubType;
				struct {
					u8 timerDirectionUp : 1;
					u8 scoreDirectionUp : 1;
				} countDirection;
				u8 yellowScore;
				u8 lapAmount;
				u16 respawnCoinTimer;
				u8 completeCondition;
				ImproveTricksMode improvedTricks;
				u16 ccSelectorSpeed;

				inline MarioKartTimer GetInitialTimer() {return MarioKartTimer(initialTimer);}
				inline MarioKartTimer GetFinishRaceTimer() {return MarioKartTimer(finishRaceTimer);}
				inline MarioKartTimer GetMinGradeTimer() {return MarioKartTimer(minGrade);}
				inline MarioKartTimer GetMaxGradeTimer() {return MarioKartTimer(maxGrade);}

				inline SubTypes::Gate SubTypeGate() {return (SubTypes::Gate)missionSubType;}
				inline SubTypes::Objects SubTypeObjects() {return (SubTypes::Objects)missionSubType;}
				inline SubTypes::Boosts SubTypeBoosts() {return (SubTypes::Boosts)missionSubType;}

				inline bool HasCompleteCondition(CompleteCondition condition) {return ((completeCondition & 0xF) == (u8)condition) || (completeCondition >> 4) == (u8)condition;}
			};

			struct CMSNItemOptionsSection {
				enum ItemMode {
					All = 0,
					Shells = 1,
					Bananas = 2,
					Mushrooms = 3,
					Bobombs = 4,
					None = 5,
					Custom = 6
				};

				struct ConfigEntry
				{
					enum ConfigMode {
						BoxID = 0,
						Rank = 1,
						DriverID = 2,
						TrueRank = 3
					};
					u16 rouletteSpeed;
					u16 giveItemOffset;
					u16 giveItemEach;
					bool disableCooldown;
					u8 configMode;
					u8 probabilities[8][EItemSlot::ITEM_SIZE];
				};
				u16 configEntryOffsets[2];
				u8 itemMode;
				u8 spawnItemBoxes;
				u16 respawnItemBoxTimer;
				
				inline ConfigEntry* GetConfig(bool playerConfig) {
					if (configEntryOffsets[playerConfig ? 0 : 1] != 0xFFFF)
						return (ConfigEntry*)((u32)this + configEntryOffsets[playerConfig ? 0 : 1]);
					return nullptr;
				}
			};

			struct CMSNTextSection {
				struct TextEntry {
					u16 langCodeOffset;
					u16 titleOffset;
					u16 descriptionOffset;
					u8 descriptionNewlines;

					inline const char* GetLangCode() {
						return (char*)((u32)this + langCodeOffset);
					}

					inline const char* GetTitle() {
						return (char*)((u32)this + titleOffset);
					}

					inline const char* GetDescription() {
						return (char*)((u32)this + descriptionOffset);
					}
				};

				u32 entryCount;
				u32 entryOffsets[];

				inline TextEntry* GetEntry(int id) {
					return (TextEntry*)((u32)this + entryOffsets[id]);
				}

				TextEntry* GetLangEntry(const char* langCode) {
					for (int i = 0; i < entryCount; i++) {
						TextEntry* curr = GetEntry(i);
						if (strcmp(curr->GetLangCode(), langCode) == 0)
							return curr;
					}

					const char* defaultLang = "ENG";
					if (langCode != defaultLang) 
						return GetLangEntry("ENG");
					else
						return nullptr;
				}
			};

			struct CMSNInformationSection {
				char uniqueMissionID[0xC];
				u32 saveIteration;
				u32 checksum;
				u32 key;
				u8 missionWorld;
				u8 missionLevel;
			};

			bool Loaded = false;
			CMSNDriverOptionsSection* DriverOptions = nullptr;
			CMSNMissionFlagsSection* MissionFlags = nullptr;
			CMSNItemOptionsSection* ItemOptions = nullptr;
			CMSNTextSection* TextString = nullptr;
			CMSNInformationSection* InfoSection = nullptr;
		private:
			static const u32 CMSNSignature = 0x4E534D43;
			static const u16 CMSNSupportedVersion = 1;
			struct CMSNHeader {
				u32 signature;
				u32 fileSize;
				u16 version;
				u16 sectionTableOffset;
				u32 sectionsOffset;
			};
			enum CMSNSectionType
			{
				SectionDriverOptions = 0,
				SectionTimingsOptions = 1,
				SectionMissionFlags = 2,
				SectionItemOptions = 3,
				SectionText = 4,
				SectionInformation = 5
			};
			struct CMSNSectionTable {
				struct TableEntry
				{
					CMSNSectionType type;
					u32 offset;
				};
				u32 amount;
				TableEntry entries[];
			};
			

			u8* fileData = nullptr;
			u32 fileSize = 0;
			CMSNHeader* Header = nullptr;

			bool ProcessFileData();
		};

		class SaveData {			
		public:
			
			class SaveEntry {
			private:
				friend class SaveData;
				u32 score = 0;

				MissionParameters::CMSNMissionFlagsSection::CalculationType calcType;
				u32 saveIteration = 0;
				bool isChecksumValid = false;
			public:

				SaveEntry(MissionParameters::CMSNMissionFlagsSection::CalculationType calcTypeV, u32 saveIterationV) :
				calcType(calcTypeV), saveIteration(saveIterationV) {}

				bool hasData = false;
				bool isDirty = false;

				u8 grade = 0;
				inline u32 GetScore() {return score;}
				inline void SetScore(u32 newScore) {score = newScore;}
				inline MarioKartTimer GetTime() {return MarioKartTimer(score);}
				inline void SetTime(const MarioKartTimer& time) {score = time.GetFrames();}
				inline void SetChecksumValid(bool isValid) {isChecksumValid = isValid;}
				inline bool IsChecksumValid() {return !hasData || isChecksumValid;}
			};

			static void Load();
			static void Save();

			static void GetMissionSave(const char missionID[0xC], SaveEntry& entry);
			static void SetMissionSave(const char missionID[0xC], SaveEntry& entry);
			static void ClearMissionSave(const char missionID[0xC]);

			static void SetFullGradeFlag(u32 world, u32 level);
			static bool GetFullGradeFlag(u32 world, u32 level);
			static std::pair<int, int> GetAllFullGradeFlag();

		private:
			static minibson::encdocument save;
			union PackedSavedInfo {
				struct {
					u32 grade : 4;
					u32 calcType : 4;
				};
				u32 raw;
			};
		};

		struct MissionResult
		{
			bool isCalculated = false;

			u32 resultBarOrder = 0;
			int points = 0;

			bool hasBestGrade;
			int bestGrade;
			u32 bestScore;
		};

		static MissionParameters* missionParam;
		static MissionParameters* temporaryParam[4];
		static bool lastLoadedCupAccessible;
		static u32 lastLoadedCup;

		static void* gameBuffer;
		static constexpr u32 gameBufferSize = 300000;

		static bool isMissionMode;
		static u16 cupNameFormatReplace[5];
		static const std::string GetMissionDirPath(u32 world);
		static void InitializeText();
		static void setupExtraResource();
		static void onModeMissionEnter();
		static void onModeMissionExit();
		static void OnRaceEnter();
		static void OnRaceExit();
		static void OnRaceFinish();
		static void calculateCompleteCondition();
		static void calculateResult();		
		static float* setResultBarPosition(u32 index, float* original);
		static void updateResultBars(u32 racePage);
		static void resultBarAmountSetName(u32 resultbar, u32 message);
		static MissionParameters::CMSNDriverOptionsSection::OptionsEntry GetFixedDriverOptions(MissionParameters::CMSNDriverOptionsSection::OptionsEntry& original);
		static void GetCurrentCupGrade(SaveHandler::CupRankSave::GrandPrixData* out);
		static void OnGetGrandPrixData(SaveHandler::CupRankSave::GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror);
		static void OnCupSelect(u32 retCup);
		static u32 OnGetCupText(u32 cup, u32 track);
		static void OnCPUSetupParts(u32 slotID);
		static void OnApplySettingsGP();
		static void OnRacePageGenGP(void* racePage);
		static void LoadCoursePreview(u16* dst);
		static bool LoadCustomMusic(u16* dst, bool isFinalLap);
		static bool LoadCustomMusicData(u32* out, u32 dataMode, bool isNormalLap);
		static bool LoadCustomMusicNameAuthor(std::string& name, std::string& author);
		static void OnKMPConstruct();
		static void OnGPKartFinishRace(u32* finishPosition, u32* halfDriverAmount, u32 playerID);
		static u8* GetReplacementFile(SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo, u8* courseSarc);
		static bool isScoreEnabled();
		static bool isScoreVisible();
		static bool forceBackwards();
		static bool blockLapJingle();
		static bool blockCrashCoinSpawn();
		static u8* GetReplacementScoreIcon(ExtraResource::SARC::FileInfo* fileInfo);
		static u8* GetReplacementItemBox(ExtraResource::SARC::FileInfo* fileInfo);
		static u8* GetReplacementCoin(ExtraResource::SARC::FileInfo* fileInfo, bool isLod);
		static const char* OnObjBaseSetupMdlGetName(u32* objectBase, const char* origName);
		static void OnGateThrough(u32* objectMpBoard, int gateID);
		static void OnMpBoardObjectInit(u32* objectMpBoard);
		static bool tcBoardAllowPlayAnim(u32* objectTcBoard);
		static void OnObjectBaseSetScale(u32* objectBase, Vector3& copyTo, const Vector3& copyFrom);
		static void OnObjectMeltIceInitObj(u32* objectBase);
		static u32 onKartItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 vehicleReactObject, u32 objCallFuncPtr, bool isItem);
		static void onKartHitCoin(int playerID);
		static void OnKartDash(u32* vehicleMove, EDashType dash, bool isImprovedTrick);
		static void OnKartAccident(u32* vehicleReact, ECrashType* type);
		static u32 GetCoinRespawnFrames();
		static u32 GetItemBoxRespawnFrames();
		static float CalculateSpeedMod(float baseSpeedMod);
		static bool AllowImprovedTricks();
		static bool ForceImprovedTricks();
		static bool neeedsCustomItemHandling();
		static u16 handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag);
		static u32 calcItemSlotTimer(u32 timer, u32 driverID);
		static void GivePoint();
		static void onFrame(bool raceFinished);
		static bool shouldFinishRace(MarioKartTimer& raceTimer);
		static void timerHandler(MarioKartTimer& timerObject);
		static void setInitialTimer(MarioKartTimer& timerObject);
		static void setInitialCounter();
		static bool isRankVisible();
		static bool isLakituVisible();
		static u8 getLapAmount();
		static u32 getInitialMissionTime();
		static bool pointControlShouldBeYellow(u32 val);
		static u32 isItemBoxAllowed(u32 itemMode);
		static bool OnKartEffectTrigger(u32 vehicleMove, u32 effectType);
		static int AddPoints;
		static MarioKartTimer finalTime;
		static u32 finalPoints;
		static std::string& GetPtsString(u32 num);


		static u32 calculatedChecksum;
	private:
		static std::pair<std::string, std::string> pointsStr;
		static int AddPointsQueue;
		static int currMissionWorld;
		static int currMissionLevel;
		static bool raceBeenFinished;
		static bool missionConditionFailed;
		static int failMissionFrames;
		static bool coinSoundReplaced;
		static bool cmsnCalculatedChecksum;
		static bool sarcCalculatedChecksum;
		static MarioKartTimer lastMainTimer;
		static MissionResult lastMissionResult;
		static std::vector<u32*> missionGates;
		static ExtraResource::SARC* extraSarc;
		static ExtraResource::SARC* gameCourseSarc;
		static ExtraResource::StreamedSarc* replacementSarc;
		static std::vector<u32> replacedSarcFiles;
		static std::map<u32, MusicSlotMngr::MusicSlotEntry> customMusic;
		static void tryOverwriteFile(SafeStringBase* file, ExtraResource::SARC::FileInfo&);
		static void counterUIAdd(int value, bool playSound = true);
		static void counterUISet(u32 value, bool playSound = false);
		static u32 counterUIGet(bool includeQueue = false);
		static void OpenKbd();
		static void OnKbdEvent(Keyboard& kbd, KeyboardEvent &event);
	};
}