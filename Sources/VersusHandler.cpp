/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VersusHandler.cpp
Open source lines: 683/683 (100.00%)
*****************************************************/

#include "VersusHandler.hpp"
#include "CourseManager.hpp"
#include "ExtraResource.hpp"
#include "MissionHandler.hpp"
#include "Lang.hpp"
#include "3ds.h"
#include "csvc.h"
#include "Sound.hpp"
#include "main.hpp"
#include "SaveHandler.hpp"
#include "UserCTHandler.hpp"
#include "CharacterHandler.hpp"
#include "MenuPage.hpp"
#include "str16utils.hpp"
#include <algorithm>

namespace CTRPluginFramework {

	static const char* g_cupIconName[MAXCUPS + 2]{
		"cup_kinoko",
		"cup_flower",
		"cup_star",
		"cup_special",
		"cup_koura",
		"cup_banana",
		"cup_konoha",
		"cup_thunder",
		"cup_arrowleft",
		"cup_arrowright",
		"cup_bell",
		"cup_acorn",
		"cup_cloud",
		"cup_boo",
		"cup_spring",
		"cup_egg",
		"cup_bullet",
		"cup_rainbow",
		"cup_blooper",
		"cup_feather",
		"cup_fireball",
		"cup_bobomb",
		"cup_cherry",
		"cup_coin",
		"cup_pickaxe",
		"cup_mega",
		"cup_propeller",
		"cup_pow",
		"cup_rock",
		"cup_moon",
		"cup_hammer",
		"cup_wonder"
	};

	std::vector<CustomIcon> VersusHandler::cupIcons;
	CustomIcon VersusHandler::DisabledIcon{ nullptr, 30, 30, false };
	VersusHandler::SettingsObject* VersusHandler::SetObject = new VersusHandler::SettingsObject();
	bool VersusHandler::IsVersusMode = false;
	void (*VersusHandler::ApplyVSSetting)(VersusHandler::SettingsObject*) = (void(*)(VersusHandler::SettingsObject*))0;
	bool VersusHandler::canShowVSSet = false;
	u32 VersusHandler::MenuSingleClassPage = 0;
	u32 VersusHandler::invalidLRVal3 = 0;
	u8 VersusHandler::currentVersusCupTableEntry = 0;
	int VersusHandler::selectedCupButtonKeyboard = 0;
	u32 VersusHandler::versusCupTable[32] = { 0 };
	std::vector<std::string> VersusHandler::settingsOpts;
	std::vector<std::string> VersusHandler::cpudifficulty;
	std::vector<std::string> VersusHandler::courseOpts;
	std::vector<std::string> VersusHandler::itemOpts;
	std::vector<std::string> VersusHandler::itemNames;
	std::string VersusHandler::textRandom;
	VersusHandler::KeyboardArguments VersusHandler::keyboardArgs;

	void VersusHandler::Initialize()
	{
		for (int i = 0; i < MAXCUPS + 2; i++) {
			ExtraResource::SARC::FileInfo finfo;
			std::string filename = std::string("Plugin/cupIcons/") << g_cupIconName[i] << ".bin";
			u8* iconData = ExtraResource::mainSarc->GetFile(filename, &finfo);
			if (finfo.fileSize != 30 * 30 * 4) iconData = nullptr;
			cupIcons.push_back(CustomIcon((CustomIcon::Pixel*)iconData, iconData == nullptr ? 0 : 30, 30, true));
		}
	}

