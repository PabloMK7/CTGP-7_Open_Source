/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartFramework.hpp
Open source lines: 782/782 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "LED_Control.hpp"
#include "Math.hpp"
#include "MarioKartTimer.hpp"
#include "DataStructures.hpp"
#include "ItemHandler.hpp"
#include "random"
#include "MK7NetworkBuffer.hpp"
#include "MK7Memory.hpp"
#include "rt.hpp"

extern "C" u32 g_lapVal;
extern "C" float g_speedValsqrt;
extern "C" float g_speedVal;
extern "C" u32 isCTWW;
extern "C" u32 isAltGameMode;

namespace CTRPluginFramework {

	enum EGHTReaction
	{
		EGHTREACT_NONE = 0x0,
		EGHTREACT_COLLIDE = 0x1,
		EGHTREACT_ITEM = 0x2,
		EGHTREACT_IGNORE = 0x7,
		EGHTREACT_WALL1 = 0x8,
		EGHTREACT_WALL2 = 0x9,
		EGHTREACT_SPIN = 0xA,
		EGHTREACT_SPEEDSPIN = 0xB,
		EGHTREACT_MUDSPIN = 0xC,
		EGHTREACT_FIRESPIN = 0xD,
		EGHTREACT_WATERSPIN = 0xE,
		EGHTREACT_BIGHITFAR = 0xF,
		EGHTREACT_FORWARDSHIT = 0x10,
		EGHTREACT_HUGEHIT = 0x11,
		EGHTREACT_BIGHITCLOSE = 0x12,
		EGHTREACT_HUGEHIT2 = 0x13,
		EGHTREACT_THWOMP = 0x14,
		EGHTREACT_THWOMP2 = 0x15,
		EGHTREACT_WALLCONSTANTHIT = 0x16,
		EGHTREACT_WALLCANTMOVE = 0x17,
		EGHTREACT_WALLREPEL = 0x18,
		EGHTREACT_WALLCONSTANTHIT2 = 0x19,
		EGHTREACT_WALLFLIPPER = 0x1A,
	};
	enum EObjectReactType {
		OBJECTREACTTYPE_NONE = 0,
		OBJECTREACTTYPE_DESTRUCTVE = 1,
	};
	enum EVoiceType {
		ITEM_USE = 0,
		ITEM_DROP = 1,
		ITEM_HIT_SUCCESS = 2,
		OVERTAKE = 3,
		ITEM_THROW = 4,
		DASH_BOOST = 5,
		DASH_GLIDER = 6,
		JUMP_TRICK = 7,
		GLIDER_TRICK = 8,
		STAR_START = 9,
		GLIDER_CANNON = 10,
		DAMAGE_SMALL = 11,
		DAMAGE_HIT = 12,
		DAMAGE_BIG = 13,
		DAMAGE_SPIN = 14,
		DAMAGE_OOB = 15
	};

	enum ECrashType : u8 {
		ACDTYPE_NONE = 0,
		CRASHTYPE_SPINFIRE = 0x1,
		CRASHTYPE_GREENSPARK = 0x2,
		CRASHTYPE_SPIN = 0x5,
		CRASHTYPE_LIGHTNING = 0x8,
		CRASHTYPE_BIG1 = 0xA,
		CRASHTYPE_BIG2 = 0xB,
		CRASHTYPE_BIG3 = 0xD,
		CRASHTYPE_FLIP = 0xF,
		CRASHTYPE_FALSESTART = 0x10
	};

    enum EOnlineRaceMode : u8 {
		NOACTION = 0,
		ONLINEBTCOIN = 1,
		ONLINEBTBALLOON = 2,
		ONLINERACE150 = 3,
		ONLINERACE100 = 4,
		NOACTION2 = 5,
		ONLINERACEMIRROR = 6
	};
	
	enum GameState
    {
        None = 0,
        Enter = 1,
        Exit = 2,
    };

	struct ResizeInfo {

		float targetSize = 1.f;
		float fromSize = 1.f;
		int animationTimer = 0;
		int prevAnimationStep = 3;
		int animationStep = 3;
		const int totalAnimationStep = 3;
		const int eachAnimationFrames = 15;
		bool isGrowing = false;
		float lastKartScale = 1.f;

		void Reset() {
			targetSize = 1.f;
			fromSize = 1.f;
			animationTimer = 0;
			prevAnimationStep = 3;
			animationStep = 3;
			isGrowing = false;
			lastKartScale = 1.f;
		}
	};

