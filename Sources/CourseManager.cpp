/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CourseManager.cpp
Open source lines: 1014/1151 (88.10%)
*****************************************************/

#include <locale>
#include "CourseManager.hpp"
#include "MarioKartFramework.hpp"
#include "Lang.hpp"
#include "3ds.h"
#include "csvc.h"
#include "Sound.hpp"
#include "entrystructs.hpp"
#include "cheats.hpp"
#include "LED_Control.hpp"
#include "MusicSlotMngr.hpp"
#include "CharacterManager.hpp"
#include "ExtraResource.hpp"
#include "VersusHandler.hpp"
#include "str16utils.hpp"
#include "CrashReport.hpp"
#include "MissionHandler.hpp"
#include "MarioKartTimer.hpp"
#include "SaveHandler.hpp"
#include "UserCTHandler.hpp"
#include "foolsday.hpp"
#include "CustomTextEntries.hpp"
#include "MenuPage.hpp"

extern u32 isCTWW;
extern u32 isAltGameMode;

namespace CTRPluginFramework
{
	u32 CourseManager::lastLoadedCourseID = 0;
	bool CourseManager::getBGMFromNormalLap = true;
	u32 CourseManager::lastPlayedCourses[4] = { INVALIDTRACK, INVALIDTRACK, INVALIDTRACK, INVALIDTRACK };
	u8 CourseManager::customTracksAllowedFlag = (Utils::Random() | 1) & ~0x2;
	u8 CourseManager::originalTracksAllowedFlag = (Utils::Random() | 1) & ~0x2;
	void (*CourseManager::BaseMenuButtonControl_setTex)(u32 buttonObject, u32 texPtrn, u32 texID) = NULL;
	void (*CourseManager::MenuSingle_CupBase_buttonHandler_OK)(u32 menuObject, u32 cupID) = NULL;
	u32  (*CourseManager::Sequence_GetGPCourseNameID)(u32 cup, u32 track) = NULL;
	bool CourseManager::hadToDisableSpeed = false;
	bool CourseManager::isRandomTracksForcedComm = false;
	int CourseManager::multiCupButtonsIDs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	u32* CourseManager::finalGlobalCupTranslateTable = nullptr;
	u32 CourseManager::finalGlobalCupTranslateTableSize = 0;

	void CourseManager::resetGhost(MenuEntry* entry)
	{
		Keyboard kbd("dummy");
		Keyboard kbd2("dummy");
		std::string empty = "\n";
		u32 tracksel = INVALIDTRACK;
		int initialButton = MenuPageHandler::MenuSingleCupBasePage::startingButtonID;
		int selectedButton = MenuPageHandler::MenuSingleCupGPPage::selectedCupIcon;
		if (selectedButton < 0) selectedButton = 0;
		int opt;
		kbd.Populate({ NAME("select"), NAME("exit") });
		kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		kbd2.Populate({ Language::MsbtHandler::GetString(2003), Language::MsbtHandler::GetString(2004) });
		do
		{
			kbd.GetMessage() = NAME("resghost") + "\n\n" + NAME("ghostseltrack") + "\n";
			if (tracksel != INVALIDTRACK) {
				std::string out = "";
				getCourseText(out, tracksel, true); 
				kbd.GetMessage() += out + "\n\n" << Color::LimeGreen << NOTE("ghostcantrecov") + ResetColor() + "\n" + NAME("reboot_req");
			}
			opt = kbd.Open();
			if (opt == 0) {
				tracksel = VersusHandler::OpenCupCourseKeyboard(&initialButton, &selectedButton, true, empty);
				if (tracksel == INVALIDTRACK) continue;
				std::string out = "";
				getCourseText(out, tracksel, true);
				kbd2.GetMessage() = NAME("resghost") + "\n\n" + NAME("ghostseltrack") + "\n" + out + "\n\n" << Color::Yellow << NAME("warning") + "\n" + NAME("ghostcantrecov");
				int opt2 = kbd2.Open();
				if (opt2 == 0) {
					resetCourseGhost(tracksel);
				}
				else {
					tracksel = INVALIDTRACK;
				}
			}
		} while (opt == 0);

	}

	void CourseManager::resetCourseGhost(u32 courseID)
	{
		std::string filename = Utils::Format("/CTGP-7/savefs/game/replay/replay%02u.dat", courseID);
		File::Remove(filename);
	}

	void CourseManager::setCustomTracksAllowed(bool mode)
	{
		do { customTracksAllowedFlag = rol<u8>(customTracksAllowedFlag, 1); } while ((customTracksAllowedFlag & 1) != mode);
	}

	bool CourseManager::getCustomTracksAllowed()
	{
		return (customTracksAllowedFlag & 1);
	}

	void CourseManager::setOriginalTracksAllowed(bool mode)
	{
		do { originalTracksAllowedFlag = rol<u8>(originalTracksAllowedFlag, 1); } while ((originalTracksAllowedFlag & 1) != mode);
	}

	bool CourseManager::getOriginalTracksAllowed()
	{
		return (originalTracksAllowedFlag & 1);
	}

	u32 CourseManager::getCupGlobalIDName(u32 cupID) {
		if (cupID == USERCUPID)
			return UserCTHandler::GetCupTextID();
		else
			return CustomTextEntries::customCupStart + cupID;
	}

	u32 CourseManager::getSingleCourseGlobalIDName(u32 courseID) {
		if (courseID == USERTRACKID)
			return UserCTHandler::GetCourseTextID();
		else
			return CustomTextEntries::customTrackStart + courseID;
	}