	void VersusHandler::InitializeText() {
		int symbolID = Render::CreateRandomSet("¡|!Il1()[]{}");
		settingsOpts.insert(settingsOpts.end(), { Language::MsbtHandler::GetString(2001), Language::MsbtHandler::GetString(2310), NAME("CpuAm"), NAME("CourOrd"), NAME("RacAm"), Language::MsbtHandler::GetString(2370)});
		cpudifficulty.insert(cpudifficulty.end(), { NAME("default"), Color::Orange << SetShake(true, true, 1) + NAME("extra_diff") + SetShake(true, true, 0) + ResetColor(), Color::Red << SetRandomText(symbolID) + "AA" + SetRandomText(-1) + SetShake(false, true, 2) + " " + NOTE("extra_diff") + " " + SetShake(false, true, 0) + SetRandomText(symbolID) + "AA" + SetRandomText(-1) + ResetColor()}); 
		courseOpts.insert(courseOpts.end(), { Language::MsbtHandler::GetString(2321), NAME("RanCT"), NAME("RanOri"), NAME("RanAll"), NAME("OrderH"), NAME("OrderV") });
		itemOpts.insert(itemOpts.end(), { Language::MsbtHandler::GetString(2371), Language::MsbtHandler::GetString(2372), Language::MsbtHandler::GetString(2373), Language::MsbtHandler::GetString(2374), Language::MsbtHandler::GetString(2375), Language::MsbtHandler::GetString(2376), NAME("custom"), Language::MsbtHandler::GetString(2322)});
		itemNames.insert(itemNames.end(), {Language::MsbtHandler::GetString(1023), Language::MsbtHandler::GetString(1022), Language::MsbtHandler::GetString(1026), Language::MsbtHandler::GetString(1018), Language::MsbtHandler::GetString(1027), Language::MsbtHandler::GetString(1029), Language::MsbtHandler::GetString(1028), Language::MsbtHandler::GetString(1033), Language::MsbtHandler::GetString(1020), Language::MsbtHandler::GetString(1030),
											Language::MsbtHandler::GetString(1025), Language::MsbtHandler::GetString(1021), Language::MsbtHandler::GetString(1019), Language::MsbtHandler::GetString(1024), Language::MsbtHandler::GetString(1031), NAME("item_fib"), NAME("item_mega"), Language::MsbtHandler::GetString(1035), Language::MsbtHandler::GetString(1034), Language::MsbtHandler::GetString(1036)});
		textRandom = Language::MsbtHandler::GetString(2322);
	}

	void VersusHandler::PopulateCupKbdForPage(std::vector<CustomIcon>& icons, int page)
	{
		u32 size;
		const u32* cupTransTable = CourseManager::getCupTranslatetable(&size, true);
		icons.resize(3 * 4);
		for (int i = 0; i < 4; i++) {
			icons[i] = cupIcons[cupTransTable[(page + i) % (size / 2)]];
			icons[i + 4] = cupIcons[cupTransTable[(page + i) % (size / 2) + (size / 2)]];
		}
		icons[8] = cupIcons[8];
		icons[9] = icons[10] = DisabledIcon;
		icons[11] = cupIcons[9];
	}
	
    void VersusHandler::CupKeyboardCallback(Keyboard& k, KeyboardEvent& event) {
		if (event.type == KeyboardEvent::EventType::SelectionChanged) {
			keyboardArgs.lastSelectedIndex = event.selectedIndex;
			u32 maxButton;
			CourseManager::getCupTranslatetable(&maxButton, true);
			int val = event.selectedIndex;
			if (val == 8 || val == 11 || val == -1) {
				k.GetMessage() = *keyboardArgs.topMessage + "\n" + HorizontalSeparator() + "\n\n\n\n" + HorizontalSeparator();
			}
			else {
				if (val < 4) val = ((*keyboardArgs.startCupButton) + val) % (maxButton / 2);
				else val = (((*keyboardArgs.startCupButton) + (val - 4)) % (maxButton / 2)) + maxButton / 2;
				getCupKeyboardText(k.GetMessage(), val, *keyboardArgs.topMessage);
			}
		}
		else if (event.type == KeyboardEvent::EventType::KeyPressed) {
			if (event.affectedKey & Key::L) {
				SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
				keyboardArgs.closedByScroll = 1;
				k.Close();
			}
			else if (event.affectedKey & Key::R) {
				SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
				keyboardArgs.closedByScroll = 2;
				k.Close();
			}
		}
	}

