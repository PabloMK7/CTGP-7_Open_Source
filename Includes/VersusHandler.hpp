/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VersusHandler.hpp
Open source lines: 190/190 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "SaveHandlerInfos.hpp"
#include "DataStructures.hpp"
#include "ItemHandler.hpp"
#include "Minibson.hpp"

#define VERSUSCUPID 63

namespace CTRPluginFramework {
	class VersusHandler
	{
	public:
		enum VSSettings
		{
			CC = 0,
			CPUENABLED = 1,
			CPU = 2,
			STAGE = 3,
			ITEMS = 4,
			TEAMS = 5,
			INVALID = 0xFFFFFFFF
		};
		enum VSCCOption
		{
			CC50 = 0,
			CC100 = 1,
			CC150 = 2,
			MIRROR = 3
		};
		enum VSCPUEnabledOption
		{
			CPUNO = 0,
			CPUYES = 1
		};
		enum VSCPUDifficulty
		{
			STANDARD = 1,
			VERY_HARD = 2,
			INSANE = 3
		};
		enum VSStageOption
		{
			STGCHOOSE = 0,
			STGRANDOM = 1,
			STGINORDER = 2
		};
		enum VSTeamOption
		{
			TEAMNO = 0,
			TEAMYES = 1
		};
		enum VSCourseOption
		{
			CHOOSE = 0,
			RANDOMCT = 1,
			RANDOMORI = 2,
			RANDOMORICT = 3,
			INORDERH = 4,
			INORDERV = 5
		};
		struct SettingsObject
		{
			u32 _gap1[0x1F];
			u32 VSOption;
			u32 _gap2[0xE3];
			VSSettings VSSetting;
			u32 _gap3[0x7];
			u32 always0;
			SettingsObject() {
				always0 = 0;
			}
		};
		struct CurrentSettings
		{
			u8 cpuOption;
			u8 cpuAmount;
			u8 itemOption;
			u8 roundAmount;
			u8 courseOption;
			u32 customItemsRandom;
			u32 customItemsCustom;
			CurrentSettings() {
				cpuOption = VSCPUDifficulty::STANDARD;
				cpuAmount = 7;
				itemOption = EItemMode::ITEMMODE_ALL;
				roundAmount = 4;
				courseOption = VSCourseOption::CHOOSE;
				customItemsCustom = 0xFFFFFFFF;
				customItemsCustom = 0xFFFFFFFF;
			}

			CurrentSettings(const minibson::document& doc) {
				cpuOption = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CPU_OPTION), (int)VSCPUDifficulty::STANDARD);
				cpuAmount = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CPU_AMOUNT), 7);
				itemOption = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_ITEM_OPTION), (int)EItemMode::ITEMMODE_ALL);
				roundAmount = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_ROUND_AMOUNT), 4);
				courseOption = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_COURSE_OPTION), (int)VSCourseOption::CHOOSE);
				customItemsCustom = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CUSTOM_ITEMS_CUSTOM), (int)-1);
				customItemsRandom = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CUSTOM_ITEMS_RANDOM), (int)-1);
			}

			void serialize(minibson::document& doc) {
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CPU_OPTION), (int)cpuOption);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CPU_AMOUNT), (int)cpuAmount);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_ITEM_OPTION), (int)itemOption);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_ROUND_AMOUNT), (int)roundAmount);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_COURSE_OPTION), (int)courseOption);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CUSTOM_ITEMS_CUSTOM), (int)customItemsCustom);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VS_CUSTOM_ITEMS_RANDOM), (int)customItemsRandom);
			}

			void inline SetItemEnabled(EItemSlot item, bool isRandom, bool enabled) {
				if (isRandom)
					customItemsRandom = (customItemsRandom & ~(1 << (u32)item)) | ((enabled ? 1 : 0) << (u32)item);
				else
					customItemsCustom = (customItemsCustom & ~(1 << (u32)item)) | ((enabled ? 1 : 0) << (u32)item);
			}
			bool inline GetItemEnabled(EItemSlot item, bool isRandom) {
				if (isRandom)
					return customItemsRandom & (1 << (u32)item);
				else
					return customItemsCustom & (1 << (u32)item);
			}
		};
		static std::vector<CustomIcon> cupIcons;
		static CustomIcon DisabledIcon;
		static SettingsObject* SetObject;
		static bool IsVersusMode;
		static u32 versusCupTable[32];
		static int selectedCupButtonKeyboard;
		static u8 currentVersusCupTableEntry;

		static void Initialize();
		static void InitializeText();
		static void PopulateCupKbdForPage(std::vector<CustomIcon>& icons, int page);
		static u32 OpenCupKeyboard(int* cupButton, int* selectedCupButton, std::string& topMessage);
		static u32 OpenCourseKeyboard(u32 cupId, bool canAbort, const std::string& topMessage = "", void(*event)(Keyboard&, KeyboardEvent&) = nullptr);
		static u32 OpenCupCourseKeyboard(int* startingCupButton, int* selectedCupButton, bool canAbort, std::string& topText);
		static void ApplyVSModeSettings();
		static void ApplySpecifiedSetting(VSSettings setting, u32 value);
		static void OpenSettingsKeyboardCallback();
		static void OpenSettingsKeyboard();
		static bool IsRandomCourseMode();
		static void GenerateVersusCupTable(u32 start);
		static void OnCupSelectCallback();
		static void OnCupSelect(u32 cupID);
		static void OnNextTrackLoadCallback();
		static void OnNextTrackLoad();
		static void OnNextMenuShow();

		static u32 openSettingsKeyboardMode;
		static bool canShowVSSet;
		static u32 MenuSingleClassPage;
		static u32 invalidLRVal3;
		static void OnMenuSingleEnterCallback();
		
		static void (*ApplyVSSetting)(SettingsObject* set);

		static void OpenItemSelectorMenu(bool isRandom);
		static bool neeedsCustomItemHandling();

	private:
		struct KeyboardArguments {
			int* startCupButton;
			std::string* topMessage;
			int closedByScroll;
			int lastSelectedIndex;
		};
		static KeyboardArguments keyboardArgs;
		static void CupKeyboardCallback(Keyboard& k, KeyboardEvent& event);
		static void getSettingsKeyboardText(std::string& string);
		static void getCupKeyboardText(std::string& string, u32 startCup, std::string& topMessage);
		static std::vector<std::string> settingsOpts;
		static std::vector<std::string> cpudifficulty;
		static std::vector<std::string> courseOpts;
		static std::vector<std::string> itemOpts;
		static std::vector<std::string> itemNames;
		static std::string textRandom;
	};
}
