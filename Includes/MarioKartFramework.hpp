/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartFramework.hpp
Open source lines: 729/729 (100.00%)
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
#include "rt.hpp"

#define COMMUSETVER 6

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
	enum EDashType {
		MUSHROOM = (1 << 0),
		BOOST_PAD = (1 << 1),
		LAKITU_RECOVERY = (1 << 2),
		MINITURBO = (1 << 3),
		SUPERMINITURBO = (1 << 4),
		START_VERYSMALL = (1 << 5),
		START_SMALL = (1 << 6),
		START_MEDIUM = (1 << 7),
		START_BIG = (1 << 8),
		START_PERFECT = (1 << 9),
		TRICK_GROUND = (1 << 10),
		TRICK_WATER_DIVE = (1 << 11),
		TRICK_WATER = (1 << 12),
		STAR_RING = (1 << 13),
		COIN_GRAB = (1 << 14),
		WATER_DIVE = (1 << 15)
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
		GLIDER_FLY = 10,
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

    struct OnlineSettings
    {
        u8  ver{0};
        u16 speedandcount{0};
        u32 enabledTracks{0};
        u8  checksum{0};
    } PACKED;

	struct OnlineSettingsv2
	{
		u8  ver;
		u16 rounds : 2;
		u16 speed : 14;
		u32	isBackcamAllowed : 1;
		u32 isLEDItemsAllowed : 1;
		u32 areRandomTracksForced : 1;
		u32 areOrigTracksAllowed : 1;
		u32 areCustomTracksAllowed : 1;
		u32 cpuRacers : 1;
		u32 improvedTricksAllowed : 1;
		u32 customItemsAllowed : 1;
		u32 unused : 24;
		u8  checksum;
	} PACKED;

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
            static std::string onlineCode;
            static int getCodeByChar(char c, u8 pos);
            static u64 decodeFromStr(std::u16string &s);
            static char getCharByCode(u8 num, u8 pos);
            static u32 oldStrptr;
			static u8 previousCDScore;

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
			static u32 baseAllPointerXor;
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
			static u32 getKartDirector();
			static u32 getRaceDirector();
			static u32 getGarageDirector();
			static u32 getEffectDirector();
			static u32 getModeManagerBase();
			static u32 getBaseRacePage();
			static u32 getMenuData();
			static ModeManagerData* getModeManagerData();
			static CRaceInfo* getRaceInfo(bool global);
			static FixedStringBase<u16, 0x20>* getPlayerNames();
			static KartLapInfo* getKartLapInfo(u32 playerID);
			static Vector3* GetGameCameraPosition();
			static void BasePageSetCup(u8 cupID);
			static u32 BasePageGetCup();

			static CRaceMode currentRaceMode;
			static void (*BasePage_SetRaceMode)(CRaceMode* mode);
			static EPlayerInfo playerInfos[8];
			static u32 currGatheringID;
			static bool imRoomHost;
			static bool allowOpenCTRPFMenu();
			static bool allowTakeScreenshot();
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
			//

            static void getLaunchInfo();
            static bool isCompatible(); //true success, false fail
            static bool isGameInRace(); // true in race, false not in race
			static void onRaceEvent(u32 raceEventID);

			static void changeFilePath(u16* dst, bool isDir);
            static void encodeFromVal(char out[14], u64 in);
            static u8 getOnlinechecksum(OnlineSettingsv2* onlineset);
            static void applycommsettings(u32* commdescptr);
            static void changeNumberRounds(u32 newval);
            static void loadCustomSetOnline(OnlineSettings& onlineset);
            static void restoreComTextPtr(u32* strptr = nullptr);
            static void restoreViewTextPtr(u32* strptr = nullptr);
            static u16* commDisplayText;
            static u32* oldStrloc;
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
			static u32 handleBackwardsCamera(u32 pad);
			static bool allowBackwardsCamera();
			static void kartCameraSmooth(u32* camera, float smoothVal);
			static bool playCountDownCameraAnim;
			static bool startedRaceScene;
			static bool isBackCamBlockedComm;
			static bool isWarnItemBlockedComm;
            //
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
			//
			static void dialogBlackOut(const std::string& message = "");
			static bool onNetworkErrorCheckCalc();
			static u32 onNetworkErrorCheckWindowOpen();
			//
			static void setCTWWState(bool enabled);
			static void adjustVisualElementsCTWW(bool enabled);
			static bool applyVisualElementsCTWWOSD(const Screen& screen);
			static bool toggleCTWWHandler(const Screen& screen);
			static u32 onOnlineModePreStep(u32 mode);
			static void onOnlineModeButOK(u32 btn);
			static void onOnlineSubModePreStep();
			static void onOnlineSubModeButtonOK();
			static void (*nwlytReplaceText)(u32 a0, u32 a1, u32 msgPtr, u32 a3, u32 a4);
			//
			static int handleBootSequenceCallback();
			static void OnBootTaskFinish();
			//
			static bool needsOnlineCleanup;
			static void DoOnlineCleanup();
			static void OnInternetConnection();
			static void handleTitleEnterCallback();
			static void handleTitleCompleteCallback(u32 option);
			static u32 handleTitleMenuPagePreStep(u32 timerVal);
			//
			static void updateReplayFileName();
			//
			static void warnLedItem(u32 item);
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
			static bool isWatchRaceMode;
			static bool allowCPURacersComm;
			static bool allowCustomItemsComm;
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
			static BaseResultBar resultBarArray[8];
			static bool resultBarHideRank;
			static void(*BaseResultBar_setName)(BaseResultBar bar, u16** name);
			//static void(*BaseResultBar_setCharaTex)(BaseResultBar bar, u32* driverID, int driverSkin, bool unknown);
			static void(*BaseResultBar_setPoint)(BaseResultBar bar, u32 point);
			static void(*BaseResultBar_addPoint)(BaseResultBar bar, u32 point);
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
			static u16 itemProbabilities[EItemSlot::ITEM_SIZE];
			static int (*CsvParam_getDataInt)(void* csvObject, int row, int column);
			static void storeItemProbability(u32 itemID, u16 prob);
			static int nextForcedItem;
			static u16 handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag);
			static u32 pullRandomItemID();
			static const char* getOnlineItemTable(bool isAI);
			static const char* getOnlineBattleItemTable(bool isCoin, bool isAI);
			static void improvedRouletteSettings(MenuEntry* entry);
			//
			static void raceTimerOnFrame();
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
			static void(*SndActorKArt_PlayDriverVoice)(u32 sndActorKartObj, EVoiceType voice);
			//
			static ImprovedTrickInfo trickInfos[8];
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
			static u32 getVehicle(u32 playerID); // 0xD14 = last collosion ID, 0xD18 = last collision ID bitmap, 0xD20 = last collision subtype
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
			static u32 (*objectBaseGetRandom32)(u32 fieldObjectbase, u32 maxValue);
			static u32 (*vehicleMove_startDokanWarp)(u32 vehicleMove, u32 chosenPoint, Vector3* targetPosition, Vector3* kartDirection);
			static void OnHsAirCurrentSetReactionFromKart(u32 hsAirObject, u32 vehicleReact);
			static void OnUpdateNetRecvStartWarp(u32 vehicleMove, u32 randomChoice);
			static bool VehicleMove_DokanWarp(u32 vehicleMove, POTIRoute* route, u32 randomChoice);
			static void OnLapRankCheckerDisconnectCheck(u32* laprankchecker, u32* kartinfo);
			static u32 countdownDisconnectTimer;
			static MarioKartTimer graceDokanPeriod;
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
			//
			static bool bulletBillAllowed;
			static bool speedupMusicOnLap;
			static u8 lastCheckedLap;
			static float raceMusicSpeedMultiplier;
			//
			static void OnRaceNextMenuLoadCallback();
			static void OnRaceNextMenuLoad();
			//
			static std::default_random_engine randomDriverChoicesGenerator;
			static std::vector<std::pair<u8, FixedStringBase<u16, 0x20>>> onlineBotPlayerIDs;
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
			static bool onlinePlayersRunningCTGP[8];
			static void BaseResultBar_SetTeam(u32 resultbar, int teamMode); // 0 -> None, 1 -> Red, 2 -> Blue
			static void BaseResultBar_SetCTGPOnline(u32 resultBar);
			static void UpdateResultBars(u32 racePage);
			static void KartNetDataSend(u32* kartData, int playerID);
			static bool KartNetDataRead(u32* kartData, MK7NetworkBuffer* dataBuf, u32** kData1, u32** kData2);
			static void OnSendCustomKartData(int playerID, CustomCTGP7KartData& data);
			static void OnRecvCustomKartData(int playerID, CustomCTGP7KartData& data);
			//
			static u8 brakeDriftAllowFrames[8];
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
			static void handleThunderResize(u32* thunderdata, u32* staticdata, u32* vehicleMove, float* currSize);
			static void startSizeChangeAnimation(int playerID, float targetSize, bool isGrowing);
			static ResizeInfo resizeInfos[8];
			static int megaMushTimers[8];
			static bool calculatePressReact(u32* vehicleMove1, u32* vehicleMove2, int someValueSP);
			static u32* sndActorKartCalcEnemyStar(u32* sndActorKart);
			static SndLfoSin pitchCalculators[8];
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
    };
    bool checkCompTID(u64 tid);
    u32 SafeRead32(u32 addr);
}

enum CTMode {
	OFFLINE = 0,
	ONLINE_NOCTWW = 1,
	ONLINE_COM = 2,
	ONLINE_CTWW = 3,
	ONLINE_CTWW_CD = 4,
	MAXVAL = 0xFFFFFFFF
};

extern "C" CTMode g_setCTModeVal;
extern "C" CTMode g_getCTModeVal;

extern "C" void g_updateCTMode();