	u32 VersusHandler::OpenCupKeyboard(int* startCupButton, int* selectedCupButton, std::string& topMessage)
	{
		Keyboard cupKbd("dummy");
		cupKbd.OnKeyboardEvent(CupKeyboardCallback);
		std::vector<CustomIcon> cups;
		if (*selectedCupButton >= 8 || *selectedCupButton < 0)
			*selectedCupButton = 0;
		bool loop = true;
		u32 maxButton;
		u32 selCup = 0x8;
		const u32* cupTransTable = CourseManager::getCupTranslatetable(&maxButton, true);
		int isArrow = 0;
		while (loop) {
			keyboardArgs.startCupButton = startCupButton;
			keyboardArgs.topMessage = &topMessage;
			keyboardArgs.closedByScroll = 0;
			keyboardArgs.lastSelectedIndex = 0;
			PopulateCupKbdForPage(cups, *startCupButton);
			cupKbd.Populate(cups);
			cupKbd.ChangeEntrySound(8, SoundEngine::Event::SELECT);
			cupKbd.ChangeEntrySound(11, SoundEngine::Event::SELECT);
			cupKbd.ChangeSelectedEntry((isArrow) ? isArrow - 1 : *selectedCupButton);
			if (isArrow == 1) { // Update callback isn't called when the index is 0, for whatever reason
				int tempval = ((*keyboardArgs.startCupButton)) % (maxButton / 2);
				getCupKeyboardText(cupKbd.GetMessage(), tempval, *keyboardArgs.topMessage);
			}
			int val = cupKbd.Open();
			if (val == -1 && keyboardArgs.closedByScroll) { // L or R pressed
				if (keyboardArgs.closedByScroll == 1) val = 8;
				else if (keyboardArgs.closedByScroll == 2) val = 11;
				isArrow = keyboardArgs.lastSelectedIndex + 1;
			}
			else isArrow = 0;
			switch (val)
			{
			case 11:
				if (!isArrow) isArrow = 11 + 1;
				(*startCupButton)++;
				if (*startCupButton >= (maxButton / 2)) {
					*startCupButton = 0;
				}
				break;
			case 8:
				if (!isArrow) isArrow = 8 + 1;
				(*startCupButton)--;
				if (*startCupButton < 0) {
					*startCupButton = (maxButton / 2) - 1;
				}
				break;
			case -1:
				selCup = 0x8;
				loop = false;
				break;
			case -2:
				{
					int lastVal = cupKbd.GetLastSelectedEntry();
					if (lastVal >= 0 && lastVal <= 7)
						val = lastVal;
					else
						val = 0;
				}
			default:
				isArrow = 0;
				*selectedCupButton = val;
				loop = false;
				if (val < 4) selCup = cupTransTable[((*startCupButton) + val) % (maxButton / 2)];
				else selCup = cupTransTable[(((*startCupButton) + (val - 4)) % (maxButton / 2)) + maxButton / 2];
				break;
			}
		}
		return selCup;
	}

	u32 VersusHandler::OpenCourseKeyboard(u32 cupId, bool canAbort)
	{
		if (cupId == 0x8) return 0xFFFFFFFF;
		Keyboard courseKbd("");
		courseKbd.CanAbort(canAbort);
		courseKbd.DisplayTopScreen = false;
		std::vector<std::string> courses;
		std::string s;
		for (int i = 0; i < 4; i++) {
			s.clear();
			CourseManager::getCourseText(s, globalCupData[FROMBUTTONTOCUP(cupId)][i], true);
			courses.push_back(s);
		}
		courseKbd.Populate(courses);
		while (true) {
			int val = courseKbd.Open();
			if (val == -2) {
				int lastVal = courseKbd.GetLastSelectedEntry();
				if (lastVal >= 0 && lastVal <= 3)
					val = lastVal;
				else
					val = 0;
			}
			if (val == -1) return INVALIDTRACK;
			u32 courseID = globalCupData[FROMBUTTONTOCUP(cupId)][val];
			if (!MenuPageHandler::MenuSingleCourseBasePage::IsBlockedCourse(courseID))
				return courseID;
		}
	}

	u32 VersusHandler::OpenCupCourseKeyboard(int* startingCupButton, int* selectedCupButton, bool canAbort, std::string& topText)
	{
		u32 track;
		Process::Pause();
		do {
			track = OpenCourseKeyboard(OpenCupKeyboard(startingCupButton, selectedCupButton, topText), true);
		} while (track == INVALIDTRACK || (track == 0xFFFFFFFF && !canAbort));
		if (track == 0xFFFFFFFF) track = INVALIDTRACK;
		Process::Play();
		return track;
	}