	u32 CourseManager::getCourseGlobalIDName(u32 cup, int track, bool forceOrig) {
		if (track < 4 && track >= 0) {
			if (cup == USERCUPID) return UserCTHandler::GetCurrentCupText(track);
			else if (!forceOrig && MissionHandler::isMissionMode) return MissionHandler::OnGetCupText(FROMBUTTONTOCUP(cup), track);
			else if (ISGAMEONLINE && SaveHandler::saveData.flags1.isAlphabeticalEnabled && !forceOrig) return CustomTextEntries::customTrackStart + alphabeticalGlobalCupData[FROMBUTTONTOCUP(cup)][track];
			else return CustomTextEntries::customTrackStart + globalCupData[FROMBUTTONTOCUP(cup)][track];
		}
		return 0;
	}


	void CourseManager::updatelastPlayedCourseArray(u32 course)
	{
		if (course == lastPlayedCourses[0]) return;
		lastPlayedCourses[3] = lastPlayedCourses[2];
		lastPlayedCourses[2] = lastPlayedCourses[1];
		lastPlayedCourses[1] = lastPlayedCourses[0];
		lastPlayedCourses[0] = course;
	}
	
	void CourseManager::getRandomCourseOnline(u32* result, bool isRace)
	{
		bool repeat;
		u32 chosen;
		do {
			repeat = false;
			if (!isRace) chosen = Utils::Random(BATTLETRACKLOWER, BATTLETRACKUPPER);
			else {
				if (getCustomTracksAllowed()) {
					if (getOriginalTracksAllowed()) {
						chosen = Utils::Random(ORIGINALTRACKLOWER, CUSTOMTRACKUPPER - ((CUSTOMTRACKLOWER - ORIGINALTRACKUPPER) - 1));
						if (chosen > ORIGINALTRACKUPPER) chosen += (CUSTOMTRACKLOWER - ORIGINALTRACKUPPER) - 1;
					}
					else {
						chosen = Utils::Random(CUSTOMTRACKLOWER, CUSTOMTRACKUPPER);
					}
				}
				else {
					chosen = Utils::Random(ORIGINALTRACKLOWER, ORIGINALTRACKUPPER);
				}
			}
			for (int i = 0; i < 4; i++) {
				if (chosen == lastPlayedCourses[i]) {
					repeat = true;
					break;
				}
			}
		} while (repeat);
		*result = chosen;
	}

	u32 CourseManager::getRandomCourseDemo(bool isRace)
	{
		static bool isCT = true;
		static u32 lastChosen[2] = { 0xFFFFFFFF, 0xFFFFFFFF };
		u32 chosen = INVALIDTRACK;
		if (isCT) {
			do {
				chosen = globalCustomTracksWithReplay[Utils::Random(0, (sizeof(globalCustomTracksWithReplay) / sizeof(u32)) - 1)];
			} while (chosen == lastChosen[isCT]);
		}
		else {
			do {
				chosen = Utils::Random(ORIGINALTRACKLOWER, ORIGINALTRACKUPPER);
			} while (chosen == lastChosen[isCT]);
		}
		lastChosen[isCT] = chosen;
		isCT = !isCT;
		return chosen;
	}

	u32* CourseManager::getGPCourseID(u32* ptr, u32 cup, int track) {
		if (cup == VERSUSCUPID) {
			if (VersusHandler::IsVersusMode)
				*ptr = VersusHandler::versusCupTable[VersusHandler::currentVersusCupTableEntry];
			else
				*ptr = INVALIDTRACK;
		}
		else if (cup == MISSIONCUPID) {
			if (MissionHandler::isMissionMode)
				*ptr = MissionHandler::missionParam->MissionFlags->courseID;
			else
				*ptr = INVALIDTRACK;
		} else if (cup == USERCUPID) {
			if (MarioKartFramework::currentRaceMode.type == 0 && MarioKartFramework::currentRaceMode.mode == 1) { // Time trials
				UserCTHandler::TimeTrialsSetTrack(track);
			}
			*ptr = USERTRACKID;
		} else {
			if (track >= 0 && track < 4) {
				if (ISGAMEONLINE && SaveHandler::saveData.flags1.isAlphabeticalEnabled) *ptr = alphabeticalGlobalCupData[FROMBUTTONTOCUP(cup)][track];
				else *ptr = globalCupData[FROMBUTTONTOCUP(cup)][track];
			}
			else {
				*ptr = INVALIDTRACK;
			}
		}
		return ptr;
	}

	u32 CourseManager::getCourseName(u32* courseID) {
		if (*courseID == USERTRACKID) {
			return UserCTHandler::GetCourseInternalName();
		}
		lastLoadedCourseID = *courseID;
		return (u32)&globalNameData.entries[*courseID];
	}

	int CourseManager::getCourseIDFromName(const std::string& str)
	{
		if (str.empty()) return -1;
		for (int i = 0; i < sizeof(globalNameData.entries) / sizeof(globalNameData.entries[0]); i++) {
			if (str.compare(globalNameData.entries[i].name) == 0) return i;
		}
		return -1;
	}

	const CTNameData::CTNameEntry* CourseManager::getCourseData(const std::string& str)
	{
		for (int i = 0; i < sizeof(globalNameData.entries) / sizeof(globalNameData.entries[0]); i++) {
			if (str.compare(globalNameData.entries[i].name) == 0) return &globalNameData.entries[i];
		}
		return nullptr;
	}

	const CTNameData::CTNameEntry* CourseManager::getCourseData(u32 courseID)
	{
		if (courseID < sizeof(globalNameData.entries) / sizeof(globalNameData.entries[0]))
			return &globalNameData.entries[courseID];
		else
			return nullptr;
	}

	u32 CourseManager::getOriginalCourseID(u32 courseID) {
		if (courseID == 0x26 && !hadToDisableSpeed) { //Credits
			hadToDisableSpeed = true;
			ccSelOnlineEntry->setOnlineMode(true);
			ccselectorentry->SetArg(&ccsettings[1]);
			ccsettings[1].enabled = false;
			ccselector_apply(ccselectorentry);
		}
		if (CrashReport::stateID != CrashReport::StateID::STATE_TROPHY)
			CrashReport::stateID = CrashReport::StateID::STATE_RACE;
		MarioKartFramework::forceFinishRace = false;
		if (courseID != USERTRACKID) {
			updatelastPlayedCourseArray(courseID);
			return globalNameData.entries[courseID].originalSlot;
		} else {
			return UserCTHandler::GetCurrentCourseOrigSlot();
		}
	}