    enum GameRegion {
        JAPAN = 3,
        EUROPE = 1,
        AMERICA = 2,
		KOREA = 4
    };

    enum GameRevision {
        REV2 = 3,
    };

	enum RacePageInitID {
		NAME,
		TEXTURE,
		EFFECT,
		ITEM_SLOT,
		RANK,
		MAP,
		MAP_ICON,
		RANK_BOARD,
		LAP,
		COIN,
		WIPE,
		TEXT,
		CAPTION,

		POINT,
		BATTLE_COUNTDOWN,
		TIMER,

		AMOUNT
	};

	enum SaveDataEvent {
		SAVEEVENT_NONE,
		SAVEEVENT_INIT,
		SAVEEVENT_SAVE,
		SAVEEVENT_UNUSED,
		SAVEEVENT_LOADGHOST,
		SAVEEVENT_SAVEGHOST,
		SAVEEVENT_FORMAT,
		SAVEEVENT_LOADGHOSTLIST,
	};

	class DialogFlags {
	public:
		enum class Mode {
			OK = 0,
			YESNO = 1,
			LOADING = 2,
			NOBUTTON = 3
		};

		constexpr DialogFlags(Mode mode, bool isAsync = true) : raw((u32)mode | ((isAsync ? 1 : 0) << 2)) {}
		
		u32 raw;
	};
	

	enum DialogResult {
		NO = 0,
		YES = 1,
		NONE = 2,
		ERROR = 3
	};

	struct IntroCameraAnim
	{
		u32 fromFrame;
		Vector3 pos;
		float FOV;
	};

	struct CPUDataSettings {
		u32 kartBody;
		u32 kartTire;
		u32 kartWing;
		u32 gap;
		u32 character;
	};

	struct KartLapInfo {
		u32 _gap[0x58/4];
		u8 currentLap;
	};

    class MarioKartFramework {
        private:
            static bool calculateRaceCond();
			static u8 previousCDScore;
			static void* gobjrelocptr;

        public:

			struct CRaceMode { // Demo race 3 6 2, Demo coin 3 7 0, Demo balloon 3 7 1, Winning run 3 4 2, live 2 2 0
				u32 type; // 0 -> SP, 1 -> MP, 2 -> Online 3 -> Demo
				u32 mode; // 0 -> GP, 1 -> TT, 2 -> VS, 3 -> Bt
				u32 submode; // 3 0 -> coin, 3 1 -> balloon, 0 2 -> Race
			};

			struct EPlayerInfo
			{
				EDriverID driver;
				EPlayerType type;
			};

			struct SavePlayerData
			{
				u32 vr;
				u32 wins;
				u32 losses;
				u32 unknown[2];
				u16 unknown2[4];
				void* unkPtr;
				u32 unknown3[4];
				u32 pid;
				u32 unknown4[2];
				MiiData miiData;
				u32 checksum;
				u8 loaded;
			}PACKED;

			struct ImprovedTrickInfo {
				Vector3 trickRotationAxes;
				MarioKartTimer trickCooldown;
				u8 trickMode;
				bool readyToDoFasterBoost;
				bool doFasterBoost;

				ImprovedTrickInfo() {
					Reset();
				}
				void Reset() {
					trickRotationAxes = Vector3();
					trickCooldown = MarioKartTimer();
					trickMode = 0;
					doFasterBoost = false;
					readyToDoFasterBoost = false;
				}
			};

			static u32 baseAllPointer;
			static constexpr u32 baseAllPointerXor = 0x75F1B26B;
			static u32 getSequenceEngine();
			static u32 getRootSequence();
			static u32 getSystemEngine();
			static u32 getSystemSaveData();
			static u32 getItemDirector();
			static u32 getObjectDirector();
			static u32 getMiiEngine();
			static u32 getRaceEngine();
			static u32 getSndEngine();
			static u32 getCameraEngine();
			static u32 getNetworkEngine();
			static MK7::Net::NetworkStationBufferManager* getNetworkStationBufferManager();
			static u32 getKartDirector();
			static u32 getRaceDirector();
			static u32 getGarageDirector();
			static u32 getEffectDirector();
			static u32 getModeManagerBase();
			static u32 getBaseRacePage();
			static u32 getMenuData();
			static ModeManagerData* getModeManagerData();
			static CRaceInfo* getRaceInfo(bool global);
			static FixedStringBase<char16_t, 0x20>* getPlayerNames();
			static int getCurrentRaceNumber();
			static bool isFirstRace() {return getCurrentRaceNumber() <= 0;}
			static KartLapInfo* getKartLapInfo(u32 playerID);
			static Vector3* GetGameCameraPosition();
			static void BasePageSetCup(u8 cupID);
			static void BasePageSetCC(EEngineLevel engineLevel);
			static void BasePageSetMirror(bool mirror);
			static u32 BasePageGetCup();

