/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartFramework.cpp
Open source lines: 4123/4271 (96.53%)
*****************************************************/

#include "MarioKartFramework.hpp"
#include "main.hpp"
#include "entrystructs.hpp"
#include "3ds.h"
#include "CourseManager.hpp"
#include "OnionFS.hpp"
#include "Sound.hpp"
#include "rt.hpp"
#include "plgldr.h"
#include "HookInit.hpp"
#include "SaveHandler.hpp"
#include "StatsHandler.hpp"
#include "Net.hpp"
#include "VersusHandler.hpp"
#include "ExtraResource.hpp"
#include "CrashReport.hpp"
#include "MissionHandler.hpp"
#include "SequenceHandler.hpp"
#include "MarioKartTimer.hpp"
#include "foolsday.hpp"
#include "UserCTHandler.hpp"
#include "ExtraUIElements.hpp"
#include "CustomTextEntries.hpp"
#include "MenuPage.hpp"
#include "str16utils.hpp"
#include "VisualControl.hpp"
#include "CharacterHandler.hpp"
#include "BlueCoinChallenge.hpp"
#include "PointsModeHandler.hpp"
#include "VoiceChatHandler.hpp"
#include "Unicode.h"
#include "Stresser.hpp"
#include "AsyncRunner.hpp"
#include "BadgeManager.hpp"

extern "C" u32 g_gameModeID;
extern "C" u32 g_altGameModeIsRaceOver;
extern "C" u32 g_altGameModeplayerStatPointer;
extern "C" u32 g_altGameModeHurryUp;
extern "C" u32 g_downTimerStartTime;
extern "C" u32 g_lastChosenTrickVoiceVariant;

namespace CTRPluginFramework {
	extern "C" void drawMdlReplaceTextureRefThunk(void* cgfxRes, SafeStringBase* dst, SafeStringBase* src, void* funcptr);

	u32 SafeStringBase::vTableSafeStringBase = 0;

	u32 MarioKartFramework::baseAllPointer = 0;
	MarioKartFramework::CRaceMode MarioKartFramework::currentRaceMode;
	void (*MarioKartFramework::BasePage_SetRaceMode)(MarioKartFramework::CRaceMode* mode) = nullptr;
	MarioKartFramework::EPlayerInfo MarioKartFramework::playerInfos[MAX_PLAYER_AMOUNT];
	u32 MarioKartFramework::currGatheringID;
	u8 MarioKartFramework::myRoomPlayerID = -1;
	u8* MarioKartFramework::_currgamemode = nullptr;
	char* MarioKartFramework::replayFileName = nullptr;
	u32 MarioKartFramework::ctgp7ver = SYSTEM_VERSION(0, 0, 0);
	u32 MarioKartFramework::region = 0;
	u32 MarioKartFramework::revision = 0;
	u32 MarioKartFramework::lastLoadedMenu = 0;
	u8* MarioKartFramework::cameraKartMode = 0;
	u32* MarioKartFramework::currNumberTracksPtr = nullptr;
	u32* MarioKartFramework::currCommNumberTracksPtr = nullptr;
	u32* MarioKartFramework::worldWideButtonPtr = nullptr;
	u32* MarioKartFramework::battleWideButtonPtr = nullptr;
	u32 MarioKartFramework::battleWideMessageReplacePtr[2] = { 0 };
	RGBLedPattern MarioKartFramework::ledBlueShellPat = { 0 };
	RGBLedPattern MarioKartFramework::ledThunderPat = { 0 };
	RGBLedPattern MarioKartFramework::ledBlooperPat = { 0 };
	RGBLedPattern MarioKartFramework::ledFinalRainbowPat = { 0 };
	u32* (*MarioKartFramework::getSaveManagerFuncptr)(void) = nullptr;
	void (*MarioKartFramework::getMyPlayerDatafuncptr)(u32 systemEngine, MarioKartFramework::SavePlayerData* data, bool something);
	STGIEntry** (*MarioKartFramework::getStageInfoFuncptr)(void) = nullptr;
	GOBJAccessor* (*MarioKartFramework::getGeoObjAccessor)(void) = nullptr;
	POTIAccessor* (*MarioKartFramework::getPathAccessor)(void) = nullptr;
	float* MarioKartFramework::localMapPtr = nullptr;
	bool MarioKartFramework::isRaceState = false;
	bool MarioKartFramework::isRacePaused = false;
	bool MarioKartFramework::isRaceGoal = false;
	bool MarioKartFramework::isLoadingAward = false;
	u32 MarioKartFramework::origChangeMapInst = 0xE320F000;
	u32* MarioKartFramework::changeMapCallPtr = nullptr;
	u64 MarioKartFramework::kouraGProbability = 1;
	u64 MarioKartFramework::kouraRProbability = 1;
	u8 MarioKartFramework::previousCDScore = 0;
	void* MarioKartFramework::gobjrelocptr = nullptr;
	void (*MarioKartFramework::openDialogImpl)(DialogFlags::Mode mode, int messageID, void* args) = nullptr;
	bool (*MarioKartFramework::isDialogClosedImpl)() = nullptr;
	bool (*MarioKartFramework::isDialogYesImpl)() = nullptr;
	void (*MarioKartFramework::closeDialogImpl)() = nullptr;
	int MarioKartFramework::playTitleFlagOpenTimer = 0;
	void (*MarioKartFramework::nwlytReplaceText)(u32 a0, u32 a1, u32 msgPtr, u32 a3, u32 a4) = nullptr;
	bool MarioKartFramework::pendingVoiceChatInit = false;
	bool MarioKartFramework::needsOnlineCleanup = false;
	bool MarioKartFramework::playCountDownCameraAnim = false;
	bool MarioKartFramework::startedRaceScene = false;
	std::vector<std::tuple<u32, void(*)(u32*)>> MarioKartFramework::regButtons;
	bool MarioKartFramework::wasKeyInjected = false;
	Key MarioKartFramework::injectedKey = (Key)0;
	bool MarioKartFramework::areKeysBlocked = false;
	bool MarioKartFramework::isPauseBlocked = false;
	bool MarioKartFramework::isPauseAllowForced = false;
	std::tuple<u32, u32*> MarioKartFramework::soundThreadsInfo[2] = { std::tuple<u32, u32*>(0xFFFFFFFF, nullptr), std::tuple<u32, u32*>(0xFFFFFFFF, nullptr) };
	bool MarioKartFramework::forceDisableSndOnPause = false;
	void (*MarioKartFramework::BaseMenuPageApplySetting_CPU)(int cpuAmount, int startingCPUIndex, int* playerChar) = nullptr;
	void (*MarioKartFramework::SequenceCorrectAIInfo)() = nullptr;
	bool MarioKartFramework::isWatchRaceMode = false;
	u32 MarioKartFramework::cpuRandomSeed = 1;
	u32 MarioKartFramework::neededCPU = 0;
	u32 MarioKartFramework::neededCPUCurrentPlayers = 0;
	u8 MarioKartFramework::randomdriverchoices[16] = { 0 };
	bool MarioKartFramework::israndomdriverchoicesshuffled = true;
	u8* (*MarioKartFramework::networkGetPlayerStatus)(void* networkmenproc, int playerID, u8 copy[0x9C]) = nullptr;

	void (*MarioKartFramework::BasePage_SetDriver)(int slot, s32* driverID, u32* playerType) = nullptr;
	void (*MarioKartFramework::BasePage_SetParts)(int slot, s32* bodyID, s32* tireID, s32* wingID, u32* screwID) = nullptr;
	void (*MarioKartFramework::BasePage_UpdateRaceInfo)() = nullptr;
	bool MarioKartFramework::forceFinishRace = false;
	bool MarioKartFramework::skipGPCoursePreview = false;
	bool MarioKartFramework::allowWigglerAngry = true;
	s8 MarioKartFramework::forcedResultBarAmount = -1;
	u8 MarioKartFramework::realResultBarAmount = 8;
	MarioKartFramework::BaseResultBar MarioKartFramework::resultBarArray[MAX_PLAYER_AMOUNT] = { 0 };
	bool MarioKartFramework::resultBarHideRank = false;
	void (*MarioKartFramework::BaseResultBar_setName)(MarioKartFramework::BaseResultBar, u16**) = nullptr;
	//void (*MarioKartFramework::BaseResultBar_setCharaTex)(BaseResultBar, u32*, int, bool) = (void(*)(u32, u32*, int, bool))0x001525D8;
	void (*MarioKartFramework::BaseResultBar_setPoint)(MarioKartFramework::BaseResultBar, u32) = nullptr;
	void (*MarioKartFramework::BaseResultBar_addPoint)(MarioKartFramework::BaseResultBar, u32) = nullptr;
	void (*MarioKartFramework::RacePage_genPause)(u32 racePage) = nullptr;
	void (*MarioKartFramework::RacePage_genReplayPause)(u32 racePage) = nullptr;
	void (*MarioKartFramework::DrawMdl_changeMatAnimation)(u32 drawmdl, int id, float value) = nullptr;
	void (*MarioKartFramework::KDGndColBlock_remove)(u32 kBlock) = nullptr;
	int MarioKartFramework::masterPlayerID = -1;
	int MarioKartFramework::startedItemSlot = -1;
	int MarioKartFramework::startedBoxID = -1;
	RT_HOOK MarioKartFramework::itemDirectorStartSlotHook = {0};
	void (*MarioKartFramework::ItemDirector_StartSlot)(u32 itemDirector, u32 playerID, u32 boxID) = nullptr;
	u16 MarioKartFramework::itemProbabilitiesForImprovedRoulette[EItemSlot::ITEM_SIZE] = { 0 };
	int (*MarioKartFramework::CsvParam_getDataInt)(void*, int, int) = nullptr;
	int MarioKartFramework::nextForcedItem = -1;
	bool MarioKartFramework::nextForcedInstantSlot = false;
	u32 MarioKartFramework::nextForcedItemBlockedFlags = 0;
	void (*MarioKartFramework::SndActorKArt_PlayDriverVoice)(u32* sndActorKart, EVoiceType voice) = nullptr;
	void (*MarioKartFramework::RacePageInitFunctions[RacePageInitID::AMOUNT])(void* baseRacePage) = {nullptr};
	u8 MarioKartFramework::numberPlayersRace = 0;
	std::vector<std::pair<u32, u32>> MarioKartFramework::carRouteControllerPair;
	void (*MarioKartFramework::kartStartTwist)(u32 kartVehicleMoveObj, u32 restartBool) = nullptr;
	MarioKartTimer MarioKartFramework::loopVoiceCooldown[MAX_PLAYER_AMOUNT];
	bool MarioKartFramework::loopMasterWPSoundEffectPlayed;
	MarioKartFramework::ImprovedTrickInfo MarioKartFramework::trickInfos[MAX_PLAYER_AMOUNT];
	bool MarioKartFramework::improvedTricksAllowed = true;
	bool MarioKartFramework::improvedTricksForced = false;
	u32 MarioKartFramework::nexNetworkInstance = 0;
	u32 (*MarioKartFramework::nexKeyConstr)(u8 keyObject[0x18], const u8* signature, u32 signatureSize) = nullptr;
	void (*MarioKartFramework::keyedChecksumAlgSetKey)(u32 object, u32 keyRetObject) = nullptr;
	void (*MarioKartFramework::nexKeyDestr)(u8 keyObject[0x18]) = nullptr;
	bool MarioKartFramework::is3DEnabled = true;
	bool MarioKartFramework::kmpRenderOptimizationsForced = false;
	void (*MarioKartFramework::FrameBuffer_Bind)(u32 fb) = nullptr;
	void (*MarioKartFramework::TreeMngr_drawRenderTree)(u32 treeMngr) = nullptr;
	DepthBufferReaderVTable* MarioKartFramework::bufferReaderVtable = nullptr;
	void (*MarioKartFramework::bufferReaderRenderL)(u32 self) = nullptr;
	void (*MarioKartFramework::bufferReaderRenderR)(u32 self) = nullptr;
	std::vector<GOBJEntry*> MarioKartFramework::dokanWarps;
	u8 MarioKartFramework::dokanWarpCooldown[MAX_PLAYER_AMOUNT] = {0};
	u32 (*MarioKartFramework::objectBaseGetRandom32)(u32 fieldObjectbase, u32 maxValue) = nullptr;
	u32 (*MarioKartFramework::vehicleMove_startDokanWarp)(u32 vehicleMove, u32 chosenPoint, Vector3* targetPosition, Vector3* kartDirection) = nullptr;
	u32 MarioKartFramework::countdownDisconnectTimer;
	MarioKartTimer MarioKartFramework::graceDokanPeriod;
	std::vector<std::tuple<float, float, float, float, float>> MarioKartFramework::bannedUltraShortcuts;
	float MarioKartFramework::masterPrevRaceCompletion = 0.f;
	int MarioKartFramework::masterSuspectUltraShortcut = -1;
	bool MarioKartFramework::lastPolePositionLeft = false;
	bool MarioKartFramework::bulletBillAllowed = true;
	bool MarioKartFramework::speedupMusicOnLap = false;
	u8 MarioKartFramework::lastCheckedLap = 1;
	float MarioKartFramework::raceMusicSpeedMultiplier = 1.f;
	std::default_random_engine MarioKartFramework::randomDriverChoicesGenerator(1);
	std::vector<std::pair<u8, FixedStringBase<char16_t, 0x20>>> MarioKartFramework::onlineBotPlayerIDs;
	float MarioKartFramework::rubberBandingMultiplier = 1.f; // hard 1.75 and 0.15
	float MarioKartFramework::rubberBandingOffset = 0.0f;
	u32 MarioKartFramework::NetUtilStartWriteKartSendBufferAddr = 0;
	u32 MarioKartFramework::NetUtilEndWriteKartSendBufferAddr = 0;
	void (*MarioKartFramework::BaseResultBar_SetGrade)(MarioKartFramework::BaseResultBar, u32* grade) = nullptr;
	u8 MarioKartFramework::brakeDriftAllowFrames[MAX_PLAYER_AMOUNT];
	bool MarioKartFramework::brakeDriftAllowed = true;
	bool MarioKartFramework::brakeDriftForced = false;
	u32 MarioKartFramework::playjumpeffectAddr = 0;
	u32 MarioKartFramework::playcoineffectAddr = 0;
	u32 MarioKartFramework::VehicleMove_StartPressAddr = 0;
	u32 MarioKartFramework::VehicleReact_ReactPressMapObjAddr = 0;
	u32 MarioKartFramework::VehicleReact_ReactAccidentCommonAddr = 0;
	ResizeInfo MarioKartFramework::resizeInfos[MAX_PLAYER_AMOUNT];
	int MarioKartFramework::megaMushTimers[MAX_PLAYER_AMOUNT] = { 0 };
	int MarioKartFramework::packunStunCooldownTimers[MAX_PLAYER_AMOUNT] = { 0 };
	SndLfoSin MarioKartFramework::pitchCalculators[MAX_PLAYER_AMOUNT];
	bool MarioKartFramework::ignoreKartAccident = false;
	u32 MarioKartFramework::SndActorBase_SetCullingSafeDistVolRatioAddr = 0;
	float MarioKartFramework::playerMegaCustomFov = 0.f;
	bool MarioKartFramework::thankYouDisableNintendoLogo = false;
	RT_HOOK MarioKartFramework::OnSimpleModelManagerHook = { 0 };
	bool MarioKartFramework::automaticDelayDriftAllowed = true;
	RT_HOOK MarioKartFramework::enemyAIControlRaceUpdateHook = { 0 };
	u8 MarioKartFramework::onMasterStartKillerPosition;
	RT_HOOK MarioKartFramework::loadResGraphicFileHook = { 0 };
	u64 MarioKartFramework::rotateCharacterID = 0;
	u64 MarioKartFramework::syncJumpVoiceCharacterID = 0;
	u8 MarioKartFramework::characterRotateAmount[MAX_PLAYER_AMOUNT];
	RT_HOOK MarioKartFramework::jugemSwitchReverseUpdateHook = {0};
	RT_HOOK MarioKartFramework::calcRateForLoseHook = {0};
	RT_HOOK MarioKartFramework::baseRacePageEnterHook = {0};
	RT_HOOK MarioKartFramework::baseRacePageCalcSaveHook = {0};
	RT_HOOK MarioKartFramework::baseRacePageCalcPointHook = {0};
	bool MarioKartFramework::useCustomItemProbability = false;
	bool MarioKartFramework::customItemProbabilityRandom = false;
	std::array<bool, EItemSlot::ITEM_SIZE> MarioKartFramework::customItemProbabilityAllowed = {0};
	RT_HOOK MarioKartFramework::vehicleReactExecPostCrashHook;
	RT_HOOK MarioKartFramework::raceDirectorCreateBeforeStructureHook = {0};
	RT_HOOK MarioKartFramework::sequenceSubGoalKartHook = {0};
	bool MarioKartFramework::kartReachedGoal[MAX_PLAYER_AMOUNT] = {0};
	RT_HOOK MarioKartFramework::lapRankCheckerCalcHook;
	RT_HOOK MarioKartFramework::racePageGenResultHook;
	void (*MarioKartFramework::RacePage_GenResultWifi)(u32) = nullptr;
	RT_HOOK MarioKartFramework::networkBufferControllerAllocHook = {0};
	u32* MarioKartFramework::originalNetworkBufferSizes = nullptr;
	RT_HOOK MarioKartFramework::networkSelectMenuReceivedCoreHook = {0};
	bool MarioKartFramework::disableLensFlare = false;
	bool MarioKartFramework::hasOOBAreas = false;

	extern "C" u32 myPlayerIDValue;

	extern bool g_ComForcePtrRestore;

	bool checkCompTID(u64 tid) {
		switch(tid) {
			case 0x0004000000030700:
				return true;
			case 0x0004000000030800:
				return true;
			case 0x0004000000030600:
				return true;
			case 0x0004000000030A00:
				return true;
			default:
				return false;
		}
	}

	void MarioKartFramework::getLaunchInfo()
	{
		u32 ret;
		resetdriverchoices();
		FwkSettings &settings = FwkSettings::Get();
	}

	bool MarioKartFramework::isCompatible() {
		if (revision && region) {
			return true;
		}
		return false;
	}

	static bool g_patchedBcsar = false;
	static u8 g_randomChoiceTitleAlt = 0;
	void MarioKartFramework::changeFilePath(char16_t* dst, bool isDir) {
		if (!isDir && !g_patchedBcsar && strfind16(dst, u".bcsar")) {
			CrashReport::stateID = CrashReport::StateID::STATE_MAIN;
			g_patchedBcsar = true;
			OnionFS::initGameFsFileMap();
		}
		if (strfind16(dst, u"chive-")) {
			Language::FixRegionSpecificFile(dst);
			return;
		}
		if (strfind16(dst, u"UI/") && strfind16(dst + 12, u"-")) {

			Language::SZSID id = Language::NONE;
			if (strfind16(dst, u"I/m")) id = Language::MENU;
			else if (strfind16(dst, u"I/co")) id = Language::COMMON;
			else if (strfind16(dst, u"I/r")) id = Language::RACE;
			else if (strfind16(dst, u"I/th")) id = Language::THANKYOU;
			else if (strfind16(dst, u"I/tr")) id = Language::TROPHY;
			if (id != Language::NONE) {Language::GetLangSpecificFile(dst, id, strfind16(dst, u"Pat")); return;}
		}
		int coursePos;
		bool usingCustomCup = UserCTHandler::IsUsingCustomCup();
		if ((usingCustomCup || CourseManager::lastLoadedCourseID < BATTLETRACKLOWER) && strfind16(dst, u"Course/", &coursePos) && strfind16(dst, u".szs")) {
			if (UserCTHandler::IsUsingCustomCup() && CourseManager::lastLoadedCourseID != 0x26) UserCTHandler::GetCouseSZSPath(dst, strfind16(dst + ((coursePos >= 0) ? coursePos : 0), u"-"));
			else if (CourseManager::lastLoadedCourseID < BATTLETRACKLOWER) {dst[0] = '\0'; return;}
		}

		if (isDir) return;
		
		if (MissionHandler::isMissionMode && strfind16(dst, u"SE_I")) {MissionHandler::LoadCoursePreview(dst); return;}
		if (MenuPageHandler::MenuEndingPage::loadCTGPCredits && strfind16(dst, u"RM_STA")) {strcpy16(dst, "ram:/CTGP-7/gamefs/Sound/stream/STRM_STAFF_MOD.bcstm"); return;}
		if (strfind16(dst, u"RM_TI")) {
			if (!g_randomChoiceTitleAlt) {
				g_randomChoiceTitleAlt = (Utils::Random() & 1) + 1;
			}
			if (SaveHandler::saveData.GetPendingAchievementCount() != 0) {
				g_randomChoiceTitleAlt = 2;
			}
			if (SaveHandler::saveData.GetCompletedAchievementCount() != 0 && g_randomChoiceTitleAlt == 2) {
				strcpy16(dst, "ram:/CTGP-7/gamefs/Sound/stream/STRM_TITLE_ALT.bcstm"); return;
			}
		}
		if (CharacterHandler::customKartsEnabled && strfind16(dst, u"fs/Ka")) {
			std::u16string file(dst);
			file = u"ram:/CTGP-7/MyStuff/Karts/" + file.substr(19);
			strcpy16(dst, file.c_str());
			return;
		}
		if (strfind16(dst, u"GRP_C")) {
			if ((CourseManager::lastLoadedCourseID >= CUSTOMTRACKLOWER && CourseManager::lastLoadedCourseID <= CUSTOMTRACKUPPER) || CourseManager::lastLoadedCourseID == 0x26) {
				std::u16string file(u"ram:/CTGP-7/gamefs/Sound/extData/");
				std::u16string courseName;
				Utils::ConvertUTF8ToUTF16(courseName, CourseManager::getCourseData(CourseManager::lastLoadedCourseID)->name);
				file = file + courseName + u".bcgrp";
				strcpy16(dst, file.c_str());
				return;
			}
		}

		if (!strfind16(dst, u"RM_C")) return;
		// Only music files at this point.
		bool lastLap = strfind16(dst, u"F.");
		if (lastLap && g_isFoolActive) {
			playSirenJoke();
		}
		else if (lastLap && CourseManager::isRainbowTrack(CourseManager::lastLoadedCourseID)) {
			LED::PlayLEDPattern(MarioKartFramework::ledFinalRainbowPat, Seconds(2 * 3));
		}

#ifdef NOMUSIC
		strcpy16(dst, u"ram:/CTGP-7/resources/nomusic.bcstm");
		return;
#endif
		if (MissionHandler::isMissionMode && MissionHandler::LoadCustomMusic(dst, lastLap)) return;
		u32 courseIDforMusic = CourseManager::lastLoadedCourseID;
		if (UserCTHandler::IsUsingCustomCup()) courseIDforMusic = UserCTHandler::GetCurrentCourseOrigSlot();
		auto it = MusicSlotMngr::entryMap.find(courseIDforMusic);
		if (it != MusicSlotMngr::entryMap.end()) {
			std::u16string filestring((char16_t*)dst);
			filestring.resize(filestring.find(u"-7/") + 3);
			if ((*it).second.isMyStuff) {
				filestring.append(u"MyStuff/stream/STRM_C");
			}
			else {
				filestring.append(u"gamefs/Sound/stream/STRM_C");
			}
			char16_t tmpBuf[50];
			strcpy16(tmpBuf, (*it).second.musicFileName.data());
			filestring.append((char16_t*)tmpBuf);
			if (lastLap) {
				filestring.append(u"_F.bcstm");
			}
			else {
				filestring.append(u"_N.bcstm");
			}
			strcpy16(dst, filestring.data());
		}
		return;
	}

	/*static float* g_savedKartXPos[8] = {0};
	static int g_checkmode = 0;
	static int g_maxcheckframes = 10000;
	static int g_currcheckframe[8] = {0};*/

	void MarioKartFramework::onRaceEvent(u32 raceEventID)
	{
		// 0 -> Start countdown, 1 -> Start race, 2 -> Pause, 3 -> Unpause, 4 -> End race
		if (raceEventID == 2) 
			isRacePaused = true;
		if (raceEventID == 3)
			isRacePaused = false;
		if (raceEventID == 0 && startedRaceScene)
		{
			if (g_isFoolActive)
			{
				applyRaceJoke();
				areKeysBlocked = true;
			}
		}
		if (raceEventID == 1 && startedRaceScene)
		{
			if (PointsModeHandler::isPointsMode) PointsModeHandler::OnRaceStart(false);
			startedRaceScene = false;
		}
		if (raceEventID == 4 && userCanControlKart()) {
			if (userCanControlKart()) {
				isRaceGoal = true;
				MissionHandler::OnRaceFinish();
				if (!MissionHandler::isMissionMode) StatsHandler::OnCourseFinish();
				g_StresserUpdateMode(StressMode::MOVE_PRESS_A);
			}
			if (BlueCoinChallenge::coinSpawned) BlueCoinChallenge::DespawnCoin();
			if (g_isFoolActive) {
				playMemEraseJoke();
			}
			SaveHandler::SaveSettingsAll();
		}

		/*if (raceEventID == 4) g_checkmode = 3;*/
	}

