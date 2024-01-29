/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MarioKartFramework.cpp
Open source lines: 3319/3423 (96.96%)
*****************************************************/

#include "MarioKartFramework.hpp"
#include "cheats.hpp"
#include "entrystructs.hpp"
#include "3ds.h"
#include "CourseManager.hpp"
#include "CharacterManager.hpp"
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

extern "C" u32 g_gameModeID;
extern "C" u32 g_altGameModeIsRaceOver;
extern "C" u32 g_altGameModeplayerStatPointer;
extern "C" u32 g_altGameModeHurryUp;
extern "C" u32 g_downTimerStartTime;

namespace CTRPluginFramework {
	extern "C" void drawMdlReplaceTextureRefThunk(void* cgfxRes, SafeStringBase* dst, SafeStringBase* src, void* funcptr);

	u32 SafeStringBase::vTableSafeStringBase = 0;

	u32 MarioKartFramework::baseAllPointer = 0;
	MarioKartFramework::CRaceMode MarioKartFramework::currentRaceMode;
	void (*MarioKartFramework::BasePage_SetRaceMode)(MarioKartFramework::CRaceMode* mode) = nullptr;
	MarioKartFramework::EPlayerInfo MarioKartFramework::playerInfos[8];
	u32 MarioKartFramework::currGatheringID;
	bool MarioKartFramework::imRoomHost = false;
	u8* MarioKartFramework::_currgamemode = nullptr;
	char* MarioKartFramework::replayFileName = nullptr;
	u32 MarioKartFramework::ctgp7ver = SYSTEM_VERSION(0, 0, 0);
	u32 MarioKartFramework::region = 0;
	u32 MarioKartFramework::revision = 0;
	std::string MarioKartFramework::onlineCode = "miHwotFrqehybxGvaAunYdzsfBkjpQgc";
	u16* MarioKartFramework::commDisplayText = nullptr;
	u32 MarioKartFramework::lastLoadedMenu = 0;
	u32* MarioKartFramework::oldStrloc = nullptr;
	u32 MarioKartFramework::oldStrptr = 0;
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
	u32 MarioKartFramework::origChangeMapInst = 0xE320F000;
	u32* MarioKartFramework::changeMapCallPtr = nullptr;
	u64 MarioKartFramework::kouraGProbability = 1;
	u64 MarioKartFramework::kouraRProbability = 1;
	u8 MarioKartFramework::previousCDScore = 0;
	void (*MarioKartFramework::openDialogImpl)(DialogFlags::Mode mode, int messageID, void* args) = nullptr;
	bool (*MarioKartFramework::isDialogClosedImpl)() = nullptr;
	bool (*MarioKartFramework::isDialogYesImpl)() = nullptr;
	void (*MarioKartFramework::closeDialogImpl)() = nullptr;
	void (*MarioKartFramework::nwlytReplaceText)(u32 a0, u32 a1, u32 msgPtr, u32 a3, u32 a4) = nullptr;
	bool MarioKartFramework::needsOnlineCleanup = false;
	bool MarioKartFramework::playCountDownCameraAnim = false;
	bool MarioKartFramework::startedRaceScene = false;
	std::vector<std::tuple<u32, void(*)(u32*)>> MarioKartFramework::regButtons;
	bool MarioKartFramework::wasKeyInjected = false;
	Key MarioKartFramework::injectedKey = (Key)0;
	bool MarioKartFramework::areKeysBlocked = false;
	std::tuple<u32, u32*> MarioKartFramework::soundThreadsInfo[2] = { std::tuple<u32, u32*>(0xFFFFFFFF, nullptr), std::tuple<u32, u32*>(0xFFFFFFFF, nullptr) };
	bool MarioKartFramework::forceDisableSndOnPause = false;
	void (*MarioKartFramework::BaseMenuPageApplySetting_CPU)(int cpuAmount, int startingCPUIndex, int* playerChar) = nullptr;
	void (*MarioKartFramework::SequenceCorrectAIInfo)() = nullptr;
	bool MarioKartFramework::isWatchRaceMode = false;
	bool MarioKartFramework::isBackCamBlockedComm = false;
	bool MarioKartFramework::isWarnItemBlockedComm = false;
	bool MarioKartFramework::allowCPURacersComm = false;
	bool MarioKartFramework::allowCustomItemsComm = true;
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
	MarioKartFramework::BaseResultBar MarioKartFramework::resultBarArray[8] = { 0 };
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
	void (*MarioKartFramework::ItemDirector_StartSlot)(u32 itemDirector, u32 playerID, u32 boxID) = nullptr;
	u16 MarioKartFramework::itemProbabilities[EItemSlot::ITEM_SIZE] = { 0 };
	int (*MarioKartFramework::CsvParam_getDataInt)(void*, int, int) = nullptr;
	int MarioKartFramework::nextForcedItem = -1;
	void (*MarioKartFramework::SndActorKArt_PlayDriverVoice)(u32 sndActorKartObj, EVoiceType voice) = nullptr;
	void (*MarioKartFramework::RacePageInitFunctions[RacePageInitID::AMOUNT])(void* baseRacePage) = {nullptr};
	u8 MarioKartFramework::numberPlayersRace = 0;
	std::vector<std::pair<u32, u32>> MarioKartFramework::carRouteControllerPair;
	void (*MarioKartFramework::kartStartTwist)(u32 kartVehicleMoveObj, u32 restartBool) = nullptr;
	MarioKartFramework::ImprovedTrickInfo MarioKartFramework::trickInfos[8];
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
	u32 (*MarioKartFramework::objectBaseGetRandom32)(u32 fieldObjectbase, u32 maxValue) = nullptr;
	u32 (*MarioKartFramework::vehicleMove_startDokanWarp)(u32 vehicleMove, u32 chosenPoint, Vector3* targetPosition, Vector3* kartDirection) = nullptr;
	u32 MarioKartFramework::countdownDisconnectTimer;
	MarioKartTimer MarioKartFramework::graceDokanPeriod;
	bool MarioKartFramework::lastPolePositionLeft = false;
	bool MarioKartFramework::bulletBillAllowed = true;
	bool MarioKartFramework::speedupMusicOnLap = false;
	u8 MarioKartFramework::lastCheckedLap = 1;
	float MarioKartFramework::raceMusicSpeedMultiplier = 1.f;
	std::default_random_engine MarioKartFramework::randomDriverChoicesGenerator(1);
	std::vector<std::pair<u8, FixedStringBase<u16, 0x20>>> MarioKartFramework::onlineBotPlayerIDs;
	float MarioKartFramework::rubberBandingMultiplier = 1.f; // hard 1.75 and 0.15
	float MarioKartFramework::rubberBandingOffset = 0.0f;
	u32 MarioKartFramework::NetUtilStartWriteKartSendBufferAddr = 0;
	u32 MarioKartFramework::NetUtilEndWriteKartSendBufferAddr = 0;
	StarGrade MarioKartFramework::onlinePlayersStarGrade[8];
	void (*MarioKartFramework::BaseResultBar_SetGrade)(MarioKartFramework::BaseResultBar, u32* grade) = nullptr;
	u8 MarioKartFramework::brakeDriftAllowFrames[8];
	bool MarioKartFramework::brakeDriftAllowed = true;
	bool MarioKartFramework::brakeDriftForced = false;
	u32 MarioKartFramework::playjumpeffectAddr = 0;
	u32 MarioKartFramework::playcoineffectAddr = 0;
	u32 MarioKartFramework::VehicleMove_StartPressAddr = 0;
	u32 MarioKartFramework::VehicleReact_ReactPressMapObjAddr = 0;
	ResizeInfo MarioKartFramework::resizeInfos[8];
	int MarioKartFramework::megaMushTimers[8] = { 0 };
	int MarioKartFramework::packunStunCooldownTimers[8] = { 0 };
	SndLfoSin MarioKartFramework::pitchCalculators[8];
	bool MarioKartFramework::ignoreKartAccident = false;
	u32 MarioKartFramework::SndActorBase_SetCullingSafeDistVolRatioAddr = 0;
	float MarioKartFramework::playerMegaCustomFov = 0.f;
	bool MarioKartFramework::thankYouDisableNintendoLogo = false;
	RT_HOOK MarioKartFramework::OnSimpleModelManagerHook = { 0 };
	bool MarioKartFramework::automaticDelayDriftAllowed = true;
	RT_HOOK MarioKartFramework::enemyAIControlRaceUpdateHook = { 0 };
	u8 MarioKartFramework::onMasterStartKillerPosition;

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
	void MarioKartFramework::changeFilePath(u16* dst, bool isDir) {
		if (!isDir && !g_patchedBcsar && strfind16(dst, (u16*)u".bcsar")) {
			CrashReport::stateID = CrashReport::StateID::STATE_MAIN;
			g_patchedBcsar = true;
			CharacterManager::applyDirectoryPatches();
			CharacterManager::applySoundPatches();
			CharacterManager::currSave.needsPatching = false;
			CharacterManager::saveSettings();
			OnionFS::initGameFsFileMap();
		}
		if (strfind16(dst, (u16*)u"chive-")) {
			Language::FixRegionSpecificFile(dst);
			return;
		}
		if (strfind16(dst, (u16*)u"UI/") && strfind16(dst + 12, (u16*)u"-")) {

			Language::SZSID id = Language::NONE;
			if (strfind16(dst, (u16*)u"I/m")) id = Language::MENU;
			else if (strfind16(dst, (u16*)u"I/co")) id = Language::COMMON;
			else if (strfind16(dst, (u16*)u"I/r")) id = Language::RACE;
			else if (strfind16(dst, (u16*)u"I/th")) id = Language::THANKYOU;
			else if (strfind16(dst, (u16*)u"I/tr")) id = Language::TROPHY;
			if (id != Language::NONE) {Language::GetLangSpecificFile(dst, id, strfind16(dst, (u16*)u"Pat")); return;}
		}
		int coursePos;
		bool usingCustomCup = UserCTHandler::IsUsingCustomCup();
		if ((usingCustomCup || CourseManager::lastLoadedCourseID < BATTLETRACKLOWER) && strfind16(dst, (u16*)u"Course/", &coursePos) && strfind16(dst, (u16*)u".szs")) {
			if (UserCTHandler::IsUsingCustomCup() && CourseManager::lastLoadedCourseID != 0x26) UserCTHandler::GetCouseSZSPath(dst, strfind16(dst + ((coursePos >= 0) ? coursePos : 0), (u16*)u"-"));
			else if (CourseManager::lastLoadedCourseID < BATTLETRACKLOWER) {dst[0] = '\0'; return;}
		}
		if (CharacterManager::thankYouFilterCharacterFiles && CharacterManager::filterChararacterFileThankYou(dst)) {return;}

		if (isDir) return;
		
		if (MissionHandler::isMissionMode && strfind16(dst, (u16*)u"SE_I")) {MissionHandler::LoadCoursePreview(dst); return;}
		if (MenuPageHandler::MenuEndingPage::loadCTGPCredits && strfind16(dst, (u16*)u"RM_STA")) {strcpy16(dst, (u8*)"ram:/CTGP-7/gamefs/Sound/stream/STRM_STAFF_MOD.bcstm"); return;}
		if (strfind16(dst, (u16*)u"RM_TI")) {
			if (!g_randomChoiceTitleAlt) {
				g_randomChoiceTitleAlt = (Utils::Random() & 1) + 1;
			}
			if (SaveHandler::saveData.GetPendingAchievementCount() != 0) {
				g_randomChoiceTitleAlt = 2;
			}
			if (SaveHandler::saveData.GetCompletedAchievementCount() != 0 && g_randomChoiceTitleAlt == 2) {
				strcpy16(dst, (u8*)"ram:/CTGP-7/gamefs/Sound/stream/STRM_TITLE_ALT.bcstm"); return;
			}
		}
		if (!strfind16(dst, (u16*)u"RM_C")) return;
		// Only music files at this point.
		bool lastLap = strfind16(dst, (u16*)u"F.");
		if (lastLap && g_isFoolActive) {
			playSirenJoke();
		}
		else if (lastLap && CourseManager::isRainbowTrack(CourseManager::lastLoadedCourseID)) {
			LED::PlayLEDPattern(MarioKartFramework::ledFinalRainbowPat, Seconds(2 * 3));
		}

#ifdef NOMUSIC
		strcpy16(dst, (u16*)u"ram:/CTGP-7/resources/nomusic.bcstm");
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
			u16 tmpBuf[50];
			strcpy16(tmpBuf, (const u8*)(*it).second.musicFileName.data());
			filestring.append((char16_t*)tmpBuf);
			if (lastLap) {
				filestring.append(u"_F.bcstm");
			}
			else {
				filestring.append(u"_N.bcstm");
			}
			strcpy16(dst, (const u16*)filestring.data());
		}
		return;
	}

	void MarioKartFramework::onRaceEvent(u32 raceEventID)
	{
		// 0 -> Start countdown, 1 -> Start race, 2 -> Pause, 3 -> Unpause, 4 -> End race
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
			startedRaceScene = false;
		}
		if (raceEventID == 4 && userCanControlKart()) {
			isRaceGoal = true;
			MissionHandler::OnRaceFinish();
			if (!MissionHandler::isMissionMode) StatsHandler::OnCourseFinish();
		}
		else if (raceEventID == 4 && g_isFoolActive)
		{
			playMemEraseJoke();
		}
	}

	bool MarioKartFramework::isGameInRace() {
		return isRaceState;
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

	u32 MarioKartFramework::getRaceEngine()
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

	FixedStringBase<u16, 0x20>* MarioKartFramework::getPlayerNames() {
		return (FixedStringBase<u16, 0x20>*)(getMenuData() + 0x3C0);
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

	u32 MarioKartFramework::BasePageGetCup() {
		return *(u8*)(getMenuData() + 0x620);
	}

	static bool g_isUserInControlWW = true;
	static bool g_hasGameReachedTitle = false;
	static Clock g_InvalidSoundTimer;
	
	bool MarioKartFramework::allowOpenCTRPFMenu()
	{
		if (!g_hasGameReachedTitle || MenuPageHandler::MenuSingleCharaPage::isInSingleCharaPage)
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
	bool MarioKartFramework::allowTakeScreenshot() {
		bool ret = !ISGAMEONLINE;
		if (g_InvalidSoundTimer.HasTimePassed(Seconds(0.5))) {
			Snd::PlayMenu(ret ? Snd::SLOT_STOP_TIRE : Snd::BUTTON_INVALID);
			g_InvalidSoundTimer.Restart();
		}
		return ret;
	}
	// Code: miHwotFrqehybxGvaAunYdzsfBkjpQgc
	void MarioKartFramework::restoreComTextPtr(u32* strptr) {
		if (oldStrloc != nullptr) {
			*oldStrloc = oldStrptr;
		}
		oldStrloc = nullptr;
		if (strptr != nullptr) {
			oldStrloc = strptr;
			oldStrptr = *oldStrloc;
		}
	}
	int MarioKartFramework::getCodeByChar(char c, u8 pos) {
		for (int i = 0; i < onlineCode.size(); i++) {
			if (c == onlineCode[i]) {
				u8 index = (32 + i - pos) % 32;
				return index;
			}
		}
		return 31;
	}
	u64 MarioKartFramework::decodeFromStr(std::u16string &s) {
		u64 retval = 0;
		for (int i = 0; i < 13; i++) {
			retval = (retval << 5) | getCodeByChar((char)s[i], i);
		}
		return retval;
	}
	char MarioKartFramework::getCharByCode(u8 num, u8 pos) {
		u8 index = (32 + num + pos) % 32;
		return onlineCode[index];
	}
	
	void MarioKartFramework::encodeFromVal(char out[14], u64 in) {
		out[13] = 0;
		for (int i = 12, j = 0; i >= 0; i--) {
			out[i] = getCharByCode((in >> (5 * j)) & 0x1F, i);
			j++;
		}
	}
	u8 MarioKartFramework::getOnlinechecksum(OnlineSettingsv2* onlineset) {
	}
	void MarioKartFramework::loadCustomSetOnline(OnlineSettings& onlineset) {
		g_setCTModeVal = CTMode::ONLINE_COM;
		g_updateCTMode();

		CCSettings   *settings = static_cast<CCSettings *>(ccselectorentry->GetArg());
		int speed;
		int trackamount;
		if (onlineset.ver >= 4 && onlineset.ver <= 7) {
			OnlineSettingsv2& setv2 = *(OnlineSettingsv2*)&onlineset;
			speed = setv2.speed;
			trackamount = setv2.rounds;
			if (speed != 0) {
				settings->value = speed;
				settings->enabled = true;
				ccselector_apply(ccselectorentry);
			}
			else {
				settings->enabled = false;
				ccselector_apply(ccselectorentry);
			}
			changeNumberRounds(1 << (setv2.rounds + 1));
			CourseManager::setCustomTracksAllowed(setv2.areCustomTracksAllowed);
			CourseManager::setOriginalTracksAllowed(setv2.areOrigTracksAllowed);
			isBackCamBlockedComm = !setv2.isBackcamAllowed;
			isWarnItemBlockedComm = !setv2.isLEDItemsAllowed;
			allowCPURacersComm = setv2.cpuRacers;
			CourseManager::isRandomTracksForcedComm = setv2.areRandomTracksForced;
			bool hasImprTricks = false;
			if (onlineset.ver >= 5) hasImprTricks = setv2.improvedTricksAllowed;
			improvedTricksAllowed = hasImprTricks;
			improvedTricksForced = hasImprTricks;
			bool allowCustomItems = true;
			if (onlineset.ver >= 6) allowCustomItems = setv2.customItemsAllowed;
			allowCustomItemsComm = allowCustomItems;
			ItemHandler::allowFasterItemDisappear = allowCustomItems;
			bool automaticDelayDrift = true;
			if (onlineset.ver >= 7) automaticDelayDrift = setv2.automaticDelayDriftAllowed;
			automaticDelayDriftAllowed = automaticDelayDrift;
		} else {
			if (onlineset.ver == 1) {
				speed = onlineset.speedandcount;
				trackamount = 0;
			}
			else if (onlineset.ver == 2 || onlineset.ver == 3) {
				speed = onlineset.speedandcount & 0x3FFF;
				trackamount = onlineset.speedandcount >> 14;
			}
			if (speed != 0) {
				settings->value = speed;
				settings->enabled = true;
				ccselector_apply(ccselectorentry);
			}
			else {
				settings->enabled = false;
				ccselector_apply(ccselectorentry);
			}
			changeNumberRounds(1 << (trackamount + 2));
			if (onlineset.enabledTracks == 0xFFFFFFFF) {
				CourseManager::setCustomTracksAllowed(true);
				CourseManager::setOriginalTracksAllowed(true);
			}
			else if (onlineset.enabledTracks == 0x7FFFFFFF) {
				CourseManager::setCustomTracksAllowed(true);
				CourseManager::setOriginalTracksAllowed(false);
			}
			else {
				CourseManager::setCustomTracksAllowed(false);
				CourseManager::setOriginalTracksAllowed(false);
			}
		}
	}
	void MarioKartFramework::applycommsettings(u32* commdescptr) {
		restoreComTextPtr();
		if (commDisplayText != nullptr) {
			delete commDisplayText;
			commDisplayText = nullptr;
		}
		std::u16string commdesc((char16_t*)(*(commdescptr+1)));
		if (commdesc.size() < 13) {
			g_setCTModeVal = CTMode::ONLINE_NOCTWW;
			g_updateCTMode();
			return;
		}
		u64 settings = decodeFromStr(commdesc);
		OnlineSettings* onlineset = (OnlineSettings*)&settings;
		if (onlineset->ver > COMMUSETVER  || onlineset->ver < 1 || getOnlinechecksum((OnlineSettingsv2*)onlineset) != onlineset->checksum) {
			g_setCTModeVal = CTMode::ONLINE_NOCTWW;
			g_updateCTMode();
			return;
		}

		u32 speed;
		u32 trackamount;
		std::string trackstate = "";
		u32 realRoundAmount;
		if (onlineset->ver >= 4 && onlineset->ver <= 7) {
			OnlineSettingsv2* setver2 = (OnlineSettingsv2*)onlineset;

			if (setver2->areCustomTracksAllowed && setver2->areOrigTracksAllowed) trackstate.append("CT,OT ");
			else if (setver2->areCustomTracksAllowed) trackstate.append("CT ");
			else trackstate.append("OT ");

			speed = setver2->speed;
			trackamount = setver2->rounds;

			realRoundAmount = 1 << (setver2->rounds + 1);
		}
		else if (onlineset->ver <= 3) {
			if (onlineset->enabledTracks == 0xFFFFFFFF) trackstate.append("CT,OT ");
			else if (onlineset->enabledTracks == 0x7FFFFFFF) trackstate.append("CT ");
			else trackstate.append("OT ");

			if (onlineset->ver == 1) {
				speed = onlineset->speedandcount;
				trackamount = 0;
			}
			else if (onlineset->ver == 2 || onlineset->ver == 3) {
				speed = onlineset->speedandcount & 0x3FFF;
				trackamount = onlineset->speedandcount >> 14;
			}
			realRoundAmount = 1 << (trackamount + 2);
		}

		if (speed > 9999) speed = 9999;

		if (speed != 0) trackstate.append(std::to_string(speed) + "cc,");

		trackstate.append(std::to_string(realRoundAmount) + "r");

		if (commDisplayText != nullptr) delete commDisplayText;

		Snd::PlayMenu(Snd::SLOT_OK);

		commDisplayText = new u16[0x30];
		memset(commDisplayText, 0, sizeof(u16) * 0x30);
		utf8_to_utf16(commDisplayText, (u8*)trackstate.c_str(), sizeof(u16) * 0x30);
		restoreComTextPtr(commdescptr+1);
		*(commdescptr+1) = (u32)commDisplayText;

		if (lastLoadedMenu == 0x27 || lastLoadedMenu == 0x2C) loadCustomSetOnline(*onlineset);
		if (lastLoadedMenu == 0x2C) g_ComForcePtrRestore = true;
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
	u32 MarioKartFramework::handleBackwardsCamera(u32 pad)
	{
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
			return pad;
		}
		// Only applies to race from now on
		u32 pressedpad = (pad ^ g_prevPad) & pad;
		bool missionForceBackwards = MissionHandler::forceBackwards();
		bool autoaccelenabled = !MissionHandler::isMissionMode && SaveHandler::saveData.flags1.autoacceleration;
		if (((bool)SaveHandler::saveData.flags1.backCamEnabled) && !isRaceGoal && !isRacePaused && !missionForceBackwards && !playCountDownCameraAnim && (g_getCTModeVal != CTMode::ONLINE_COM || !isBackCamBlockedComm)) {
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
		return pad;
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
	void MarioKartFramework::kartCameraSmooth(u32* camera, float smoothVal)
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

	u16 MarioKartFramework::getSTGIEntryCount(STGIEntry** s) {
		if (!s) return 0;
		return *((u16*)((u32*)(*s) - 1));
	}

	void MarioKartFramework::kmpConstructCallback() {

		previousCDScore = 0;
		STGIEntry** stginfo = getStageInfoFuncptr();
		u32 entrycount = getSTGIEntryCount(stginfo);

		// Clar car controller vector
		carRouteControllerPair.clear();
		dokanWarps.clear();

		if (entrycount > 0)
			lastPolePositionLeft = (*stginfo)[0].PolePosition == 1;

		for (int i = 0; i < 8; i++)
		{
			megaMushTimers[i] = 0;
			packunStunCooldownTimers[i] = 0;
			pitchCalculators[i].Start(false);
			pitchCalculators[i].Stop();
			pitchCalculators[i].SetSpeed(5.851f * 1.25f);
			onlinePlayersStarGrade[i] = StarGrade::INVALID;
			ItemHandler::MegaMushHandler::growMapFacePending[i] = 0;
		}


		MissionHandler::OnKMPConstruct();

		rubberBandingMultiplier = 1.0f;
		rubberBandingOffset = 0.0f;
		if (g_getCTModeVal == ONLINE_CTWW || g_getCTModeVal == ONLINE_CTWW_CD) {
			rubberBandingMultiplier = Net::ctwwCPURubberBandMultiplier;
			rubberBandingOffset = Net::ctwwCPURubberBandOffset;
		} else if (g_getCTModeVal == OFFLINE && VersusHandler::IsVersusMode) {
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
			s32 driverId = playerInfos[0].driver;
			u32 playerType = EPlayerType::TYPE_CPU;
			BasePage_SetDriver(0, &driverId, &playerType);
		}

		kmpRenderOptimizationsForced = entrycount > 1 && (*stginfo)[1].CustomFlags.forceRenderOptim;
		bulletBillAllowed = !(entrycount > 1 && (*stginfo)[1].CustomFlags.disableBulletBill);
		speedupMusicOnLap = entrycount > 1 && (*stginfo)[1].CustomFlags.speedupMusicOnLap;
		lastCheckedLap = 1;
		raceMusicSpeedMultiplier = 1.f;

		float speedmod = 1;
		if (entrycount > 1) {
			u8 poleval = (float)(*stginfo)[1].PolePosition;
			if (poleval > 1)
				speedmod = poleval / 16.f;
		}
		if (!MissionHandler::isMissionMode) {
			CCSettings* ccset = static_cast<CCSettings*>(ccselectorentry->GetArg());
			float ccvalaue = ccset->enabled ? (ccset->value * CC_SINGLE) : 1.f;

			if (ccset->enabled && ccset->value < 150) {
				speedmod = ccvalaue * speedmod;
			} else {
				if (ccvalaue > speedmod) {
					speedmod = ccvalaue;
				}
			}
		} else speedmod = MissionHandler::CalculateSpeedMod(speedmod);

		//u32 speedval = 0x7effffff;
		//speedmod = 40282346638528859811704183484516925440.f;//*(float*)&speedval;	////340282346638528859811704183484516925440.f / 10.f;
		
		g_speedVal = speedmod;
		g_speedValsqrt = (sqrtf(speedmod) + speedmod) / 2.f;

		kouraGProbability = rol<u64>(kouraGProbability, Utils::Random(0, 63));
		kouraRProbability = rol<u64>(kouraRProbability, Utils::Random(0, 63));
		applyGOBJPatches(getGeoObjAccessor());

		return;
	}

	void MarioKartFramework::OnRaceEnter() {
		isRaceState = true;
		isRaceGoal = false;
		MissionHandler::OnRaceEnter();
	}

	void MarioKartFramework::OnRaceExit(u32 mode) {
		isRaceState = false;
		isRacePaused = false;
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
				if (currEntry.settings[2] == 1 && currentRaceMode.mode != 1) { //If not in time trials and settings3 = 1
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
		if (((kouraGProbability & 1) && (Utils::Random() & 1)) && g_getCTModeVal != CTMode::ONLINE_COM) ret = (u32)eggKouraG;
		else ret = (u32)origKouraG;
		kouraGProbability = rol<u64>(kouraGProbability, 1);
		return ret;
	}
	u32 MarioKartFramework::getKouraRModelName()
	{
		u32 ret;
		if (((kouraRProbability & 1) && (Utils::Random() & 1)) && g_getCTModeVal != CTMode::ONLINE_COM) ret = (u32)eggKouraR;
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
			u32* soundThreadTls = std::get<1>(soundThreadsInfo[i]);
			if (soundThreadID == 0xFFFFFFFF) continue;
			Handle soundThreadHandle;
			Result res = svcOpenThread(&soundThreadHandle, CUR_PROCESS_HANDLE, soundThreadID);
			if (R_FAILED(res)) return;

			if (playMusic) {
				tlsBackup[i] = *soundThreadTls;
				*soundThreadTls = THREADVARS_MAGIC;
				svcGetThreadPriority(&prioBackup[i], soundThreadHandle);
				svcSetThreadPriority(soundThreadHandle, FwkSettings::Get().ThreadPriority - 1);
			}
			else {
				*soundThreadTls = tlsBackup[i];
				svcSetThreadPriority(soundThreadHandle, prioBackup[i]);
			}
			svcCloseHandle(soundThreadHandle);
		}
	}

	DialogFlags::Mode g_dialogMode;
	void* g_dialogArgs;
	bool g_dialogFinishedOSD = true;
	static bool openDialogCallBack(const Screen& screen) {
		if (screen.IsTop) return false;
		MarioKartFramework::openDialogImpl(g_dialogMode, CustomTextEntries::dialog, g_dialogArgs);
		OSD::Stop(openDialogCallBack);
		g_dialogFinishedOSD = true;
		return false;
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
		OSD::Run(openDialogCallBack);
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
			string16 wctdw;
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

	bool MarioKartFramework::applyVisualElementsCTWWOSD(const Screen& screen) {
		adjustVisualElementsCTWW(SaveHandler::saveData.flags1.isCTWWActivated && SaveHandler::saveData.flags1.useCTGP7Server);
		OSD::Stop(applyVisualElementsCTWWOSD);
		return false;
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
				resetdriverchoices();
				currCommNumberTracksPtr = nullptr;
				g_firstTimeInPage = false;
				g_needsToCheckUpdate = true;
				OSD::Run(applyVisualElementsCTWWOSD);
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
						openDialog(DialogFlags::Async::YES | DialogFlags::Mode::OK, "Online access has\nbeen disabled for\nthis build.");
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
					OSD::Run(applyVisualElementsCTWWOSD);
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
	void MarioKartFramework::OnInternetConnection() { // Called as soon as the game tries to connect online
		Lock lock(g_internetConnectionMutex);         // including online multiplayer and mario kart channel
		if (!needsOnlineCleanup) {
			ccSelOnlineEntry->setOnlineMode(true);
			comCodeGenOnlineEntry->setOnlineMode(true);
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

	void MarioKartFramework::DoOnlineCleanup() {
		Lock lock(g_internetConnectionMutex);
		if (needsOnlineCleanup) { // This variable is set to true if the online 
			ccSelOnlineEntry->setOnlineMode(false);
			comCodeGenOnlineEntry->setOnlineMode(false);
			numbRoundsOnlineEntry->setOnlineMode(false);
			serverOnlineEntry->setOnlineMode(false);
			improvedTricksOnlineEntry->setOnlineMode(false);
			ccselectorentry->SetArg(&ccsettings[0]);
			ccselector_apply(ccselectorentry);
			MarioKartFramework::changeNumberRounds(SaveHandler::saveData.numberOfRounds);
			MarioKartFramework::restoreComTextPtr();
			if (MarioKartFramework::commDisplayText != nullptr) {
				delete MarioKartFramework::commDisplayText;
				MarioKartFramework::commDisplayText = nullptr;
			}
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
		Net::customAuthData.populated = false;
		Net::customAuthData.result = 0;
		Net::temporaryRedirectCTGP7 = false;
		VersusHandler::IsVersusMode = false;
		MissionHandler::onModeMissionExit();
		MarioKartFramework::setSkipGPCoursePreview(false);
		UserCTHandler::UpdateCurrentCustomCup(0);
		UserCTHandler::CleanTextureSarc();
		MenuPageHandler::MenuEndingPage::loadCTGPCredits = false;
		thankYouDisableNintendoLogo = false;
		if (UserCTHandler::skipConfig.enabled) {
			Process::ReturnToHomeMenu();
		}
		DoOnlineCleanup();
		SaveHandler::UpdateAchievementsConditions();
		if (g_hasGameReachedTitle)
			SaveHandler::SaveSettingsAll();
		g_hasGameReachedTitle = true;
	}

	static void OnTitleOnlineButtonPressed() {
		*(PluginMenu::GetRunningInstance()) -= OnTitleOnlineButtonPressed;
		#if CITRA_MODE == 0
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
		useCTGP7server_apply(true);
		#endif
		Net::UpdateOnlineStateMahine(Net::OnlineStateMachine::IDLE, true);
	}

	void MarioKartFramework::handleTitleCompleteCallback(u32 option)
	{
		// 0 -> Singleplayer, 1 -> Multiplayer, 2 -> Online, 3 -> Channel, 7 -> Demo race
		if (option < 4) {
			SaveHandler::SaveSettingsAll();
			StatsHandler::UploadStats();
		}
		DoOnlineCleanup();
		if (option == 2)
		{
			*(PluginMenu::GetRunningInstance()) += OnTitleOnlineButtonPressed;
			OnInternetConnection();
		}
	}

	static u32 g_welccounter = 1;
	u32 MarioKartFramework::handleTitleMenuPagePreStep(u32 timerVal)
	{
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
		if (g_isFoolActive)
			return 0;
#ifdef RELEASE_BUILD
		return timerVal;
#else
		return 0;
#endif // RELEASE_BUILD
		
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
		if (SaveHandler::saveData.flags1.warnItemEnabled && (g_getCTModeVal != CTMode::ONLINE_COM || !isWarnItemBlockedComm) && !g_isFoolActive) {
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
		string16 utf16;
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

	bool MarioKartFramework::onBasePageCanFinishFadeCallback(u32 page, u32 currFrame, u32 maxFrame)
	{
		bool ret = true;
		if (page == VersusHandler::MenuSingleClassPage) {
			if (VersusHandler::IsVersusMode && VersusHandler::canShowVSSet) {
				static u8 counter = 0;
				static bool startedCounter = false;
				if (currFrame == maxFrame && counter == 0 && !startedCounter) {
					counter = 0x2A;
					startedCounter = true;
					ret = false;
				}
				else if (counter == 0 && currFrame != maxFrame && startedCounter) {
					startedCounter = false;
					VersusHandler::canShowVSSet = false;
				}
				if (counter > 0) {
					counter--;
					ret = false;
				}
				if (counter == 0x8) {
					*(PluginMenu::GetRunningInstance()) += VersusHandler::OpenSettingsKeyboardCallback;
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
		else return
			track == 3;
	}

	void MarioKartFramework::OnGPKartFinishRace(u32* finishPosition, u32* halfDriverAmount, u32 playerID) {
		if (MissionHandler::isMissionMode) MissionHandler::OnGPKartFinishRace(finishPosition, halfDriverAmount, playerID);
	}

	void MarioKartFramework::generateCPUDataSettings(CPUDataSettings* buffer, u32 playerID, u32 totalPlayers, ModeManagerData* modeData)
	{
		static u32 chosenValue = 0;
		if (ISGAMEONLINE) {
			if (g_getCTModeVal != CTMode::ONLINE_COM || !allowCPURacersComm || isWatchRaceMode) {
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
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD || g_getCTModeVal == CTMode::ONLINE_COM)
			return EDriverID::DRIVER_MIIM;
		else
			return original;
	}

	void MarioKartFramework::BasePage_SetWifiRate(int slot, u32 vrAmount) {
		if (slot < 8) {
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

	void MarioKartFramework::OnRacePageGenGP(void* racePage) {
		if (MissionHandler::isMissionMode) {MissionHandler::OnRacePageGenGP(racePage); return;}
		// Original
		RacePageInitFunctions[RacePageInitID::NAME](racePage);
		if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
			for (auto it = onlineBotPlayerIDs.cbegin(); it != onlineBotPlayerIDs.cend(); it++) {
				FixedStringBase<u16, 0x20>* playerNames = getPlayerNames();
				const u16* realName = it->second.strData;
				strcpy16n(playerNames[it->first].strData, realName, playerNames[it->first].bufferSize * sizeof(u16));
			}
		}
		onlineBotPlayerIDs.clear();
		RacePageInitFunctions[RacePageInitID::TEXTURE](racePage);
		RacePageInitFunctions[RacePageInitID::EFFECT](racePage);
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0 && ((u32*)racePage)[0x26C/4] != 4) {
			RacePageInitFunctions[RacePageInitID::ITEM_SLOT](racePage);
			RacePageInitFunctions[RacePageInitID::RANK](racePage);
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
		}
		RacePageInitFunctions[RacePageInitID::WIPE](racePage);
		RacePageInitFunctions[RacePageInitID::TEXT](racePage);
		RacePageInitFunctions[RacePageInitID::CAPTION](racePage);
		// Custom
		RacePageInitFunctions[RacePageInitID::TIMER](racePage);
		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD) RacePageInitFunctions[RacePageInitID::POINT](racePage);
	}

	void MarioKartFramework::OnRacePageGenTA(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
	}

	void MarioKartFramework::OnRacePageGenBBT(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
	}
	
	void MarioKartFramework::OnRacePageGenCBT(void* racePage) {
		u8 unkFlag = ((u8*)((u32)racePage + 0x3100))[0xF8];
		if (unkFlag == 0) {
			SpeedometerController::Load();
			if (SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::initAutoAccelIcon();
			MusicCreditsController::Init();
		}
	}

	bool MarioKartFramework::userCanControlKart()
	{
		for (int i = 0; i < numberPlayersRace && i < 8; i++)
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

	void MarioKartFramework::storeItemProbability(u32 itemID, u16 prob)
	{
		if (startedItemSlot == masterPlayerID)
			itemProbabilities[itemID] = prob;
	}

	u16 MarioKartFramework::handleItemProbability(u16* dstArray, u32* csvPtr, u32 rowIndex, u32 blockedBitFlag)
	{
		bool isDecided = strcmp((char*)csvPtr + 0x50, "ItemSlotTable_Decided") == 0;
		if (MissionHandler::neeedsCustomItemHandling()) return MissionHandler::handleItemProbability(dstArray, csvPtr, rowIndex, blockedBitFlag);
		if (!isDecided && VersusHandler::neeedsCustomItemHandling()) return VersusHandler::handleItemProbability(dstArray, csvPtr, rowIndex, blockedBitFlag);
		u16 totalProb = 0;
		if (!bulletBillAllowed) blockedBitFlag |= (1 << EItemSlot::ITEM_KILLER);
		for (int i = 0; i < EItemSlot::ITEM_SIZE; i++) {
			if ((blockedBitFlag & (1 << i)) == 0 || nextForcedItem >= 0) {
				u16 currProb = (nextForcedItem >= 0) ? ((i == nextForcedItem) ? 200 : 0) : CsvParam_getDataInt(csvPtr, rowIndex, i);
				totalProb += currProb;
			}
			dstArray[i] = totalProb;
			storeItemProbability(i, totalProb);
		}
		nextForcedItem = -1;
		return totalProb;
	}

	u32 MarioKartFramework::pullRandomItemID()
	{
		if (SaveHandler::saveData.flags1.improvedRoulette || MissionHandler::isMissionMode) {
			int max = itemProbabilities[EItemSlot::ITEM_SIZE - 1] - 1;
			u32 randomChoice = Utils::Random(0, ((max < 0) ? 0 : max));
			int i = 0;
			for (; i < EItemSlot::ITEM_SIZE; i++) {
				if (itemProbabilities[i] > randomChoice) return i;
			}
			return EItemSlot::ITEM_KINOKO;
		}
		else
			return Utils::Random(0, EItemSlot::ITEM_SIZE - 1);
	}

	const char* MarioKartFramework::getOnlineItemTable(bool isAI) {
		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
			return isAI ? "ItemSlotTable_WiFi_AI" : "ItemSlotTable_CD";
		else if ((g_getCTModeVal == CTMode::ONLINE_CTWW || (currentRaceMode.type == 1 && (currentRaceMode.mode == 2 || currentRaceMode.mode == 0))) || 
				(g_getCTModeVal == CTMode::ONLINE_COM && allowCustomItemsComm))
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

	void MarioKartFramework::raceTimerOnFrame()
	{
		for (int i = 0; i < 8; i++)
		{
			if (trickInfos[i].trickCooldown > MarioKartTimer(0)) {
				--trickInfos[i].trickCooldown;
			}
		}
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
					startItemSlot(masterPlayerID, 0);
				} else {
					nextForcedItem = -1;
				}
			}
			if (UserCTHandler::skipConfig.useLeftToFinish && Controller::IsKeyPressed(Key::DPadLeft)) {
				forceFinishRace = true;
				// g_altGameModeIsRaceOver = 0x69; For online mode
			}
		}
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
		raceTimerOnFrame();
	}

	void MarioKartFramework::handleDecrementTimer(u32* timer)
	{
		MarioKartTimer clock(timer[1]);
		if (clock.GetFrames() != 0)
		{
			--clock;
			timer[1] = clock.GetFrames();
		}
		raceTimerOnFrame();
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

		if (objID == 0xCF) {
			GOBJEntry* entry = ((GOBJEntry***)object)[2][0];
			if ((entry->settings[6] & (1 << 1)) && !megaMushTimers[playerID] && kartCanAccident((u32*)vehicleReactObject))
				EGTHReact = EGHTREACT_SPEEDSPIN;
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
			return ret;
		}

		//NOXTRACE("dfffdsd", "oid: 0x%x, obt: %d, egth: 0x%x", objID, eObjectReactType, EGTHReact);

		if (MissionHandler::isMissionMode) return MissionHandler::onKartItemHitGeoObject(object, EGTHReact, eObjectReactType, vehicleReactObject, objCallFuncPtr, false);
		else return ((u32(*)(u32, u32, u32, u32))objCallFuncPtr)(object, EGTHReact, eObjectReactType, vehicleReactObject);
	}
	
	u32 MarioKartFramework::onItemHitGeoObject(u32 object, u32 EGTHReact, u32 eObjectReactType, u32 itemReactProxyObject, u32 objCallFuncPtr)
	{
		if (MissionHandler::isMissionMode) return MissionHandler::onKartItemHitGeoObject(object, EGTHReact, eObjectReactType, itemReactProxyObject, objCallFuncPtr, true);
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
		return !starTimer && !unknownTimer && !isBullet && !isHangingLakitu;
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

	void MarioKartFramework::onKartCountFrameGround(u32* vehicleMoveObj) {
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
		trickInfos[playerID].doFasterBoost = false;
		if (trickInfos[playerID].trickCooldown != MarioKartTimer(0)) return;
		KartFlags& flags = KartFlags::GetFromVehicle((u32)vehicleMoveObj);
		BoostFlags& boostFlags = BoostFlags::GetFromVehicle((u32)vehicleMoveObj);
		float* spinCounter = (float*)((u32)(vehicleMoveObj) + 0x1400 + 0xAC);

		// 0xF9C / 4 -> dash timer
		if (
			((SaveHandler::saveData.flags1.improvedTricks && (improvedTricksAllowed && MissionHandler::AllowImprovedTricks())) || 
			(improvedTricksForced || MissionHandler::ForceImprovedTricks())) && 
			vehicleMoveObj[0xF9C / 4] > 0 && !(flags.isInGliderPad | flags.isGliding | flags.wingTricked) && *spinCounter == 0.f
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
		if (g_getCTModeVal == CTMode::ONLINE_CTWW)
		{
			sv->vr = SaveHandler::saveData.ctVR;
		}
		else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
		{
			sv->vr = SaveHandler::saveData.cdVR;
		}
	}

	bool MarioKartFramework::OnNetSavePlayerSave(u32 newVR)
	{
		if (g_getCTModeVal == CTMode::ONLINE_CTWW)
		{
			SaveHandler::saveData.ctVR = newVR;
			return false;
		}
		else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
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
	void MarioKartFramework::DrawGameHook(u32 gameFramework) {
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
		if (((u32*)vehicleMove)[0x84 / 4] == masterPlayerID && g_getCTModeVal != CTMode::OFFLINE && g_getCTModeVal != CTMode::ONLINE_NOCTWW) graceDokanPeriod = MarioKartTimer(0, 15, 0);
		Vector3* chosenPosition = &route->points[randomChoice].position;
		vehicleMove_startDokanWarp(vehicleMove, randomChoice, chosenPosition, (Vector3*)(vehicleMove + 0xF44));
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

	static float g_prevRaceProgress = 0.f;
	void MarioKartFramework::OnLapRankCheckerDisconnectCheck(u32* laprankchecker, u32* kartinfo) {
		//u32 vehicle = ((u32**)kartInfo)[0][0];

		u32* currTimer = laprankchecker + 0x20/4;
		float* secondaryTimer = (float*)laprankchecker + 0x24/4;
		float biggestRaceProgress = *((float*)kartinfo + 0x18/4);
		float currRaceProgress = *((float*)kartinfo + 0x14/4);

#ifdef NO_DISCONNECT_ONLINE
		*currTimer = 0; *secondaryTimer = 0.f;
		return;
#endif

		if (++(*currTimer) > MarioKartTimer::ToFrames(0, 12, 0))
			kartinfo[0x24/4] |= 8;
		
		if (graceDokanPeriod != MarioKartTimer(0)) {--graceDokanPeriod; *currTimer = 0; *secondaryTimer = 0.f;}

		if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {

			if (((currRaceProgress + 0.00001f) < biggestRaceProgress) || ((currRaceProgress - g_prevRaceProgress) < 0.00001f))
				countdownDisconnectTimer+=2;

			if (countdownDisconnectTimer > (CourseManager::getCountdownCourseTime(CourseManager::lastLoadedCourseID) - MarioKartTimer::ToFrames(0, 10, 0))) {
				kartinfo[0x24/4] |= 8;
				*currTimer = MarioKartTimer::ToFrames(1,0,0);
				*secondaryTimer = MarioKartTimer::ToFrames(1,0,0);
			} else {
				*currTimer = 0;
				*secondaryTimer = 0.f;
			}
			g_prevRaceProgress = currRaceProgress;
		}
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
		isRacePaused = pauseOpen;
		SpeedometerController::OnPauseEvent(pauseOpen);
		if (!MissionHandler::isMissionMode && SaveHandler::saveData.flags1.autoacceleration) AutoAccelerationController::OnPauseEvent(pauseOpen);
	}

	void MarioKartFramework::OnRaceStartCountdown() {
		SpeedometerController::OnRaceStart();
		MusicCreditsController::OnRaceStart();
	}

	void MarioKartFramework::GetTerrainParameters(u32 vehicleMove, u8 kclID) {
		KCLTerrainInfo* currTerrainInfo = ((KCLTerrainInfo**)(vehicleMove + 0x10A8))[kclID];
		KCLTerrainInfo* vehicleTerrainInfo = (KCLTerrainInfo*)(vehicleMove + 0xD3C);
		u32 inkTimer = *(u32*)(vehicleMove + 0xFF8);
		if (inkTimer && ((g_getCTModeVal != CTMode::ONLINE_NOCTWW && g_getCTModeVal != CTMode::ONLINE_COM) || (g_getCTModeVal == CTMode::ONLINE_COM && allowCustomItemsComm))) {
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
	
	void MarioKartFramework::OnTireEffectGetCollision(u32 out[2], u32 vehicleMove) {
		u32 inkTimer = *(u32*)(vehicleMove + 0xFF8);
		u32 mainType = *(u32*)(vehicleMove + 0xD14);
		u32 subType = *(u32*)(vehicleMove + 0xD20);
		if (inkTimer > 30 && g_getCTModeVal != CTMode::ONLINE_NOCTWW) {
			out[0] = 5; // Off-road
			out[1] = 6; // Mud
		} else if (mainType == 8 && subType == 2) { // Loop kcl
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

	static void NAKED __attribute__((no_instrument_function)) ObjModelBaseChangeAnimationImpl(u32 objModelBase, int anim, u32 drawMdlFunc, float value) {
        __asm__ __volatile__(
            "PUSH            {R4, LR}\n"
            "MOV             R4, R0\n"
            "LDR             R0, [R4,#0xF4]\n"
            "BLX             R2\n"
            "LDR             R1, [R4,#0xF4]\n"
            "VLDR            S0, one\n"
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
            "VSTR            S0, [R0,#0x40]\n"
            "POP             {R4, PC}\n"
            
            "one: .float 1.0\n"
        );
	}

	void MarioKartFramework::ObjModelBaseChangeAnimation(u32 objModelBase, int anim, float value) {
		ObjModelBaseChangeAnimationImpl(objModelBase, anim, (u32)DrawMdl_changeMatAnimation, value);
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
			*(PluginMenu::GetRunningInstance()) += OnRaceNextMenuLoadCallback;
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
			onlineBotPlayerIDs.push_back(std::make_pair(playerID, FixedStringBase<u16,0x20>()));
			const u16* driverName = Language::MsbtHandler::GetText(1050 + CharacterManager::msbtOrder[CharacterManager::driverIDOrder[driverID]]);
			strcpy16(onlineBotPlayerIDs.back().second.strData, (u16*)u"*");
			strcpy16n(onlineBotPlayerIDs.back().second.strData + 1, driverName, 0x1F * sizeof(u16));
		}
	}

	void MarioKartFramework::OnCourseVotePagePreStep(u32 courseVotePage) {
		u32 state = ((u32*)courseVotePage)[0x2BC / 4];
		if (state == 6 && neededCPU) {
			if (imRoomHost) {
				if (neededCPU > neededCPUCurrentPlayers)
					AddBotPlayerOnline(neededCPU - neededCPUCurrentPlayers);
			}
			neededCPU = 0;
		}
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
	}

	void MarioKartFramework::startSizeChangeAnimation(int playerID, float targetSize, bool isGrowing) {
		resizeInfos[playerID].targetSize = targetSize;
		resizeInfos[playerID].fromSize = resizeInfos[playerID].lastKartScale;
		resizeInfos[playerID].animationStep = 1;
		resizeInfos[playerID].animationTimer = resizeInfos[playerID].eachAnimationFrames;
		resizeInfos[playerID].isGrowing = isGrowing;
	}

	bool MarioKartFramework::calculatePressReact(u32* vehicleMove1, u32* vehicleMove2, int someValueSP) {
		struct {u32 shrunkTimer; u32 pressTimer;} *timers1 = (decltype(timers1))(vehicleMove1 + 0x1000/4);
		struct {u32 shrunkTimer; u32 pressTimer;} *timers2 = (decltype(timers2))(vehicleMove2 + 0x1000/4);
		bool ret = false;

		int playerID1 = vehicleMove1[0x21];
		int playerID2 = vehicleMove2[0x21];

		KartFlags& flags1 = KartFlags::GetFromVehicle((u32)vehicleMove1);
		KartFlags& flags2 = KartFlags::GetFromVehicle((u32)vehicleMove2);

		void (*VehicleMove_StartPress)(u32* vehicleMove, bool dontStop, u32 timer) = (decltype(VehicleMove_StartPress))VehicleMove_StartPressAddr;
		void (*VehicleReact_ReactPressMapObj)(u32* vehicleReact, u32 stopTimer, u32 pressedTimer) = (decltype(VehicleReact_ReactPressMapObj))VehicleReact_ReactPressMapObjAddr;

		// vehicle1 presses vehicle2
		if (timers2->shrunkTimer > 0 && timers2->pressTimer <= 0 && !flags2.isWingOpened && someValueSP == 0) {
			if (timers1->pressTimer <= 0 && timers1->shrunkTimer <= 0) {
				VehicleMove_StartPress(vehicleMove2, true, 90);
			}
		}
		if (megaMushTimers[playerID1] > 0 && megaMushTimers[playerID2] <= 0 && timers2->pressTimer <= 0 && kartCanAccident(vehicleMove2) && !flags2.isWingOpened && someValueSP == 0) {
			VehicleReact_ReactPressMapObj(vehicleMove2, 1 * 60, 8 * 60); 
		}
		// vehicle2 presses vehicle1
		if (timers1->shrunkTimer > 0 && timers1->pressTimer <= 0 && !flags1.isWingOpened) {
			if (timers2->pressTimer <= 0 && timers2->shrunkTimer <= 0) {
				VehicleMove_StartPress(vehicleMove1, true, 90);
			}
		}
		if (megaMushTimers[playerID2] > 0 && megaMushTimers[playerID1] <= 0 && timers1->pressTimer <= 0 && kartCanAccident(vehicleMove1) && !flags1.isWingOpened) {
			VehicleReact_ReactPressMapObj(vehicleMove1, 1 * 60, 8 * 60); 
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

	void MarioKartFramework::OnSndActorKartCalcInner(u32* sndActorKart) {
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
			if (isStar) SndActorKArt_PlayDriverVoice((u32)sndActorKart, EVoiceType::STAR_START);
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
		bool isInLoop = vehicleIsInLoopKCL(((u32**)kartCamera)[0xD8/4][0]);
		if (isInLoop) {
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
		CharacterManager::OnThankYouLoad(true);
		u32 ret = ((u32(*)(u32))OnSimpleModelManagerHook.callCode)(own);
		CharacterManager::OnThankYouLoad(false);
		return ret;
	}

	bool MarioKartFramework::vehicleForceAntigravityCamera(u32 vehicle) {
		return vehicleIsInLoopKCL(vehicle);
	}

	bool MarioKartFramework::vehicleDisableSteepWallPushback(u32 vehicle) {
		return vehicleIsInLoopKCL(vehicle);
	}

	bool MarioKartFramework::vehicleDisableUpsideDownFloorUnstick(u32 vehicle, float* gravityAttenuator) {
		// gravityAttenuator = 0 -> No change in gravity, gravityAttenuator = 1 -> Zero gravity 
		bool isInLoop = vehicleIsInLoopKCL(vehicle);
		bool res = false;
		if (isInLoop) {
			constexpr float loopStickThreshold = 0.75f;
			float speedFactor = *(float*)(vehicle + 0x330 + 0xC00);
			res = speedFactor > loopStickThreshold;
		}
		return res;
	}

	void MarioKartFramework::OnKartGravityApply(u32 vehicle, Vector3& gravityForce) {
		bool isInLoop = vehicleIsInLoopKCL(vehicle);
		if (isInLoop) {
			float speedFactor = *(float*)(vehicle + 0x330 + 0xC00);
			constexpr float invertedGravityFactor = 0.95f;
			float gravityInterpolationFactor = std::min(invertedGravityFactor, speedFactor) / invertedGravityFactor;
			float currentGravityMagnitude = gravityForce.Magnitude();
			Vector3 kartDownVec = (*(Vector3*)(((u32*)vehicle)[0xC] + 0xC)) * -1.f;
			gravityForce.InvCerp(kartDownVec * currentGravityMagnitude, gravityInterpolationFactor);
		}
	}

	float MarioKartFramework::ApplyGndTurningSpeedPenaly(u32 vehicle, float speed, float penaltyFactor) {
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

	void MarioKartFramework::BaseResultBar_SetCTGPOnline(u32 resultBar) {
		VisualControl::GameVisualControl& barControl = *(VisualControl::GameVisualControl*)resultBar;
		barControl.GetAnimationFamily(2)->SetAnimation(0, 7.f);
	}

	void MarioKartFramework::UpdateResultBars(u32 racePage) {
		if (MissionHandler::isMissionMode) {
			MissionHandler::updateResultBars(racePage);
			return;
		}
		CRaceInfo* raceInfo = getRaceInfo(true);
		onlinePlayersStarGrade[masterPlayerID] = Net::myGrade;
		for (int i = 0; i < raceInfo->playerAmount; i++) {
			if (onlinePlayersStarGrade[i] != StarGrade::INVALID) {
				if ((g_getCTModeVal == CTMode::ONLINE_NOCTWW || g_getCTModeVal == CTMode::ONLINE_COM) && i != masterPlayerID) BaseResultBar_SetCTGPOnline(resultBarArray[i]);
				if (onlinePlayersStarGrade[i] >= StarGrade::CUSTOM_PLAYER && onlinePlayersStarGrade[i] <= StarGrade::CUSTOM_RAINBOW) {
					u32 temp = (u32)onlinePlayersStarGrade[i];
					BaseResultBar_SetGrade(resultBarArray[i], &temp);
				}
			}
		}
	}

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
		if (playerID == masterPlayerID) {
			data.info.ctgpStarGrade = (u8)Net::myGrade;
		}
	}

	void  MarioKartFramework::OnRecvCustomKartData(int playerID, CustomCTGP7KartData& data) {
		if (onlinePlayersStarGrade[playerID] == StarGrade::INVALID) {
			onlinePlayersStarGrade[playerID] = (StarGrade)data.info.ctgpStarGrade;
		}
		ItemHandler::MegaMushHandler::CalcNetRecv(playerID, data.megaMushTimer << 2);
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
		if (g_setCTModeVal == CTMode::MAXVAL) return;
		g_getCTModeVal = g_setCTModeVal;
		g_setCTModeVal = CTMode::MAXVAL;

		switch (g_getCTModeVal)
		{
		case OFFLINE:
			CourseManager::setCustomTracksAllowed(true);
			CourseManager::setOriginalTracksAllowed(true);
			MarioKartFramework::improvedTricksAllowed = true;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = true;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::nexNetworkInstance = 0;
			MarioKartFramework::automaticDelayDriftAllowed = true;
			ItemHandler::allowFasterItemDisappear = true;
			MenuPageHandler::MenuSingleCourseBasePage::blockedCourses.clear();
			isCTWW = 0;
			isAltGameMode = 0;
			break;
		case ONLINE_NOCTWW:
			CourseManager::setCustomTracksAllowed(false);
			CourseManager::setOriginalTracksAllowed(true);
			MarioKartFramework::improvedTricksAllowed = false;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = false;
			MarioKartFramework::brakeDriftForced = false;
			MarioKartFramework::changeNumberRounds(4);
			ccsettings[1].enabled = false;
			ccselector_apply(ccselectorentry);
			MarioKartFramework::isBackCamBlockedComm = false;
			MarioKartFramework::isWarnItemBlockedComm = false;
			MarioKartFramework::allowCPURacersComm = false;
			CourseManager::isRandomTracksForcedComm = false;
			MarioKartFramework::automaticDelayDriftAllowed = false;
			ItemHandler::allowFasterItemDisappear = false;
			MenuPageHandler::MenuSingleCourseBasePage::blockedCourses.clear();
			isCTWW = 0;
			isAltGameMode = 0;
			break;
		case ONLINE_COM:
			isCTWW = 0;
			isAltGameMode = 0;
			MarioKartFramework::improvedTricksAllowed = false;
			MarioKartFramework::improvedTricksForced = false;
			MarioKartFramework::brakeDriftAllowed = false;
			MarioKartFramework::brakeDriftForced = false;
			ItemHandler::allowFasterItemDisappear = false;
			MenuPageHandler::MenuSingleCourseBasePage::blockedCourses.clear();
			break;
		case ONLINE_CTWW:
			CourseManager::setCustomTracksAllowed(true);
			CourseManager::setOriginalTracksAllowed(false);
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
			break;
		case ONLINE_CTWW_CD:
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
			break;
		case MAXVAL:
		default:
			break;
		}
	}
}

CTMode g_setCTModeVal = CTMode::MAXVAL;
CTMode g_getCTModeVal = CTMode::MAXVAL;
void g_updateCTMode() {
	CTRPluginFramework::g_updateCTModeImpl();
}