	u32 CourseManager::getCourseLapAmount(u32 courseID)	{
		// If we are in mission mode, return the amount of laps from the mission config.
		if (MissionHandler::isMissionMode) return MissionHandler::getLapAmount();
		else {
			// If we are online, in ctww, and in countdown mode
			if (ISGAMEONLINE && isCTWW && isAltGameMode) {
				// Return 69 laps (there is room to store 70 laps afaik)
				return 69;
			}
			else {
				if (courseID == USERTRACKID) {
					return UserCTHandler::GetCurrentCourseLapAmount();
				}
				// Else, look at the custom defined course information to get the lap amount.
				return globalNameData.entries[courseID].lapAmount;
			}
		}
	}

	u32 CourseManager::getCountdownCourseTime(u32 courseID){
		if (courseID == USERTRACKID) return MarioKartTimer::ToFrames(2, 0);
	    return globalNameData.entries[courseID].countdownTimerValue;
	}

	u32 CourseManager::getLatestLoadedCourseID() {
		return lastLoadedCourseID;
	}

	void CourseManager::findCupCourseID(u32 found[2], u32 courseID) {
		if (courseID == USERTRACKID) {
			found[0] = USERCUPID;
			found[1] = UserCTHandler::GetCurrentTrackInCup();
			return;
		}
		if (courseID > 0x1F && courseID < 0x26) { //Battle
			found[0] = 0xFFFFFFFF;
			switch (courseID)
			{
			case 0x20:
				found[1] = 5;
				break;
			case 0x21:
				found[1] = 3;
				break;
			case 0x22:
				found[1] = 4;
				break;
			case 0x23:
				found[1] = 2;
				break;
			case 0x24:
				found[1] = 1;
				break;
			case 0x25:
				found[1] = 0;
				break;
			default:
				break;
			}
			return;
		}
		for (int i = 0; i < MAXCUPS; i++) {
			for (int j = 0; j < 4; j++) {
				if (ISGAMEONLINE && SaveHandler::saveData.flags1.isAlphabeticalEnabled) {
					if (alphabeticalGlobalCupData[i][j] == courseID) {
						found[0] = i;
						found[1] = j;
						return;
					}
				}
				else {
					if (globalCupData[i][j] == courseID) {
						found[0] = i;
						found[1] = j;
						return;
					}
				}
			}
		}
		found[0] = 0;
		found[1] = 0;
		return;
	}
	u32	CourseManager::getCupFromCourseID(u32 courseID) {
		u32 data[2];
		findCupCourseID(data, courseID);
		return (courseID == USERTRACKID) ? data[0] : FROMCUPTOBUTTON(data[0]);
	}
	u32	CourseManager::getTrackFromCourseID(u32 courseID) {
		u32 data[2];
		findCupCourseID(data, courseID);
		return data[1];
	}
	u32 CourseManager::getTrophyName(u32 cupID) {
		if (cupID == USERCUPID) return (u32)&generalThrophy;
		if (hadToDisableSpeed) {
			hadToDisableSpeed = false;
			ccSelOnlineEntry->setOnlineMode(false);
			ccselectorentry->SetArg(&ccsettings[0]);
			ccselector_apply(ccselectorentry);
		}
		if (VersusHandler::IsVersusMode || cupID == 8 || cupID == 9) return (u32)&generalThrophy;
		else return (u32)&globalTrophyData.entries[FROMBUTTONTOCUP(cupID)];
	}

	bool CourseManager::isRainbowTrack(u32 trackID) {
		switch (trackID)
		{
		case 0xD:
		case 0x1F:
		case 0x40:
		case 0x46:
		case 0x47:
		case 0x49:
		case 0x61:
		case 0x72:
		case 0x78:
			return true;
		default:
			return false;
		}
	}

	u32 CourseManager::getTrackChannelMode() {
#ifdef NOMUSIC
		return 0;
#endif
		if (MissionHandler::isMissionMode) {
			u32 ret = 0;
			if (MissionHandler::LoadCustomMusicData(&ret, 2, false)) {
				return ret;
			}
		}
		u32 courseID = lastLoadedCourseID;
		if (UserCTHandler::IsUsingCustomCup()) courseID = UserCTHandler::GetCurrentCourseOrigSlot();
		auto it = MusicSlotMngr::entryMap.find(courseID);
		if (it != MusicSlotMngr::entryMap.end()) {
			switch ((*it).second.mode)
			{
			case MusicSlotMngr::BgmMode::SINGLE:
				return 0;
			case MusicSlotMngr::BgmMode::MULTI_WATER:
				return 0xC;
			case MusicSlotMngr::BgmMode::MULTI_AREA:
				return 0xD;
			default:
				break;
			}
		}
		return getOriginalCourseID(courseID);
	}

	u8 CourseManager::getCustomBeatBPM(u8 orig)
	{
#ifdef NOMUSIC
		return 0;
#endif
		if (MissionHandler::isMissionMode) {
			u32 ret = 0;
			if (MissionHandler::LoadCustomMusicData(&ret, 0, getBGMFromNormalLap)) {
				return (u8)ret;
			}
		}
		u32 courseID = lastLoadedCourseID;
		if (UserCTHandler::IsUsingCustomCup()) courseID = UserCTHandler::GetCurrentCourseOrigSlot();
		auto it = MusicSlotMngr::entryMap.find(courseID);
		if (it != MusicSlotMngr::entryMap.end()) {
			if (getBGMFromNormalLap) {
				return (*it).second.normalBeatBPM;
			}
			else {
				return (*it).second.fastBeatBPM;
			}
		}
		return orig;
	}