	static bool g_isRosalinaMenuBlocked = 0;
	void MarioKartFramework::setRosalinaMenuBlock(bool blocked)
	{
		g_isRosalinaMenuBlocked = blocked;
	}
	bool MarioKartFramework::getRosalinaMenuBlock()
	{
		return g_isRosalinaMenuBlocked;
	}
	void MarioKartFramework::InitializeLedPatterns()
	{
		LED::GeneratePattern(ledBlueShellPat, Color::Blue, LED_PatType::WAVE_ASC, Seconds(0.5), Seconds(0), 1);
		LED::GeneratePattern(ledThunderPat, Color::Yellow, LED_PatType::DESCENDENT, Seconds(1), Seconds(1));
		LED::GeneratePattern(ledBlooperPat, Color::White, LED_PatType::WAVE_ASC, Seconds(0.25), Seconds(0), 1);
		LED::GeneratePattern(ledFinalRainbowPat, Color(170, 170, 170), LED_PatType::WAVE_DESC, Seconds(2), Seconds(0), 0x20, 0, 2.0 / 3, 1.0 / 3);
	}

	u32 MarioKartFramework::getSequenceEngine()
	{
		u32 ptr2 = *(u32*)(baseAllPointer + 0x10) + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x38);
		if (!isValidPtr) return 0;
		u32 sequenceEngine = *(u32*)(ptr2 + 0x34) ^ baseAllPointerXor;
		return sequenceEngine;
	}
	u32 MarioKartFramework::getRootSequence()
	{
		u32 sequenceEngine = getSequenceEngine();
		u32 rootSequence = *(u32*)(sequenceEngine + 0x8);
		return *(u32*)rootSequence;
	}

	u32 MarioKartFramework::getSystemEngine()
	{
		u32 ptr2 = *(u32*)(baseAllPointer + 0x10) + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x2C);
		if (!isValidPtr) return 0;
		u32 systemengine = *(u32*)(ptr2 + 0x28) ^ baseAllPointerXor;
		return systemengine;
	}

	u32 MarioKartFramework::getSystemSaveData()
	{
		u32 engine = getSystemEngine();
		if (!engine) return 0;
		return ((u32*)engine)[0x28/4] + 0xC0;
	}

	u32 MarioKartFramework::getMiiEngine()
	{
		u32 ptr2 = *(u32*)(baseAllPointer + 0x10) + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x44);
		if (!isValidPtr) return 0;
		u32 miiengine = *(u32*)(ptr2 + 0x40) ^ baseAllPointerXor;
		return miiengine;
	}

	u32 MarioKartFramework::getNetworkEngine()
	{
		u32 ptr2 = *(u32*)(baseAllPointer + 0x10) + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x5C);
		if (!isValidPtr) return 0;
		u32 networkEngine = *(u32*)(ptr2 + 0x58) ^ baseAllPointerXor;
		return networkEngine;
	}

    MK7::Net::NetworkStationBufferManager *MarioKartFramework::getNetworkStationBufferManager()
    {
        return ((MK7::Net::NetworkEngine*)getNetworkEngine())->m_network_station_buffer_manager;
    }

    u32 __attribute__((hot)) MarioKartFramework::getRaceEngine()
	{
		u32 ptr2 = ((u32**)baseAllPointer)[1][1] + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x8);
		if (!isValidPtr) return 0;
		return *(u32*)(ptr2 + 0x4) ^ baseAllPointerXor;
	}

	u32 MarioKartFramework::getSndEngine()
	{
		u32 ptr2 = *(u32*)(baseAllPointer + 0x10) + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x50);
		if (!isValidPtr) return 0;
		u32 sndEngine = *(u32*)(ptr2 + 0x4C) ^ baseAllPointerXor;
		return sndEngine;
	}

	u32 MarioKartFramework::getCameraEngine()
	{
		u32 ptr2 = ((u32**)baseAllPointer)[1][1] + 0x1E0;
		bool isValidPtr = *(u8*)(ptr2 + 0x14);
		if (!isValidPtr) return 0;
		return *(u32*)(ptr2 + 0x10) ^ baseAllPointerXor;
	}

	u32 MarioKartFramework::getItemDirector()
	{
		return ((u32**)getRaceEngine())[0x1C/4][0x38/4];
	}

	u32 MarioKartFramework::getObjectDirector()
	{
		return ((u32**)getRaceEngine())[0x1C/4][0x18/4];
	}

	u32 MarioKartFramework::getKartDirector()
	{
		u32** raceEngine = (u32**)getRaceEngine();
		return raceEngine ? raceEngine[0x1C/4][0x10/4] : 0;
	}

	u32 MarioKartFramework::getRaceDirector()
	{
		return ((u32**)getRaceEngine())[0x1C/4][0x1C/4];
	}

	u32 MarioKartFramework::getGarageDirector()
	{
		return ((u32**)getRaceEngine())[0x1C/4][0x48/4];
	}

	u32 MarioKartFramework::getEffectDirector()
	{
		return ((u32**)getRaceEngine())[0x1C/4][0x2C/4];
	}

	u32 MarioKartFramework::getModeManagerBase()
	{
		return ((u32*)getRaceDirector())[0x1BC/4];
	}

	u32 MarioKartFramework::getMenuData()
	{
		return ((u32*)(getSequenceEngine()))[0xC0/4];
	}

	u32 MarioKartFramework::getBaseRacePage()
	{
		return ((u32*)(getMenuData()))[0x30/4];
	}

	ModeManagerData* MarioKartFramework::getModeManagerData() {
		return (ModeManagerData*)(getModeManagerBase() + 0x64);
	}

	CRaceInfo* MarioKartFramework::getRaceInfo(bool global) {
		if (global)
			return (CRaceInfo*)(getMenuData() + 0x74); // Applies to entire GP
		else
			return (CRaceInfo*)(getMenuData() + 0x204); // Applies to current race
	}

	FixedStringBase<char16_t, 0x20>* MarioKartFramework::getPlayerNames() {
		return (FixedStringBase<char16_t, 0x20>*)(getMenuData() + 0x3C0);
	}

	int MarioKartFramework::getCurrentRaceNumber() {
		return *(int*)(getMenuData() + 0x670);
	}

	KartLapInfo* MarioKartFramework::getKartLapInfo(u32 playerID) {
		u32 modeManagerBase = getModeManagerBase();
		KartLapInfo** infos = (KartLapInfo**)(((u32*)modeManagerBase)[0x4CC/4]);
		return infos[playerID];
	}

	Vector3* MarioKartFramework::GetGameCameraPosition()
	{
		u32 camEngine = getCameraEngine();
		if (camEngine && ((u32*)camEngine)[0x40/4])
			return (Vector3*)(((u32*)camEngine)[0x40/4] + 0x2C);
		else
			return nullptr;
	}

	void MarioKartFramework::BasePageSetCup(u8 cupID) {
		*(u8*)(getMenuData() + 0x620) = cupID;
	}

    void MarioKartFramework::BasePageSetCC(EEngineLevel engineLevel)
    {
		*(u32*)(getMenuData() + 0x1E4) = engineLevel;
    }

    void MarioKartFramework::BasePageSetMirror(bool mirror)
    {
		*(bool*)(getMenuData() + 0x1E8) = mirror;
    }

    u32 MarioKartFramework::BasePageGetCup() {
		return *(u8*)(getMenuData() + 0x620);
	}

	static bool g_isUserInControlWW = true;
	static bool g_hasGameReachedTitle = false;
	static Clock g_InvalidSoundTimer;
	
	bool MarioKartFramework::allowOpenCTRPFMenu()
	{
		if (!g_hasGameReachedTitle)
			return false;
		if (isGameInRace() || (ISGAMEONLINE && lastLoadedMenu != 0x1A) || (ISGAMEONLINE && !g_isUserInControlWW) || MenuPageHandler::MenuEndingPage::IsLoaded()) {
			if (g_InvalidSoundTimer.HasTimePassed(Seconds(0.5))) {
				Snd::PlayMenu(Snd::BUTTON_INVALID);
				g_InvalidSoundTimer.Restart();
			}
			return false;
		}
		return true;
	}

	void MarioKartFramework::changeNumberRounds(u32 newval) {
		if (newval < 0 || newval > 32) return;
		SaveHandler::saveData.numberOfRounds = newval;
		if (currNumberTracksPtr) *currNumberTracksPtr = newval;
		if (currCommNumberTracksPtr) *currCommNumberTracksPtr = newval;
		numbRoundsOnlineEntry->updateName();
	}

	void MarioKartFramework::unlockDriver(EDriverID driverID)
	{
		const s8 saveDriverIDs[] = {-1, 0x0, -1, 0x5, -1, 0x7, -1, -1, 0x3, 0x8, -1, -1, 0x2, 0x4, -1, 0x1, 0x6, -1}; 
		s8 id = saveDriverIDs[(u32)driverID];
		if (id < 0) return; //Cannot unlock the default unlocked drivers
		u32* saveFile = getSaveManagerFuncptr() + 0x30;
		u16* driverData = (u16*)(saveFile + 0x1390) + 1;
		driverData[0] |= 1 << id;
		driverData[1] |= 1 << id;
	}

	void MarioKartFramework::unlockKartBody(EBodyID kartID)
	{
		const s8 saveKartIDs[] = {-1, -1, -1, 0x0, 0x4, 0x3, 0x7, 0x1, 0x6, 0xA, 0x5, 0x8, 0xB, 0x2, 0x9, 0xC, 0xD};
		s8 id = saveKartIDs[(u32)kartID];
		if (id < 0) return; //Cannot unlock the default unlocked parts
		u32* saveFile = getSaveManagerFuncptr() + 0x30;
		u16* kartData = (u16*)(saveFile + 0x1392);
		*kartData |= 1 << id;
	}

	void MarioKartFramework::unlockKartTire(ETireID tireID)
	{
		const s8 saveTireIDs[] = {-1, -1, -1, 0x0, 0x1, 0x2, 0x6, 0x5, 0x3, 0x4};
		s8 id = saveTireIDs[(u32)tireID];
		if (id < 0) return; //Cannot unlock the default unlocked parts
		u32* saveFile = getSaveManagerFuncptr() + 0x30;
		u16* tireData = (u16*)(saveFile + 0x1393);
		*tireData |= 1 << id;
	}

	void MarioKartFramework::unlockKartWing(EWingID wingID)
	{
		const s8 saveWingIDs[] = {-1, 0x0, 0x1, 0x2, 0x3, 0x4, 0x5};
		s8 id = saveWingIDs[(u32)wingID];
		if (id < 0) return; //Cannot unlock the default unlocked parts
		u32* saveFile = getSaveManagerFuncptr() + 0x30;
		u16* wingData = (u16*)(saveFile + 0x1394);
		*wingData |= 1 << id;
	}

	void MarioKartFramework::OnSaveDataEvent(SaveDataEvent event) {
		if (event == SaveDataEvent::SAVEEVENT_INIT || event == SaveDataEvent::SAVEEVENT_FORMAT) {
			for (int i = EDriverID::DRIVER_BOWSER; i <= EDriverID::DRIVER_YOSHI; i++) {
				unlockDriver((EDriverID)i);
			}
			for (int i = EBodyID::BODY_STD; i <= EBodyID::BODY_GOLD; i++) {
				if (i != EBodyID::BODY_GOLD)
					unlockKartBody((EBodyID)i);
			}
			for (int i = ETireID::TIRE_STD; i <= ETireID::TIRE_MUSH; i++) {
				if (i != ETireID::TIRE_GOLD)
					unlockKartTire((ETireID)i);
			}
			for (int i = EWingID::WING_STD; i <= EWingID::WING_GOLD; i++) {
				if (i != EWingID::WING_GOLD)
					unlockKartWing((EWingID)i);
			}
		}
	}

	void MarioKartFramework::injectKey(Key key)
	{
		injectedKey = (Key)((u32)injectedKey | (u32)key);
		wasKeyInjected = true;
	}

	void MarioKartFramework::blockKeys(bool block)
	{
		areKeysBlocked = block;
	}

	static bool g_backCamEnabled;
	static bool g_playSfx = true;
	static u32 g_prevPad = 0;
	bool g_isAutoAccel = false;
	// nn::hidlow::CTR::PadLifoRing::ReadData
	void __attribute__((hot)) MarioKartFramework::handleBackwardsCamera(PadData* padData)
	{
		g_StresserCalc(padData);
		u32& pad = padData->currentPad; 
		if (areKeysBlocked) pad = 0;
		if (wasKeyInjected) {
			pad |= (u32)injectedKey;
			injectedKey = (Key)0;
			wasKeyInjected = false;
		}
		if (!isRaceState) {
			g_isAutoAccel = false;
			g_playSfx = true;
			g_backCamEnabled = false;
			return;
		}
		// Only applies to race from now on
		u32 pressedpad = (pad ^ g_prevPad) & pad;
		bool missionForceBackwards = MissionHandler::forceBackwards();
		bool autoaccelenabled = !MissionHandler::isMissionMode && SaveHandler::saveData.flags1.autoacceleration;
		if (((bool)SaveHandler::saveData.flags1.backCamEnabled) && !isRaceGoal && !isRacePaused && !missionForceBackwards && !playCountDownCameraAnim) {
			if (allowBackwardsCamera() && ((pad & Key::X))) {
				pad &= ~Key::X;
				g_backCamEnabled = true;
				if (g_playSfx) {
					Snd::PlayMenu(Snd::SETTING_CHANGE);
					g_playSfx = false;
				}
			}
			else {
				g_backCamEnabled = false;
				if (!g_playSfx) {
					Snd::PlayMenu(Snd::SETTING_SELECT_BAR);
					g_playSfx = true;
				}
			}
		}
		else {
			if (missionForceBackwards) {
				g_backCamEnabled = true;
				if (!isRacePaused && !isRaceGoal) pad &= ~(Key::A | Key::Y);
			}
			else {g_backCamEnabled = false; g_playSfx = true;}
		}
		if (autoaccelenabled && !isRaceGoal && !isRacePaused && allowBackwardsCamera()) {
			if ((pressedpad & (SaveHandler::saveData.flags1.autoaccelerationUsesA ? Key::A : Key::Y)) && !g_isAutoAccel) {
				g_isAutoAccel = true;
				Snd::PlayMenu(Snd::SETTING_CHANGE);
			} else if (g_isAutoAccel && (pressedpad & (Key::Y | Key::A | Key::B)))
			{
				g_isAutoAccel = false;
				Snd::PlayMenu(Snd::SETTING_SELECT_BAR);
			}
		} else {
			g_isAutoAccel = false;
		}
		g_prevPad = pad;
		if (g_isAutoAccel)
			pad |= Key::A;
	}

	bool MarioKartFramework::allowBackwardsCamera()
	{
		if (g_isFoolActive)
			return false;
		return (cameraKartMode ? (cameraKartMode[0x4E] != 2) : false);
	}

	#define SECONDSTOFRAMES(x) (u32)((x) * 60)
	static const IntroCameraAnim camAnim[8] = {
		{SECONDSTOFRAMES(0),						     {0,           8,    50},        35},
		{SECONDSTOFRAMES(0.1 + 0.42 * 0.8),			     {0,           10,   50},        52},
		{SECONDSTOFRAMES(0.1 + 0.5 * 0.8),               {0,           10,   50},        52},
		{SECONDSTOFRAMES(0.1 + (0.25 + 0.444) * 0.75),   {-14.8859425, 11.7, 46.82485},  52},
		{SECONDSTOFRAMES(0.1 + (0.25 + 0.832) * 0.75),   {-36.659874 , 15.1, 27.49491},  52},
		{SECONDSTOFRAMES(0.1 + (0.25 + 1.220) * 0.75),   {-44.2138443, 18.5, 0}        , 52},
		{SECONDSTOFRAMES(0.1 + (0.25 + 1.608) * 0.75),   {-36.659874 , 21.9, -27.49491}, 52},
		{SECONDSTOFRAMES(0.1 + (0.25 + 1.996) * 0.75),   {-14.8859425, 25.3, -46.82485}, 52},
	};
	
	static bool g_returnCamera = false;
	void __attribute__((hot)) MarioKartFramework::kartCameraSmooth(u32* camera, float smoothVal)
	{
		auto needsXMirror = []() {
			int position = getModeManagerData()->modeRaceInfo.kartInfos[masterPlayerID].raceRank;
			int playerCount = getModeManagerData()->modeRaceInfo.playerAmount;
			if (playerCount == 1)
				return false;
			else if (!lastPolePositionLeft)	
				return position == 1 || position == 2 || position == 5 || position == 6;
			else
				return position == 3 || position == 4 || position == 7 || position == 8;
		};

		Vector3* srcCamParams = (Vector3*)((u32)camera + 0x244); //This uses a Coord3D struct, however they aren't coordinates
		Vector3* dstCamParams = (Vector3*)((u32)camera + 0x258);

		float* srcCamFovX = (float*)((u32)camera + 0x250), *otherCam = (float*)((u32)camera + 0x254);
		float* dstCamFovX = (float*)((u32)camera + 0x264), *otherDst = (float*)((u32)camera + 0x268);

		if (playCountDownCameraAnim && (!allowBackwardsCamera() || MissionHandler::forceBackwards())) playCountDownCameraAnim = false;

		if (playCountDownCameraAnim) {
			static u32 currentFrame = 0, currentIndex = 0, animSize = sizeof(camAnim) / sizeof(IntroCameraAnim);
			Vector3* cameraPos = (Vector3*)((u32)camera + 0x1F4);
			*(u8*)((u32)camera + 0x224) = 2; //Force camera collision with ground

			if (currentFrame >= camAnim[currentIndex + 1].fromFrame) currentIndex++;
			if (currentIndex == animSize - 1) {
				playCountDownCameraAnim = false;
				currentFrame = 0;
				currentIndex = 0;
			}
			else {
				Vector3 animSrc = camAnim[currentIndex].pos;
				Vector3 animDst = camAnim[currentIndex + 1].pos;
				if (needsXMirror()) {
					animSrc.x *= -1;
					animDst.x *= -1;
				}

				float fovsrc = camAnim[currentIndex].FOV;
				float fovdst = camAnim[currentIndex + 1].FOV;
				float* fovOther = (float*)((u32)camera + 0x170);

				float howMuch = (currentFrame - camAnim[currentIndex].fromFrame) / (float)(camAnim[currentIndex + 1].fromFrame - camAnim[currentIndex].fromFrame);

				cameraPos->x = animSrc.x +	((animDst.x - animSrc.x) * howMuch);
				cameraPos->y = animSrc.y + ((animDst.y - animSrc.y) * howMuch);
				cameraPos->z = animSrc.z + ((animDst.z - animSrc.z) * howMuch);

				*fovOther = *srcCamFovX = fovsrc + ((fovdst - fovsrc) * howMuch);

				currentFrame++;
			}
		}
		/*else if (backButtonFool) {
			Coord3D* cameraPos = (Coord3D*)((u32)camera + 0x1F4);
			*(u8*)((u32)camera + 0x224) = 2; //Force camera collision with ground
			static Clock timer;
			if (timer.HasTimePassed(Seconds(0.5))) {
				timer.Restart();
				float x = Utils::Random(-50, 50), y = Utils::Random(0, 50), z = Utils::Random(-50, 50);
				cameraPos->x = x;
				cameraPos->y = y;
				cameraPos->z = z;
			}
		}*/
		else {
			srcCamParams->x = srcCamParams->x + ((dstCamParams->x - srcCamParams->x) * smoothVal);
			if (g_backCamEnabled) {
				srcCamParams->y = -48;
				g_returnCamera = true;
			}
			else if (g_returnCamera) {
				g_returnCamera = false;
				srcCamParams->y = dstCamParams->y;
			}
			else {
				srcCamParams->y = srcCamParams->y + ((dstCamParams->y - srcCamParams->y) * smoothVal);
			}
			srcCamParams->z = srcCamParams->z + ((dstCamParams->z - srcCamParams->z) * smoothVal);

			*srcCamFovX = *srcCamFovX + ((*dstCamFovX - *srcCamFovX) * smoothVal);
			*otherCam = *otherCam + ((*otherDst - *otherCam) * smoothVal);
		}
	}

	void MarioKartFramework::getMyPlayerData(SavePlayerData* data)
	{
		getMyPlayerDatafuncptr(getSystemEngine(), data, false);
	}

	u8* MarioKartFramework::GetSelfMiiIcon() { // 64x64 RGB555A1 (8192 bytes)
		u32 miiEngine = getMiiEngine();
		SeadArrayPtr<u8**>* miiIcons = *(SeadArrayPtr<u8**>**)(miiEngine + 0x2C);
		u8** miiIconPtr = (*miiIcons)[0];
		if (!miiIconPtr)
			return nullptr;
		return *miiIconPtr;
	}

	u32 MarioKartFramework::GetSelfMiiIconChecksum() {
	}

	void MarioKartFramework::lockBottomMapChange(bool lock) {
		/*
		if (!changeMapCallPtr) return;
		if (lock) *changeMapCallPtr = 0xE320F000;
		else *changeMapCallPtr = origChangeMapInst;
		*/
	}

	void MarioKartFramework::patchKMP(void* kmp) {
		auto getAdditionalGOBJ = []() -> u32 {
			return BlueCoinChallenge::HasChallenge() ? 1 : 0;
		};
		auto addGBOJEntries = [](GOBJEntryList* objlist) -> void {
			if (BlueCoinChallenge::coinSpawned) {
				for (int i = 0; i < objlist->count; i++) {
					if (objlist->entries[i].objID == 0x7D) {
						panic("IceBoundStar is not allowed in CTs");
					}
				}
				objlist->entries[objlist->count++] = BlueCoinChallenge::GetBlueCoinGOBJ();
			}
			return;
		};

		struct KMPHeader {
			u32 signature;
			u32 filesize;
			u16 sectionCount;
			u16 headerSize;
			u32 version;
			u32 sectionOffsets[];
		};
		KMPHeader* header = (KMPHeader*)kmp;
		u8* sections = (u8*)kmp + header->headerSize;
		
		// GOBJ patches
		u32 gobjOffset = header->sectionOffsets[0x7];
		GOBJEntryList* objlist = (GOBJEntryList*)(sections + gobjOffset);
		if (!((u32)objlist > 0x06000000 && (u32)objlist < 0x07000000)) { // Prevent repatching
			if (objlist->signature != 0x474F424A)
				panic();
			if (gobjrelocptr) operator delete(gobjrelocptr);
			u32 objrelocptrsz = 0x8 + (objlist->count + getAdditionalGOBJ()) * sizeof(GOBJEntry);
			gobjrelocptr = operator new(objrelocptrsz);
			memcpy(gobjrelocptr, objlist, objrelocptrsz);
			addGBOJEntries((GOBJEntryList*)gobjrelocptr);
			header->sectionOffsets[0x7] += (u32)gobjrelocptr - (u32)objlist;
		}
	}

	u16 MarioKartFramework::getSTGIEntryCount(STGIEntry** s) {
		if (!s) return 0;
		return *((u16*)((u32*)(*s) - 1));
	}

	void MarioKartFramework::kmpConstructCallback() {
		STGIEntry** stginfo = getStageInfoFuncptr();
		u32 entrycount = getSTGIEntryCount(stginfo);

		// Clar car controller vector
		carRouteControllerPair.clear();
		dokanWarps.clear();

		if (entrycount > 0)
			lastPolePositionLeft = (*stginfo)[0].PolePosition == 1;

		isLoadingAward = CourseManager::lastLoadedCourseID == 0x26;

		kmpRenderOptimizationsForced = entrycount > 1 && (*stginfo)[1].CustomFlags.forceRenderOptim;
		bulletBillAllowed = !(entrycount > 1 && (*stginfo)[1].CustomFlags.disableBulletBill);
		speedupMusicOnLap = entrycount > 1 && (*stginfo)[1].CustomFlags.speedupMusicOnLap;
		disableLensFlare = entrycount > 1 && (*stginfo)[1].CustomFlags.disableLensFlare;

		float speedmod = 1;
		if (entrycount > 1) {
			u8 poleval = (float)(*stginfo)[1].PolePosition;
			if (poleval > 1)
				speedmod = poleval / 16.f;
		}
		if (!MissionHandler::isMissionMode && !PointsModeHandler::isPointsMode) {
			CCSettings* ccset = static_cast<CCSettings*>(ccselectorentry->GetArg());
			float ccvalaue = ccset->enabled ? (ccset->value * CC_SINGLE) : 1.f;

			if (ccset->enabled && ccset->value < 150) {
				speedmod = ccvalaue * speedmod;
			} else {
				if (ccvalaue > speedmod) {
					speedmod = ccvalaue;
				}
			}
		} 
		if (MissionHandler::isMissionMode) {
			speedmod = MissionHandler::CalculateSpeedMod(speedmod);
		}

		//u32 speedval = 0x7effffff;
		//speedmod = 40282346638528859811704183484516925440.f;//*(float*)&speedval;	////340282346638528859811704183484516925440.f / 10.f;
		
		g_speedVal = speedmod;
		g_speedValsqrt = (sqrtf(speedmod) + speedmod) / 2.f;

		applyGOBJPatches(getGeoObjAccessor());

		hasOOBAreas = false;
		auto* areaAccessor = MK7::Field::GetAreaAccessor();
		for (auto it = areaAccessor->m_entries.begin(); it != areaAccessor->m_entries.end(); ++it) {
			if (it->m_area_calc->m_area_type == 10) {
				hasOOBAreas = true;
			}
		}

		if (MarioKartFramework::currentRaceMode.type != 3)
			g_StresserUpdateMode(StressMode::RACE);

		return;
	}

	void MarioKartFramework::OnRaceEnter() {
		isRaceState = true;
		isRaceGoal = false;
		MissionHandler::OnRaceEnter();
		ItemHandler::MegaMushHandler::OnRaceEnter();
	}

	void MarioKartFramework::OnRaceExit(u32 mode) {
		isRaceState = false;
		isRacePaused = false;
		isLoadingAward = false;
		MissionHandler::OnRaceExit();
		//0 -> next race, 1->replay, 2->quit, 3->view results, 4->Other (Demo/WinningRun)
		if (mode == 0 && VersusHandler::IsVersusMode) {
			VersusHandler::OnNextTrackLoad();
		}
		UserCTHandler::CleanTextureSarc();
	}

	void MarioKartFramework::applyGOBJPatches(GOBJAccessor* access)
	{
		for (int i = 0; i < access->entryAmount; i++) {
			GOBJEntry& currEntry = *access->entries[i][0];
			switch (currEntry.objID)
			{
			case 0x1D: // Honey Stage Air Current
				dokanWarps.push_back(&currEntry);
				break;
			case 0xCB: // Thwomp
				if ((currEntry.settings[2] == 1 && currentRaceMode.mode != 1) || currEntry.settings[2] == 2) { //If not in time trials and settings3 = 1
					currEntry.settings[0] = 65535;
					currEntry.settings[1] = 65535;
					currEntry.position.y -= 100.f;
				}
				break;
			case 0x143: // Alternate itembox
				currEntry.objID = 4;
				break;
			}
		}
	}

	char origKouraG[] = "Item/itemKouraGreen/itemKouraGreen.bcmdl";
	char eggKouraG[] = "Item/itemKouraGreen/itemEggGreen.bcmdl";

	char origKouraR[] = "Item/itemKouraRed/itemKouraRed.bcmdl";
	char eggKouraR[] = "Item/itemKouraRed/itemEggRed.bcmdl";

	u32 MarioKartFramework::getKouraGModelName()
	{
		u32 ret;
		if ((kouraGProbability & 1) && (Utils::Random() & 1)) ret = (u32)eggKouraG;
		else ret = (u32)origKouraG;
		kouraGProbability = rol<u64>(kouraGProbability, 1);
		return ret;
	}
	u32 MarioKartFramework::getKouraRModelName()
	{
		u32 ret;
		if ((kouraRProbability & 1) && (Utils::Random() & 1)) ret = (u32)eggKouraR;
		else ret = (u32)origKouraR;
		kouraRProbability = rol<u64>(kouraRProbability, 1);
		return ret;
	}

	void MarioKartFramework::playMusicAlongCTRPF(bool playMusic)
	{
		static bool isPlayMusic = false;
		if (forceDisableSndOnPause)
			playMusic = false;
		if (isPlayMusic == playMusic) return;
		isPlayMusic = playMusic;
		static u32 tlsBackup[2];
		static s32 prioBackup[2];
		for (int i = 0; i < SOUNDTHREADSAMOUNT; i++) {
			u32 soundThreadID = std::get<0>(soundThreadsInfo[i]);
			// The game does not follow this structure, we do it only to modify members at the proper locs
			ThreadVars* soundThreadTls = reinterpret_cast<ThreadVars*>(std::get<1>(soundThreadsInfo[i]));
			if (soundThreadID == 0xFFFFFFFF) continue;
			Handle soundThreadHandle;
			Result res = svcOpenThread(&soundThreadHandle, CUR_PROCESS_HANDLE, soundThreadID);
			if (R_FAILED(res)) return;

			if (playMusic) {
				tlsBackup[i] = soundThreadTls->magic;
				soundThreadTls->magic = THREADVARS_MAGIC;
				ThreadVars* currThreadVars = (ThreadVars*)getThreadLocalStorage();
				soundThreadTls->tls_tp = currThreadVars->tls_tp;
				svcGetThreadPriority(&prioBackup[i], soundThreadHandle);
				svcSetThreadPriority(soundThreadHandle, FwkSettings::Get().ThreadPriority - 1);
			}
			else {
				soundThreadTls->magic = tlsBackup[i];
				soundThreadTls->tls_tp = nullptr;
				svcSetThreadPriority(soundThreadHandle, prioBackup[i]);
			}
			svcCloseHandle(soundThreadHandle);
		}
	}

	DialogFlags::Mode g_dialogMode;
	void* g_dialogArgs;
	bool g_dialogFinishedOSD = true;
	static void openDialogCallBack() {
		MarioKartFramework::openDialogImpl(g_dialogMode, CustomTextEntries::dialog, g_dialogArgs);
		g_dialogFinishedOSD = true;
		AsyncRunner::StopAsync(openDialogCallBack);
	}

	DialogResult MarioKartFramework::openDialog(const DialogFlags& flags, const std::string& text, void* args, bool alreadyReplaced)
	{
		if (isDialogOpened()) return DialogResult::ERROR;
		DialogFlags::Mode mode = (DialogFlags::Mode)(flags.raw & 3);
		bool async = (bool)(flags.raw & 4);
		if (!alreadyReplaced) Language::MsbtHandler::SetString(CustomTextEntries::dialog, text);
		g_dialogMode = mode;
		g_dialogArgs = args;
		g_dialogFinishedOSD = false;
		AsyncRunner::StartAsync(openDialogCallBack);
		if (async) return DialogResult::NONE;
		else {
			while (true)
			{
				if (isDialogOpened()) Sleep(Milliseconds(1));
				else break;
			}
			return getLastDialogResult();
		}
	}

	DialogResult MarioKartFramework::getLastDialogResult()
	{
		if (isDialogOpened()) return DialogResult::NONE;
		bool isYes = isDialogYesImpl();
		if (isYes) return DialogResult::YES;
		else return DialogResult::NO;
	}

	bool MarioKartFramework::isDialogOpened()
	{
		return !isDialogClosedImpl() || !g_dialogFinishedOSD;
	}

	void MarioKartFramework::closeDialog() {
		closeDialogImpl();
	}

	static bool g_startDialogBlackOut = false;
	static bool g_isDoingDialogBlackout = false;
	static bool g_usesCustomText = true;
	void MarioKartFramework::dialogBlackOut(const std::string& message)
	{
		if (!g_startDialogBlackOut)
		{
			g_usesCustomText = !message.empty();
			if(g_usesCustomText) Language::MsbtHandler::SetString(CustomTextEntries::dialog, message);
			g_startDialogBlackOut = true;
		}
	}

	bool MarioKartFramework::onNetworkErrorCheckCalc()
	{
		g_isDoingDialogBlackout = g_startDialogBlackOut;
		return g_startDialogBlackOut;
	}

	u32 MarioKartFramework::onNetworkErrorCheckWindowOpen()
	{
		if (g_isDoingDialogBlackout) {
			g_isDoingDialogBlackout = false;
			g_startDialogBlackOut = false;
			if (g_usesCustomText)
				return CustomTextEntries::dialog;
			else
				return 5012;
		}
		return 5012;
	}

	void MarioKartFramework::setCTWWState(bool enabled)
	{
		return;
	}

	void MarioKartFramework::adjustVisualElementsCTWW(bool enabled) {
		int wwTex = enabled ? 0xC : 0x10;
		int battleTex = enabled ? 0x5 : 0x9;
		CourseManager::BaseMenuButtonControl_setTex((u32)worldWideButtonPtr, wwTex, 2);
		CourseManager::BaseMenuButtonControl_setTex((u32)battleWideButtonPtr, battleTex, 2);
		if (enabled) {
			std::string ctdw = NAME("cntdwn");
			std::u16string wctdw;
			Utils::ConvertUTF8ToUTF16(wctdw, ctdw);
			u16* ptr = (u16*)wctdw.c_str();
			u16** ptr2 = &ptr;
			nwlytReplaceText(battleWideMessageReplacePtr[0], battleWideMessageReplacePtr[1], (u32)ptr2, 0, 0);
		}
		else {
			u16* ptr = (u16*)Language::MsbtHandler::GetText(2206);
			u16** ptr2 = &ptr;
			nwlytReplaceText(battleWideMessageReplacePtr[0], battleWideMessageReplacePtr[1], (u32)ptr2, 0, 0);
		}

	}

	void MarioKartFramework::applyVisualElementsCTWWOSD() {
		adjustVisualElementsCTWW(SaveHandler::saveData.flags1.isCTWWActivated && SaveHandler::saveData.flags1.useCTGP7Server);
		AsyncRunner::StopAsync(applyVisualElementsCTWWOSD);
	}

	static bool g_hasDialogBeenOpened = false;
	static bool g_firstTimeInPage = true;
	static bool g_needsToCheckUpdate = true;

	u32 MarioKartFramework::onOnlineModePreStep(u32 mode)
	{
		bool isBadVersion = false;
		Net::CTWWLoginStatus loginStat;
		if (mode == 0) {
			g_isUserInControlWW = false;
			loginStat = Net::GetLoginStatus();
			if (g_firstTimeInPage) {
				g_setCTModeVal = CTMode::ONLINE_NOCTWW;
				g_updateCTMode();
				CharacterHandler::ResetCharacters();
				CharacterHandler::DisableOnlineLock();
				bannedUltraShortcuts.clear();
				Net::vrMultiplier = 1.f;
				Net::allowedTracks = "";
				Net::specialCharVrMultiplier = 1.f;
				Net::useSpecialCharVRMultiplier = false;
				for (int i = 0; i < MAX_PLAYER_AMOUNT; i++) {
					Net::othersBadgeIDs[i] = 0;
				}
				resetdriverchoices();
				MarioKartFramework::SetWatchRaceMode(false);
				MarioKartFramework::ClearCustomItemMode();
				ItemHandler::SetBlueShellShowdown(false);
				currCommNumberTracksPtr = nullptr;
				g_firstTimeInPage = false;
				g_needsToCheckUpdate = true;
				AsyncRunner::StartAsync(applyVisualElementsCTWWOSD);
				Net::UpdateOnlineStateMahine(Net::OnlineStateMachine::IDLE);
			}
			if (loginStat == Net::CTWWLoginStatus::PROCESSING) {
				if (!isDialogOpened()) {
					openDialog(DialogFlags::Mode::LOADING, NAME("update_check"));
				}
			}
			else if (loginStat != Net::CTWWLoginStatus::SUCCESS) {
				static bool showLatestVerMsg = false;
				static bool isShowingLatestVerMsg = false;
				if (!showLatestVerMsg) {
					showLatestVerMsg = true;
					closeDialog();
				}
				if (showLatestVerMsg && !isShowingLatestVerMsg && !isDialogOpened()) {
					isShowingLatestVerMsg = true;
					if (loginStat == Net::CTWWLoginStatus::VERMISMATCH)
#ifndef DISABLE_ONLINE_ACCESS
						openDialog(DialogFlags::Mode::OK, NOTE("update_check"));
#else
						openDialog(DialogFlags::Mode::OK, "Online access has\nbeen disabled for\nthis build.");
#endif
					else if (loginStat == Net::CTWWLoginStatus::FAILED)
						openDialog(DialogFlags::Mode::OK, Utils::Format("%s\nErr: 0x%08X", NAME("fail_conn").c_str(), Net::GetLastErrorCode()));
					else if (loginStat == Net::CTWWLoginStatus::MESSAGE || loginStat == Net::CTWWLoginStatus::MESSAGEKICK)
						openDialog(DialogFlags::Mode::OK, Net::GetServerMessage());
				} else if (showLatestVerMsg && isShowingLatestVerMsg && !isDialogOpened()) {
					isShowingLatestVerMsg = false;
					showLatestVerMsg = false;
					if (loginStat == Net::CTWWLoginStatus::MESSAGE)
						Net::AcknowledgeMessage();
					else
						mode = 1;
				}
			}
			else {
				if (isDialogOpened() && g_needsToCheckUpdate) {
					g_needsToCheckUpdate = false;
					closeDialog();
					return 0;
				}
				g_isUserInControlWW = true;
				if (Controller::IsKeyPressed(Key::Y) && !g_hasDialogBeenOpened && SaveHandler::saveData.flags1.useCTGP7Server) {
					g_hasDialogBeenOpened = true;
					openDialog(DialogFlags::Mode::YESNO, SaveHandler::saveData.flags1.isCTWWActivated ? NOTE("CTWWtog") : NAME("CTWWtog"));
				}
				if (g_hasDialogBeenOpened && !isDialogOpened()) {
					if (getLastDialogResult() == DialogResult::YES) SaveHandler::saveData.flags1.isCTWWActivated = !SaveHandler::saveData.flags1.isCTWWActivated;
					AsyncRunner::StartAsync(applyVisualElementsCTWWOSD);
					g_hasDialogBeenOpened = false;
				}
			}
		}
		if (mode != 0) {
			g_isUserInControlWW = true;
			g_hasDialogBeenOpened = false;
			g_needsToCheckUpdate = false;
			g_firstTimeInPage = true;
		}
		return mode;
	}
	void MarioKartFramework::onOnlineModeButOK(u32 btn)
	{
		if (btn != 3 ) {
			g_isUserInControlWW = true;
			g_firstTimeInPage = true;
			g_needsToCheckUpdate = false;
			g_hasDialogBeenOpened = false;
		}
		if (btn == 0) {
			if (SaveHandler::saveData.flags1.isCTWWActivated && SaveHandler::saveData.flags1.useCTGP7Server) g_setCTModeVal = CTMode::ONLINE_CTWW;
			else g_setCTModeVal = CTMode::ONLINE_NOCTWW;
			g_updateCTMode();
		}
		else {
			g_setCTModeVal = CTMode::ONLINE_NOCTWW;
			g_updateCTMode();
		}
	}

	void MarioKartFramework::onOnlineSubModePreStep()
	{
		Net::WaitOnlineStateMachine();
		svcInvalidateProcessDataCache(CUR_PROCESS_HANDLE, 0, 0);
	}

	void MarioKartFramework::onOnlineSubModeButtonOK(int buttonID)
	{
		Net::lastPressedOnlineModeButtonID = buttonID;
	}
	
	static Mutex g_internetConnectionMutex;
	void MarioKartFramework::OnSocInit() {
		if (pendingVoiceChatInit) {
			pendingVoiceChatInit = false;
			VoiceChatHandler::OnSocketInit();
		}

		Lock lock(g_internetConnectionMutex);
		if (!needsOnlineCleanup) {
			ccSelOnlineEntry->setOnlineMode(true);
			numbRoundsOnlineEntry->setOnlineMode(true);
			serverOnlineEntry->setOnlineMode(true);
			improvedTricksOnlineEntry->setOnlineMode(true);
			ccsettings[1].enabled = false;
			ccselectorentry->SetArg(&ccsettings[1]);
			ccselector_apply(ccselectorentry);
			MarioKartFramework::changeNumberRounds(4);
			g_setCTModeVal = CTMode::ONLINE_NOCTWW;
			g_updateCTMode();
			while (!ISGAMEONLINE) g_isOnlineMode = rol<u8>(g_isOnlineMode, 1);
			needsOnlineCleanup = true;
		}
	}

	void MarioKartFramework::OnSocExit() {
		VoiceChatHandler::OnSocketExit();
	}

	void MarioKartFramework::DoOnlineCleanup() {
		Lock lock(g_internetConnectionMutex);
		if (needsOnlineCleanup) { // This variable is set to true if the online 
			ccSelOnlineEntry->setOnlineMode(false);
			numbRoundsOnlineEntry->setOnlineMode(false);
			serverOnlineEntry->setOnlineMode(false);
			improvedTricksOnlineEntry->setOnlineMode(false);
			ccselectorentry->SetArg(&ccsettings[0]);
			ccselector_apply(ccselectorentry);
			MarioKartFramework::changeNumberRounds(SaveHandler::saveData.numberOfRounds);
			g_setCTModeVal = CTMode::OFFLINE;
			g_updateCTMode();
			while (ISGAMEONLINE) g_isOnlineMode = rol<u8>(g_isOnlineMode, 1);
			Net::UpdateOnlineStateMahine(Net::OnlineStateMachine::OFFLINE);
			needsOnlineCleanup = false;
		}
	}

	void MarioKartFramework::handleTitleEnterCallback() { // Called every time the game enters the title screen
		isRaceState = false;
		isRacePaused = false;
		isLoadingAward = false;
		SetWatchRaceMode(false);
		CharacterHandler::ResetCharacters();
		CharacterHandler::DisableOnlineLock();
		Net::customAuthData.populated = false;
		Net::customAuthData.result = 0;
		Net::temporaryRedirectCTGP7 = false;
		Net::privateRoomID = 0;
		Net::vrMultiplier = 1.f;
		Net::whiteListedCharacters.clear();
		Net::specialVRCharacters.clear();
		Net::useSpecialCharVRMultiplier = false;
		Net::specialCharVrMultiplier = 1.f;
		Net::allowedTracks = "";
		for (int i = 0; i < MAX_PLAYER_AMOUNT; i++) {
			Net::othersServerNames[i].clear();
			Net::othersBadgeIDs[i] = 0;
		}
		VersusHandler::IsVersusMode = false;
		MissionHandler::onModeMissionExit();
		PointsModeHandler::onPointsModeExit();
		MarioKartFramework::setSkipGPCoursePreview(false);
		UserCTHandler::UpdateCurrentCustomCup(0, -1, -1);
		UserCTHandler::CleanTextureSarc();
		MenuPageHandler::MenuEndingPage::loadCTGPCredits = false;
		thankYouDisableNintendoLogo = false;
		if (UserCTHandler::skipConfig.enabled) {
			SaveHandler::disableSaving = true;
			Process::ReturnToHomeMenu();
		}
		DoOnlineCleanup();
		SaveHandler::UpdateAchievementsConditions();
		if (g_hasGameReachedTitle)
			SaveHandler::SaveSettingsAll();
		g_hasGameReachedTitle = true;
		bannedUltraShortcuts.clear();
		ClearCustomItemMode();
		ItemHandler::SetBlueShellShowdown(false);
		
		StatsHandler::UploadStats();
		BadgeManager::ClearBadgeCache();

		g_StresserUpdateMode(StressMode::PRESS_A);
		g_StresserRandomizeSettings();
	}

	static std::string g_build_lobby_message() {
		return NAME("choose_lobby") + "\n\n" + 
			NAME("proximity_chat") + " " + NOTE("proximity_chat") + ":\n" + (SaveHandler::saveData.flags1.enableVoiceChat ? (
				(Color::LimeGreen << NAME("state_mode"))
			) : (
				(Color::Red << NOTE("state_mode"))
			)) + ResetColor();
	}

	static void OnTitleOnlineButtonPressed() {
		AsyncRunner::StopAsync(OnTitleOnlineButtonPressed);
		#if false
		Keyboard keyboard("dummy");
		std::string ctgp7Network = NOTE("server_name");
		std::string nintendoNetwork = NAME("server_name");
		keyboard.Populate({ctgp7Network, nintendoNetwork});
		keyboard.ChangeSelectedEntry(SaveHandler::saveData.flags1.useCTGP7Server ? 0 : 1);
		std::string text(500, '\0');
		std::snprintf(text.data(), text.size(), NAME("server_choice").c_str(), (ToggleDrawMode(Render::UNDERLINE) + ctgp7Network + ToggleDrawMode(Render::UNDERLINE)).c_str(), (ToggleDrawMode(Render::UNDERLINE) + nintendoNetwork + ToggleDrawMode(Render::UNDERLINE)).c_str());
		text.erase(std::find(text.begin(), text.end(), '\0'), text.end());
		keyboard.GetMessage() = text;
		Process::Pause();
		int res = keyboard.Open();
		switch (res) {
			case 0:
				SaveHandler::saveData.flags1.useCTGP7Server = true;
				useCTGP7server_apply(true);
				break;
			case 1:
				SaveHandler::saveData.flags1.useCTGP7Server = false;
				useCTGP7server_apply(false);
				break;
		}
		Process::Play();
		#else
		// Remove this line when the if false is removed!
		SaveHandler::saveData.flags1.useCTGP7Server = true;
		useCTGP7server_apply(true);
		Net::privateRoomID = 0;
		Process::Pause();

		if (SaveHandler::CheckAndShowServerCommunicationDisabled())
		{
			Keyboard keyboard(g_build_lobby_message());
			keyboard.Populate({NAME("lobby_type"), std::string(NOTE("lobby_type"))});
			keyboard.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
				if (event.type == KeyboardEvent::EventType::KeyPressed) {
					if (event.affectedKey == Key::X) {
						SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
						SaveHandler::saveData.flags1.enableVoiceChat = !SaveHandler::saveData.flags1.enableVoiceChat;
						k.GetMessage() = g_build_lobby_message();
					}
				}
			});
			int res = keyboard.Open();
			switch (res)
			{
			case 1:
			{
				Keyboard privkbd(NAME("enter_lobby_code"));
				privkbd.CanAbort(false);
				std::string name;
				privkbd.SetCompareCallback([](const void * buf, std::string& err) -> bool{
					const std::string* in = reinterpret_cast<const std::string*>(buf);
					if (in->size() <= 6) {
						if (in->size() != 0)
							err = "\n\n" + NOTE("enter_lobby_code");
						else
							err = "";
						return false;
					}
					err = "";
					return true;
				});
				int res2 = privkbd.Open(name);
				if (res >= 0) {
					// Generate the secret private room ID and store it to Net::privateRoomID
				}
			}
			default:
				break;
			}
		}

		MarioKartFramework::pendingVoiceChatInit = true;

		Process::Play();

		#endif
		Net::UpdateOnlineStateMahine(Net::OnlineStateMachine::IDLE, true);
	}
	void MarioKartFramework::handleTitleCompleteCallback(u32 option)
	{
		// 0 -> Singleplayer, 1 -> Multiplayer, 2 -> Online, 3 -> Channel, 7 -> Demo race
		if (option < 4) {
			SaveHandler::SaveSettingsAll();
		}
		DoOnlineCleanup();
		if (option == 2)
		{
			AsyncRunner::StartAsync(OnTitleOnlineButtonPressed);
		}
		if (option != 7)
			g_StresserUpdateMode(StressMode::MOVE_PRESS_A);
	}

	static u32 g_welccounter = 1;
	u32 MarioKartFramework::handleTitleMenuPagePreStep(u32 timerVal)
	{
		if (playTitleFlagOpenTimer) {
			playTitleFlagOpenTimer--;
		}
		if (playTitleFlagOpenTimer == 1) {
			Snd::PlayMenu(Snd::SoundID::FLAG_OPEN);
		}

#ifdef BETA_BUILD
		static bool showingBeta = true, openedFirst = false;
		if (showingBeta) {
			if (Net::GetBetaState() == Net::BetaState::NONE && !openedFirst) {
				if (isDialogOpened()) return 0;
				openedFirst = true;
				Net::StartBetaRequest();
				openDialog(DialogFlags(DialogFlags::Mode::LOADING), "Checking your beta build...");
			}
			else {
				if (Net::GetBetaState() == Net::BetaState::GETTING) return 0;
				closeDialog();
				if (isDialogOpened()) return 0;
				if (Net::GetBetaState() != Net::BetaState::YES) {
					if (Net::GetBetaState() == Net::BetaState::NO || Net::GetBetaState() == Net::BetaState::FAILED) {
						if (Net::GetBetaState() == Net::BetaState::NO)
						{
							openDialog(DialogFlags(DialogFlags::Mode::NOBUTTON), "Your beta build is currently\ndisabled, please check for an\nupdated build of the beta.\n\nPress HOME to exit.");
							File betaConfig("/CTGP-7/config/betaConfig.bin", File::RW);
							if (betaConfig.IsOpen())
							{
								bool disable = false;
								betaConfig.Write(&disable, sizeof(disable));
							}
						} else {
							openDialog(DialogFlags(DialogFlags::Mode::NOBUTTON), "Failed to connect to the\nCTGP-7 server to check the\nbeta build. Try again later.\n\nPress HOME to exit.");
						}
					} else if (Net::GetBetaState() == Net::BetaState::NODISCORD) {
						openDialog(DialogFlags(DialogFlags::Mode::NOBUTTON), "You are not allowed to use\nthis beta build.\nPlease check your discord beta\nstatus from the ingame menu.\n\nPress HOME to exit.");
					}
				}
				showingBeta = false;
				openedFirst = false;
			}
		}
		if (Net::GetBetaState() == Net::BetaState::NO || Net::GetBetaState() == Net::BetaState::FAILED || Net::GetBetaState() == Net::BetaState::NODISCORD) return 0;
#endif // BETA_BUILD
		if (SaveHandler::saveData.flags1.firstOpening) {
			if (isDialogOpened()) return 0;
			if (g_welccounter == 3) PluginMenu::ForceOpen();
			openDialog(DialogFlags::Mode::OK, NAME("welc_" << std::to_string(g_welccounter)));
			g_welccounter++;
			if (g_welccounter > 7) {
				SaveHandler::saveData.flags1.firstOpening = false;
			}
			return 0;
		} else if (SaveHandler::CheckAndShowAchievementMessages())
			return 0;
		else if (BadgeManager::CheckAndShowBadgePending())
			return 0;
		if (g_isFoolActive || MarioKartFramework::isDialogOpened())
			return 0;

		return g_StresserGetTitleMenuTimer(timerVal);
	}

	void MarioKartFramework::updateReplayFileName()
	{
		if (replayFileName == nullptr) return;
		CCSettings* settings = static_cast<CCSettings*>(ccselectorentry->GetArg());
		char* replace;
		char fmt[5];
		if (settings->enabled) {
			sprintf(fmt, "%04hu", (u16)settings->value);
			replace = fmt;
		}
		else {
			replace = (char*)"play";
		}
		memcpy(replayFileName + 0x9, replace, 4);
	}
	
	void MarioKartFramework::warnLedItem(u32 vehicle, u32 item)
	{
		if (SaveHandler::saveData.flags1.warnItemEnabled && !g_isFoolActive) {
        	int playerID = ((u32*)vehicle)[0x84/4];
			int playerPosition = getModeManagerData()->driverPositions[playerID];
			int myPosition = getModeManagerData()->driverPositions[masterPlayerID];

			if (item == EItemSlot::ITEM_KOURAB) {
				if (myPosition < playerPosition || (myPosition == 1 && playerPosition == 1))
					LED::PlayLEDPattern(ledBlueShellPat, Seconds(5));
			}
			else if (item == EItemSlot::ITEM_THUNDER) {
				if (playerID != masterPlayerID)
					LED::PlayLEDPattern(ledThunderPat, Seconds(1));
			}
			else if (item == EItemSlot::ITEM_GESSO) {
				if (myPosition < playerPosition || (myPosition == 1 && playerPosition == 1))
					LED::PlayLEDPattern(ledBlooperPat, Seconds(2.5f));
			}
		}
	}

	extern "C" u32* g_timerPointer;
	void MarioKartFramework::handleItemCD(u32 vehicle, u32 item) {
		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD && (item == EItemSlot::ITEM_GESSO || item == EItemSlot::ITEM_THUNDER) && ((u32*)vehicle)[0x84 / 4] == masterPlayerID) {
			u32* points = (u32*)(vehicle + 0xBD4);
			if (*points < 10) {
				(*points)++;
				playCDPointUpSE(*points);
				u32* timerAddr = (u32*)(g_timerPointer[0] + 0x4/4);
				*timerAddr = MarioKartTimer::ToFrames(0,3);
			}
		}
	}

	void MarioKartFramework::playCDPointUpSE(u32 points)
	{
		if (points > previousCDScore) {
			if (points < 10) Snd::PlayMenu(Snd::PAUSE_OFF);
			else Snd::PlayMenu(Snd::PAUSE_TO_NEXT);
			previousCDScore = points;
		}
	}

	int MarioKartFramework::registerButtonReturnCode(u32 button, void(*callback)(u32* retCode), int id)
	{
		if (!callback) return -1;
		if (id < 0) {
			regButtons.push_back({ button, callback });
			return regButtons.size() - 1;
		}
		else {
			if (id >= (int)regButtons.size()) return -1;
			regButtons[id] = { button, callback };
			return id;
		}
	}

	u32 MarioKartFramework::getButtonByID(int id)
	{
		if (id >= 0 && id < regButtons.size()) {
			return std::get<0>(regButtons[id]);
		}
		return 0;
	}

	void MarioKartFramework::performButtonCallback(u32 button, u32* val)
	{
		for (auto it = regButtons.begin(); it != regButtons.end(); it++) {
			if (std::get<0>(*it) == button) {
				std::get<1>(*it)(val);
				break;
			}
		}
	}

	void MarioKartFramework::changeButtonText(u32 button, std::string& text, const char* textElement)
	{
		std::u16string utf16;
		Utils::ConvertUTF8ToUTF16(utf16, text);
		changeButtonText(button, (u16*)utf16.c_str(), textElement);
	}

	void MarioKartFramework::changeButtonText(u32 button, u16* text, const char* textElement)
	{
		u32 layout = ((u32*)button)[0x1A];
		SafeStringBase element(textElement);
		u32(*getElementHandle)(u32, u32, u32) = (u32(*)(u32,u32,u32))((u32**)layout)[0][6];
		u32 elementHandle = getElementHandle(layout, (u32)&element, 1);
		u16* ptr = text;
		u16** ptr2 = &ptr;
		nwlytReplaceText(layout, elementHandle, (u32)ptr2, 0, 0);
	}

	static u8 g_page_finish_counter = 0;
	static bool g_page_finish_started_counter = false;
	bool MarioKartFramework::onBasePageCanFinishFadeCallback(u32 page, u32 currFrame, u32 maxFrame)
	{
		bool ret = true;
		MenuPageHandler::MenuSingleModePage* menuSingleMode = MenuPageHandler::GetMenuSingleModePage();
		if (page == VersusHandler::MenuSingleClassPage) {
			if (VersusHandler::IsVersusMode && VersusHandler::canShowVSSet) {
				if (currFrame == maxFrame && g_page_finish_counter == 0 && !g_page_finish_started_counter) {
					g_page_finish_counter = 0x2A;
					g_page_finish_started_counter = true;
					ret = false;
				}
				else if (g_page_finish_counter == 0 && currFrame != maxFrame && g_page_finish_started_counter) {
					g_page_finish_started_counter = false;
					VersusHandler::canShowVSSet = false;
				}
				if (g_page_finish_counter > 0) {
					g_page_finish_counter--;
					ret = false;
				}
				if (g_page_finish_counter == 0x8) {
					VersusHandler::openSettingsKeyboardMode = 1;
					AsyncRunner::StartAsync(VersusHandler::OpenSettingsKeyboardCallback);
				}
			}
		} else if (menuSingleMode && (page == (u32)(menuSingleMode->gameSection))) {
			if (menuSingleMode->openCTGP7Setting) {
				if (currFrame == maxFrame && g_page_finish_counter == 0 && !g_page_finish_started_counter) {
					g_page_finish_counter = 0x2A;
					g_page_finish_started_counter = true;
					ret = false;
				}
				else if (g_page_finish_counter == 0 && currFrame != maxFrame && g_page_finish_started_counter) {
					g_page_finish_started_counter = false;
					menuSingleMode->openCTGP7Setting = false;
				}
				if (g_page_finish_counter > 0) {
					g_page_finish_counter--;
					ret = false;
				}
				if (g_page_finish_counter == 0x8) {
					VersusHandler::openSettingsKeyboardMode = 2;
					AsyncRunner::StartAsync(VersusHandler::OpenSettingsKeyboardCallback);
				}
			}
		}
		return ret;
	}

	bool MarioKartFramework::IsLastGPTrack(u32 track)
	{
		if (VersusHandler::IsVersusMode)
			return track == SaveHandler::saveData.vsSettings.roundAmount - 1;
		else if (UserCTHandler::IsUsingCustomCup())
			return UserCTHandler::IsLastRace(track);
		else {
			return track == 3;
		}
	}

	void MarioKartFramework::OnGPKartFinishRace(u32* finishPosition, u32* halfDriverAmount, u32 playerID) {
		if (MissionHandler::isMissionMode) MissionHandler::OnGPKartFinishRace(finishPosition, halfDriverAmount, playerID);
	}

	void MarioKartFramework::generateCPUDataSettings(CPUDataSettings* buffer, u32 playerID, u32 totalPlayers, ModeManagerData* modeData)
	{
		static u32 chosenValue = 0;
		if (ISGAMEONLINE) {
			if (true || isWatchRaceMode) {
				modeData->isMultiCPUEnabled = false;
				modeData->multiCPUCount = 0;
				return;
			}
			if (totalPlayers >= 8) {
				modeData->isMultiCPUEnabled = false;
				modeData->multiCPUCount = 0;
			}
			else {
				if (playerID + 1 == 8) {
					modeData->isMultiCPUEnabled = true;
					modeData->multiCPUCount = 8 - totalPlayers;
				}
			}
			
			if (!israndomdriverchoicesshuffled) {
				for (int i = 0; i < totalPlayers; i++) {
					u8 currSeed = getPlayerNetworkSelectMenuBuffer(i)[0x12];
					if (currSeed != 0) cpuRandomSeed = cpuRandomSeed * currSeed * 0x6F46E833;
				}
			}
			u8 randomChoices[4];
			getRandomDriverChoice(randomChoices, playerID);
			buffer->character = randomChoices[0];
			buffer->kartBody = randomChoices[1];
			buffer->kartTire = randomChoices[2];
			buffer->kartWing = randomChoices[3];
		}
		else {
			if (modeData->isMultiCPUEnabled) {
				modeData->multiCPUCount = 8 - totalPlayers;
			}
		}
		
	}

	void MarioKartFramework::SetWatchRaceMode(bool setMode) {
		isWatchRaceMode = setMode;
		CharacterHandler::OnWatchRaceEnabled(isWatchRaceMode);
	}

	u32 MarioKartFramework::getNumberOfRacers(u32 players, bool isFake)
	{
		u32 ret = players;
		if (players == 8 && VersusHandler::IsVersusMode) ret = SaveHandler::saveData.vsSettings.cpuAmount + 1;
		else if (players == 8 && MissionHandler::isMissionMode) ret = MissionHandler::missionParam->DriverOptions->playerAmount;
		else if (players == 8 && UserCTHandler::skipConfig.enabled) ret = UserCTHandler::skipConfig.cpuAmount;
		if (!isFake) numberPlayersRace = ret;
		return ret;
	}

	bool MarioKartFramework::blockWinningRun() {
		CrashReport::stateID = CrashReport::StateID::STATE_TROPHY;
		if (VersusHandler::IsVersusMode && SaveHandler::saveData.vsSettings.cpuAmount < 2) return true;
		if (UserCTHandler::skipConfig.enabled && UserCTHandler::skipConfig.cpuAmount < 2) return true;
		return false;
	}

	void MarioKartFramework::resetdriverchoices()
	{
		israndomdriverchoicesshuffled = false;
		for (int i = 0, j = 0; i < sizeof(randomdriverchoices); i++, j++) {
			if (i == 9) j+=2;
			randomdriverchoices[i] = j;
		}
		cpuRandomSeed = 1;
		neededCPU = 0;
		neededCPUCurrentPlayers = 0;
	}

	static u32 g_chosenCPURandomValue = 0;
	void MarioKartFramework::getRandomDriverChoice(u8 out[4], int playerID) {
		if (!israndomdriverchoicesshuffled) {
			israndomdriverchoicesshuffled = true;
			randomDriverChoicesGenerator.seed(cpuRandomSeed);
			for (int i = 0; i < 0x40; i++) {
				int id0 = randomDriverChoicesGenerator() % sizeof(randomdriverchoices), id1 = randomDriverChoicesGenerator() % sizeof(randomdriverchoices);
				u8 temp = randomdriverchoices[id0];
				randomdriverchoices[id0] = randomdriverchoices[id1];
				randomdriverchoices[id1] = temp;
			}
			g_chosenCPURandomValue = randomDriverChoicesGenerator();
		}
		out[0] = randomdriverchoices[playerID];
		out[1] = ((g_chosenCPURandomValue) * (playerID + 0x5FFF5)) % 17;
		out[2] = ((g_chosenCPURandomValue >> 8) * (playerID + 0x5FFF5)) % 10;
		out[3] = ((g_chosenCPURandomValue >> 16) * (playerID + 0x5FFF5)) % 7;
	}

	int MarioKartFramework::getRandomDriverWifiRateOffset(int playerID) {
		int value = ((g_chosenCPURandomValue >> 16) * (playerID + 0x5FFF5)) % 1000;
		value -= 500;
		return value;
	}

	void* MarioKartFramework::getNetworkSelectMenuProcess()
	{
		u32 sequenceEngine = getSequenceEngine();
		if (!sequenceEngine) return nullptr;
		return (void*)(*(u32*)(getMenuData() + 0x7A0));
	}

	u8* MarioKartFramework::getMyNetworkSelectMenuBuffer()
	{
		return (u8*)((u32)getNetworkSelectMenuProcess() + 0x68);
	}

	u8* MarioKartFramework::getPlayerNetworkSelectMenuBuffer(int playerID)
	{
		return networkGetPlayerStatus(getNetworkSelectMenuProcess(), playerID, nullptr);
	}

	EDriverID MarioKartFramework::GetVoteBarDriver(EDriverID original) {
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
			return EDriverID::DRIVER_MIIM;
		else
			return original;
	}

	void MarioKartFramework::BasePage_SetWifiRate(int slot, u32 vrAmount) {
		if (slot < MAX_PLAYER_AMOUNT) {
			getRaceInfo(true)->kartInfos[slot].vr = vrAmount;
		}
	}

	void MarioKartFramework::setSkipGPCoursePreview(bool skip)
	{
		skipGPCoursePreview = skip;
		SequenceHandler::addFlowPatch(SequenceHandler::rootSequenceID, 0x634, skip ? 3 : 2, skip ? 5 : 1);
		SequenceHandler::addFlowPatch(SequenceHandler::rootSequenceID, 0x67C, skip ? 3 : 2, skip ? 5 : 1);
	}

	u32 MarioKartFramework::forceFinishRaceCallback(u32 flags)
	{
		if (forceFinishRace) {
			isRaceGoal = true;
			forceFinishRace = false;
			flags |= 1;
		}
		return flags;
	}

	u32 MarioKartFramework::getResultBarAmount(u32 amount)
	{
		if (forcedResultBarAmount != -1) realResultBarAmount = forcedResultBarAmount;
		else realResultBarAmount = amount;
		return realResultBarAmount;
	}

    void MarioKartFramework::BaseResultBar_setCountryVisible(BaseResultBar bar, bool visible)
    {
		u32 flagHandle = ((u32*)bar)[0x8C/4];
		VisualControl::NwlytControlSight* nwlyt = ((VisualControl::GameVisualControl*)bar)->GetNwlytControl();
		nwlyt->vtable->setVisibleImpl(nwlyt, flagHandle, visible);
    }

    void MarioKartFramework::OnRacePageGenGP(void *racePage)
    {
        if (MissionHandler::isMissionMode) {MissionHandler::OnRacePageGenGP(racePage); return;}
		// Original
		RacePageInitFunctions[RacePageInitID::NAME](racePage);
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			for (auto it = onlineBotPlayerIDs.cbegin(); it != onlineBotPlayerIDs.cend(); it++) {
				FixedStringBase<char16_t, 0x20>* playerNames = getPlayerNames();
				const char16_t* realName = it->second.strData;
				strcpy16n(playerNames[it->first].strData, realName, playerNames[it->first].bufferSize * sizeof(char16_t));
			}
		}
		onlineBotPlayerIDs.clear();
		CharacterHandler::UpdateRaceCharaText();
		RacePageInitFunctions[RacePageInitID::TEXTURE](racePage);
		RacePageInitFunctions[RacePageInitID::EFFECT](racePage);
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0 && ((u32*)racePage)[0x26C/4] != 4) {
			RacePageInitFunctions[RacePageInitID::ITEM_SLOT](racePage);
			#ifndef NO_HUD_RACE
			RacePageInitFunctions[RacePageInitID::RANK](racePage);
			#endif
		}
		if (unkFlag == 0) SpeedometerController::Load();
		RacePageInitFunctions[RacePageInitID::MAP](racePage);
		RacePageInitFunctions[RacePageInitID::MAP_ICON](racePage);
		if (unkFlag == 0) {
			RacePageInitFunctions[RacePageInitID::RANK_BOARD](racePage);
			RacePageInitFunctions[RacePageInitID::LAP](racePage);
			RacePageInitFunctions[RacePageInitID::COIN](racePage);
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
			PointsModeHandler::InitDisplayController();
		}
		RacePageInitFunctions[RacePageInitID::WIPE](racePage);
		RacePageInitFunctions[RacePageInitID::TEXT](racePage);
		RacePageInitFunctions[RacePageInitID::CAPTION](racePage);
		// Custom
		#ifndef NO_HUD_RACE
		RacePageInitFunctions[RacePageInitID::TIMER](racePage);
		#endif
		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD) RacePageInitFunctions[RacePageInitID::POINT](racePage);
    }

    void MarioKartFramework::OnRacePageGenTA(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
		CharacterHandler::UpdateRaceCharaText();
	}

	void MarioKartFramework::OnRacePageGenBBT(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
		CharacterHandler::UpdateRaceCharaText();
	}
	
	void MarioKartFramework::OnRacePageGenCBT(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
		CharacterHandler::UpdateRaceCharaText();
	}

	bool MarioKartFramework::userCanControlKart()
	{
		for (int i = 0; i < numberPlayersRace && i < MAX_PLAYER_AMOUNT; i++)
			if (playerInfos[i].type == EPlayerType::TYPE_USER)
				return true;
		return false;
	}

	bool MarioKartFramework::blockStartGridFormation()
	{
		return getNumberOfRacers(8, true) == 1;
	}

	void MarioKartFramework::startItemSlot(u32 playerID, u32 boxID)
	{
		ItemDirector_StartSlot(getItemDirector(), playerID, boxID);
	}

    void MarioKartFramework::OnItemDirectorStartSlot(u32 itemDirector, u32 playerID, u32 boxID)
    {
		if (playerID == masterPlayerID && ItemHandler::GetItemSlotStatus(playerID).mode == ItemHandler::ItemSlotStatus::MODE_EMPTY)
			ExtendedItemBoxController::NotifyItemBoxStarted();
		if (PointsModeHandler::isPointsMode) PointsModeHandler::OnItemDirectorStartSlot(playerID, boxID);
		((void(*)(u32, u32, u32))itemDirectorStartSlotHook.callCode)(itemDirector, playerID, boxID);
    }

    void MarioKartFramework::storeImprovedRouletteItemProbability(u32 itemID, u16 prob)
	{
		if (startedItemSlot == masterPlayerID)
			itemProbabilitiesForImprovedRoulette[itemID] = prob;
	}

	u16 MarioKartFramework::handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag)
	{
		bool isDecided = strcmp((char*)csvPtr + 0x50, "ItemSlotTable_Decided") == 0;
		if (MissionHandler::neeedsCustomItemHandling()) return MissionHandler::handleItemProbability(dstArray, csvPtr, rowIndex, blockedBitFlag);
		if (!isDecided && useCustomItemProbability) {
			u16 ret = handleCustomItemProbabilityRecursive(dstArray, csvPtr, rowIndex, blockedBitFlag, 0);
			if (!ret) {
				dstArray[EItemSlot::ITEM_KINOKO] = 200;
				return 200;
			}
			return ret;
		}
		u16 totalProb = 0;
		if (!bulletBillAllowed) blockedBitFlag |= (1 << EItemSlot::ITEM_KILLER);
		if (nextForcedItemBlockedFlags) {blockedBitFlag |= nextForcedItemBlockedFlags; nextForcedItemBlockedFlags = 0;}
		if (ItemHandler::isBlueShellShowdown && !ItemHandler::isLessThan30Seconds) {
			blockedBitFlag &= ~(1 << EItemSlot::ITEM_KOURAB);
		}
		for (int i = 0; i < EItemSlot::ITEM_SIZE; i++) {
			if ((blockedBitFlag & (1 << i)) == 0 || nextForcedItem >= 0) {
				u16 currProb = (nextForcedItem >= 0) ? ((i == nextForcedItem) ? 200 : 0) : CsvParam_getDataInt(csvPtr, rowIndex, i);
				totalProb += currProb;
			}
			dstArray[i] = totalProb;
			storeImprovedRouletteItemProbability(i, totalProb);
		}
		nextForcedItem = -1;
		return totalProb;
	}

	u32 MarioKartFramework::pullRandomItemID()
	{
		if (SaveHandler::saveData.flags1.improvedRoulette || MissionHandler::isMissionMode) {
			int max = itemProbabilitiesForImprovedRoulette[EItemSlot::ITEM_SIZE - 1] - 1;
			u32 randomChoice = Utils::Random(0, ((max < 0) ? 0 : max));
			int i = 0;
			for (; i < EItemSlot::ITEM_SIZE; i++) {
				if (itemProbabilitiesForImprovedRoulette[i] > randomChoice) return i;
			}
			return EItemSlot::ITEM_KINOKO;
		}
		else
			return Utils::Random(0, EItemSlot::ITEM_SIZE - 1);
	}

	const char* MarioKartFramework::getOnlineItemTable(bool isAI) {
		if (ItemHandler::isBlueShellShowdown)
			return isAI ? "ItemSlotTable_BlueShow_AI" : "ItemSlotTable_BlueShow";
		else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
			return isAI ? "ItemSlotTable_WiFi_AI" : "ItemSlotTable_CD";
		else if ((g_getCTModeVal == CTMode::ONLINE_CTWW || (currentRaceMode.type == 1 && (currentRaceMode.mode == 2 || currentRaceMode.mode == 0))))
			return isAI ? "ItemSlotTable_CTWW_AI" : "ItemSlotTable_CTWW";
		else 
			return isAI ? "ItemSlotTable_WiFi_AI" : "ItemSlotTable_WiFi";
	}

	const char* MarioKartFramework::getOnlineBattleItemTable(bool isCoin, bool isAI) {
		if (currentRaceMode.type == 1) {
			if (isCoin)
				return isAI ? "ItemSlotTable_Coin_AI" : "ItemSlotTable_Coin";
			else
				return isAI ? "ItemSlotTable_Balloon_AI" : "ItemSlotTable_Balloon";
		} else {
			if (isCoin)
				return isAI ? "ItemSlotTable_Coin_WiFi_AI" : "ItemSlotTable_Coin_WiFi";
			else
				return isAI ? "ItemSlotTable_Balloon_WiFi_AI" : "ItemSlotTable_Balloon_WiFi";
		}
	}

	void MarioKartFramework::improvedRouletteSettings(MenuEntry* entry)
	{
		Keyboard kbd(NAME("itmprb") + "\n\n" + NAME("state") + ": " + (SaveHandler::saveData.flags1.improvedRoulette ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) + ResetColor());
		kbd.Populate({ NAME("state_inf"), NOTE("state_inf") });
		kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = kbd.Open();
		if (ret < 0) return;
		SaveHandler::saveData.flags1.improvedRoulette = ret == 0;
		entry->Name() = NAME("itmprb") + " (" + (SaveHandler::saveData.flags1.improvedRoulette ? NAME("state_mode") : NOTE("state_mode")) + ")";
	}

	void MarioKartFramework::raceTimerOnFrame(MarioKartTimer timer)
	{
		if (speedupMusicOnLap && !isAltGameMode) {
			KartLapInfo* info = MarioKartFramework::getKartLapInfo(MarioKartFramework::masterPlayerID);
			u8 currLap = info->currentLap;
			if (currLap != lastCheckedLap) {
				lastCheckedLap = currLap;
				u8 totLaps = 0;
				if (UserCTHandler::IsUsingCustomCup())
					totLaps = UserCTHandler::GetCurrentCourseLapAmount();
				else
					totLaps = CourseManager::getCourseLapAmount(CourseManager::lastLoadedCourseID);
				if (currLap == totLaps)
					raceMusicSpeedMultiplier = 1.f;
				else
					raceMusicSpeedMultiplier = ((currLap - 1) / (float)totLaps) * 0.2625f + 1.f;
			}
		}
		if (UserCTHandler::skipConfig.enabled) {
			if (UserCTHandler::skipConfig.itemID >= 0) {
				if (Controller::IsKeyDown(Key::DPadDown)) {
					nextForcedItem = UserCTHandler::skipConfig.itemID;
					nextForcedInstantSlot = true;
					bool prev = PointsModeHandler::isPointsMode;
					PointsModeHandler::isPointsMode = false;
					startItemSlot(masterPlayerID, 0);
					PointsModeHandler::isPointsMode = prev;
				} else {
					nextForcedItem = -1;
					nextForcedInstantSlot = false;
				}
			}
			/*if (Controller::IsKeyDown(Key::DPadLeft)) {
				nextForcedItem = EItemSlot::ITEM_KOURAG;
				nextForcedInstantSlot = true;
				startItemSlot(masterPlayerID, 0);
			} else if (Controller::IsKeyDown(Key::DPadRight)) {
				nextForcedItem = EItemSlot::ITEM_BANANA;
				nextForcedInstantSlot = true;
				startItemSlot(masterPlayerID, 0);
			} else {
				nextForcedInstantSlot = false;
				nextForcedItem = -1;
			}*/
			if (UserCTHandler::skipConfig.useLeftToFinish && Controller::IsKeyPressed(Key::DPadLeft)) {
				forceFinishRace = true;
				// g_altGameModeIsRaceOver = 0x69; For online mode
			}
		}
		ItemHandler::raceTimerOnFrame(timer);
	}

	void MarioKartFramework::handleIncrementTimer(u32* timer)
	{
		u32 secondaryTimer = timer[0xB];
		if (secondaryTimer != 0xFFFFFFFF) timer[0xB]++;
		MarioKartTimer mainTimer(timer[1]);
		if (MissionHandler::isMissionMode) MissionHandler::timerHandler(mainTimer);
		else if (isAltGameMode) {
			if (g_gameModeID == 0) { // Countdown
				if (mainTimer.GetFrames() == 0) {
					if (g_altGameModeIsRaceOver != 0x69) g_altGameModeIsRaceOver = 0x69;
				}
				else --mainTimer;

				g_altGameModeHurryUp = mainTimer < MarioKartTimer(MarioKartTimer::ToFrames(0, 40, 0));
				if (countdownDisconnectTimer) countdownDisconnectTimer--;

			}
			if (g_gameModeID == 1) { // Elimination
				if (mainTimer.GetFrames() == 0) {
					mainTimer = MarioKartTimer(MarioKartTimer::ToFrames(0, 30, 0));
				}
				else --mainTimer;
			}
		}
		else ++mainTimer;
		timer[1] = mainTimer.GetFrames();
		if (ItemHandler::isBlueShellShowdown) {
			ItemHandler::isLessThan30Seconds = mainTimer < MarioKartTimer(0, 30, 0);
		}
		raceTimerOnFrame(mainTimer);
	}

	void MarioKartFramework::handleDecrementTimer(u32* timer)
	{
		MarioKartTimer clock(timer[1]);
		if (clock.GetFrames() != 0)
		{
			--clock;
			timer[1] = clock.GetFrames();
		}
		raceTimerOnFrame(MarioKartTimer(timer[1]));
	}

	void MarioKartFramework::setTimerInitialValue(u32* timer)
	{
		MarioKartTimer mainTimer(0);
		if (isAltGameMode) {
			if (g_gameModeID == 0) { // Countdown
				mainTimer = MarioKartTimer(g_downTimerStartTime);
			}
			else if (g_gameModeID == 1) {
				mainTimer = MarioKartTimer(MarioKartTimer::ToFrames(0, 30, 0));
			}
		}
		else if (MissionHandler::isMissionMode) {
			MissionHandler::setInitialTimer(mainTimer);
		}
		timer[1] = mainTimer.GetFrames();
		timer[0xB] = 0;
		countdownDisconnectTimer = 0;
	}

	bool MarioKartFramework::pointControlShouldBeYellow(u32 val)
	{
		if (MissionHandler::isMissionMode) return MissionHandler::pointControlShouldBeYellow(val);
		else return val >= 10;
	}

	u32 MarioKartFramework::carGetController(u32 carObject)
	{
		GOBJEntry* entry = (GOBJEntry*)(((u32**)carObject)[2][0]);
		u16 route = entry->routeID;
		for (auto it = carRouteControllerPair.cbegin(); it != carRouteControllerPair.cend(); it++) {
			if (std::get<0>(*it) == route)
				return std::get<1>(*it);
		}
		return 0;
	}

	void MarioKartFramework::carStoreController(u32 carObject, u32 carController)
	{
		GOBJEntry* entry = (GOBJEntry*)(((u32**)carObject)[2][0]);
		u16 route = entry->routeID;
		for (auto it = carRouteControllerPair.cbegin(); it != carRouteControllerPair.cend(); it++) {
			if (std::get<0>(*it) == route)
				return;
		}
		carRouteControllerPair.push_back(std::make_pair(route, carController));
	}

	u32 MarioKartFramework::penguinGeneratorGetGenNum(u32 generator, u32* penguinGeoObject)
	{
		GOBJEntry* penguin = ((GOBJEntry**)penguinGeoObject)[0];
		CRaceInfo* raceInfo = &(getModeManagerData()->modeRaceInfo);
		
		if ((raceInfo->raceMode.mode == 5 || raceInfo->raceMode.mode == 8) && (penguin->settings[6] & 1) == 0)
			return 3;
		else
			return penguin->settings[2];
	}

	u32 MarioKartFramework::onKartHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 vehicleReactObject, u32 objCallFuncPtr)
	{
		u16 objID = ((GOBJEntry***)object)[2][0]->objID;
		int playerID = ((u32*)vehicleReactObject)[0x84 / 4];

		if (objID == 0xCF) { // Penguin
			GOBJEntry* entry = ((GOBJEntry***)object)[2][0];
			if ((entry->settings[6] & (1 << 1)) && !megaMushTimers[playerID] && kartCanAccident((u32*)vehicleReactObject))
				EGTHReact = EGHTREACT_SPEEDSPIN;
		}
		if (objID == 0xE0) { // Goat
			GOBJEntry* entry = ((GOBJEntry***)object)[2][0];
			if ((entry->settings[6] & (1 << 0)) && !megaMushTimers[playerID] && kartCanAccident((u32*)vehicleReactObject))
				EGTHReact = EGHTREACT_BIGHITFAR;
		}
		if (objID == 0x70) { // DS Moving tree
			GOBJEntry* entry = ((GOBJEntry***)object)[2][0];
			if (entry->settings[6] & (1 << 0)) {
				eObjectReactType = EObjectReactType::OBJECTREACTTYPE_NONE;
				if (!megaMushTimers[playerID] && kartCanAccident((u32*)vehicleReactObject))
					return EGHTREACT_BIGHITCLOSE;
				else
					EGTHReact = EGHTREACT_NONE;
			}
		}

		if (megaMushTimers[playerID] && kartCanAccident((u32*)vehicleReactObject)) {
			if (objID == 0xCE) { // Star Thwomp
				eObjectReactType = EObjectReactType::OBJECTREACTTYPE_NONE;
				EGTHReact = EGHTREACT_THWOMP;
			} else if (objID == 0x7C || objID == 0x79 || objID == 0x6B) { // chain chomp/boulder//pinball
				eObjectReactType = EObjectReactType::OBJECTREACTTYPE_NONE;
				EGTHReact = EGHTREACT_WALL1;
			} else if (objID == 0xD2) { // Airship Bullets
				eObjectReactType = EObjectReactType::OBJECTREACTTYPE_NONE;
				EGTHReact = EGHTREACT_BIGHITFAR;
			} else if (objID == 0x181 || objID == 0x165) { // Water puddle/oil puddle
				EGTHReact |= 0x80000000;
			}
		}

		if (objID == 0x7D && BlueCoinChallenge::coinWasSpawned) {
			if (BlueCoinChallenge::coinSpawned && playerID == MarioKartFramework::masterPlayerID) 
				BlueCoinChallenge::OnKartHitCoin(vehicleReactObject);
			return EGHTREACT_NONE;
		}

		// Special handling for some objects...
		// (keep in mind for mission mode)
		if (objID == 0xDC) { // Piranha plant
			u8* state1 = ((u8*)object) + 0x18E;
			u8* state2 = ((u8*)object) + 0x1A5;
			u8 prevState1 = *state1;
			u8 prevState2 = *state2;
			u32 ret = ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, vehicleReactObject);
			if (prevState2 != *state2 && *state2 == 3) { // Plant decided to attack
				if (packunStunCooldownTimers[playerID]) { // Cancel attack
					*state1 = prevState1;
					*state2 = prevState2;
				} else {
					packunStunCooldownTimers[playerID] = MarioKartTimer::ToFrames(0, 5, 0);
				}	
			}
			if (prevState2 != *state2 && *state2 == 5) {
				if (PointsModeHandler::isPointsMode) PointsModeHandler::DoPointAction(PointsModeHandler::PointAction::HIT_PIRANHAPLANT);
			}
			return ret;
		}

		if (objID == 0x1D) {
			if (dokanWarpCooldown[playerID] != 0)
				return EGHTREACT_NONE;
		}

		//NOXTRACE("dfffdsd", "oid: 0x%x, obt: %d, egth: 0x%x", objID, eObjectReactType, EGTHReact);

		if (MissionHandler::isMissionMode) return MissionHandler::onKartItemHitGeoObject(object, EGTHReact, eObjectReactType, vehicleReactObject, objCallFuncPtr, 0);
		if (PointsModeHandler::isPointsMode) return PointsModeHandler::OnKartItemHitGeoObject(object, EGTHReact, eObjectReactType, vehicleReactObject, objCallFuncPtr, 0);
		else return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, vehicleReactObject);
	}
	
	u32 MarioKartFramework::onItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 itemReactProxyObject, u32 objCallFuncPtr)
	{
		// NOTE: If itemReactProxyObject == nullptr, function call comes from ObjectDirector::ReactThunder
		int mode = itemReactProxyObject == 0 ? 2 : 1;
		u16 objID = ((GOBJEntry***)object)[2][0]->objID;

		if (objID == 0xE0) { // Goat
			GOBJEntry* entry = ((GOBJEntry***)object)[2][0];
			if ((entry->settings[6] & (1 << 0))) {
				eObjectReactType = EObjectReactType::OBJECTREACTTYPE_NONE;
			}				
		}

		if (MissionHandler::isMissionMode) return MissionHandler::onKartItemHitGeoObject(object, EGTHReact, eObjectReactType, itemReactProxyObject, objCallFuncPtr, mode);
		if (PointsModeHandler::isPointsMode) return PointsModeHandler::OnKartItemHitGeoObject(object, EGTHReact, eObjectReactType, itemReactProxyObject, objCallFuncPtr, mode);
		else return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, itemReactProxyObject);
	}

	void MarioKartFramework::OnKartAccident(u32* vehicleReact, ECrashType* type) {
		MissionHandler::OnKartAccident(vehicleReact, type);
		if (*type == ECrashType::CRASHTYPE_FLIP && ItemHandler::FakeBoxHandler::isHittingFib) {
			ItemHandler::FakeBoxHandler::isHittingFib = false;
			*type = ECrashType::CRASHTYPE_GREENSPARK;
		}
	}

	bool MarioKartFramework::kartCanAccident(u32* vehicleReact) {
		u32 starTimer = vehicleReact[(0xC00 + 0x3F4) / 4];
		u32 unknownTimer = vehicleReact[(0x1000 + 0x2C) / 4];
		bool isBullet = vehicleReact[(0xC30) / 4] & 0x400000;
		bool isHangingLakitu = vehicleReact[(0xC30) / 4] & 0x800000;
		bool isRespawning = vehicleReact[(0xC30) / 4] & 0xA000060;
		return !starTimer && !unknownTimer && !isBullet && !isHangingLakitu && !isRespawning;
	}

	void MarioKartFramework::OnSndActorKartPlayDriverVoice(u32* sndActorKart, EVoiceType voice) {
		if (voice == EVoiceType::DASH_BOOST) {
			u32* vehicle = (u32*)sndActorKart[0x1E0/4];
			int playerID = vehicle[0x21];
			if (vehicleIsInLoopKCL((u32)vehicle)) {
				if (loopVoiceCooldown[playerID] == 0) {
					SndActorKArt_PlayDriverVoice(sndActorKart, EVoiceType::GLIDER_CANNON);
				}
				loopVoiceCooldown[playerID] = MarioKartTimer(0, 3, 0);
				return;
			} else if (loopVoiceCooldown[playerID] >= MarioKartTimer(0, 2, 800))
				return;
		}
		SndActorKArt_PlayDriverVoice(sndActorKart, voice);
	}

	void MarioKartFramework::resetTrickInfos(int playerID, u32 kartUnitObj)
	{
		trickInfos[playerID].Reset();
	}

	u32 MarioKartFramework::onGliderTwistStart(u32* kartVehicleMoveObj, u32 restartBool)
	{
		if (!(restartBool & 0x80000000)) {
			trickInfos[kartVehicleMoveObj[0x21]].trickMode = 0;
		}
		return restartBool & 0x7FFFFFFF;
	}

	float* MarioKartFramework::onGliderTwistCalc(u32* kartVehicleObj, int callVal)
	{
		int playerID = kartVehicleObj[0x21];

		if (callVal == 0) {
			float* rotAngle = (float*)((u32)kartVehicleObj + 0x1400 + 0xB8);
			u8 dirMode = trickInfos[playerID].trickMode >> 4;
			if (dirMode == 1) *rotAngle = -1.0f;
			else if (dirMode == 2) *rotAngle = 1.0f;
			return nullptr;
		}
		else if (callVal == 1) {

			Vector3* normalVec = (Vector3*)(kartVehicleObj[0xC] + 0xC);
			Vector3* directionVec = (Vector3*)(kartVehicleObj[0xC] + 0x18);

			switch (trickInfos[playerID].trickMode & 0xF)
			{
			case 1:
				trickInfos[playerID].trickRotationAxes = *normalVec;
				break;
			case 2:
				sead_crossProduct(&trickInfos[playerID].trickRotationAxes, directionVec, normalVec);
				sead_normalize(&trickInfos[playerID].trickRotationAxes);
				break;
			case 0:
			default:
				trickInfos[playerID].trickRotationAxes = *directionVec;
				break;
			}

			return (float*)&trickInfos[playerID].trickRotationAxes;
		}
		else if (callVal == 2) {
			return (float*)&trickInfos[playerID].trickRotationAxes;
		}
		return nullptr;
	}

	float MarioKartFramework::onGliderTwistGetDecreaseAmount(u32* vehicleMove, float origAmount) {
		int playerID = vehicleMove[0x21];
		if (trickInfos[playerID].trickMode & 0xF)
			return origAmount * 0.9f;
		return origAmount;
	}

	u32 MarioKartFramework::onDashExecCallback(u32* kartVehicleMoveObj, u32 dashType, bool flag1, bool flag2)
	{
		//u32* driverObj = ((u32***)kartVehicleMoveObj)[0x1D][0x1F];
		u32 ret = dashType;
		int playerID = kartVehicleMoveObj[0x21];

		MissionHandler::OnKartDash(kartVehicleMoveObj, (EDashType)dashType, trickInfos[playerID].readyToDoFasterBoost);
		PointsModeHandler::OnKartDash(kartVehicleMoveObj, (EDashType)dashType);

		/*if (dashType == EDashType::SUPERMINITURBO)
		{
			u32 sndActorKart = kartVehicleMoveObj[0x37];
			SndActorKArt_PlayDriverVoice(sndActorKart, EVoiceType::DASH_BOOST);
		}*/

		if (trickInfos[playerID].readyToDoFasterBoost)
		{
			if (dashType == EDashType::TRICK_GROUND)
			{
				ret = EDashType::SUPERMINITURBO;
				trickInfos[playerID].readyToDoFasterBoost = false;
			}
		}
		if (dashType == EDashType::TRICK_WATER || dashType == EDashType::TRICK_WATER_DIVE)
		{
			trickInfos[playerID].doFasterBoost = false;
			trickInfos[playerID].readyToDoFasterBoost = false;
		}

		return ret;
	}

	void __attribute__((hot)) MarioKartFramework::onKartCountFrameGround(u32* vehicleMoveObj) {
		KartFlags& flags = KartFlags::GetFromVehicle((u32)vehicleMoveObj);
		int playerID = vehicleMoveObj[0x21];
		if (trickInfos[playerID].doFasterBoost) {
			trickInfos[playerID].doFasterBoost = false;
			trickInfos[playerID].readyToDoFasterBoost = flags.hasTricked;
		}
	}

	void MarioKartFramework::startJumpActionCallback(u32* vehicleMoveObj)
	{
		int playerID = vehicleMoveObj[0x21];
		if (playerID == masterPlayerID && CharacterHandler::GetSelectedCharacter(playerID) == syncJumpVoiceCharacterID) {
			g_lastChosenTrickVoiceVariant = 0xFFFFFFFF;
		} else {
			g_lastChosenTrickVoiceVariant = 0xFFFFFFFE;
		}
		trickInfos[playerID].doFasterBoost = false;
		if (trickInfos[playerID].trickCooldown != MarioKartTimer(0)) return;
		KartFlags& flags = KartFlags::GetFromVehicle((u32)vehicleMoveObj);
		BoostFlags& boostFlags = BoostFlags::GetFromVehicle((u32)vehicleMoveObj);
		float* spinCounter = (float*)((u32)(vehicleMoveObj) + 0x1400 + 0xAC);

		// 0xF9C / 4 -> dash timer
		if (
			((SaveHandler::saveData.flags1.improvedTricks && (improvedTricksAllowed && MissionHandler::AllowImprovedTricks())) || 
			(improvedTricksForced || MissionHandler::ForceImprovedTricks())) && 
			vehicleMoveObj[0xF9C / 4] > 0 && !(flags.isInGliderPad | flags.isWingOpened | flags.wingTricked) && *spinCounter == 0.f
		) {
			trickInfos[playerID].trickMode = 0;
			float* stickData = (float*)((u32)vehicleMoveObj + 0x100);
			float xStick = stickData[0];
			float yStick = stickData[2];
			if (abs(xStick) > abs(yStick)) {
				if (xStick > 0.1f) {
					trickInfos[playerID].trickMode = 1 | (2 << 4);
				}
				else if (xStick < -0.1f) {
					trickInfos[playerID].trickMode = 1 | (1 << 4);
				}
			}
			else {
				if (yStick > 0.1f) {
					trickInfos[playerID].trickMode = 2 | (1 << 4);
				}
				else if (yStick < -0.1f) {
					trickInfos[playerID].trickMode = 2 | (2 << 4);
				}
			}
			trickInfos[playerID].doFasterBoost = trickInfos[playerID].trickMode != 0;
			if (trickInfos[playerID].trickMode) {
				trickInfos[playerID].trickCooldown = MarioKartTimer(MarioKartTimer::ToFrames(0, 0, 500));
				kartStartTwist((u32)(vehicleMoveObj), 0x80000001);
				flags.wingTricked = false;
				boostFlags.trickBoost = false;
				vehicleMoveObj[0xBDC / 4] &= ~0x800;
				*spinCounter *= 0.9f;
			}
		}
		else trickInfos[playerID].trickMode = 0;

		if (PointsModeHandler::isPointsMode && playerID == masterPlayerID) {
			PointsModeHandler::DoPointAction(trickInfos[playerID].trickMode != 0 ? PointsModeHandler::PointAction::IMPROVED_TRICK : PointsModeHandler::PointAction::TRICK);
		}
	}

	u32 MarioKartFramework::getMyPlayerID() {
		return masterPlayerID;
	}

	u32 MarioKartFramework::getVehicle(u32 playerID) { // Vehicle, VehicleMove
		u32 kartDirector = getKartDirector();
		if (!kartDirector)
			return 0;
		u32 kartCount = ((u32*)kartDirector)[0x28 / 4];
		u32* kartUnits = ((u32**)kartDirector)[0x2C / 4];
		if (!kartUnits || playerID >= kartCount)
			return 0;
		u32 currUnit = kartUnits[playerID];
		u32 vehicle = ((u32*)currUnit)[0x2C / 4];
		return vehicle;
	}
	
	void MarioKartFramework::setPacketSignature(const u8 signature[0x10])
	{
	}

	void MarioKartFramework::generatePacketSignature(u8 outKey[0x10], u64 keySeed)
	{
	}

	void MarioKartFramework::OnNetLoadPlayerSave(SavePlayerData* sv)
	{
		if (g_getCTModeVal == CTMode::ONLINE_CTWW && !Net::IsPrivateNetwork())
		{
			sv->vr = SaveHandler::saveData.ctVR;
		}
		else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD && !Net::IsPrivateNetwork())
		{
			sv->vr = SaveHandler::saveData.cdVR;
		}
	}

	bool MarioKartFramework::OnNetSavePlayerSave(u32 newVR)
	{
		if (g_getCTModeVal == CTMode::ONLINE_CTWW && !Net::IsPrivateNetwork())
		{
			SaveHandler::saveData.ctVR = newVR;
			return false;
		}
		else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD && !Net::IsPrivateNetwork())
		{
			SaveHandler::saveData.cdVR = newVR;
			return false;
		}
		return true;
	}

	EOnlineRaceMode MarioKartFramework::OnHostSetClass(EOnlineRaceMode original)
	{
		if (g_getCTModeVal != CTMode::ONLINE_CTWW && g_getCTModeVal != CTMode::ONLINE_CTWW_CD)
			return original;

		if ((Utils::Random() & 0x7) == 0) return EOnlineRaceMode::ONLINERACEMIRROR;
		else return EOnlineRaceMode::ONLINERACE150;
	}

	s8 g_system3DState = -1;
	void __attribute__((hot)) MarioKartFramework::DrawGameHook(u32 gameFramework) {
		bool allow3Drender = is3DEnabled && !ISGAMEONLINE && !(isRaceState && kmpRenderOptimizationsForced);

		bufferReaderVtable->renderMainL = allow3Drender ? bufferReaderRenderL : bufferReaderRenderR;

		u32 frameBuffers = gameFramework + 0x2000;
		u32 drawTreeMngr = ((u32*)gameFramework)[0x20 / 4];

		if (g_system3DState < 0)
			g_system3DState = *(u8*)0x1FF81084;

		void (*FrameBuffer_Clear)(u32 gameFramework, u32 flags) = (void(*)(u32,u32))((u32**)gameFramework)[0][0x78 / 4];
		
		void (*FrameBuffer_PresentLeft)(u32 gameFramework) = (void(*)(u32))((u32**)gameFramework)[0][0x88 / 4];
		void (*FrameBuffer_PresentRight)(u32 gameFramework) = (void(*)(u32))((u32**)gameFramework)[0][0x8C / 4];
		void (*FrameBuffer_PresentBottom)(u32 gameFramework) = (void(*)(u32))((u32**)gameFramework)[0][0x6C / 4];
		
		// Left eye
		FrameBuffer_Bind(*(u32*)(frameBuffers + 0x4C));
		FrameBuffer_Clear(gameFramework, 0xC);
		TreeMngr_drawRenderTree(drawTreeMngr + 0x110);
		FrameBuffer_PresentLeft(gameFramework);
		
		*(u8*)0x1FF81084 = (allow3Drender) ? g_system3DState : 1;
		if (allow3Drender) {
			// Right eye
			FrameBuffer_Bind(*(u32*)(frameBuffers + 0x4C));
			FrameBuffer_Clear(gameFramework, 0xF);
			TreeMngr_drawRenderTree(drawTreeMngr + 0x394);
		}
		FrameBuffer_PresentRight(gameFramework);
		// Bottom
		u32 displayFlags = gameFramework + 0x2100;
		u8 bottomRender = ((u8*)displayFlags)[6] | ((u8*)displayFlags)[8];
		if (bottomRender) {
			FrameBuffer_Bind(*(u32*)(frameBuffers + 0x50));
			FrameBuffer_Clear(gameFramework, 0x9);
			TreeMngr_drawRenderTree(drawTreeMngr + 0x250);
			FrameBuffer_PresentBottom(gameFramework);
		}
	}

	bool MarioKartFramework::VehicleMove_DokanWarp(u32 vehicleMove, POTIRoute* route, u32 randomChoice) {
		int playerID = ((u32*)vehicleMove)[0x84 / 4];
		if (playerID == masterPlayerID && g_getCTModeVal != CTMode::OFFLINE && g_getCTModeVal != CTMode::ONLINE_NOCTWW) graceDokanPeriod = MarioKartTimer(0, 15, 0);
		Vector3 chosenPosition = route->points[randomChoice].position;
		chosenPosition.y += 10.f;
		vehicleMove_startDokanWarp(vehicleMove, randomChoice, &chosenPosition, (Vector3*)(vehicleMove + 0xF44));
		dokanWarpCooldown[playerID] = 1;
		return true;
	}

	void MarioKartFramework::OnUpdateNetRecvStartWarp(u32 vehicleMove, u32 randomChoice) {
		// The net code only reports a dokan warp started, but not which dokan warp was the one that caused the start
		// so we need to guess by finding the closest one to the kart
		Vector3& fromPoint = *(Vector3*)(vehicleMove + 0x24);
		GOBJEntry* bestOption = NULL;
		float closest = HUGE_VALF;
		for (int i = 0; i < dokanWarps.size(); i++) {
			Vector3& dokanPos = dokanWarps[i]->position;
			float dist = DistanceNoSqrt(dokanPos, fromPoint);
			if (dist < closest) {
				closest = dist;
				bestOption = dokanWarps[i];
			}
		}
		if (bestOption && bestOption->routeID >= 0) {
			POTIAccessor* accessor = getPathAccessor();
			POTIRoute* route = accessor->entries[bestOption->routeID][0];
			if (randomChoice < route->amount)
				VehicleMove_DokanWarp(vehicleMove, route, randomChoice);
		}
	}

	void MarioKartFramework::OnHsAirCurrentSetReactionFromKart(u32 hsaircurrent, u32 vehicleReact) {
		GOBJEntry* entry = (GOBJEntry*)(((u32**)hsaircurrent)[2][0]);
		s16 routeID = entry->routeID;
		if (routeID >= 0) {
			POTIAccessor* accessor = getPathAccessor();
			POTIRoute* route = accessor->entries[routeID][0];
			u32 pointAmount = route->amount;
			u32 chosenPoint = objectBaseGetRandom32(hsaircurrent, pointAmount);
			
			if (!((u8*)vehicleReact)[0x9E]) {
				VehicleMove_DokanWarp(vehicleReact, route, chosenPoint);
			}
		}
	}

	// Stops being called once disconnect flag is set (online and offline)
	void MarioKartFramework::OnLapRankCheckerDisconnectCheck(u32* laprankchecker, LapRankCheckerKartInfo* kartinfo) {
		//u32 vehicle = ((u32**)kartInfo)[0][0];

		u32* currTimer = laprankchecker + 0x20/4;
		float* secondaryTimer = (float*)laprankchecker + 0x24/4;
		float biggestRaceProgress = kartinfo->maxRaceProgress;
		float currRaceProgress = kartinfo->currRaceProgress;

#ifdef NO_DISCONNECT_ONLINE
		*currTimer = 0; *secondaryTimer = 0.f;
#else
		
		u32 disconnectTimer = MarioKartTimer::ToFrames(0, 12, 0);
		if (ItemHandler::isBlueShellShowdown)
			disconnectTimer = MarioKartTimer::ToFrames(0, 24, 0);

		if (++(*currTimer) > disconnectTimer)
			kartinfo->flags |= 8;
		
		if (graceDokanPeriod != MarioKartTimer(0)) {--graceDokanPeriod; *currTimer = 0; *secondaryTimer = 0.f;}

		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {

			if (((currRaceProgress + 0.00001f) < biggestRaceProgress) || ((currRaceProgress - masterPrevRaceCompletion) < 0.00001f))
				countdownDisconnectTimer+=2;

			if (countdownDisconnectTimer > (CourseManager::getCountdownCourseTime(CourseManager::lastLoadedCourseID) - MarioKartTimer::ToFrames(0, 10, 0))) {
				kartinfo->flags |= 8;
				*currTimer = MarioKartTimer::ToFrames(1,0,0);
				*secondaryTimer = MarioKartTimer::ToFrames(1,0,0);
			} else {
				*currTimer = 0;
				*secondaryTimer = 0.f;
			}
		}
#endif
#ifdef REPORT_RACE_PROGRESS
		if (Controller::IsKeyPressed(Key::DPadLeft)) {
			float curProgDecimal = currRaceProgress - (int)currRaceProgress;
			OSD::Notify(Utils::Format("Progress: %.4f", curProgDecimal));
		}
#endif
		if (!bannedUltraShortcuts.empty()) {
			float curProgDecimal = currRaceProgress - (int)currRaceProgress;
			float prevProgDecimal = masterPrevRaceCompletion - (int)masterPrevRaceCompletion;
			int i = 0;
			for (auto it = bannedUltraShortcuts.begin(); it != bannedUltraShortcuts.end(); it++, i++) {
				if (masterSuspectUltraShortcut == i) {
					if (curProgDecimal >= std::get<0>(*it) && curProgDecimal <= std::get<1>(*it)) {
						masterSuspectUltraShortcut = -1;
					} else if (curProgDecimal >= std::get<4>(*it)) {
						masterSuspectUltraShortcut = -1;
						Net::ReportUltraShortcut();
					}
				} else {
					if (
						prevProgDecimal >= std::get<0>(*it) && prevProgDecimal <= std::get<1>(*it) &&
						curProgDecimal >= std::get<2>(*it) && curProgDecimal <= std::get<3>(*it)
					) {
						masterSuspectUltraShortcut = i;
					}
				}
			}
		}

		masterPrevRaceCompletion = currRaceProgress;
	}

	void MarioKartFramework::OnRRAsteroidInitObj(u32* rrAsteroid) {
		GOBJEntry* entry = (GOBJEntry*)(((u32**)rrAsteroid)[2][0]);
		float multiplier = (((s16)entry->settings[0]) + 32768.f) / 32768.f;
		rrAsteroid[0x194/4] *= multiplier;
		rrAsteroid[0x198/4] *= multiplier;
	}

	void MarioKartFramework::OnObjectBaseSetScale(u32* objectBase, Vector3& copyTo, const Vector3& copyFrom) {
		if (MissionHandler::isMissionMode) MissionHandler::OnObjectBaseSetScale(objectBase, copyTo, copyFrom);
		else copyTo = copyFrom;
	}

	void MarioKartFramework::OnRacePauseEvent(bool pauseOpen) {
		SpeedometerController::OnPauseEvent(pauseOpen);
		if (!MissionHandler::isMissionMode && SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::OnPauseEvent(pauseOpen);
	}

	void MarioKartFramework::OnRaceStartCountdown() {
		SpeedometerController::OnRaceStart();
		MusicCreditsController::OnRaceStart();
		PointsModeHandler::OnRaceStart(true);
		if (VoiceChatHandler::Initialized) {
			CRaceInfo* raceInfo = getRaceInfo(true);
			VoiceChatHandler::SetRaceMode(true, raceInfo->isMirror);
		}

		/*
		if (!g_savedKartXPos[0]) {
			g_checkmode = 1;
			for (int i = 0; i < 8; i++) g_savedKartXPos[i] = (float*)malloc(4 * g_maxcheckframes);
		} else {
			g_checkmode = 2;
		}
		for (int i = 0; i < 8; i++) g_currcheckframe[i] = 0;
		*/
	}

	void __attribute__((hot)) MarioKartFramework::GetTerrainParameters(u32 vehicleMove, u8 kclID) {
		KCLTerrainInfo* currTerrainInfo = ((KCLTerrainInfo**)(vehicleMove + 0x10A8))[kclID];
		KCLTerrainInfo* vehicleTerrainInfo = (KCLTerrainInfo*)(vehicleMove + 0xD3C);
		u32 inkTimer = *(u32*)(vehicleMove + 0xFF8);
		if (inkTimer && (g_getCTModeVal != CTMode::ONLINE_NOCTWW )) {
			float progress = (inkTimer / 250.f);
			if (progress > 1.f) progress = 1.f;
			vehicleTerrainInfo->speedLoss = std::min(currTerrainInfo->speedLoss, (1 - progress) * 0.00075f + 0.99925f);
			vehicleTerrainInfo->accelLoss = std::min(currTerrainInfo->accelLoss, (1 - progress) * 0.55f + 0.45f);
			vehicleTerrainInfo->slippery = std::max(currTerrainInfo->slippery, progress * 1.025f);
		} else {
			vehicleTerrainInfo->speedLoss = currTerrainInfo->speedLoss;
			vehicleTerrainInfo->accelLoss = currTerrainInfo->accelLoss;
			vehicleTerrainInfo->slippery = currTerrainInfo->slippery;
		}
		
		//u32 playerID = *(u32*)(vehicleMove + 0x21 * 4);
		//if (playerID == 0) NOXTRACE("df", "Speed: %.2f%%, Acceleration: %.2f%%, Slippery: %.2f%%", vehicleTerrainInfo->speedLoss * 100, vehicleTerrainInfo->accelLoss * 100, vehicleTerrainInfo->slippery * 100);
	}
	
	void __attribute__((hot)) MarioKartFramework::OnTireEffectGetCollision(u32 out[2], u32 vehicleMove) {
		u32 inkTimer = *(u32*)(vehicleMove + 0xFF8);
		u32 mainType = *(u32*)(vehicleMove + 0xD14);
		u32 subType = *(u32*)(vehicleMove + 0xD20);
		if (inkTimer > 30 && g_getCTModeVal != CTMode::ONLINE_NOCTWW) {
			out[0] = 5; // Off-road
			out[1] = 6; // Mud
		} else if (mainType == 8 && (subType == 1 || subType == 2)) { // Loop kcl
			out[0] = 8;
			out[1] = 0;
		} else {
			out[0] = mainType;
			out[1] = subType;
		}
	}

	ObjectGeneratorBase* MarioKartFramework::GenerateItemBox(CustomFieldObjectCreateArgs& createArgs) {
		ObjectGeneratorBase* ret = (ObjectGeneratorBase*)GameAlloc::game_operator_new_autoheap(0x210);
		ObjectGeneratorBase*(*generatorBaseCons)(ObjectGeneratorBase* self, FieldObjectCreateArgs& args) = (ObjectGeneratorBase*(*)(ObjectGeneratorBase*,FieldObjectCreateArgs&))ItemHandler::GameAddr::itemBoxGeneratorConsAddr;
		generatorBaseCons(ret, createArgs.args);
		createArgs.vtablePtr = operator new(sizeof(ObjectGeneratorVtable));
		memcpy(createArgs.vtablePtr, (void*)ItemHandler::GameAddr::itemBoxGeneratorVtableAddr, sizeof(ObjectGeneratorVtable));
		ret->vtable = (ObjectGeneratorVtable*)createArgs.vtablePtr;
		ret->actorStart = (u32)(createArgs.vtablePtr) + 0x110;
		((u32*)ret)[0x20C/4] = ((u8*)ret)[0x99] == 0 ? 1 : 0;
		return ret;
	}

	static void ObjModelBaseChangeAnimationImpl(u32 objModelBase, int anim, u32 drawMdlFunc, float value) {
		const float one_const = 1.0f;
        __asm__ __volatile__(
            "MOV             R4, %0\n"
			"MOV             R1, %1\n"
			"VMOV            S0, %3\n"
            "LDR             R0, [R4,#0xF4]\n"
            "BLX             %2\n"
            "LDR             R1, [R4,#0xF4]\n"
            "LDR             R0, [R1,#0x8C]\n"
            "LDR             R1, [R1,#0xA4]\n"
            "LDR             R2, [R0]\n"
            "CMP             R2, R1\n"
            "LDRHI           R3, [R0,#4]\n"
            "LDRLS           R2, [R0,#4]\n"
            "LDR             R0, [R0,#4]\n"
            "ADDHI           R2, R3, R1,LSL#4\n"
            "ADDHI           R0, R0, R1,LSL#4\n"
            "LDR             R2, [R2,#0xC]\n"
            "LDR             R1, [R0]\n"
            "LDR             R0, [R0,#4]\n"
            "CMP             R1, R2\n"
            "ADDHI           R0, R0, R2,LSL#2\n"
            "LDR             R0, [R0]\n"
			"VMOV            S0, %4\n"
            "VSTR            S0, [R0,#0x40]\n"
			:
			: "r"(objModelBase), "r"(anim), "r"(drawMdlFunc), "t"(value), "t"(one_const)
			: "r0", "r1", "r2", "r3", "r4", "r12", "lr", "memory", "cc", "s0"
        );
	}

	void MarioKartFramework::ObjModelBaseChangeAnimation(u32 objModelBase, int anim, float value) {
		ObjModelBaseChangeAnimationImpl(objModelBase, anim, (u32)DrawMdl_changeMatAnimation, value);
	}

	static void ObjModelBaseRotateImpl(u32 objModelBase, u32 sincosidxfunc, u32 amount) {
		const float one_const = 1.0f;
		const float zero_const = 0.0f;
		__asm__ __volatile__(
			"MOV             R4, %0\n"
			"MOV			 R2, %2\n"
			"SUB             SP, SP, #0x30\n"
			"ADD             R1, SP, #0x28\n"
			"ADD             R0, SP, #0x24\n"
			"BLX             %1\n"
			"LDR             R0, [R4,#8]\n"
			"VLDR            S1, [SP,#0x28]\n"
			"VLDR            S5, [SP,#0x24]\n"
			"VMOV            S0, %4\n"
			"VLDR            S9, [R0,#8]\n"
			"VLDR            S10, [R0,#0x14]\n"
			"VLDR            S11, [R0,#0x20]\n"
			"VMUL.F32        S2, S9, S1\n"
			"VMUL.F32        S17, S9, S5\n"
			"VMUL.F32        S18, S9, S0\n"
			"VNEG.F32        S6, S5\n"
			"VMUL.F32        S9, S10, S5\n"
			"VMUL.F32        S3, S10, S1\n"
			"VMUL.F32        S4, S11, S1\n"
			"VMUL.F32        S5, S11, S5\n"
			"VMUL.F32        S11, S11, S0\n"
			"VLDR            S8, [R0,#0xC]\n"
			"VMUL.F32        S10, S10, S0\n"
			"VLDR            S12, [R0,#0x18]\n"
			"VLDR            S13, [R0,#0x24]\n"
			"VMLA.F32        S2, S8, S6\n"
			"VLDR            S14, [R0,#0x10]\n"
			"VMLA.F32        S3, S12, S6\n"
			"VMLA.F32        S4, S13, S6\n"
			"VMLA.F32        S5, S13, S1\n"
			"VMLA.F32        S11, S13, S0\n"
			"VLDR            S15, [R0,#0x1C]\n"
			"VMLA.F32        S10, S12, S0\n"
			"VMLA.F32        S9, S12, S1\n"
			"VMOV            S7, %3\n"
			"VLDR            S16, [R0,#0x28]\n"
			"VMLA.F32        S18, S8, S0\n"
			"VMLA.F32        S2, S14, S0\n"
			"VMLA.F32        S17, S8, S1\n"
			"VMLA.F32        S3, S15, S0\n"
			"VMLA.F32        S11, S16, S7\n"
			"VMLA.F32        S5, S16, S0\n"
			"VMLA.F32        S4, S16, S0\n"
			"VMLA.F32        S10, S15, S7\n"
			"VMLA.F32        S9, S15, S0\n"
			"ADD             R0, R4, #0x38\n"
			"VSTR            S2, [R4,#0x18]\n"
			"VMLA.F32        S18, S14, S7\n"
			"VSTR            S3, [R4,#0x28]\n"
			"VMLA.F32        S17, S14, S0\n"
			"VSTR            S11, [R4,#0x40]\n"
			"ADD             R2, SP, #0xC\n"
			"VSTMIA          R0, {S4-S5}\n"
			"ADD             R0, R4, #0x2C\n"
			"MOV             R1, SP\n"
			"VSTMIA          R0, {S9-S10}\n"
			"ADD             R0, R4, #0x1C\n"
			"VSTMIA          R0, {S17-S18}\n"
			"LDR             R0, [R4,#0x6C]\n"
			"ORR             R0, R0, #2\n"
			"STR             R0, [R4,#0x6C]\n"
			"VSTMIA          SP, {S2-S4}\n"
			"VLDR            S0, [R4,#0x1C]\n"
			"VLDR            S1, [R4,#0x2C]\n"
			"VLDR            S2, [R4,#0x3C]\n"
			"VSTMIA          R2, {S0-S2}\n"
			"ADD             R2, SP, #0x18\n"
			"VLDR            S0, [R4,#0x20]\n"
			"VLDR            S1, [R4,#0x30]\n"
			"VLDR            S2, [R4,#0x40]\n"
			"VSTMIA          R2, {S0-S2}\n"
			"ADD             SP, SP, #0x30\n"
			:
			: "r"(objModelBase), "r"(sincosidxfunc), "r"(amount), "t"(one_const), "t"(zero_const)
			: "r0", "r1", "r2", "r3", "r4", "r5", "r12", "lr", "memory", "cc", "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "s12", "s13", "s14", "s15", "s16", "s17", "s18"
		);
	}

	void MarioKartFramework::ObjModelBaseRotate(u32 objModelBase, u32 amount) {
		ObjModelBaseRotateImpl(objModelBase, 0x001151A4, amount);
	}

	void MarioKartFramework::OnRaceNextMenuLoadCallback() {
	}

	void MarioKartFramework::OnRaceNextMenuLoad() {
		if (!MissionHandler::isMissionMode && currentRaceMode.type == 0 && currentRaceMode.mode == 0 && (currentRaceMode.submode == 2 || currentRaceMode.submode == 0) && ExtraResource::lastTrackFileValid) {
			CCSettings* ccset = static_cast<CCSettings*>(ccselectorentry->GetArg());
			if (!ccset->enabled || (ccset->enabled && ccset->value >= 150 && ccset->value <= 200)) {
				int points = getRaceInfo(false)->kartInfos[masterPlayerID].racePoints - getRaceInfo(true)->kartInfos[masterPlayerID].racePoints;
				StatsHandler::IncreaseStat(StatsHandler::Stat::RACE_POINTS, -1, points);
			}
		}
		if (currentRaceMode.type == 0 && currentRaceMode.mode == 1) {
			AsyncRunner::StartAsync(OnRaceNextMenuLoadCallback);
		}
	}

	void MarioKartFramework::AddBotPlayerOnline(int amount) {
	}

	int MarioKartFramework::GetValidStationIDAmount() {
	}

	void MarioKartFramework::OnWifiFinishLoadDefaultPlayerSetting(int playerID) {
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			u8 randomChoices[4];
			getRandomDriverChoice(randomChoices, playerID);
			s32 driverID = randomChoices[0];
			u32 playerType = EPlayerType::TYPE_CONTROLLEDUSER;
			s32 bodyID = randomChoices[1];
			s32 tireID = randomChoices[2];
			s32 wingID = randomChoices[3];
			u32 screwID = 0;
			BasePage_SetDriver(playerID, &driverID, &playerType);
			BasePage_SetParts(playerID, &bodyID, &tireID, &wingID, &screwID);

			int baseVR = std::log2f(std::max((int)Net::lastRoomVRMean, 1000)) * 1833.f - 18211.f;
			baseVR = std::max(baseVR, 1400);
			baseVR += getRandomDriverWifiRateOffset(playerID);
			BasePage_SetWifiRate(playerID, baseVR);

			// When the page is generating, the msbt is not available, so we need an intermediate step...
			onlineBotPlayerIDs.push_back(std::make_pair(playerID, FixedStringBase<char16_t,0x20>()));
			const char16_t* driverName = Language::MsbtHandler::GetText(1050 + CharacterHandler::driverMsbtOrder[(int)driverID]);
			strcpy16(onlineBotPlayerIDs.back().second.strData, u"*");
			strcpy16n(onlineBotPlayerIDs.back().second.strData + 1, driverName, 0x1F * sizeof(char16_t));
		}
	}

	static u32 g_courseVotePagePreStep_prevState = 0;
	void MarioKartFramework::OnCourseVotePagePreStep(u32 courseVotePage) {
		u32 state = ((u32*)courseVotePage)[0x2BC / 4];
		if (state == 6 && neededCPU) {
			if (myRoomPlayerID == 0) {
				if (neededCPU > neededCPUCurrentPlayers)
					AddBotPlayerOnline(neededCPU - neededCPUCurrentPlayers);
			}
			neededCPU = 0;
		}
		if (state == 6 && g_courseVotePagePreStep_prevState != 6 && (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD)) {
			CharacterHandler::StartFetchOnlineSelectedCharacters();
		}
		g_courseVotePagePreStep_prevState = state;
	}

	bool MarioKartFramework::OnWifiLoadDriverShouldForceDefault(int playerID) {
		if (neededCPUCurrentPlayers && playerID >= neededCPUCurrentPlayers)
			return true;
		return false;
	}
	
	void MarioKartFramework::handleThunderResize(u32* thunderdata, u32* staticdata, u32* vehicleMove, float* currSize) {
		struct {u32 thunderCounter; u32 unknown; float targetSize; float actualSize; float growIncrement;} *thunder = (decltype(thunder))thunderdata;
		float kartsize = *currSize;
		int playerID = vehicleMove[0x21];
		if (thunder->thunderCounter) {
			float sizeDecrement = ((float*)staticdata)[0x2C0/4];
			kartsize -= sizeDecrement;
			thunder->actualSize = kartsize;
			if (thunder->targetSize > kartsize) {
				thunder->actualSize = thunder->targetSize;
			}
			thunder->growIncrement = 0.f;
		} else {
			float constant1 = ((float*)staticdata)[0x2C4/4];
			float constant2 = ((float*)staticdata)[0x2C8/4];

			if (resizeInfos[playerID].animationTimer <= 0) {
				if (resizeInfos[playerID].animationStep < resizeInfos[playerID].totalAnimationStep) {
					resizeInfos[playerID].animationStep++;
					resizeInfos[playerID].animationTimer = resizeInfos[playerID].eachAnimationFrames;
					thunder->growIncrement = 0.f;
				}
			} else {
				if (resizeInfos[playerID].prevAnimationStep != resizeInfos[playerID].animationStep) {
					if (resizeInfos[playerID].isGrowing) {
						u32 effectDirector = getEffectDirector();
						void(*playcoineffect)(u32 effectDirector, int playerID, Vector3* position) = (decltype(playcoineffect))playcoineffectAddr;
						Vector3& kartPos = *(Vector3*)(vehicleMove + 0x24/4);
						playcoineffect(effectDirector, playerID, &kartPos);
						void(*playjumpeffect)(u32 effectDirector, int playerID) = (decltype(playjumpeffect))playjumpeffectAddr;
						playjumpeffect(effectDirector, playerID);
					} else {
						ItemHandler::MegaMushHandler::PlayChangeSizeSound(vehicleMove, false);
					}
				}
				resizeInfos[playerID].prevAnimationStep = resizeInfos[playerID].animationStep;
				resizeInfos[playerID].animationTimer--;
			}
			
			float targetSize = (resizeInfos[playerID].animationStep / (float)resizeInfos[playerID].totalAnimationStep) * (resizeInfos[playerID].targetSize - resizeInfos[playerID].fromSize) + resizeInfos[playerID].fromSize;
			if (playerID == masterPlayerID) playerMegaCustomFov = ((targetSize - 1.f) / 1.f) * (65.f - 52.f) + 52.f;

			thunder->growIncrement = (thunder->growIncrement + (targetSize - kartsize) * constant1) * constant2;
			thunder->actualSize = thunder->growIncrement + kartsize;
		}
		resizeInfos[playerID].lastKartScale = thunder->actualSize;
		if (megaMushTimers[playerID] == 1) {
			ItemHandler::MegaMushHandler::End(playerID, false);
		} else if (megaMushTimers[playerID]) --megaMushTimers[playerID];

		if (packunStunCooldownTimers[playerID]) --packunStunCooldownTimers[playerID];

		if (ItemHandler::MegaMushHandler::growMapFacePending[playerID] > 0) {
			if (ItemHandler::MegaMushHandler::growMapFacePending[playerID] == 1 && megaMushTimers[playerID])
				ItemHandler::MegaMushHandler::SetMapFaceState(playerID, true);
			ItemHandler::MegaMushHandler::growMapFacePending[playerID]--;
		}
		if (trickInfos[playerID].trickCooldown > 0) {
			--trickInfos[playerID].trickCooldown;
		}
		if (loopVoiceCooldown[playerID] > 0) {
			--loopVoiceCooldown[playerID];
			if (loopVoiceCooldown[playerID] == 0 && playerID == masterPlayerID)
				loopMasterWPSoundEffectPlayed = false;
		}
		if (dokanWarpCooldown[playerID]) {
			if (vehicleMove[0xDA8/4] >= 0xFF)
				dokanWarpCooldown[playerID]--;
		}
		

		PointsModeHandler::OnVehicleCalc(vehicleMove);
		ItemHandler::OnVehicleCalc((u32)vehicleMove);

		if (hasOOBAreas) {
			CalcOOBArea((u32)vehicleMove);
		}

		// Check position differences on replay
		/*
		if (g_currcheckframe[playerID] < g_maxcheckframes)
		{
			Vector3& kartPos = *(Vector3*)(vehicleMove + 0x24/4);
			if (g_checkmode == 1) {
				g_savedKartXPos[playerID][g_currcheckframe[playerID]++] = kartPos.x;
			} else if (g_checkmode == 2) {
				float curr = g_savedKartXPos[playerID][g_currcheckframe[playerID]++];
				int diff =  curr - kartPos.x;
				char id[2] = {0};
				id[0] = playerID + '0';
				NOXTRACEPOS(id, 10, 10*playerID, "%d Diff: %d", playerID, diff);
			}
		}
		*/
	}

	void MarioKartFramework::startSizeChangeAnimation(int playerID, float targetSize, bool isGrowing) {
		resizeInfos[playerID].targetSize = targetSize;
		resizeInfos[playerID].fromSize = resizeInfos[playerID].lastKartScale;
		resizeInfos[playerID].animationStep = 1;
		resizeInfos[playerID].animationTimer = resizeInfos[playerID].eachAnimationFrames;
		resizeInfos[playerID].isGrowing = isGrowing;
	}

    void MarioKartFramework::ReactPressMegaMush(u32 *vehicleMove, int causingPlayerID)
    {
		constexpr int stunFrames = 1 * 60;
		constexpr int pressedFrames = 8 * 60;
		bool isNet = ((u8*)vehicleMove)[0x9E];
		void (*VehicleMove_StartPress)(u32* vehicleMove, bool dontStop, u32 timer) = (decltype(VehicleMove_StartPress))VehicleMove_StartPressAddr;
		void (*VehicleReact_ReactAccidentCommon)(u32* vehicleReact, u32 callType, int causeItemType, int causePlayerID, u32 eacdtype, Vector3* vec0, Vector3* vec1) = (decltype(VehicleReact_ReactAccidentCommon))VehicleReact_ReactAccidentCommonAddr;

		if (!isNet) VehicleMove_StartPress(vehicleMove, false, stunFrames + pressedFrames);
		Vector3 vec0{};
		vec0.x = stunFrames * 0.01;
		VehicleReact_ReactAccidentCommon(vehicleMove, 0, EItemType::ITYPE_BIGKINOKO, causingPlayerID, 17, &vec0, nullptr);
    }

    bool MarioKartFramework::calculatePressReact(u32 *vehicleMove1, u32 *vehicleMove2, int someValueSP)
    {
        struct {u32 shrunkTimer; u32 pressTimer;} *timers1 = (decltype(timers1))(vehicleMove1 + 0x1000/4);
		struct {u32 shrunkTimer; u32 pressTimer;} *timers2 = (decltype(timers2))(vehicleMove2 + 0x1000/4);
		bool ret = false;

		int playerID1 = vehicleMove1[0x21];
		int playerID2 = vehicleMove2[0x21];

		KartFlags& flags1 = KartFlags::GetFromVehicle((u32)vehicleMove1);
		KartFlags& flags2 = KartFlags::GetFromVehicle((u32)vehicleMove2);

		void (*VehicleMove_StartPress)(u32* vehicleMove, bool dontStop, u32 timer) = (decltype(VehicleMove_StartPress))VehicleMove_StartPressAddr;

		// vehicle1 presses vehicle2
		if (megaMushTimers[playerID1] > 0 && megaMushTimers[playerID2] <= 0 && timers2->pressTimer <= 0 && kartCanAccident(vehicleMove2) && !flags2.isWingOpened && someValueSP == 0) {
			ReactPressMegaMush(vehicleMove2, playerID1);
		} else if (timers2->shrunkTimer > 0 && timers2->pressTimer <= 0 && !flags2.isWingOpened && someValueSP == 0) {
			if (timers1->pressTimer <= 0 && timers1->shrunkTimer <= 0) {
				VehicleMove_StartPress(vehicleMove2, true, 90);
				if (PointsModeHandler::isPointsMode) PointsModeHandler::OnKartPress(vehicleMove1, vehicleMove2);
			}
		}

		// vehicle2 presses vehicle1
		if (megaMushTimers[playerID2] > 0 && megaMushTimers[playerID1] <= 0 && timers1->pressTimer <= 0 && kartCanAccident(vehicleMove1) && !flags1.isWingOpened) {
			ReactPressMegaMush(vehicleMove1, playerID2);
		} else if (timers1->shrunkTimer > 0 && timers1->pressTimer <= 0 && !flags1.isWingOpened) {
			if (timers2->pressTimer <= 0 && timers2->shrunkTimer <= 0) {
				VehicleMove_StartPress(vehicleMove1, true, 90);
				if (PointsModeHandler::isPointsMode) PointsModeHandler::OnKartPress(vehicleMove2, vehicleMove1);
			}
		}
		
		return timers1->pressTimer > 0 || timers2->pressTimer > 0;
    }

    u32* MarioKartFramework::sndActorKartCalcEnemyStar(u32* sndActorKart) {
		u32* vehicle = (u32*)sndActorKart[0x1E0/4];
		int playerID = vehicle[0x21];
		return ItemHandler::MegaMushHandler::CalcEnemyMegaTheme(sndActorKart);
	}

	static void SoundHandleSetPitch(u32* soundHandle, float pitch) {
		u32* internalHandle = (u32*)*soundHandle;
		if (internalHandle) {
			((float*)internalHandle)[0xB8/4] = pitch;
		}
	}

	void __attribute__((hot)) MarioKartFramework::OnSndActorKartCalcInner(u32* sndActorKart) {
		u32* vehicle = (u32*)sndActorKart[0x1E0/4];
		int playerID = vehicle[0x21];
		u32* actorVoice = sndActorKart + 0xF0/4;
		if (vehicle[(0x1000) / 4] > 0 || vehicle[(0x1004) / 4] > 0) {
			pitchCalculators[playerID].Start(true);
			pitchCalculators[playerID].Calc();
			SoundHandleSetPitch((u32*)actorVoice[0x4/4], 1.25f + 0.15f * pitchCalculators[playerID].Get());
		} else if (megaMushTimers[playerID] > 0) {
			pitchCalculators[playerID].Stop();
			SoundHandleSetPitch((u32*)actorVoice[0x4/4], 0.8f);
		} else
		{
			pitchCalculators[playerID].Stop();
			SoundHandleSetPitch((u32*)actorVoice[0x4/4], 1.f);
		}
	}

	bool MarioKartFramework::OnThunderAttackKart(u32* vehicleMove) {
		int playerID = vehicleMove[0x21];
        u32 starTimer = vehicleMove[0xFF4 / 4];
		if (megaMushTimers[playerID] && !starTimer) {
			ItemHandler::MegaMushHandler::End(playerID, false);
			return true;
		}
		return false;
	}

	void MarioKartFramework::OnStartKiller(u32* vehicleMove) {
		int playerID = vehicleMove[0x21];
		if (playerID == masterPlayerID)
			onMasterStartKillerPosition = getModeManagerData()->driverPositions[masterPlayerID];
		ItemHandler::MegaMushHandler::End(playerID, true);
	}

	bool MarioKartFramework::OnReactAccidentCommonShouldIgnore(u32* vehicleMove) {
		int playerID = vehicleMove[0x21];
		if (!ignoreKartAccident && megaMushTimers[playerID]) {
			ItemHandler::MegaMushHandler::End(playerID, false);
		}
		return ignoreKartAccident;
	}

	void MarioKartFramework::SndActorKartSetStarState(u32* sndActorKart, u8 modeBits) {
		
		void(*SndActorBase_SetCullingSafeDistVolRatio)(u32* sndActorBase, float ratio) = (decltype(SndActorBase_SetCullingSafeDistVolRatio))SndActorBase_SetCullingSafeDistVolRatioAddr;
		
		u32* vehicle = (u32*)sndActorKart[0x1E0/4];
		bool isStar = modeBits & 1;
		bool isMega = modeBits & 2;
		bool endMega = modeBits & 4;
		int playerID = vehicle[0x21];
		bool starTimer = vehicle[0xFF4/4] > 0;
        bool megaTimer = MarioKartFramework::megaMushTimers[playerID] > 0;
		if (!megaTimer && !isStar && !isMega && !endMega) // Only happens online, the game doesn't clear the timer before calling this func
			starTimer = 0;

		if (isStar || isMega || starTimer || megaTimer) {
			SndActorBase_SetCullingSafeDistVolRatio(sndActorKart, 0.8f);
			if (isStar) SndActorKArt_PlayDriverVoice(sndActorKart, EVoiceType::STAR_START);
			((u8*)sndActorKart)[0x1FF] = true;
		} else if (!isStar && !isMega && !starTimer && !megaTimer) {
			SndActorBase_SetCullingSafeDistVolRatio(sndActorKart, 0.5f);
			((u8*)sndActorKart)[0x1FF] = false;
		}
	}

	void MarioKartFramework::OnVehicleMoveInit(u32* vehicleMove) {
		int playerID = vehicleMove[0x21];
		trickInfos[playerID].Reset();
		if (playerID == masterPlayerID)
			g_isAutoAccel = false;
		dokanWarpCooldown[playerID] = 0;
		ItemHandler::OnVehicleInit((u32)vehicleMove);
	}

	void MarioKartFramework::applyKartCameraParams(u32* kartCamera, float* cameraparams) {
		struct CamParams {
			float yOffsetLookAt; // 19
			float zOffsetCamera; // 50
			float yOffsetCamera; // 13
			float fov; // 52
			float unknown; // 57
		};
		CamParams& dstParams = *(CamParams*)(kartCamera + 0x258/4);
		CamParams& srcParams = *(CamParams*)(cameraparams + 0x8/4);
		dstParams = srcParams;
		if (playerMegaCustomFov && playerMegaCustomFov != 52.f) dstParams.fov = playerMegaCustomFov;
		int isInLoop = vehicleIsInLoopKCL(((u32**)kartCamera)[0xD8/4][0]);
		if (isInLoop == 1) {
			dstParams.fov *= 1.15f;
			dstParams.yOffsetCamera -= 5.f;
		} else if (isInLoop == 2) {
			dstParams.yOffsetLookAt = 24.f;
			dstParams.yOffsetCamera = -15.f;
			dstParams.fov *= 1.5f;
		}
	}

	void MarioKartFramework::OnTrophySelectNextScene(u32* lastCup) {
		bool showCTGPCredits = false;
		if ((SaveHandler::saveData.flags1.creditsWatched && (*lastCup == 0x11 || *lastCup == 0x17)) || (!SaveHandler::saveData.flags1.creditsWatched && (*lastCup >= CUSTOMCUPLOWER && *lastCup <= CUSTOMCUPUPPER) && SaveHandler::CupRankSave::CheckModAllSatisfy(SaveHandler::CupRankSave::SatisfyCondition::COMPLETED))) {
			*lastCup = 3;
			MenuPageHandler::MenuEndingPage::loadCTGPCredits = true;
			thankYouDisableNintendoLogo = true;
		}
	}

	bool MarioKartFramework::OnThankyouPageInitControl() {
		if (thankYouDisableNintendoLogo) {
			thankYouDisableNintendoLogo = false;
			return true;
		}
		return false;
	}

	u32 MarioKartFramework::OnSimpleModelManager(u32 own) {
		CharacterHandler::OnThankYouScreen(true);
		u32 ret = ((u32(*)(u32))OnSimpleModelManagerHook.callCode)(own);
		CharacterHandler::OnThankYouScreen(false);
		return ret;
	}

	bool MarioKartFramework::vehicleForceAntigravityCamera(u32 vehicle) {
		return vehicleIsInLoopKCL(vehicle) != 0;
	}

	bool MarioKartFramework::vehicleDisableSteepWallPushback(u32 vehicle) {
		return vehicleIsInLoopKCL(vehicle) != 0;
	}

	bool MarioKartFramework::vehicleDisableUpsideDownFloorUnstick(u32 vehicle, float* gravityAttenuator) {
		// gravityAttenuator = 0 -> No change in gravity, gravityAttenuator = 1 -> Zero gravity 
		int isInLoop = vehicleIsInLoopKCL(vehicle);
		bool res = false;
		if (isInLoop) {
			constexpr float loopStickThreshold = 0.75f;
			float speedFactor = *(float*)(vehicle + 0x330 + 0xC00);
			res = speedFactor > loopStickThreshold;
		}
		return res;
	}

	void MarioKartFramework::OnKartGravityApply(u32 vehicle, Vector3& gravityForce) {
		int isInLoop = vehicleIsInLoopKCL(vehicle);
		if (isInLoop) {
			float speedFactor = *(float*)(vehicle + 0x330 + 0xC00);
			constexpr float invertedGravityFactor = 0.95f;
			float gravityInterpolationFactor = std::min(invertedGravityFactor, speedFactor) / invertedGravityFactor;
			float currentGravityMagnitude = gravityForce.Magnitude();
			Vector3 kartDownVec = (*(Vector3*)(((u32*)vehicle)[0xC] + 0xC)) * -1.f;
			gravityForce.InvCerp(kartDownVec * currentGravityMagnitude, gravityInterpolationFactor);
		}
	}

	float __attribute__((hot)) MarioKartFramework::ApplyGndTurningSpeedPenaly(u32 vehicle, float speed, float penaltyFactor) {
		float hopPenalty = (SaveHandler::saveData.flags1.automaticDelayDrift && automaticDelayDriftAllowed) ? 1.f : penaltyFactor;
		KartFlags& flags = KartFlags::GetFromVehicle(vehicle);
		return (flags.isJumping) ? (speed * hopPenalty) : (speed * penaltyFactor);
	}

	void MarioKartFramework::OnEnemyAIControlRaceUpdate(u32 enemyAI) {
		((void(*)(u32))enemyAIControlRaceUpdateHook.callCode)(enemyAI);
		if (((u32*)enemyAI)[0x4C/4] == masterPlayerID) {
			// Bullet Bill logic:
			// When it is used, it will set a target position to bring the kart to. The bullet will keep running until it reaches
			// that position. Once it reaches the position, it will stop if it has been running for at least 120 frames. If it doesn't 
			// manage to reach the target position it will run at maximum 420 frames. Once it has run out, it will last for 60 more frames 
			// to play the stopping animation.
			// Even then it will not be able to fully stop if certain conditions are met (such as flags set in enemy points),
			// however it will stop as a failsafe after 960 frames.
			// The code below approximates this behaviour for the item box progress bar

			u32 globalTimer = ((u32*)enemyAI)[0x78/4];
			bool isStopping = ((u8*)enemyAI)[0x90];
			u32 currentStateTimer = ((u32*)enemyAI)[0x74/4];
			u32 targetPosition = ((u32*)enemyAI)[0x6C/4];
			u32 currentPosition = getModeManagerData()->driverPositions[masterPlayerID];

			float globalTimerProgress = (420.f - globalTimer) / 420.f;
			float reachedPositionProgress;
			if (onMasterStartKillerPosition == targetPosition)
				reachedPositionProgress = 1.f;
			else
				reachedPositionProgress = 1.f - std::clamp((currentPosition - targetPosition) / (float)(onMasterStartKillerPosition - targetPosition), 0.f, 1.f);
			float gracePeriod = std::clamp((120.f - globalTimer) / 120.f, 0.f, 1.f);
			float firstPartProgress = std::clamp(globalTimerProgress * (1.f - reachedPositionProgress), 0.f, 1.f) * 0.70f;
			float secondPartProgress = gracePeriod * 0.15f;
			float thirdPartProgress = std::clamp(isStopping ? ((60.f - currentStateTimer) / 60.f) : 1.f, 0.f, 1.f) * 0.15f;
			ExtendedItemBoxController::killerProgress = isStopping ? thirdPartProgress : (firstPartProgress + secondPartProgress + thirdPartProgress);
		}
	}

	void* MarioKartFramework::OnLoadResGraphicFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs) {
		void* buffer = CharacterHandler::OnLoadResGraphicsFile(resourceLoader, path, loadArgs);
		if (!buffer)
			buffer = ((void*(*)(u32, SafeStringBase*, ResourceLoaderLoadArg*))loadResGraphicFileHook.callCode)(resourceLoader, path, loadArgs);
		return CryptoResource::ProcessCryptoFile(buffer);
	}

	void MarioKartFramework::OnKartUnitCalcDrawKartOn(u32 kartUnit, float* transformMatrix) {
		using Matrix3x4 = std::array<std::array<float, 4>, 3>;
		u32 vehicle = ((u32*)kartUnit)[0x2C/4];
        int playerID = ((u32*)vehicle)[0x84/4];
		if (!rotateCharacterID || CharacterHandler::GetSelectedCharacter(playerID) != rotateCharacterID) {
			*(Matrix3x4*)(((u32*)kartUnit)[0x1E4/4] + 0x14) = *(Matrix3x4*)transformMatrix;
			return;
		}
		auto rotateMatrixY = [](const Matrix3x4& matrix, float angle) -> Matrix3x4 {
			float cosAngle = cosf(angle);
			float sinAngle = sinf(angle);

			Matrix3x4 rotatedMatrix = matrix;

			for (int i = 0; i < 3; ++i) {
				float x = matrix[i][0];
				float z = matrix[i][2];

				rotatedMatrix[i][0] = x * cosAngle + z * sinAngle;
				rotatedMatrix[i][2] = -x * sinAngle + z * cosAngle;
			}

			return rotatedMatrix;
		};
		characterRotateAmount[playerID] += 2;
		*(Matrix3x4*)(((u32*)kartUnit)[0x1E4/4] + 0x14) = rotateMatrixY(*(Matrix3x4*)transformMatrix, (characterRotateAmount[playerID] / 256.f) * 2 * M_PI);
	}

	void MarioKartFramework::OnJugemSwitchReverseUpdate(u32 switchreverse) {
		u32 vehicle = ((u32***)switchreverse)[0x4/4][0x78/4][0];
		if (!MissionHandler::isLakituVisible() || vehicleIsInLoopKCL(vehicle))
			return;
		((void(*)(u32))jugemSwitchReverseUpdateHook.callCode)(switchreverse);
	}

	u32 MarioKartFramework::SndActorKartDecideBoostSndID(u32 sndActorKart, u32 courseID) {
		if (courseID != 0x17) // Waluigi Pinball
			return Snd::SoundID::KART_DASH;
		
		u32 vehicle = ((u32*)sndActorKart)[0x1E0/4];
        int playerID = ((u32*)vehicle)[0x84/4];
		if (playerID == masterPlayerID && vehicleIsInLoopKCL(vehicle)) {
			if (!loopMasterWPSoundEffectPlayed) {
				u32*(*sndactorkart_startsound)(u32 soundPlayer, u32 soundID, u32* soundHandle) = (decltype(sndactorkart_startsound))(((u32**)sndActorKart)[0][0x70/4]);
				sndactorkart_startsound(sndActorKart, Snd::SoundID::CANNON_START_WALUIGI_PINBALL, nullptr);
				loopMasterWPSoundEffectPlayed = true;
			}
			return Snd::SoundID::KART_DASH;
		}
		return Snd::SoundID::KART_DASH_WALUIGI_PINBALL;
	}

	u32 MarioKartFramework::AdjustVRIncrement(u32 playerID, s32 vr, s32 vrIncrement, u32 modemanagerbase) {
		u32 upperLimit = 99999;
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			upperLimit = 999999;
			if (vrIncrement < 0) vrIncrement = 0;
			if (vrIncrement == 0) {
				bool wonAll = true;
				u32 playerAmount = ((u32*)modemanagerbase)[0x1E4/4] & 0xFF;
				for (int i = 0; i < playerAmount; i++) {
					if (i == playerID) continue;
					// ModeManagerBase::checkLose
					bool lose = ((bool(*)(u32, u32, u32))(((u32**)modemanagerbase)[0][0x8C/4]))(modemanagerbase, playerID, i);
					if (lose) {
						wonAll = false;
						break;
					}
				}
				if (wonAll) vrIncrement = 1;
			}

			if (vrIncrement > 0) vrIncrement *= Net::useSpecialCharVRMultiplier ? Net::specialCharVrMultiplier : Net::vrMultiplier;
		}

		if (std::abs(vrIncrement) > 1000) {
			vrIncrement = 0;
		}

		vr += vrIncrement;
		if (vr > upperLimit) vr = upperLimit;
		else if (vr < 1) vr = 1;
		return vr;
	}

	u32 MarioKartFramework::OnCalcRateForLose(u32 modemanagerbase, u32 playerID) {
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			u32 vrDataStart = modemanagerbase + 0x64;
			vrDataStart += 0x2C * playerID;
			u32 vr = ((u32*)vrDataStart)[0x28/4];
			return vr;
		} else {
			return ((u32(*)(u32, u32))calcRateForLoseHook.callCode)(modemanagerbase, playerID);
		}
	}

    void MarioKartFramework::OnBaseRacePageEnter(u32 baseracepage, u32 fadekind, u32 unk)
    {
		u32 funcaddr = baseRacePageEnterHook.funcAddr;
		u32* maxRate = (u32*)(funcaddr + (0x0047BB2C - 0x0047B574));
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			maxRate[0] = 999999;
			maxRate[1] = 999998;
		}
		((void(*)(u32,u32,u32))baseRacePageEnterHook.callCode)(baseracepage, fadekind, unk);
		maxRate[0] = 99999;
		maxRate[1] = 99998;
    }

    void MarioKartFramework::OnBaseRacePageCalcSave(u32 baseracepage)
    {
		u32 funcaddr = baseRacePageCalcSaveHook.funcAddr;
		u32* maxRate = (u32*)(funcaddr + (0x0047CEAC - 0x0047C600));
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			maxRate[0] = 999999;
			maxRate[1] = 999998;
		}
		((void(*)(u32))baseRacePageCalcSaveHook.callCode)(baseracepage);
		maxRate[0] = 99999;
		maxRate[1] = 99998;
    }

    void MarioKartFramework::OnBaseRacePageCalcPoint(u32 baseracepage)
    {
		u32 funcaddr = baseRacePageCalcPointHook.funcAddr;
		u32* maxRate = (u32*)(funcaddr + (0x0047F12C - 0x0047E94C));
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			maxRate[0] = 999999;
		}
		((void(*)(u32))baseRacePageCalcPointHook.callCode)(baseracepage);
		maxRate[0] = 99999;
    }

    bool MarioKartFramework::OnRacePauseAllow(bool gamePauseAllowed) {
		if (isPauseBlocked) return false;
		if (isPauseAllowForced) return true;
		return gamePauseAllowed;
	}

	u16 MarioKartFramework::handleCustomItemProbabilityRecursive(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag, int recursionMode) {
		u16 totalProb = 0;
		bool isRandomMode = customItemProbabilityRandom;
		constexpr u32 unblockItems = (1 << EItemSlot::ITEM_GESSO) | (1 << EItemSlot::ITEM_KILLER) | (1 << EItemSlot::ITEM_THUNDER) | 
							(1 << EItemSlot::ITEM_TEST4) | (1 << EItemSlot::ITEM_KINOKO) | (1 << EItemSlot::ITEM_KINOKO3) | (1 << EItemSlot::ITEM_STAR) |
							(1 << EItemSlot::ITEM_KINOKOP) | (1 << EItemSlot::ITEM_KONOHA);
		if (!bulletBillAllowed) blockedBitFlag |= (1 << EItemSlot::ITEM_KILLER);
		for (int i = 0; i < EItemSlot::ITEM_SIZE; i++) {
			if ((blockedBitFlag & (1 << i)) == 0) {
				u16 currProb = 0;
				if (isRandomMode) {
					currProb = customItemProbabilityAllowed[i] ? 10 : 0;
				} else {
					currProb = customItemProbabilityAllowed[i] ? MarioKartFramework::CsvParam_getDataInt(csvPtr, rowIndex, i) : 0;
				}
				totalProb += currProb;
			}
			dstArray[i] = totalProb;
			storeImprovedRouletteItemProbability(i, totalProb);
		}
		if (totalProb == 0) {
			if (recursionMode == 0) {
				return handleCustomItemProbabilityRecursive(dstArray, csvPtr, rowIndex, blockedBitFlag & ~unblockItems, 1);
			} else if (recursionMode == 1 && MarioKartFramework::startedBoxID == 0) {
				for (int i = 1; i < 7; i++) {
					int prevRow = (int)rowIndex - i;
					int postRow = (int)rowIndex + i;
					if (postRow <= 7) totalProb = handleCustomItemProbabilityRecursive(dstArray, csvPtr, (u32)postRow, blockedBitFlag, 2);
					if (!totalProb && prevRow >= 0) totalProb = handleCustomItemProbabilityRecursive(dstArray, csvPtr, (u32)prevRow, blockedBitFlag, 2);
					if (totalProb)
						break;
				}
				return totalProb;
			}
		}
		return totalProb;
	}

	void MarioKartFramework::UseCustomItemMode(const std::array<bool, EItemSlot::ITEM_SIZE>& allowedItems, bool randomItems) {
		useCustomItemProbability = true;
		customItemProbabilityAllowed = allowedItems;
		customItemProbabilityRandom = randomItems;
	}
	
	void MarioKartFramework::ClearCustomItemMode() {
		useCustomItemProbability = false;
	}

    void MarioKartFramework::OnVehicleReactExecPostCrash(u32 *vehicleReact)
    {
		if (PointsModeHandler::isPointsMode && vehicleReact[0x116C/4] > 1 
			&& vehicleReact[0x1168/4] != 4 && vehicleReact[0x1168/4] != 9
			&& vehicleReact[0x1180/4] != -1 &&  vehicleReact[0x117C/4] != -1) {
			u32 srcVehicle = getVehicle(vehicleReact[0x1180/4]);
			EItemType itemType = (EItemType)vehicleReact[0x117C/4];
			if (itemType > EItemType::ITYPE_SIZE) { // Hang item
				itemType = (EItemType)(itemType - (EItemType::ITYPE_SIZE + 1));
			}
			PointsModeHandler::DoItemHitKart((u32*)srcVehicle, vehicleReact, itemType);
		}
		((void(*)(u32*))vehicleReactExecPostCrashHook.callCode)(vehicleReact);
    }

    bool __attribute__((hot)) MarioKartFramework::OnVehicleDecideSurfaceTrickable(u32 *vehicle, bool isTrickable)
    {
		if (PointsModeHandler::isPointsMode) return PointsModeHandler::OnVehicleDecideSurfaceTrickable(vehicle, isTrickable);
        return isTrickable;
    }

    void MarioKartFramework::OnRaceDirectorCreateBeforeStructure(u32 *raceDirector, u32* arg)
    {
		((void(*)(u32*, u32*))raceDirectorCreateBeforeStructureHook.callCode)(raceDirector, arg);


		previousCDScore = 0;

		for (int i = 0; i < MAX_PLAYER_AMOUNT; i++)
		{
			megaMushTimers[i] = 0;
			packunStunCooldownTimers[i] = 0;
			pitchCalculators[i].Start(false);
			pitchCalculators[i].Stop();
			pitchCalculators[i].SetSpeed(5.851f * 1.25f);
			ItemHandler::MegaMushHandler::growMapFacePending[i] = 0;
			kartReachedGoal[i] = false;
		}

		MissionHandler::OnRaceDirectorCreateBeforeStructure();
		PointsModeHandler::OnRaceDirectorCreateBeforeStructure();

		CharacterHandler::ConfirmCharacters();

		rubberBandingMultiplier = 1.0f;
		rubberBandingOffset = 0.0f;
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			rubberBandingMultiplier = Net::ctwwCPURubberBandMultiplier;
			rubberBandingOffset = Net::ctwwCPURubberBandOffset;
		} else if (g_getCTModeVal == CTMode::OFFLINE && VersusHandler::IsVersusMode) {
			if (SaveHandler::saveData.vsSettings.cpuOption == VersusHandler::VSCPUDifficulty::VERY_HARD) {
				rubberBandingMultiplier = 1.0f;
				rubberBandingOffset = 0.175f;
			} else if (SaveHandler::saveData.vsSettings.cpuOption == VersusHandler::VSCPUDifficulty::INSANE) {
				rubberBandingMultiplier = 1.2f;
				rubberBandingOffset = 0.6f;
			} 
		}

		if (g_isFoolActive)
		{
			s32 driverId = playerInfos[MarioKartFramework::masterPlayerID].driver;
			u32 playerType = EPlayerType::TYPE_CPU;
			BasePage_SetDriver(0, &driverId, &playerType);
		}

		lastCheckedLap = 1;
		raceMusicSpeedMultiplier = 1.f;

		masterSuspectUltraShortcut = -1;
		masterPrevRaceCompletion = 0.f;

		kouraGProbability = rol<u64>(kouraGProbability, Utils::Random(0, 63));
		kouraRProbability = rol<u64>(kouraRProbability, Utils::Random(0, 63));
    }

    void MarioKartFramework::OnSequenceSubGoalKart(int playerID)
    {
		//Cannot call directly because the second instruction is a PC relative load
		((void(*)(u32, int))(sequenceSubGoalKartHook.funcAddr + 0x8))(baseAllPointer, playerID);

		if (!kartReachedGoal[playerID]) {
			kartReachedGoal[playerID] = true;
			if (PointsModeHandler::isPointsMode) PointsModeHandler::OnPlayerGoal(playerID);
		}
    }

    void MarioKartFramework::OnLapRankCheckerCalc(u32 *laprankchecker)
    {
		((void(*)(u32*))lapRankCheckerCalcHook.callCode)(laprankchecker);
		if (PointsModeHandler::isPointsMode) PointsModeHandler::OnLapRankCheckerCalc(laprankchecker);
    }

    void MarioKartFramework::OnRacePageGenResult(u32 racePage)
    {
		if (PointsModeHandler::isPointsMode) RacePage_GenResultWifi(racePage);
		else ((void(*)(u32))racePageGenResultHook.callCode)(racePage);
    }

    void MarioKartFramework::OnNetworkBufferControllerAllocBuf(MK7::Net::NetworkBufferController *networkBufferController)
    {
		size_t increasedSize = 0;
		if (networkBufferController->m_type == MK7::Net::eNetworkBufferType::SelectMenu) {
			increasedSize = sizeof(CustomCTGP7MenuData);
		}

		originalNetworkBufferSizes[(int)networkBufferController->m_type] += increasedSize;

		((void(*)(MK7::Net::NetworkBufferController *))networkBufferControllerAllocHook.callCode)(networkBufferController);

		originalNetworkBufferSizes[(int)networkBufferController->m_type] -= increasedSize;
    }

    void MarioKartFramework::OnNetworkSelectMenuSendUpdateCore(u32 NetworkDataManager_SelectMenu)
    {
		MK7::Net::eNetworkBufferType type = (MK7::Net::eNetworkBufferType)((u8*)(NetworkDataManager_SelectMenu))[0xAC];
		u8* selectMenuFormat = (u8*)(NetworkDataManager_SelectMenu + 0x10);
		constexpr size_t selectMenuFormatSize = 0x9C;

		auto* net_engine = (MK7::Net::NetworkEngine*)getNetworkEngine();

		MK7::Net::NetworkBuffer* sendBuf = net_engine->m_network_station_buffer_manager->getWriteBuffer(type);
		sendBuf->Set(selectMenuFormat, selectMenuFormatSize);

		CustomCTGP7MenuData data = {0};
		data.netVersion = Net::lastOnlineVersion;
		if (netImHost(net_engine)) {
			data.startingCupButtonID = MenuPageHandler::MenuSingleCupBasePage::startingButtonID;
		}
		data.selectedCustomCharacter = CharacterHandler::GetSelectedMenuCharacter();
		data.MakeValid();
		sendBuf->Add(&data, sizeof(data));
    }

	bool MarioKartFramework::OnNetworkSelectMenuReceivedCore(u32 NetworkSelectMenuReceived, MK7::Net::NetworkReceivedInfo *receiveInfo)
    {
		constexpr size_t selectMenuFormatSize = 0x9C;
		CustomCTGP7MenuData* data = (CustomCTGP7MenuData*)((u8*)receiveInfo->m_data + selectMenuFormatSize);
		if (receiveInfo->m_size != (selectMenuFormatSize + sizeof(CustomCTGP7MenuData)) || !data->IsValid()) {
			MarioKartFramework::dialogBlackOut(NAME("mult_error"));
		} else if (data->netVersion > Net::lastOnlineVersion) {
			MarioKartFramework::dialogBlackOut(NOTE("mult_error"));
		} else {
			MK7::Net::NetworkEngine* netEngine = (MK7::Net::NetworkEngine*)getNetworkEngine();
			int playerID = netEngine->m_network_station_buffer_manager->getPlayerID(netEngine->m_network_station_buffer_manager->m_buffer_id_to_aid[receiveInfo->m_buffer_id]);
			if (playerID >= 0 && playerID <= MAX_PLAYER_AMOUNT) {
				if (!netImHost(netEngine) && playerID == netEngine->m_master_player_id) {
					MenuPageHandler::MenuSingleCupBasePage::hostStartingButtonID = data->startingCupButtonID;
				}
				CharacterHandler::SetOnlineSelectedCharacter(playerID, data->selectedCustomCharacter);
			}
		}
        return ((bool(*)(u32, MK7::Net::NetworkReceivedInfo*))networkSelectMenuReceivedCoreHook.callCode)(NetworkSelectMenuReceived, receiveInfo);
    }

    bool MarioKartFramework::netImHost(MK7::Net::NetworkEngine* netEngine)
    {
		if (netEngine == nullptr) {
			netEngine = (MK7::Net::NetworkEngine*)getNetworkEngine();
		}
		return netEngine->m_local_player_id == netEngine->m_master_player_id;
    }

	bool MarioKartFramework::LensFlareAllowed() {
		return !disableLensFlare;
	}

    void MarioKartFramework::CalcOOBArea(u32 vehicle)
    {
		MK7::Kart::VehicleMove* vehicleMove = (MK7::Kart::VehicleMove*)vehicle;
		if (vehicleMove->m_is_net_recv) return;

		int playerID = vehicleMove->m_player_id;
		auto* areaAccessor = MK7::Field::GetAreaAccessor();
		for (auto it = areaAccessor->m_entries.begin(); it != areaAccessor->m_entries.end(); ++it) {
			if (it->m_area_calc->m_area_type == 10 && it->m_area_calc->isInside(*vehicleMove->m_position)) {
				u8 currCheckpoint = vehicleMove->m_lap_rank_checker->m_kart_infos[playerID].m_checkpoint_index;
				bool doRespawn = false;
				if (it->m_data->m_setting1 == 0 && it->m_data->m_setting2 == 0) {
					doRespawn = true;
				} else if (it->m_data->m_setting1 < it->m_data->m_setting2) {
					doRespawn = (currCheckpoint >= it->m_data->m_setting1 && currCheckpoint < it->m_data->m_setting2);
				} else {
					doRespawn = (currCheckpoint >= it->m_data->m_setting1 || currCheckpoint < it->m_data->m_setting2);
				}
				if (doRespawn) {
					vehicleMove->m_status_flags.jugem_recover_ai_oob = true;
				}
			}
		}
    }

    float MarioKartFramework::adjustRubberBandingSpeed(float initialAmount) {
		initialAmount += rubberBandingOffset;
		if (initialAmount <= 1.f)
			return initialAmount;
		else
			return ((initialAmount - 1.f) * rubberBandingMultiplier) + 1.f;
	}

	void MarioKartFramework::BaseResultBar_SetTeam(u32 resultBar, int teamMode) {
		u8 mode = ((u8*)resultBar)[0x7B];
		VisualControl::GameVisualControl& barControl = *(VisualControl::GameVisualControl*)resultBar;
		if (mode != 2) {
			barControl.GetAnimationFamily(2)->SetAnimation(0, teamMode + 1);
		} else {
			barControl.GetAnimationFamily(1)->SetAnimation(0, teamMode - 1);
		}
	}

	void MarioKartFramework::BaseResultBar_SetBarColor(u32 resultBar, int colorID) {
		//ColorID: 0 -> grey, 1 -> orange, 2 -> red, 3 -> green, 4 -> yellow
		const float table[] = {0.f, 1.f, 7.f, 14.f, 21.f};
		VisualControl::GameVisualControl& barControl = *(VisualControl::GameVisualControl*)resultBar;
		barControl.GetAnimationFamily(2)->SetAnimation(0, table[colorID]);
	}

	void MarioKartFramework::BaseResultBar_SetCTGPOnline(u32 resultBar) {
		BaseResultBar_SetBarColor(resultBar, 2);
	}

	void MarioKartFramework::UpdateResultBars(u32 racePage) {
		if (MissionHandler::isMissionMode) {
			MissionHandler::updateResultBars(racePage);
			return;
		}
		if (PointsModeHandler::isPointsMode) {
			PointsModeHandler::updateResultBars(racePage);
			return;
		}
		CRaceInfo* raceInfo = getRaceInfo(true);
		if (g_getCTModeVal != CTMode::OFFLINE) {
			for (int i = 0; i < raceInfo->playerAmount; i++) {
				if ((g_getCTModeVal == CTMode::ONLINE_NOCTWW) && i != masterPlayerID) BaseResultBar_SetCTGPOnline(resultBarArray[i]);
				if (Net::othersBadgeIDs[i] != 0) {
					BadgeManager::Badge b = BadgeManager::GetBadge(Net::othersBadgeIDs[i], BadgeManager::GetBadgeMode::BOTH);
					if (b.bID == 0 || b.icon.DataSizeNoHeader() != 0x400) {
						u32 temp = (u32)StarGrade::BADGE_EMPTY;
						BaseResultBar_SetGrade(resultBarArray[i], &temp);
						continue;
					}
					
					memcpy(BadgeManager::badge_slots[i].data, b.icon.data, 0x400);
					svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)BadgeManager::badge_slots[i].data & ~0xFFF), (0x400 & ~0xFFF) + 0x2000);

					u32 temp = ((u32)StarGrade::BADGE_SLOT_0) + i;
					BaseResultBar_SetGrade(resultBarArray[i], &temp);
				}
			}
		}
		if (VoiceChatHandler::Initialized) {
			VoiceChatHandler::SetRaceMode(false, false);
		}
	}

	static u32 g_updateVoiceDataTimer = 0;
	void MarioKartFramework::KartNetDataSend(u32* kartData, int playerID) {
		MK7NetworkBuffer dataBuffer; // Will be 0x88 bytes

		void(*NetUtilStartWriteKartSendBuffer)(MK7NetworkBuffer& outBuffer) = (decltype(NetUtilStartWriteKartSendBuffer))NetUtilStartWriteKartSendBufferAddr;
		NetUtilStartWriteKartSendBuffer(dataBuffer);
		
		dataBuffer.Set(kartData, 0x48); // Normal game data
		
		CustomCTGP7KartData customData;
		OnSendCustomKartData(playerID, customData);
		customData.MakeValid();
		dataBuffer.Add(&customData, sizeof(CustomCTGP7KartData)); // Shouldn't exceed the 0x88 limit

		void(*NetUtilEndWriteKartSendBuffer)(int playerID, MK7NetworkBuffer& outBuffer) = (decltype(NetUtilEndWriteKartSendBuffer))NetUtilEndWriteKartSendBufferAddr;
		NetUtilEndWriteKartSendBuffer(playerID, dataBuffer);

		if (VoiceChatHandler::Initialized && (g_updateVoiceDataTimer++ & 1) && playerID == masterPlayerID) {
			u32 kartDirector = getKartDirector();
			if (kartDirector) {
				Vector3T<s8> myFwd, myUp;
				std::array<Vector3T<s16>, 8> pos;
				std::array<u8, 8> flags;
				u32 kartCount = ((u32*)kartDirector)[0x28 / 4];
				u32* kartUnits = ((u32**)kartDirector)[0x2C / 4];
				for (u32 i = 0; i < kartCount; i++) {
					u32 currUnit = kartUnits[i];
					u32* vehicle = ((u32**)currUnit)[0x2C / 4];
					if (i == masterPlayerID) {
						struct CameraBasePosData {
							Vector3 postion;
							Vector3 lookAt;
							Vector3 up;
							Vector3 fwd;
						};
						u32* camera = *(((u32**)currUnit) + 0x214/4);
						u32* cameraBase = *(((u32**)camera) + 0xD0/4);
						CameraBasePosData* campos = (CameraBasePosData*)(cameraBase + 0x2C/4);
						pos[i] = campos->postion;
						myUp = campos->up * 100.f;
						myFwd = campos->fwd * 100.f;
					} else {
						pos[i] = *(Vector3*)(vehicle + 0x24/4);
					}

					bool isStar = vehicle[(0xC00 + 0x3F4) / 4] > 0;
					bool isSmall = vehicle[(0x1000) / 4] > 0 || vehicle[(0x1004) / 4] > 0;
					bool isMega = megaMushTimers[i] > 0;
					flags[i] = ((isStar ? 1 : 0) << 0) | ((isSmall ? 1 : 0) << 1) | ((isMega ? 1 : 0) << 2);
				}
				VoiceChatHandler::SetPositions(myFwd, myUp, pos, flags);
			}
		}
	}

	bool MarioKartFramework::KartNetDataRead(u32* kartData, MK7NetworkBuffer* dataBuf, u32** prevKartData, u32** nextKartData) {
		if (!dataBuf || !dataBuf->currentSize)
			return false;
		u32* incomingKartData = (u32*)dataBuf->dataPtr;
		int playerID = ((u8*)incomingKartData)[0xC];
		if (playerID != ((s8*)kartData)[0xC])
			return false;
		if (incomingKartData[0] <= kartData[0]) // Check incoming is a new sequence ID
			return false;
		
		memcpy(*nextKartData, incomingKartData, 0x46);
		u32* newBuffer = *nextKartData;
		u32* oldBuffer = *prevKartData;
		*nextKartData = oldBuffer;
		*prevKartData = newBuffer;

		if (dataBuf->currentSize >= 0x48 + sizeof(CustomCTGP7KartData)) {
			CustomCTGP7KartData* customData = (CustomCTGP7KartData*)(dataBuf->dataPtr + 0x48);
			if (customData->IsValid())
				OnRecvCustomKartData(playerID, *customData);
		}

		return true;
	}

	void MarioKartFramework::OnSendCustomKartData(int playerID, CustomCTGP7KartData& data) {
		data.megaMushTimer = megaMushTimers[playerID] >> 2;
		data.info.honkTimer = ItemHandler::playerHonkCounter[playerID] & 7;
	}

	void  MarioKartFramework::OnRecvCustomKartData(int playerID, CustomCTGP7KartData& data) {
		ItemHandler::MegaMushHandler::CalcNetRecv(playerID, data.megaMushTimer << 2);
		ItemHandler::playerHonkCounter[playerID] = data.info.honkTimer;
	}

	bool MarioKartFramework::brakeDriftingAllowed() {
		return brakeDriftForced || (brakeDriftAllowed && SaveHandler::saveData.flags1.brakedrift);
	}

	// Hooks into the calcDrift function, just before it compares your speed
	// to the drift cancel factor. This function can modify the drift cancel factor
	// which is normally 0.55
	void MarioKartFramework::checkCancelMiniturbo(u32 vehicleMove, float* driftCancelFactor) {
		float speedloss = ((KCLTerrainInfo*)(vehicleMove + 0xD3C))->speedLoss * 100.f;
		KartButtonData buttonData = KartButtonData::GetFromVehicle((u32)vehicleMove);
		float currentSpeed = *(float*)(vehicleMove + 0xC00 + 0x32C);
        int playerID = ((u32*)vehicleMove)[0x84/4];
		float driftTimer = ((float*)vehicleMove)[(0xC00 + 0x308)/4];
		if (brakeDriftingAllowed() && driftTimer >= 100.f) {
			if (buttonData.brake && speedloss >= 99.f && currentSpeed >= 0.1f) {
				brakeDriftAllowFrames[playerID] = 20;
			}
			if (brakeDriftAllowFrames[playerID] > 0) {
				brakeDriftAllowFrames[playerID]--;
				// Setting the variable below to 0 allows you to mantain your drift
				// no matter your speed for the current frame.
				*driftCancelFactor = 0.f;
			}
		} else {
			brakeDriftAllowFrames[playerID] = 0;
		}
	}

	int MarioKartFramework::getMiniturboLevel(u32 vehicleMove) {
		float miniturbo = ((float*)vehicleMove)[(0xC00 + 0x308)/4];
		float blueMiniturboLimit = 220.f;
		float orangeMiniturboLimit = 460.f;
		if (miniturbo >= orangeMiniturboLimit)
			return 3;
		if (miniturbo >= blueMiniturboLimit)
			return 2;
		if (miniturbo >= 0)
			return 1;
		return 0;
	}

	int MarioKartFramework::handleBootSequenceCallback()
	{
		return 1;
	}

	extern LightEvent mainEvent1;
	void MarioKartFramework::OnBootTaskFinish()
	{
		LightEvent_Signal(&mainEvent1);
		UserCTHandler::ApplySkipToCourseConfig();
	}
	
}