	void VersusHandler::ApplyVSModeSettings()
	{
		if (!IsVersusMode) return;
		u32 itemMode = SaveHandler::saveData.vsSettings.itemOption;
		if (itemMode == EItemMode::ITEMMODE_CUSTOM || itemMode == EItemMode::ITEMMODE_RANDOM)
			itemMode = EItemMode::ITEMMODE_ALL;
		ApplySpecifiedSetting(VSSettings::ITEMS, itemMode);

		if (neeedsCustomItemHandling()) {
			std::array<bool, EItemSlot::ITEM_SIZE> array;
			bool isRandomMode = SaveHandler::saveData.vsSettings.itemOption == EItemMode::ITEMMODE_RANDOM;
			for (int i = 0; i < array.size(); i++) {
				array[i] = SaveHandler::saveData.vsSettings.GetItemEnabled((EItemSlot)i, isRandomMode);
			}
			MarioKartFramework::UseCustomItemMode(array, isRandomMode);
		}
	}

	void VersusHandler::ApplySpecifiedSetting(VSSettings setting, u32 value) {
		SetObject->VSSetting = setting;
		SetObject->VSOption = value;
		ApplyVSSetting(SetObject);
	}

	void VersusHandler::OpenSettingsKeyboardCallback()
	{
		OpenSettingsKeyboard();
		*(PluginMenu::GetRunningInstance()) -= OpenSettingsKeyboardCallback;
	}

	void VersusHandler::getCupKeyboardText(std::string& string, u32 cup, std::string& topMessage) {
		u32 size;
		const u32* cupTransTable = CourseManager::getCupTranslatetable(&size, true);
		
		string = topMessage + "\n" + HorizontalSeparator();
		
		CourseManager::getCupText(string, cupTransTable[cup]);
		string += "\n";
		for (int j = 0; j < 4; j++) {
			string += "- ";
			CourseManager::getCourseText(string, globalCupData[FROMBUTTONTOCUP(cupTransTable[cup])][j], true);
			string += "\n";
		}
		string.pop_back();
		string += HorizontalSeparator();
	}

	void VersusHandler::getSettingsKeyboardText(std::string& string) {
		string = NAME("VSSet") + ":\n\n- " + settingsOpts[1] + ": ";
		switch (SaveHandler::saveData.vsSettings.cpuOption)
		{
		case VSCPUDifficulty::STANDARD:
			string += cpudifficulty[0];
			break;
		case VSCPUDifficulty::VERY_HARD:
			string += cpudifficulty[1];
			break;
		case VSCPUDifficulty::INSANE:
			string += cpudifficulty[2];
			break;
		default:
			break;
		}
		string += "\n- " + settingsOpts[2] + ": " + std::to_string(SaveHandler::saveData.vsSettings.cpuAmount);
		string += "\n- " + settingsOpts[3] + ": ";
		switch (SaveHandler::saveData.vsSettings.courseOption)
		{
		case VSCourseOption::CHOOSE:
			string += courseOpts[0];
			break;
		case VSCourseOption::RANDOMCT:
			string += courseOpts[1];
			break;
		case VSCourseOption::RANDOMORI:
			string += courseOpts[2];
			break;
		case VSCourseOption::RANDOMORICT:
			string += courseOpts[3];
			break;
		case VSCourseOption::INORDERH:
			string += courseOpts[4];
			break;
		case VSCourseOption::INORDERV:
			string += courseOpts[5];
			break;
		default:
			break;
		}
		string += "\n- " + settingsOpts[4] + ": " + std::to_string(SaveHandler::saveData.vsSettings.roundAmount);
		string += "\n- " + settingsOpts[5] + ": ";
		switch (SaveHandler::saveData.vsSettings.itemOption)
		{
		case EItemMode::ITEMMODE_ALL:
			string += itemOpts[0];
			break;
		case EItemMode::ITEMMODE_SHELLS:
			string += itemOpts[1];
			break;
		case EItemMode::ITEMMODE_BANANAS:
			string += itemOpts[2];
			break;
		case EItemMode::ITEMMODE_MUSHROOMS:
			string += itemOpts[3];
			break;
		case EItemMode::ITEMMODE_BOBOMBS:
			string += itemOpts[4];
			break;
		case EItemMode::ITEMMODE_NONE:
			string += itemOpts[5];
			break;
		case EItemMode::ITEMMODE_CUSTOM:
			string += itemOpts[6];
			break;
		case EItemMode::ITEMMODE_RANDOM:
			string += itemOpts[7];
			break;
		default:
			break;
		}
		string += "\n";
	}