			static CRaceMode currentRaceMode;
			static void (*BasePage_SetRaceMode)(CRaceMode* mode);
			static EPlayerInfo playerInfos[MAX_PLAYER_AMOUNT];
			static u32 currGatheringID;
			static u8 myRoomPlayerID;
			static bool allowOpenCTRPFMenu();
            static Vector3* playerCoord;
            //Coord3D* blueShellCoord;
            static u8* _currgamemode;
			static u32 ctgp7ver;
            static u32 region; // 1 EUR, 2 USA, 3 JPN, 4 KOR
            static u32 revision; // 1 rev0 1.1, 2 rev1
            static u32 lastLoadedMenu; // 0x1A Main online; 0x27 community list; 0x31 confirm community; 0x2B - 0x2C enter code; 0x1c - worldwide character; 0x20 vr screen
            static u32 origChangeMapInst;
			static u8* cameraKartMode;
            //
			static char* replayFileName;
			//
            static u32* currNumberTracksPtr;
            static u32* currCommNumberTracksPtr;
            static u32* changeMapCallPtr;
			static u32* worldWideButtonPtr;
			static u32* battleWideButtonPtr;
			static u32 battleWideMessageReplacePtr[2];
			//
			static void setRosalinaMenuBlock(bool blocked);
			static bool getRosalinaMenuBlock();
			//
			static RGBLedPattern ledBlueShellPat;
			static RGBLedPattern ledThunderPat;
			static RGBLedPattern ledBlooperPat;
			static RGBLedPattern ledFinalRainbowPat;
			static void InitializeLedPatterns();
            //
            static float* localMapPtr;
            //
			static bool isRaceState;
			static bool isRacePaused;
			static bool isRaceGoal;
            //
			static bool wasKeyInjected;
			static Key injectedKey;
			static bool areKeysBlocked;
			static bool isPauseBlocked;
			static bool isPauseAllowForced;
			//
			static bool isLoadingAward;

            static void getLaunchInfo();
            static bool isCompatible(); //true success, false fail
            static bool isGameInRace() { return isRaceState; }
			static void onRaceEvent(u32 raceEventID);