	u32 CourseManager::getCustomBeatOffset(u32 orig)
	{
#ifdef NOMUSIC
		return 0;
#endif
		if (MissionHandler::isMissionMode) {
			u32 ret = 0;
			if (MissionHandler::LoadCustomMusicData(&ret, 1, getBGMFromNormalLap)) {
				return ret;
			}
		}
		u32 courseID = lastLoadedCourseID;
		if (UserCTHandler::IsUsingCustomCup()) courseID = UserCTHandler::GetCurrentCourseOrigSlot();
		auto it = MusicSlotMngr::entryMap.find(courseID);
		if (it != MusicSlotMngr::entryMap.end()) {
			if (getBGMFromNormalLap) {
				return (*it).second.normalBeatOffset;
			}
			else {
				return (*it).second.fastBeatOffset;
			}
		}
		return orig;
	}

	void CourseManager::fixTranslatedMsbtEntries() {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 4; j++) {
				u32 origId = Sequence_GetGPCourseNameID(i, j | 0x80000000);
				const u16* origText = Language::MsbtHandler::GetText(origId);
				u32 newID = getCourseGlobalIDName(i, j, true);
				Language::MsbtHandler::SetText(newID, origText);
			}
			u32 id = 1500 + i;
			const u16* origText = Language::MsbtHandler::GetText(id);
			Language::MsbtHandler::SetText(CustomTextEntries::customCupStart + i, origText);
		}
		const u32 battleTranslate[] = { 0x21, 0x22, 0x20, 0x25, 0x24, 0x23 };
		for (int i = 0; i < 6; i++) {
			const u16* origText = Language::MsbtHandler::GetText(1800 + i);
			Language::MsbtHandler::SetText(CustomTextEntries::customTrackStart + battleTranslate[i], origText);
		}
		UserCTHandler::FixNames();
	}
	void CourseManager::replaceConsoleText(std::string& subject, const std::string search, const std::string replace, bool appendEnd) {
		size_t pos = 0, firstpos = std::string::npos;
		bool found = false;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			if (firstpos == std::string::npos) firstpos = pos;
			subject.erase(pos, search.length());
			found = true;
		}
		if (found && appendEnd) subject.append(replace);
		else if (found && !appendEnd) subject.insert(firstpos, replace);
	}
	void CourseManager::sortTracksAlphabetically() {
		std::vector<std::string> v;
		for (int i = CustomTextEntries::customTrackStart + 0x2A; i < CustomTextEntries::customTrackStart + MAXTRACKS; i++) {
			if (i > 710031 && i < 710042) continue;
			std::string s = Language::MsbtHandler::GetString(i);
			if (!s.empty()) {
				replaceConsoleText(s, "\uE020\uE021", "SFC", true);
				replaceConsoleText(s, "\uE031\uE032", "SNES", true);
				replaceConsoleText(s, "\uE024\uE025", "GBA", true);
				replaceConsoleText(s, "\uE022\uE023", "N64", true);
				replaceConsoleText(s, "\uE035", "64", true);
				replaceConsoleText(s, "\uE026\uE027", "GCN", true);
				replaceConsoleText(s, "\uE034", "GC", true);
				replaceConsoleText(s, "\uE033", "DS", true);
				replaceConsoleText(s, "\uE067", "Wii", true);
				TextFileParser::Trim(s);
				s.append(std::to_string(i));
				v.push_back(s);
			}
			else {
				std::string s2("\uE83A" + std::to_string(i));
				v.push_back(s2);
			}

		}
		std::sort(v.begin(), v.end());//, std::locale(""));
		memcpy(alphabeticalGlobalCupData, globalCupData, 32 * sizeof(u32));
		int translateStart = 0x4;
		int maxColumns = (sizeof(globalCupTranslateTable) / sizeof(u32)) / 2;
		bool isTop = true;
		int track = 0; 
		for (auto it = v.begin(); it != v.end(); it++) {
			int id = std::stoi((*it).substr((*it).length() - 6)) - CustomTextEntries::customTrackStart;
			alphabeticalGlobalCupData[FROMBUTTONTOCUP(globalCupTranslateTable[translateStart])][track++] = id;
			if (track >= 4) {
				track = 0;
				if (isTop) {
					isTop = false;
					translateStart += maxColumns;
				}
				else {
					isTop = true;
					translateStart -= (maxColumns - 1);
				}
			}
			if ((isTop && translateStart >= maxColumns) || (!isTop && translateStart >= MAXCUPS)) break;
		}
		return;
	}


	void CourseManager::getCourseText(std::string& out, u32 courseID, bool replaceSymbols)
	{
		std::string s = Language::MsbtHandler::GetString(CustomTextEntries::customTrackStart + courseID);
		if (replaceSymbols) {
			replaceConsoleText(s, "\uE020\uE021", "SFC", false);
			replaceConsoleText(s, "\uE031\uE032", "SNES", false);
			replaceConsoleText(s, "\uE024\uE025", "GBA", false);
			replaceConsoleText(s, "\uE022\uE023", "N64", false);
			replaceConsoleText(s, "\uE035", "64", false);
			replaceConsoleText(s, "\uE026\uE027", "GCN", false);
			replaceConsoleText(s, "\uE034", "GC", false);
			replaceConsoleText(s, "\uE033", "DS", false);
			replaceConsoleText(s, "\uE067", "Wii", false);
		}
		out.append(s);
	}

	void CourseManager::getCupText(std::string& out, u32 cupID)
	{
		std::string s = Language::MsbtHandler::GetString(CustomTextEntries::customCupStart + cupID);
		out.append(s);
	}

	u32	 alphabeticalGlobalCupData[MAXCUPS][4] = { 0 };

	const u32 globalCustomTracksWithReplay[9] = {
		0x2A,
		0x2D,
		0x35,
		0x41,
		0x43,
		0x44,
		0x46,
		0x49,
		0x69
	};

	const u32  globalCupData[MAXCUPS][4] = {
		{	//0x00 Mushroom Cup
			0x04,
			0x03,
			0x02,
			0x05
		},{ //0x01 Flower Cup
			0x08,
			0x00,
			0x0F,
			0x01
		},{ //0x02 Star Cup
			0x0C,
			0x0E,
			0x06,
			0x09
		},{ //0x03 Special Cup
			0x07,
			0x0A,
			0x0B,
			0x0D
		},{ //0x04 Shell Cup
			0x1A,
			0x1D,
			0x13,
			0x14
		},{ //0x05 Banana Cup
			0x1C,
			0x1E,
			0x10,
			0x17
		},{ //0x06 Leaf Cup
			0x1B,
			0x16,
			0x19,
			0x12
		},{ //0x07 Lighting Cup
			0x11,
			0x18,
			0x15,
			0x1F
		},{ //0x0A Bell Cup
			0x2A,
			0x3B,
			0x2C,
			0x2D
		},{ //0x0B Acorn Cup
			0x2E,
			0x44,
			0x2B,
			0x31
		},{ //0x0C Cloud Cup
			0x32,
			0x36,
			0x34,
			0x35
		},{ //0x0D Boo Cup
			0x37,
			0x3E,
			0x39,
			0x41
		},{ //0x0E Spring Cup
			0x43,
			0x3A,
			0x42,
			0x45
		},{ //0x0F Egg Cup
			0x3F,
			0x2F,
			0x30,
			0x33
		},{ //0x10 Bullet Cup
			0x38,
			0x59,
			0x3D,
			0x40
		},{ //0x11 Rainbow Cup
			0x46,
			0x47,
			0x72,
			0x61
		},{ //0x12 Blooper Cup
			0x4A,
			0x4B,
			0x4C,
			0x4D
		},{ //0x13 Feather Cup
			0x4E,
			0x4F,
			0x50,
			0x51
		},{ //0x14 Fireball Cup
			0x52,
			0x53,
			0x54,
			0x55
		},{ //0x15 Bob-omb Cup
			0x56,
			0x57,
			0x58,
			0x48
		},{ //0x16 Cherry Cup
			0x5A,
			0x5B,
			0x5C,
			0x5D
		},{ //0x17 Coin Cup
			0x5E,
			0x5F,
			0x60,
			0x49
		},{ //0x18 Pickaxe Cup
			0x62,
			0x63,
			0x64,
			0x65
		},{ //0x19 Mega Cup
			0x66,
			0x67,
			0x68,
			0x69
		},{ //0x1A Propeller Cup
			0x6A,
			0x6B,
			0x6C,
			0x6D
		},{ //0x1B POW Cup
			0x6E,
			0x6F,
			0x70,
			0x71
		},{ //0x1C Rock Cup
			0x3C,
			0x73,
			0x74,
			0x75,
		},{ //0x1D Moon Cup
			0x76,
			0x77,
			0x78,
			0x79
		}
	};
	const CTNameData globalNameData = {
		{0, 0, (u32)CTNameFunc::nullsub_func},
		{
			{&globalNameData.f, "Gctr_MarioCircuit", 0x00, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x00
			{&globalNameData.f, "Gctr_RallyCourse", 0x01, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x01
			{&globalNameData.f, "Gctr_MarineRoad", 0x02, 0x03, MarioKartTimer::ToFrames(1, 50)},			//0x02
			{&globalNameData.f, "Gctr_GlideLake", 0x03, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x03
			{&globalNameData.f, "Gctr_ToadCircuit", 0x04, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x04
			{&globalNameData.f, "Gctr_SandTown", 0x05, 0x03, MarioKartTimer::ToFrames(2, 30)},				//0x05
			{&globalNameData.f, "Gctr_AdvancedCircuit", 0x06, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x06
			{&globalNameData.f, "Gctr_DKJungle", 0x07, 0x03, MarioKartTimer::ToFrames(2, 30)},				//0x07
			{&globalNameData.f, "Gctr_WuhuIsland1", 0x08, 0x01, MarioKartTimer::ToFrames(1, 50)},			//0x08
			{&globalNameData.f, "Gctr_WuhuIsland2", 0x09, 0x01, MarioKartTimer::ToFrames(0, 40)},			//0x09
			{&globalNameData.f, "Gctr_IceSlider", 0x0A, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x0A
			{&globalNameData.f, "Gctr_BowserCastle", 0x0B, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x0B
			{&globalNameData.f, "Gctr_UnderGround", 0x0C, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x0C
			{&globalNameData.f, "Gctr_RainbowRoad", 0x0D, 0x01, MarioKartTimer::ToFrames(1, 45)},			//0x0D
			{&globalNameData.f, "Gctr_WarioShip", 0x0E, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x0E
			{&globalNameData.f, "Gctr_MusicPark", 0x0F, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x0F
			{&globalNameData.f, "Gwii_CoconutMall", 0x10, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x10
			{&globalNameData.f, "Gwii_KoopaCape", 0x11, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x11
			{&globalNameData.f, "Gwii_MapleTreeway", 0x12, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x12
			{&globalNameData.f, "Gwii_MushroomGorge", 0x13, 0x03, MarioKartTimer::ToFrames(1, 50)},			//0x13
			{&globalNameData.f, "Gds_LuigisMansion", 0x14, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x14
			{&globalNameData.f, "Gds_AirshipFortress", 0x15, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x15
			{&globalNameData.f, "Gds_DKPass", 0x16, 0x03, MarioKartTimer::ToFrames(2, 30)},					//0x16
			{&globalNameData.f, "Gds_WaluigiPinball", 0x17, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x17
			{&globalNameData.f, "Ggc_DinoDinoJungle", 0x18, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x18
			{&globalNameData.f, "Ggc_DaisyCruiser", 0x19, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x19
			{&globalNameData.f, "Gn64_LuigiCircuit", 0x1A, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x1A
			{&globalNameData.f, "Gn64_KalimariDesert", 0x1B, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x1B
			{&globalNameData.f, "Gn64_KoopaTroopaBeach", 0x1C, 0x03, MarioKartTimer::ToFrames(2, 0)},		//0x1C
			{&globalNameData.f, "Gagb_BowserCastle1", 0x1D, 0x03, MarioKartTimer::ToFrames(1, 50)},			//0x1D
			{&globalNameData.f, "Gsfc_MarioCircuit2", 0x1E, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x1E
			{&globalNameData.f, "Gsfc_RainbowRoad", 0x1F, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x1F
			{&globalNameData.f, "Bctr_WuhuIsland3", 0x20, 0x03, 0},								//0x20
			{&globalNameData.f, "Bctr_HoneyStage", 0x21, 0x03, 0},								//0x21
			{&globalNameData.f, "Bctr_IceRink", 0x22, 0x03, 0},									//0x22
			{&globalNameData.f, "Bds_PalmShore", 0x23, 0x03, 0},								//0x23
			{&globalNameData.f, "Bn64_BigDonut", 0x24, 0x03, 0},								//0x24
			{&globalNameData.f, "Bagb_BattleCourse1", 0x25, 0x03, 0},							//0x25
			{&globalNameData.f, "Gctr_WinningRun", 0x26, 0x03, 0},											//0x26
			{&globalNameData.f, "", 0x27, 0x03, 0},															//0x27
			{&globalNameData.f, "", 0x28, 0x03, 0},															//0x28
			{&globalNameData.f, "", 0x29, 0x03, 0},															//0x29 (INVALID_COURSE)

			{&globalNameData.f, "Ctgp_ConcTown", 0x12, 0x03, MarioKartTimer::ToFrames(2,30)},					//0x2A
			{&globalNameData.f, "Ctgp_MarioCircuit1", 0x1E, 0x05, MarioKartTimer::ToFrames(2,0)},				//0x2B
			{&globalNameData.f, "Ctgp_GalvarnyFalls", 0x03, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x2C
			{&globalNameData.f, "Ctgp_SkaiiGarden", 0x0D, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x2D
			{&globalNameData.f, "Ctgp_AutumnForest", 0x12, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x2E
			{&globalNameData.f, "Gn64_ChocoMountainn", 0x1B, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x2F
			{&globalNameData.f, "Ctgp_DSShroomRidge", 0x08, 0x03, MarioKartTimer::ToFrames(2,30)},			//0x30
			{&globalNameData.f, "Ctgp_BowserCastle3", 0x1D, 0x03, MarioKartTimer::ToFrames(2,30)},			//0x31
			{&globalNameData.f, "Ctgp_EvGre", 0x16, 0x03, MarioKartTimer::ToFrames(2,0)},						//0x32
			{&globalNameData.f, "Ctgp_CrashCov", 0x14, 0x03, MarioKartTimer::ToFrames(2,30)},					//0x33
			{&globalNameData.f, "Ctgp_ArchipAvenue", 0x11, 0x03, MarioKartTimer::ToFrames(2,10)},				//0x34
			{&globalNameData.f, "Ctgp_FrapeSnow", 0x0A, 0x03, MarioKartTimer::ToFrames(2,30)},				//0x35
			{&globalNameData.f, "Ctgp_MoooMoooFarm", 0x07, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x36
			{&globalNameData.f, "Ctgp_BanshBoardT", 0x0E, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x37
			{&globalNameData.f, "Ctgp_CortexCastleeee", 0x06, 0x03, MarioKartTimer::ToFrames(2,30)},			//0x38
			{&globalNameData.f, "Ctgp_GhostValleyT", 0x14, 0x03, MarioKartTimer::ToFrames(1,45)},				//0x39
			{&globalNameData.f, "Ctgp_MelodSanc", 0x0F, 0x03, MarioKartTimer::ToFrames(2,30)},				//0x3A
			{&globalNameData.f, "Ctgp_MarioRacewa", 0x1A, 0x03, MarioKartTimer::ToFrames(2, 30)},				//0x3B
			{&globalNameData.f, "Ctgp_WarpPipeIsland", 0x11, 0x03, MarioKartTimer::ToFrames(1, 50)},		    //0x3C
			{&globalNameData.f, "Gsfc_ChocoIsland", 0x1E, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x3D
			{&globalNameData.f, "Ctgp_ElementalCave", 0x18, 0x03, MarioKartTimer::ToFrames(1,30)},			//0x3E
			{&globalNameData.f, "Ctgp_YoshFalls", 0x03, 0x03, MarioKartTimer::ToFrames(1,30)},				//0x3F
			{&globalNameData.f, "Ctgp_StarSlopeee", 0x1F, 0x03, MarioKartTimer::ToFrames(3,0)},				//0x40
			{&globalNameData.f, "Ctgp_ChpChpBch", 0x11, 0x03, MarioKartTimer::ToFrames(2,0)},					//0x41
			{&globalNameData.f, "Ctgp_DeseHill", 0x1B, 0x03, MarioKartTimer::ToFrames(2,0)},					//0x42
			{&globalNameData.f, "Ctgp_TickTockClock", 0x17, 0x03, MarioKartTimer::ToFrames(2,30)},			//0x43
			{&globalNameData.f, "Ctgp_RiversiPark", 0x1C, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x44
			{&globalNameData.f, "Ctgp_CastlOfTime", 0x0D, 0x02, MarioKartTimer::ToFrames(1,45)},				//0x45
			{&globalNameData.f, "Ctgp_N64RainbowR", 0x1F, 0x01, MarioKartTimer::ToFrames(2,30)},				//0x46
			{&globalNameData.f, "Gagb_RainbowRoad", 0x1F, 0x03, MarioKartTimer::ToFrames(2,30)},				//0x47
			{&globalNameData.f, "Ctgp_GCBowserCastle", 0x0B, 0x03, MarioKartTimer::ToFrames(2,30)},			//0x48
			{&globalNameData.f, "Ctgp_MikuBirtSpe", 0x0D, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x49
			{&globalNameData.f, "Ctgp_SandCastle", 0x1C, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x4A
			{&globalNameData.f, "Ctgp_MarioCircuit", 0x1A, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x4B
			{&globalNameData.f, "Gcn_LuigiCircuit", 0x1A, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x4C
			{&globalNameData.f, "Ctgp_VolcanoBeachRuins", 0x02, 0x02, MarioKartTimer::ToFrames(2, 0)},		//0x4D
			{&globalNameData.f, "Gcn_YoshiCircuit", 0x1A, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x4E
			{&globalNameData.f, "Gagb_PeachCircuitt", 0x03, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x4F
			{&globalNameData.f, "Ctgp_MetroMadness", 0x1B, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x50
			{&globalNameData.f, "Ctgp_GBALuigiCirc", 0x06, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x51
			{&globalNameData.f, "Ctgp_SMORCChallen", 0x1E, 0x05, MarioKartTimer::ToFrames(2, 0)},				//0x52
			{&globalNameData.f, "Gagb_BowserCastle4", 0x1D, 0x03, MarioKartTimer::ToFrames(3, 0)},			//0x53
			{&globalNameData.f, "Gsfc_DonutPlainsThree", 0x1E, 0x03, MarioKartTimer::ToFrames(1,30)},	        //0x54
			{&globalNameData.f, "Gn64_SecretSl", 0x21, 0x03, MarioKartTimer::ToFrames(3, 0)},					//0x55
			{&globalNameData.f, "Gds_WarioStad", 0x17, 0x03, MarioKartTimer::ToFrames(2,30)},					//0x56
			{&globalNameData.f, "Ctgp_ErmiiCir", 0x1A, 0x05, MarioKartTimer::ToFrames(1,30)},                 //0x57
			{&globalNameData.f, "Ggcn_BabyParkNin", 0x00, 0x07, MarioKartTimer::ToFrames(1,15)},				//0x58
			{&globalNameData.f, "Ctgp_RevoCircuit", 0x00, 0x03, MarioKartTimer::ToFrames(1, 30)},				//0x59
			{&globalNameData.f, "Gsfc_MarioCircTh", 0x1E, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x5A
			{&globalNameData.f, "Ctgp_BigBlueFZero1", 0x00, 0x03, MarioKartTimer::ToFrames(1, 15)},			//0x5B
			{&globalNameData.f, "Ggba_ShyGuyBeach", 0x1C, 0x03, MarioKartTimer::ToFrames(2,0)},				//0x5C
			{&globalNameData.f, "Ctgp_BingoPartyyyy", 0x17, 0x03, MarioKartTimer::ToFrames(2, 30)},			//0x5D
			{&globalNameData.f, "Ctgp_DogeDesert", 0x00, 0x02, MarioKartTimer::ToFrames(3, 0)},				//0x5E
			{&globalNameData.f, "Gn64_BansheeBoard", 0x14, 0x03, MarioKartTimer::ToFrames(3,0)},				//0x5F
			{&globalNameData.f, "Ctgp_GCNMarioCirc", 0x00, 0x03, MarioKartTimer::ToFrames(2,0)},  			//0X60
			{&globalNameData.f, "Ctgp_RainbowRdDX", 0x0D, 0x01, MarioKartTimer::ToFrames(2, 30)},				//0X61
			{&globalNameData.f, "Ctgp_StarGSumm", 0x0A, 0x03, MarioKartTimer::ToFrames(1, 45)},				//0x62
			{&globalNameData.f, "Ctgp_SunsetRacewa", 0x1A, 0x03, MarioKartTimer::ToFrames(2, 00)},			//0x63
			{&globalNameData.f, "Ctgp_GBABroknPier", 0x14, 0x03, MarioKartTimer::ToFrames(2, 00)},			//0x64
			{&globalNameData.f, "Ctgp_GlacrMine", 0x0A, 0x01, MarioKartTimer::ToFrames(2, 30)},				//0x65
			{&globalNameData.f, "Ctgp_FlowerBFort", 0x01, 0x03, MarioKartTimer::ToFrames(2, 00)},				//0x66
			{&globalNameData.f, "Ctgp_SeasidePalace", 0x01, 0x03, MarioKartTimer::ToFrames(2, 00)},			//0x67
			{&globalNameData.f, "Ctgp_DKRStaCi", 0x05, 0x03, MarioKartTimer::ToFrames(2, 00)},				//0x68
			{&globalNameData.f, "Ctgp_MushroomMount", 0x13, 0x02, MarioKartTimer::ToFrames(2, 30)},			//0x69
			{&globalNameData.f, "Ctgp_N64ShbLnd", 0x0A, 0x03, MarioKartTimer::ToFrames(2, 00)},				//0x6A
			{&globalNameData.f, "Ctgp_BlockIslandd", 0x00, 0x03, MarioKartTimer::ToFrames(2, 00)},			//0x6B
			{&globalNameData.f, "Ctgp_DSBowserCastle", 0x0B, 0x03, MarioKartTimer::ToFrames(2, 15)},			//0x6C
			{&globalNameData.f, "Ctgp_DKRJunFa", 0x07, 0x03, MarioKartTimer::ToFrames(1, 30)},				//0x6D
			{&globalNameData.f, "Ctgp_RetroRaceway", 0x1A, 0x03, MarioKartTimer::ToFrames(2, 00)},			//0x6E
			{&globalNameData.f, "Ctgp_FrzGrotto", 0x0A, 0x04, MarioKartTimer::ToFrames(1, 45)},				//0x6F
			{&globalNameData.f, "Ctgp_GBALksdPrk", 0x18, 0x03, MarioKartTimer::ToFrames(2, 00)},				//0x70
			{&globalNameData.f, "Ctgp_DrgnBGrounds", 0x0B, 0x03, MarioKartTimer::ToFrames(2, 15)},			//0x71
			{&globalNameData.f, "Ctgp_RMXSFCRbwRd", 0x1F, 0x03, MarioKartTimer::ToFrames(1, 50)},				//0x72		
			{&globalNameData.f, "Ctgp_NeoMetropolisss", 0x6, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x73		
			{&globalNameData.f, "Ctgp_FrostyHeights", 0x16, 0x03, MarioKartTimer::ToFrames(2, 0)},			//0x74		
			{&globalNameData.f, "Ctgp_GnsGnoLair", 0x1F, 0x03, MarioKartTimer::ToFrames(2, 0)},				//0x75			
			{&globalNameData.f, "Ctgp_VaLkO", 0x16, 0x03, MarioKartTimer::ToFrames(2, 0)},					//0x76		
			{&globalNameData.f, "Ctgp_CliffCircuit", 0x13, 0x03, MarioKartTimer::ToFrames(1, 50)},			//0x77		
			{&globalNameData.f, "Ctgp_InterstellarLabb", 0x1F, 0x03, MarioKartTimer::ToFrames(2, 15)},		//0x78	
			{&globalNameData.f, "Ctgp_DarkMatterFortress", 0x1F, 0x03, MarioKartTimer::ToFrames(2, 15)},		//0x79		
		}
	};
	
	const u32 globalCupTranslateTable[MAXCUPS] = {
		//Top
		0x00,
		0x01,
		0x02,
		0x03,
		0x0A,
		0x0C,
		0x0E,
		0x10,
		0x13,
		0x15,
		0x18,
		0x1A,
		0x1C,
		0x17,
		//Bottom
		0x04,
		0x05,
		0x06,
		0x07,
		0x0B,
		0x0D,
		0x0F,
		0x12,
		0x14,
		0x16,
		0x19,
		0x1B,
		0x1D,
		0x11
	};
	const u32 ctwwCupTranslateTable[MAXCUPS - 8] = {
		//Top
		0x0A,
		0x0C,
		0x0E,
		0x10,
		0x13,
		0x15,
		0x18,
		0x1A,
		0x1C,
		0x17,
		//Bottom
		0x0B,
		0x0D,
		0x0F,
		0x12,
		0x14,
		0x16,
		0x19,
		0x1B,
		0x1D,
		0x11
	};
	const u32 vanillaCupTranslateTable[8] = {
		//Top
		0x00,
		0x01,
		0x02,
		0x03,
		//Bottom
		0x04,
		0x05,
		0x06,
		0x07,
	};

	const u32* CourseManager::getCupTranslatetable(u32* size, bool forceGetAll)
	{
		if (!getOriginalTracksAllowed() && !forceGetAll) {
			*size = sizeof(ctwwCupTranslateTable) / sizeof(u32);
			return ctwwCupTranslateTable;
		} else if (!getCustomTracksAllowed() && !forceGetAll) {
			*size = sizeof(vanillaCupTranslateTable) / sizeof(u32);
			return vanillaCupTranslateTable;
		} else if (g_getCTModeVal == CTMode::OFFLINE && !forceGetAll && !VersusHandler::IsVersusMode && !MissionHandler::isMissionMode && (MarioKartFramework::currentRaceMode.type == 0 || MarioKartFramework::currentRaceMode.type == 1) && (MarioKartFramework::currentRaceMode.mode == 0 || MarioKartFramework::currentRaceMode.mode == 1) && finalGlobalCupTranslateTable) {
			*size = finalGlobalCupTranslateTableSize;
			return finalGlobalCupTranslateTable;
		} else {
			*size = sizeof(globalCupTranslateTable) / sizeof(u32);
			return globalCupTranslateTable;
		}
	}

	void CourseManager::initGlobalCupTranslateTable() {
		if (finalGlobalCupTranslateTable)
			return;
		u32 size = sizeof(globalCupTranslateTable) / sizeof(u32) + UserCTHandler::GetCustomCupAmount();
		if (size & 1) size++;
		finalGlobalCupTranslateTableSize = size;
		finalGlobalCupTranslateTable = (u32*)::operator new(sizeof(u32) * finalGlobalCupTranslateTableSize);
		for (int i = 0; i < finalGlobalCupTranslateTableSize; i++) {
			finalGlobalCupTranslateTable[i] = USERCUPID;
		}
		for (int i = 0; i < sizeof(globalCupTranslateTable) / sizeof(u32) / 2; i++) {
			finalGlobalCupTranslateTable[i] = globalCupTranslateTable[i];
			finalGlobalCupTranslateTable[i + finalGlobalCupTranslateTableSize / 2] = globalCupTranslateTable[i + sizeof(globalCupTranslateTable) / sizeof(u32) / 2 ];
		}
	}
	
	TrophyNameData globalTrophyData{
		{0, 0, (u32)CTNameFunc::nullsub_func},
		{
			{&globalTrophyData.f, "Mush"},
			{&globalTrophyData.f, "Flower"},
			{&globalTrophyData.f, "Star"},
			{&globalTrophyData.f, "Special"},
			{&globalTrophyData.f, "Shell"},
			{&globalTrophyData.f, "Banana"},
			{&globalTrophyData.f, "Leaf"},
			{&globalTrophyData.f, "Lightning"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
			{&globalTrophyData.f, "General"},
		}
	};

	TrophyNameData::TrophyNameEntry generalThrophy = { &globalTrophyData.f, "General" };
}