	void VersusHandler::OpenSettingsKeyboard()
	{
		Keyboard mainSet("dummy");
		Keyboard subSet("dummy");
		subSet.IsHexadecimal(false);
		mainSet.Populate(settingsOpts);
		mainSet.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
		bool loop = true;
		u32 round;
		int val;
		Process::Pause();
		while (loop) {
			getSettingsKeyboardText(mainSet.GetMessage());
			subSet.GetMessage() = mainSet.GetMessage();
			switch (mainSet.Open())
			{
			case 1:
				subSet.Populate(cpudifficulty, true);
				switch (subSet.Open())
				{
				case 0:
					SaveHandler::saveData.vsSettings.cpuOption = VSCPUDifficulty::STANDARD;
					break;
				case 1:
					SaveHandler::saveData.vsSettings.cpuOption = VSCPUDifficulty::VERY_HARD;
					break;
				case 2:
					SaveHandler::saveData.vsSettings.cpuOption = VSCPUDifficulty::INSANE;
					break;
				default:
					break;
				}
				break;
			case 2:
				subSet.SetCompareCallback([](const void* val, std::string& error) {
					const u32* nval = (u32*)val;
					if (*nval < 0 || *nval > 7) {
						error = Utils::Format(NAME("num_betw").c_str(), 0, 7);
						return false;
					}
					return true;
				});
				val = subSet.Open(round, SaveHandler::saveData.vsSettings.cpuAmount);
				if (val == 0) SaveHandler::saveData.vsSettings.cpuAmount = round;
				break;
			case 3:
				subSet.Populate(courseOpts, true);
				switch (subSet.Open())
				{
				case 0:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::CHOOSE;
					break;
				case 1:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::RANDOMCT;
					break;
				case 2:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::RANDOMORI;
					break;
				case 3:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::RANDOMORICT;
					break;
				case 4:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::INORDERH;
					break;
				case 5:
					SaveHandler::saveData.vsSettings.courseOption = VSCourseOption::INORDERV;
					break;
				default:
					break;
				}
				break;
			case 4:
				subSet.SetCompareCallback([](const void* val, std::string & error) {
					const u32* nval = (u32*)val;
					if (*nval < 1 || *nval > 32) {
						error = Utils::Format(NAME("num_betw").c_str(), 1, 32);
						return false;
					}
					return true;
				});
				val = subSet.Open(round, SaveHandler::saveData.vsSettings.roundAmount);
				if (val == 0) SaveHandler::saveData.vsSettings.roundAmount = round;
				break;
			case 5:
				subSet.Populate(itemOpts, true);
				switch (subSet.Open())
				{
				case 0:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_ALL;
					break;
				case 1:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_SHELLS;
					break;
				case 2:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_BANANAS;
					break;
				case 3:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_MUSHROOMS;
					break;
				case 4:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_BOBOMBS;
					break;
				case 5:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_NONE;
					break;
				case 6:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_CUSTOM;
					OpenItemSelectorMenu(false);
					break;
				case 7:
					SaveHandler::saveData.vsSettings.itemOption = EItemMode::ITEMMODE_RANDOM;
					OpenItemSelectorMenu(true);
					break;
				default:
					break;
				}
				break;
			default:
				loop = false;
				break;
			}
		}
		Process::Play();
		SaveHandler::SaveSettingsAll();
	}
	
	bool VersusHandler::IsRandomCourseMode()
	{
		return SaveHandler::saveData.vsSettings.courseOption >= RANDOMCT && SaveHandler::saveData.vsSettings.courseOption <= RANDOMORICT;
	}