			static void changeFilePath(char16_t* dst, bool isDir);
            static void changeNumberRounds(u32 newval);
            static u32* (*getSaveManagerFuncptr)(void);
			static void (*getMyPlayerDatafuncptr)(u32 systemEngine, SavePlayerData* data, bool something);
			static void getMyPlayerData(SavePlayerData* data);
			static u8* GetSelfMiiIcon();
			static u32 GetSelfMiiIconChecksum();
            static STGIEntry** (*getStageInfoFuncptr)(void);
			static GOBJAccessor* (*getGeoObjAccessor)(void);
			static POTIAccessor* (*getPathAccessor)(void);
            static void lockBottomMapChange(bool lock);
			static void unlockDriver(EDriverID driverID);
            static void unlockKartBody(EBodyID kartID);
			static void unlockKartTire(ETireID tireID);
			static void unlockKartWing(EWingID wingID);
			static void OnSaveDataEvent(SaveDataEvent event);
			static void injectKey(Key key);
			static void blockKeys(bool block);
			static void handleBackwardsCamera(PadData* padData);
			static bool allowBackwardsCamera();
			static void kartCameraSmooth(u32* camera, float smoothVal);
			static bool playCountDownCameraAnim;
			static bool startedRaceScene;
            //
			static void patchKMP(void* kmp);
            static void kmpConstructCallback();
			static void OnRaceEnter();
			static void OnRaceExit(u32 mode);
			static void applyGOBJPatches(GOBJAccessor* access);
            static u16 getSTGIEntryCount(STGIEntry** s);
			//
			static u64 kouraGProbability;
			static u64 kouraRProbability;
			static u32 getKouraGModelName();
			static u32 getKouraRModelName();
			//
			static constexpr const u32 SOUNDTHREADSAMOUNT = 2;
			static std::tuple<u32, u32*> soundThreadsInfo[2];
			static void playMusicAlongCTRPF(bool playMusic);
			static bool forceDisableSndOnPause;
			//
			static void (*openDialogImpl)(DialogFlags::Mode mode, int messageID, void* args);
			static bool (*isDialogClosedImpl)();
			static bool (*isDialogYesImpl)();
			static void (*closeDialogImpl)();
			static DialogResult openDialog(const DialogFlags& flags, const std::string& text, void* args = nullptr, bool alreadyReplaced = false);
			static DialogResult getLastDialogResult();
			static bool isDialogOpened();
			static void closeDialog();
			static int playTitleFlagOpenTimer;
			//
			static void dialogBlackOut(const std::string& message = "");
			static bool onNetworkErrorCheckCalc();
			static u32 onNetworkErrorCheckWindowOpen();
			//
			static void setCTWWState(bool enabled);
			static void adjustVisualElementsCTWW(bool enabled);
			static void applyVisualElementsCTWWOSD();
			static u32 onOnlineModePreStep(u32 mode);
			static void onOnlineModeButOK(u32 btn);
			static void onOnlineSubModePreStep();
			static void onOnlineSubModeButtonOK(int buttonID);
			static void (*nwlytReplaceText)(u32 a0, u32 a1, u32 msgPtr, u32 a3, u32 a4);
			//
			static int handleBootSequenceCallback();
			static void OnBootTaskFinish();
			//
			static bool pendingVoiceChatInit;
			static bool needsOnlineCleanup;
			static void DoOnlineCleanup();
			static void OnSocInit();
			static void OnSocExit();
			static void handleTitleEnterCallback();
			static void handleTitleCompleteCallback(u32 option);
			static u32 handleTitleMenuPagePreStep(u32 timerVal);
			//
			static void updateReplayFileName();
			//
			static void warnLedItem(u32 vehicle, u32 item);
			static void handleItemCD(u32 vehicle, u32 item);
			//
			static void playCDPointUpSE(u32 points);
			//
			static std::vector<std::tuple<u32, void(*)(u32*)>> regButtons;
			static int registerButtonReturnCode(u32 button, void(*callback)(u32* retCode), int id = -1);
			static u32 getButtonByID(int id);
			static void performButtonCallback(u32 button, u32* val);
			static void changeButtonText(u32 button, std::string& text, const char* textElement);
			static void changeButtonText(u32 button, u16* text, const char* textElement);
			static bool onBasePageCanFinishFadeCallback(u32 page, u32 currFrame, u32 maxFrame);
			static bool IsLastGPTrack(u32 track);
			static void OnGPKartFinishRace(u32* finishPosition, u32* halfDriverAmount, u32 playerID);
			//
			static void (*BaseMenuPageApplySetting_CPU)(int cpuAmount, int startingCPUIndex, int* playerChar);
			static void (*SequenceCorrectAIInfo)();
			static void generateCPUDataSettings(CPUDataSettings* buffer, u32 playerID, u32 totalPlayers, ModeManagerData* raceinfo);
			static void SetWatchRaceMode(bool setMode);
			static bool isWatchRaceMode;
			static u32 getNumberOfRacers(u32 players, bool isFake = false);
			static bool blockWinningRun();
			static u32 cpuRandomSeed;
			static u32 neededCPU;
			static u32 neededCPUCurrentPlayers;
			static u8 randomdriverchoices[16];
			static bool israndomdriverchoicesshuffled;
			static void resetdriverchoices();
			static void getRandomDriverChoice(u8 out[4], int playerID);
			static int getRandomDriverWifiRateOffset(int playerID);
			static void* getNetworkManager();
			static void* getNetworkSelectMenuProcess();
			static u8* getMyNetworkSelectMenuBuffer();
			static u8* getPlayerNetworkSelectMenuBuffer(int playerID);
			static u8* (*networkGetPlayerStatus)(void* networkmenproc, int playerID, u8 copy[0x9C]);
			static EDriverID GetVoteBarDriver(EDriverID original);
			//
			static void (*BasePage_SetDriver)(int slot, s32* driverID, u32* playerType);
			static void (*BasePage_SetParts)(int slot, s32* bodyID, s32* tireID, s32* wingID, u32* screwID);
			static void (*BasePage_UpdateRaceInfo)();
			static void BasePage_SetWifiRate(int slot, u32 vrAmount);
			//
			static bool forceFinishRace;
			static bool skipGPCoursePreview;
			static bool allowWigglerAngry;
			static void setSkipGPCoursePreview(bool skip);
			static u32 forceFinishRaceCallback(u32 flags);
			static s8 forcedResultBarAmount;
			static u8 realResultBarAmount;
			static u32 getResultBarAmount(u32 amount);
			using BaseResultBar = u32;
			static BaseResultBar resultBarArray[MAX_PLAYER_AMOUNT];
			static bool resultBarHideRank;
			static void(*BaseResultBar_setName)(BaseResultBar bar, u16** name);
			//static void(*BaseResultBar_setCharaTex)(BaseResultBar bar, u32* driverID, int driverSkin, bool unknown);
			static void(*BaseResultBar_setPoint)(BaseResultBar bar, u32 point);
			static void(*BaseResultBar_addPoint)(BaseResultBar bar, u32 point);
			static void BaseResultBar_setCountryVisible(BaseResultBar bar, bool visible);
			static void(*RacePage_genPause)(u32 racePage);
			static void(*RacePage_genReplayPause)(u32 racePage);
			static void(*DrawMdl_changeMatAnimation)(u32 drawmdl, int id, float value);
			static void(*KDGndColBlock_remove)(u32 kBlock);
			//
			static void (*RacePageInitFunctions[RacePageInitID::AMOUNT])(void* baseRacePage);
			static void OnRacePageGenGP(void* racePage);
			static void OnRacePageGenTA(void* racePage);
			static void OnRacePageGenBBT(void* racePage);
			static void OnRacePageGenCBT(void* racePage);
			//
			static u8 numberPlayersRace;
			static bool userCanControlKart();
			static bool blockStartGridFormation();
			static int masterPlayerID;
			static int startedItemSlot;
			static int startedBoxID;
			static void (*ItemDirector_StartSlot)(u32 itemDirector, u32 playerID, u32 boxID);
			static void startItemSlot(u32 playerID, u32 boxID);
			static RT_HOOK itemDirectorStartSlotHook;
			static void OnItemDirectorStartSlot(u32 itemDirector, u32 playerID, u32 boxID);
			static u16 itemProbabilitiesForImprovedRoulette[EItemSlot::ITEM_SIZE];
			static int (*CsvParam_getDataInt)(void* csvObject, int row, int column);
			static void storeImprovedRouletteItemProbability(u32 itemID, u16 prob);
			static int nextForcedItem;
			static bool nextForcedInstantSlot;
			static u32 nextForcedItemBlockedFlags;
			static u16 handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag);
			static u32 pullRandomItemID();
			static const char* getOnlineItemTable(bool isAI);
			static const char* getOnlineBattleItemTable(bool isCoin, bool isAI);
			static void improvedRouletteSettings(MenuEntry* entry);
			//
			static void raceTimerOnFrame(MarioKartTimer timer);
			static void handleIncrementTimer(u32* timer);
			static void handleDecrementTimer(u32* timer);
			static void setTimerInitialValue(u32* timer);
			static bool pointControlShouldBeYellow(u32 val);
			//
			static std::vector<std::pair<u32, u32>> carRouteControllerPair;
			static u32 carGetController(u32 carObject);
			static void carStoreController(u32 carObject, u32 carController);
			static u32 penguinGeneratorGetGenNum(u32 generator, u32* penguinObject);
			//
			static u32 onKartHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 vehicleReactObject, u32 objCallFuncPtr);
			static u32 onItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 itemReactProxyObject, u32 objCallFuncPtr);
			static void OnKartAccident(u32* vehicleReact, ECrashType* type);
			static bool kartCanAccident(u32* vehicleReact);
			//
			static void(*SndActorKArt_PlayDriverVoice)(u32* sndActorKart, EVoiceType voice);
			static void OnSndActorKartPlayDriverVoice(u32* sndActorKart, EVoiceType voice);
			static MarioKartTimer loopVoiceCooldown[MAX_PLAYER_AMOUNT];
			static bool loopMasterWPSoundEffectPlayed;
			//
			static ImprovedTrickInfo trickInfos[MAX_PLAYER_AMOUNT];
			static bool improvedTricksAllowed;
			static bool improvedTricksForced;
			static void resetTrickInfos(int playerID, u32 kartUnitObj);
			static void (*kartStartTwist)(u32 kartVehicleMoveObj, u32 restartBool);
			static u32 onGliderTwistStart(u32* kartVehicleMoveObj, u32 restartBool);
			static float* onGliderTwistCalc(u32* kartVehicleObj, int callVal);
			static float onGliderTwistGetDecreaseAmount(u32* vehicleMove, float origAmount);
			static u32 onDashExecCallback(u32* kartVehicleMoveObj, u32 dashType, bool flag1, bool flag2);
			static void onKartCountFrameGround(u32* vehicleMoveObj);
			static void startJumpActionCallback(u32* vehicleMoveObj);
			static u32 getMyPlayerID();
			static u32 getVehicle(u32 playerID); // 0xD14 = last collosion ID, 0xD18 = last collision ID bitmap, 0xD20 = last collision subtype, 0x0xFE8 = collision is trickeable
			//
			static const u8 netVanillaPacketSignature[0x10];
			static u32 nexNetworkInstance;
			static u32(*nexKeyConstr)(u8 keyObject[0x18], const u8* signature, u32 signatureSize);
			static void(*keyedChecksumAlgSetKey)(u32 object, u32 keyRetObject);
			static void(*nexKeyDestr)(u8 keyObject[0x18]);
			static void setPacketSignature(const u8 signature[0x10]);
			static void generatePacketSignature(u8 outKey[0x10], u64 keySeed);
			//
			static void OnNetLoadPlayerSave(SavePlayerData* sv);
			static bool OnNetSavePlayerSave(u32 newVR);
			static EOnlineRaceMode OnHostSetClass(EOnlineRaceMode original);
			//
			static bool is3DEnabled;
			static bool kmpRenderOptimizationsForced;
			static void (*FrameBuffer_Bind)(u32 fb);
			static void (*TreeMngr_drawRenderTree)(u32 treeMngr);
			static void DrawGameHook(u32 gameFramework);
			static DepthBufferReaderVTable* bufferReaderVtable;
			static void (*bufferReaderRenderL)(u32 self);
			static void (*bufferReaderRenderR)(u32 self);
			//
			static std::vector<GOBJEntry*> dokanWarps;
			static u8 dokanWarpCooldown[MAX_PLAYER_AMOUNT];
			static u32 (*objectBaseGetRandom32)(u32 fieldObjectbase, u32 maxValue);
			static u32 (*vehicleMove_startDokanWarp)(u32 vehicleMove, u32 chosenPoint, Vector3* targetPosition, Vector3* kartDirection);
			static void OnHsAirCurrentSetReactionFromKart(u32 hsAirObject, u32 vehicleReact);
			static void OnUpdateNetRecvStartWarp(u32 vehicleMove, u32 randomChoice);
			static bool VehicleMove_DokanWarp(u32 vehicleMove, POTIRoute* route, u32 randomChoice);
			static void OnLapRankCheckerDisconnectCheck(u32* laprankchecker, LapRankCheckerKartInfo* kartinfo);
			static u32 countdownDisconnectTimer;
			static MarioKartTimer graceDokanPeriod;
			static std::vector<std::tuple<float, float, float, float, float>> bannedUltraShortcuts;
			static float masterPrevRaceCompletion;
			static int masterSuspectUltraShortcut;
			static void OnRRAsteroidInitObj(u32* rrAsteroid);
			//
			static void OnObjectBaseSetScale(u32* objectBase, Vector3& copyTo, const Vector3& copyFrom);
			//
			static void OnRacePauseEvent(bool pauseOpen);
			static void OnRaceStartCountdown();
			//
			static void GetTerrainParameters(u32 vehicleMove, u8 kclID);
			static void OnTireEffectGetCollision(u32 out[2], u32 vehicleMove);
			//
			static ObjectGeneratorBase* GenerateItemBox(CustomFieldObjectCreateArgs& createArgs);
			static void ObjModelBaseChangeAnimation(u32 objModelBase, int anim, float value);
			static void ObjModelBaseRotate(u32 objModelBase, u32 amount = 0xFF258C00);
			//
			static bool lastPolePositionLeft;
			static bool bulletBillAllowed;
			static bool speedupMusicOnLap;
			static u8 lastCheckedLap;
			static float raceMusicSpeedMultiplier;
			//
			static void OnRaceNextMenuLoadCallback();
			static void OnRaceNextMenuLoad();
			//
			static std::default_random_engine randomDriverChoicesGenerator;
			static std::vector<std::pair<u8, FixedStringBase<char16_t, 0x20>>> onlineBotPlayerIDs;
			static void AddBotPlayerOnline(int amount);
			static int GetValidStationIDAmount();
			static void OnWifiFinishLoadDefaultPlayerSetting(int playerID);
			static void OnCourseVotePagePreStep(u32 courseVotePage);
			static bool OnWifiLoadDriverShouldForceDefault(int playerID);
			//
			static float rubberBandingMultiplier;
			static float rubberBandingOffset;
			static float adjustRubberBandingSpeed(float initialAmount);
			//
			static u32 NetUtilStartWriteKartSendBufferAddr;
			static u32 NetUtilEndWriteKartSendBufferAddr;
			static void (*BaseResultBar_SetGrade)(MarioKartFramework::BaseResultBar, u32* grade);
			static void BaseResultBar_SetTeam(u32 resultbar, int teamMode); // 0 -> None, 1 -> Red, 2 -> Blue
			static void BaseResultBar_SetBarColor(u32 resultBar, int colorID);
			static void BaseResultBar_SetCTGPOnline(u32 resultBar);
			static void UpdateResultBars(u32 racePage);
			static void KartNetDataSend(u32* kartData, int playerID);
			static bool KartNetDataRead(u32* kartData, MK7NetworkBuffer* dataBuf, u32** kData1, u32** kData2);
			static void OnSendCustomKartData(int playerID, CustomCTGP7KartData& data);
			static void OnRecvCustomKartData(int playerID, CustomCTGP7KartData& data);
			//
			static u8 brakeDriftAllowFrames[MAX_PLAYER_AMOUNT];
			static bool brakeDriftAllowed;
			static bool brakeDriftForced;
			static bool brakeDriftingAllowed();
			static void checkCancelMiniturbo(u32 vehicleMove, float* driftCancelFactor);
			static int getMiniturboLevel(u32 vehicleMove);
			//
			static u32 playjumpeffectAddr;
			static u32 playcoineffectAddr;
			static u32 VehicleMove_StartPressAddr;
			static u32 VehicleReact_ReactPressMapObjAddr;
			static u32 VehicleReact_ReactAccidentCommonAddr;
			static void handleThunderResize(u32* thunderdata, u32* staticdata, u32* vehicleMove, float* currSize);
			static void startSizeChangeAnimation(int playerID, float targetSize, bool isGrowing);
			static ResizeInfo resizeInfos[MAX_PLAYER_AMOUNT];
			static int megaMushTimers[MAX_PLAYER_AMOUNT];
			static int packunStunCooldownTimers[MAX_PLAYER_AMOUNT];
			static void ReactPressMegaMush(u32* vehicleMove, int causingPlayerID);
			static bool calculatePressReact(u32* vehicleMove1, u32* vehicleMove2, int someValueSP);
			static u32* sndActorKartCalcEnemyStar(u32* sndActorKart);
			static SndLfoSin pitchCalculators[MAX_PLAYER_AMOUNT];
			static void OnSndActorKartCalcInner(u32* sndActorKart);
			static bool OnThunderAttackKart(u32* vehicleMove);
			static void OnStartKiller(u32* vehicleMove);
			static bool ignoreKartAccident;
			static bool OnReactAccidentCommonShouldIgnore(u32* vehicleMove);
			static u32 SndActorBase_SetCullingSafeDistVolRatioAddr;
			static void SndActorKartSetStarState(u32* sndActorKart, u8 modeBits);
			static void OnVehicleMoveInit(u32* vehicleMove);
			static float playerMegaCustomFov;
			static void applyKartCameraParams(u32* kartCamera, float* cameraparams);
			//
			static void OnTrophySelectNextScene(u32* lastCup);
			static bool thankYouDisableNintendoLogo;
			static bool OnThankyouPageInitControl();
			static RT_HOOK OnSimpleModelManagerHook;
			static u32 OnSimpleModelManager(u32 own);
			//
			static inline int vehicleIsInLoopKCL(u32 vehicle) { // 0 - No loop, 1 - anti-grav, 2 - loop
				if (*(u32*)(vehicle + 0xD14) == 8) {
					if (*(u32*)(vehicle + 0xD20) == 1)
						return 1;
					else if (*(u32*)(vehicle + 0xD20) == 2)
						return 2;
				}
				return 0;
			}
			static bool vehicleForceAntigravityCamera(u32 vehicle);
			static bool vehicleDisableSteepWallPushback(u32 vehicle);
			static bool vehicleDisableUpsideDownFloorUnstick(u32 vehicle, float* gravityAttenuator);
			static void OnKartGravityApply(u32 vehicle, Vector3& gravity);
			static float ApplyGndTurningSpeedPenaly(u32 vehicle, float speed, float penaltyFactor);
			static bool automaticDelayDriftAllowed;
			//
			static RT_HOOK enemyAIControlRaceUpdateHook;
			static u8 onMasterStartKillerPosition;
			static void OnEnemyAIControlRaceUpdate(u32 enemyAI);
			//
			static void* OnLoadResGraphicFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs);
        	static RT_HOOK loadResGraphicFileHook;
			//
			static u64 rotateCharacterID;
			static u64 syncJumpVoiceCharacterID;
			static u8 characterRotateAmount[MAX_PLAYER_AMOUNT];
			static void OnKartUnitCalcDrawKartOn(u32 kartUnit, float* transformMatrix);
			//
			static RT_HOOK jugemSwitchReverseUpdateHook;
			static void OnJugemSwitchReverseUpdate(u32 switchReverse);
			//
			static u32 SndActorKartDecideBoostSndID(u32 sndActorKart, u32 courseID);
			//
			static u32 AdjustVRIncrement(u32 playerID, s32 vr, s32 vrIncrement, u32 modemanagerbase);
			static RT_HOOK calcRateForLoseHook;
			static u32 OnCalcRateForLose(u32 modemanagerbase, u32 playerID);
			static RT_HOOK baseRacePageEnterHook;
			static void OnBaseRacePageEnter(u32 baseracepage, u32 fadekind, u32 unk);
			static RT_HOOK baseRacePageCalcSaveHook;
			static void OnBaseRacePageCalcSave(u32 baseracepage);
			static RT_HOOK baseRacePageCalcPointHook;
			static void OnBaseRacePageCalcPoint(u32 baseracepage);
			//
			static bool OnRacePauseAllow(bool gamePauseAllowed);
			//
			static bool useCustomItemProbability;
			static bool customItemProbabilityRandom;
			static std::array<bool, EItemSlot::ITEM_SIZE> customItemProbabilityAllowed;
			static u16 handleCustomItemProbabilityRecursive(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag, int recursionMode);
			static void UseCustomItemMode(const std::array<bool, EItemSlot::ITEM_SIZE>& allowedItems, bool randomItems);
			static void ClearCustomItemMode();
			//
			static RT_HOOK vehicleReactExecPostCrashHook;
			static void OnVehicleReactExecPostCrash(u32* vehicleReact);
			//
			static bool OnVehicleDecideSurfaceTrickable(u32* vehicle, bool isTrickable);
			//
			static RT_HOOK raceDirectorCreateBeforeStructureHook;
			static void OnRaceDirectorCreateBeforeStructure(u32* raceDirector, u32* arg);
			//
			static RT_HOOK sequenceSubGoalKartHook;
			static void OnSequenceSubGoalKart(int playerID);
			static bool kartReachedGoal[MAX_PLAYER_AMOUNT];
			//
			static RT_HOOK lapRankCheckerCalcHook;
			static void OnLapRankCheckerCalc(u32* laprankchecker);
			//
			static RT_HOOK racePageGenResultHook;
			static void OnRacePageGenResult(u32 racePage);
			static void(*RacePage_GenResultWifi)(u32);
			//
			static RT_HOOK networkBufferControllerAllocHook;
			static u32* originalNetworkBufferSizes;
			static void OnNetworkBufferControllerAllocBuf(MK7::Net::NetworkBufferController* networkbuffercontroller);
			static void OnNetworkSelectMenuSendUpdateCore(u32 NetworkDataManager_SelectMenu);
			static RT_HOOK networkSelectMenuReceivedCoreHook;
			static bool OnNetworkSelectMenuReceivedCore(u32 NetworkSelectMenuReceived, MK7::Net::NetworkReceivedInfo* receive_info);
			static bool netImHost(MK7::Net::NetworkEngine* netEngine = nullptr);
			//
			static bool disableLensFlare;
			static bool LensFlareAllowed();
			//
			static bool hasOOBAreas;
			static void CalcOOBArea(u32 vehicle);
    };
    bool checkCompTID(u64 tid);
    u32 SafeRead32(u32 addr);
}

enum class CTMode : u32 {
	OFFLINE = 0,
	ONLINE_NOCTWW = 1,
	ONLINE_CTWW = 2,
	ONLINE_CTWW_CD = 3,
	INVALID = 0xFFFFFFFF
};

extern "C" CTMode g_setCTModeVal;
extern "C" CTMode g_getCTModeVal;

extern "C" void g_updateCTMode();