namespace CTRPluginFramework {
	void g_updateCTModeImpl() {
		if (g_setCTModeVal == CTMode::INVALID) return;
		g_getCTModeVal = g_setCTModeVal;
		g_setCTModeVal = CTMode::INVALID;

		switch (g_getCTModeVal)
		{
		case CTMode::OFFLINE:
			CourseManager::setCustomTracksAllowed(true);
			CourseManager::setOriginalTracksAllowed(true);
			MarioKartFramework::improvedTricksAllowed = true;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = true;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::nexNetworkInstance = 0;
			MarioKartFramework::automaticDelayDriftAllowed = true;
			ItemHandler::allowFasterItemDisappear = true;
			MenuPageHandler::MenuSingleCourseBasePage::ClearBlockedCourses();
			isCTWW = 0;
			isAltGameMode = 0;
			MarioKartFramework::isPauseAllowForced = false;
			break;
		case CTMode::ONLINE_NOCTWW:
			CourseManager::setCustomTracksAllowed(false);
			CourseManager::setOriginalTracksAllowed(true);
			MarioKartFramework::improvedTricksAllowed = false;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = false;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::changeNumberRounds(4);
			ccsettings[1].enabled = false;
			ccselector_apply(ccselectorentry);
			MarioKartFramework::automaticDelayDriftAllowed = false;
			ItemHandler::allowFasterItemDisappear = false;
			MenuPageHandler::MenuSingleCourseBasePage::ClearBlockedCourses();
			isCTWW = 0;
			isAltGameMode = 0;
			MarioKartFramework::isPauseAllowForced = true;
			break;
		case CTMode::ONLINE_CTWW:
			CourseManager::setCustomTracksAllowed(true);
			#if CITRA_MODE == 0
			CourseManager::setOriginalTracksAllowed(true);
			#else
			CourseManager::setOriginalTracksAllowed(false);
			#endif
			MarioKartFramework::improvedTricksAllowed = true;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = true;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::automaticDelayDriftAllowed = true;
			ItemHandler::allowFasterItemDisappear = true;
			MarioKartFramework::changeNumberRounds(4);
			ccsettings[1].enabled = false;
			ccselector_apply(ccselectorentry);
			isCTWW = 1;
			isAltGameMode = 0;
			MarioKartFramework::isPauseAllowForced = true;
			break;
		case CTMode::ONLINE_CTWW_CD:
			CourseManager::setCustomTracksAllowed(true);
			#if CITRA_MODE == 0
			CourseManager::setOriginalTracksAllowed(true);
			#else
			CourseManager::setOriginalTracksAllowed(false);
			#endif
			MarioKartFramework::improvedTricksAllowed = true;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = true;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::automaticDelayDriftAllowed = true;
			ItemHandler::allowFasterItemDisappear = true;
			MarioKartFramework::changeNumberRounds(4);
			ccsettings[1].enabled = false;
			ccselector_apply(ccselectorentry);
			isCTWW = 1;
			isAltGameMode = 1;
			MarioKartFramework::isPauseAllowForced = true;
			break;
		case CTMode::INVALID:
		default:
			break;
		}
	}
}

CTMode g_setCTModeVal = CTMode::INVALID;
CTMode g_getCTModeVal = CTMode::INVALID;
void g_updateCTMode() {
	CTRPluginFramework::g_updateCTModeImpl();
}