	void VersusHandler::GenerateVersusCupTable(u32 start)
	{
		currentVersusCupTableEntry = 0;
		selectedCupButtonKeyboard = MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon;
		if (IsRandomCourseMode()) { //Random mode
			std::vector<u32> courseList;
			switch (SaveHandler::saveData.vsSettings.courseOption)
			{
			case RANDOMCT:
				for (u32 i = CUSTOMTRACKLOWER; i <= CUSTOMTRACKUPPER; i++) courseList.push_back(i);
				break;
			case RANDOMORI:
				for (u32 i = ORIGINALTRACKLOWER; i <= ORIGINALTRACKUPPER; i++) courseList.push_back(i);
				break;
			case RANDOMORICT:
				for (u32 i = ORIGINALTRACKLOWER; i <= CUSTOMTRACKUPPER; i++) {
					if (i > ORIGINALTRACKUPPER && i < CUSTOMTRACKLOWER) continue;
					courseList.push_back(i);
				}
			default:
				break;
			}
			random_shuffle_custom(courseList.begin(), courseList.end(), [](int i) {
				return Utils::Random(0, i - 1);
			});
			for (int i = 0; i < SaveHandler::saveData.vsSettings.roundAmount; i++) {
				versusCupTable[i] = courseList[i];
				if (MenuPageHandler::MenuSingleCourseBasePage::IsBlockedCourse(courseList[i])) {
					courseList.erase(std::next(courseList.begin(), i));
					i--;
				}
			}
		}
		else {
			versusCupTable[0] = start;
			u32 cupTransSize, startingCupIndex, cupTrack[2], startingTrackIndex, index = 0;
			CourseManager::findCupCourseID(cupTrack, start);
			startingTrackIndex = cupTrack[1];
			const u32* cupTransTable = CourseManager::getCupTranslatetable(&cupTransSize);
			for (startingCupIndex = 0; startingCupIndex < cupTransSize && cupTransTable[startingCupIndex] != FROMCUPTOBUTTON(cupTrack[0]); startingCupIndex++);
			if (SaveHandler::saveData.vsSettings.courseOption == VSCourseOption::INORDERH) {
				for (; index < SaveHandler::saveData.vsSettings.roundAmount; startingCupIndex = (startingCupIndex + 1) % cupTransSize) {
					for (; index < SaveHandler::saveData.vsSettings.roundAmount && startingTrackIndex < 4; startingTrackIndex++) {
						versusCupTable[index++] = globalCupData[FROMBUTTONTOCUP(cupTransTable[startingCupIndex])][startingTrackIndex];
						if (MenuPageHandler::MenuSingleCourseBasePage::IsBlockedCourse(versusCupTable[index-1])) index--;
					}
					startingTrackIndex = 0;
				}
			}
			else if (SaveHandler::saveData.vsSettings.courseOption == VSCourseOption::INORDERV) {
				for (; index < SaveHandler::saveData.vsSettings.roundAmount; startingCupIndex = (startingCupIndex + (cupTransSize / 2) + (startingCupIndex >= (cupTransSize / 2) ? 1 : 0)) % cupTransSize) {
					for (; index < SaveHandler::saveData.vsSettings.roundAmount && startingTrackIndex < 4; startingTrackIndex++) {
						versusCupTable[index++] = globalCupData[FROMBUTTONTOCUP(cupTransTable[startingCupIndex])][startingTrackIndex];
						if (MenuPageHandler::MenuSingleCourseBasePage::IsBlockedCourse(versusCupTable[index-1])) index--;
					}
					startingTrackIndex = 0;
				}
			}
			else if (SaveHandler::saveData.vsSettings.courseOption == VSCourseOption::CHOOSE) {
				for (int i = 1; i < SaveHandler::saveData.vsSettings.roundAmount; i++) versusCupTable[i] = INVALIDTRACK;
			}
		}
	}

	static u32 g_selectedCupID;
	void VersusHandler::OnCupSelectCallback()
	{
		Process::Pause();
		u32 courseID = OpenCourseKeyboard(g_selectedCupID, false);
		Process::Play();
		GenerateVersusCupTable(courseID);
		*(PluginMenu::GetRunningInstance()) -= OnCupSelectCallback;
	}

	void VersusHandler::OnCupSelect(u32 cupID)
	{
		if (!IsVersusMode) return;
		if (IsRandomCourseMode()) {
			GenerateVersusCupTable(INVALIDTRACK);
		}
		else {
			g_selectedCupID = cupID;
			*(PluginMenu::GetRunningInstance()) += OnCupSelectCallback;
		}
		ApplyVSModeSettings();
	}

	void VersusHandler::OnNextTrackLoadCallback()
	{
		if (currentVersusCupTableEntry + 1 < SaveHandler::saveData.vsSettings.roundAmount) {
			std::string topMessage = NAME("NextRac") + " " + std::to_string(SaveHandler::saveData.vsSettings.roundAmount - (currentVersusCupTableEntry + 1));
			u32 nextTrack = OpenCupCourseKeyboard(&MenuPageHandler::MenuSingleCupBasePage::startingButtonID, &selectedCupButtonKeyboard, false, topMessage);
			versusCupTable[currentVersusCupTableEntry + 1] = nextTrack;
		}
		*(PluginMenu::GetRunningInstance()) -= OnNextTrackLoadCallback;
	}

	void VersusHandler::OnNextTrackLoad()
	{
		currentVersusCupTableEntry++;
	}

	void VersusHandler::OnNextMenuShow()
	{
		if (IsVersusMode && SaveHandler::saveData.vsSettings.courseOption == VSCourseOption::CHOOSE) {
			*(PluginMenu::GetRunningInstance()) += OnNextTrackLoadCallback;
		}
	}

	extern "C" u32 g_gameModeID;
	void VersusHandler::OnMenuSingleEnterCallback()
	{
		g_gameModeID = 0;
		g_setCTModeVal = CTMode::OFFLINE;
		g_updateCTMode();

		IsVersusMode = false;
		MissionHandler::onModeMissionExit();
		UserCTHandler::UpdateCurrentCustomCup(0);
		CharacterHandler::ResetCharacters();
		MarioKartFramework::ClearCustomItemMode();
	}

	void VersusHandler::OnMenuSingleOKCallback(u32 val) {
		if (val == 2)
			IsVersusMode = true;
		else if (val == 3)
			MissionHandler::onModeMissionEnter();
	}

	void VersusHandler::OpenItemSelectorMenu(bool isRandom) {
		Keyboard kbd("dummy");

		std::vector<std::string> options;
		options.push_back(settingsOpts[0]);
		options.insert(options.end(), itemNames.begin(), itemNames.end());
		kbd.Populate(options);
		int opt;
		do {
			kbd.GetMessage() = CenterAlign(ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + NAME("cusitem") + " - " + (isRandom ? textRandom : NAME("custom")) + ToggleDrawMode(Render::FontDrawMode::UNDERLINE));

			for (int i = 0; i < itemNames.size() / 2; i++) {
				bool enabled1 = SaveHandler::saveData.vsSettings.GetItemEnabled((EItemSlot)i, isRandom);
				bool enabled2 = SaveHandler::saveData.vsSettings.GetItemEnabled((EItemSlot)(i + (itemNames.size() / 2)), isRandom);
				
				kbd.GetMessage() += "\n" << (enabled1 ? Color::Lime : Color::Red) << itemNames[i] + ResetColor();
				
				kbd.GetMessage() += RightAlign((enabled2 ? Color::Lime : Color::Red) << itemNames[i + (itemNames.size() / 2)] + ResetColor(), 35, 368);
				
				kbd.ChangeEntrySound(i + 1, enabled1 ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
				kbd.ChangeEntrySound(i + (itemNames.size() / 2) + 1, enabled2 ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			}

			opt = kbd.Open();
			if (opt > 0) {
				SaveHandler::saveData.vsSettings.SetItemEnabled((EItemSlot)(opt - 1), isRandom, !SaveHandler::saveData.vsSettings.GetItemEnabled((EItemSlot)(opt - 1), isRandom));
			}
		} while (opt > 0);
		if (!SaveHandler::saveData.vsSettings.GetItemEnabled(EItemSlot::ITEM_KINOKO, isRandom))
			MessageBox(Utils::Format(NAME("cusitem_notice").c_str(), itemNames[(int)EItemSlot::ITEM_KINOKO].c_str()) + "\n ")();
	}

	bool VersusHandler::neeedsCustomItemHandling() {
		return IsVersusMode && 
		(SaveHandler::saveData.vsSettings.itemOption == EItemMode::ITEMMODE_CUSTOM || 
		SaveHandler::saveData.vsSettings.itemOption == EItemMode::ITEMMODE_RANDOM); 
	}
}

