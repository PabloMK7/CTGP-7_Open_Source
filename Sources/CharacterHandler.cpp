/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CharacterHandler.cpp
Open source lines: 2146/2149 (99.86%)
*****************************************************/

#include "CharacterHandler.hpp"
#include "ExtraResource.hpp"
#include "TextFileParser.hpp"
#include "Lang.hpp"
#include "string.h"
#include "string_view"
#include "SaveHandler.hpp"
#include "CwavReplace.hpp"
#include "MissionHandler.hpp"
#include "str16utils.hpp"
#include "MenuPage.hpp"
#include "VoiceChatHandler.hpp"
#include "StatsHandler.hpp"

namespace CTRPluginFramework {
	BootSceneHandler::ProgressHandle CharacterHandler::progressHandle;
    std::vector<std::string> CharacterHandler::pendingDirs;
	CharacterHandler::HeapVoiceMode CharacterHandler::heapVoiceMode = CharacterHandler::HeapVoiceMode::NONE;
    RT_HOOK CharacterHandler::getCharaTextureNameHook = {0};
	RT_HOOK CharacterHandler::kartUnitConstructorHook = {0};
    RT_HOOK CharacterHandler::menu3DLoadResourceTaskHook = {0};
	RT_HOOK CharacterHandler::kartDirectorCreateBeforeStructHook = {0};
	RT_HOOK CharacterHandler::previewPartsWingReplaceColorHook = {0};
    Color4** CharacterHandler::menuStdWingPoleColors = nullptr;
    Color4** CharacterHandler::menuStdWingSheetColors = nullptr;
    Color4* CharacterHandler::raceStdWingPoleColors = nullptr;
    Color4* CharacterHandler::raceStdWingSheetColors = nullptr;
	GameAlloc::BasicHeap CharacterHandler::soundHeap;
	Mutex CharacterHandler::characterHandlerMutex;

	const s8 CharacterHandler::soundGolBankOffset[EDriverID::DRIVER_SIZE] = {
		90,
		91,
		93,
		96,
		-1,
		95,
		86,
		84,
		85,
		-1,
		-1,
		88,
		94,
		33,
		89,
		87,
		97,
		92
	};
	const s8 CharacterHandler::soundBankOffset[EDriverID::DRIVER_SIZE] = {
		14,
		16,
		21,
		27,
		20,
		25,
		6,
		2,
		4,
		-1,
		-1,
		10,
		23,
		31,
		12,
		8,
		29,
		18
	};

	const s8 CharacterHandler::soundBankCount[EDriverID::DRIVER_SIZE] = {
		31,
		31,
		31,
		31,
		18,
		31,
		31,
		29,
		31,
		0,
		0,
		31,
		31,
		25,
		30,
		31,
		34,
		29
	};

	const s8 CharacterHandler::soundMenuFileID[EDriverID::DRIVER_SIZE] = {
		24,
		16,
		26,
		36,
		20,
		40,
		10,
		6,
		8,
		-1,
		-1,
		14,
		28,
		0,
		18,
		22,
		38,
		12,
	};

    const char* CharacterHandler::shortNames[EDriverID::DRIVER_SIZE] =	{
		"bw",
		"ds",
		"dk",
		"hq",
		"kt",
		"lk",
		"lg",
		"mr",
		"mtl",
		"mii",
		"mii",
		"pc",
		"rs",
		"sh",
		"td",
		"wr",
		"wig",
		"ys"
	};
	const char* CharacterHandler::selectBclimNames[EDriverID::DRIVER_SIZE] = {
		"bowser",
		"daisy",
		"donkey",
		"honeyQueen",
		"koopaTroopa",
		"lakitu",
		"luigi",
		"mario",
		"metal",
		"",
		"",
		"peach",
		"rosalina",
		"sh_red",
		"toad",
		"wario",
		"wiggler",
		"yoshi"
	};
	const u8 CharacterHandler::wingColorOffset[EDriverID::DRIVER_SIZE] = {
		5,
		9,
		6,
		10,
		7,
		12,
		1,
		0,
		14,
		16,
		16,
		2,
		13,
		15,
		3,
		8,
		11,
		4,
	};
	const int CharacterHandler::msbtOrder[EDriverID::DRIVER_SIZE] = {
		4,
		9,
		5,
		13,
		7,
		10,
		1,
		0,
		14,
		15,
		15,
		2,
		11,
		16,
		6,
		8,
		12,
		3
	};

	
	const char* CharacterHandler::bodyNames[EBodyID::BODY_SIZE] = {
		"std",
		"rally",
		"rbn",
		"egg",
		"dsh",
		"cuc",
		"kpc",
		"boat",
		"hny",
		"sabo",
		"gng",
		"pipe",
		"trn",
		"cld",
		"race",
		"jet",
		"gold",
	};
	const char* CharacterHandler::tireNames[ETireID::TIRE_SIZE] = {
		"std",
		"big",
		"small",
		"race",
		"classic",
		"sponge",
		"gold",
		"wood",
		"bigRed",
		"mush",
	};
	const char* CharacterHandler::wingNames[EWingID::WING_SIZE] = {
		"std",
		"para",
		"umb",
		"flower",
		"basa",
		"met",
		"gold",
	};
	const char* CharacterHandler::screwNames[EScrewID::SCREW_SIZE] = {
		"std",
	};
	const EDriverID CharacterHandler::specialPartsDriver[3] = {
		EDriverID::DRIVER_DAISY,
		EDriverID::DRIVER_ROSALINA,
		EDriverID::DRIVER_HONEYQUEEN,
	};
	const char* CharacterHandler::bodyUINames[EBodyID::BODY_SIZE] = {
		"std",
		"rally",
		"rib",
		"egg",
		"dsh",
		"cuc",
		"kpc",
		"bot",
		"hny",
		"sab",
		"gng",
		"pip",
		"trn",
		"cld",
		"rac",
		"jet",
		"gld",
	};
	const char* CharacterHandler::tireUINames[ETireID::TIRE_SIZE] = {
		"std",
		"big",
		"small",
		"rac",
		"cls",
		"spg",
		"gld",
		"wod",
		"red",
		"mus",
	};
	const char* CharacterHandler::wingUINames[EWingID::WING_SIZE] = {
		"std",
		"par",
		"umb",
		"flw",
		"bas",
		"met",
		"gld",
	};
	const u8 CharacterHandler::bodyUIHasDriver[EBodyID::BODY_SIZE] = {
		1,
		0,
		2,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		0,
		0,
		0,
		0,
		0,
	};
	const u8 CharacterHandler::wingUIHasDriver[EWingID::WING_SIZE] = {
		1,
		0,
		2,
		0,
		0,
		0,
		0,
	};

	bool CharacterHandler::updateRaceCharaNamePending = false;
	std::array<u64, 8> CharacterHandler::selectedCharaceters{};
	u64 CharacterHandler::selectedMenuCharacter{};
	std::vector<std::string> CharacterHandler::authorNames;
	std::array<std::string, EDriverID::DRIVER_SIZE> CharacterHandler::origCharNames;
    std::unordered_map<u64, CharacterHandler::CharacterEntry> CharacterHandler::charEntries;
	std::array<std::vector<CharacterHandler::CharacterEntry*>, EDriverID::DRIVER_SIZE> CharacterHandler::charEntriesPerDriverID{};
	std::array<std::array<u8*, 8>, 4> CharacterHandler::textureDataPointers{};
    std::array<u8*, EDriverID::DRIVER_SIZE> CharacterHandler::menuSelectTextureDataPointers{};
    std::array<u8*, EDriverID::DRIVER_SIZE> CharacterHandler::originalSelectUIfiles;
	CharacterHandler::CharacterEntry CharacterHandler::defaultCharEntry{};
	CharacterHandler::FileLoadInfo CharacterHandler::lastFileLoadInfo{};
	std::string CharacterHandler::potentialCrashReason = "";
	bool CharacterHandler::customKartsEnabled = false;
	NetHandler::RequestHandler CharacterHandler::netRequestHandler;
	bool CharacterHandler::onlineLockUsed = false;
    LightEvent CharacterHandler::onlineCharacterFetchedEvent;

	struct ipsColorFile {
		char magic[5];
		static constexpr const char* MAGIC_VAL = "PATCH";

		typedef struct ipsColorEntry_s {
			u8 pos[3];
			u8 size[2];
			u8 color[4];
		} ipsColorEntry;
		
		ipsColorEntry poleRed;
		ipsColorEntry poleGreen;
		ipsColorEntry poleBlue;
		
		ipsColorEntry mainRed;
		ipsColorEntry mainGreen;
		ipsColorEntry mainBlue;
		
		char eof[3];
		static constexpr const char* EOF_VAL = "EOF";
	};

	void CharacterHandler::RegisterProgress() {
	#ifndef LOAD_MINIMUM_RESOURCES
		Directory charRootDir("/CTGP-7/MyStuff/Characters");
		if (!charRootDir.IsOpen()) return;
        charRootDir.ListFiles(pendingDirs, ".chpack");
	#endif
		progressHandle = BootSceneHandler::RegisterProgress(pendingDirs.size());
	}


	static std::unordered_map<std::string, int> g_groupMap;
	static int g_currentGroup = 0;
	static Mutex g_processMutex;
	void CharacterHandler::ProcessChPack(const std::string& it) {
		auto getIDFromGroup = [](std::unordered_map<std::string, int>& map, const std::string& g, int current) {
			if (g.empty())
				return current;
			if (map.contains(g)) {
				return map[g];
			} else {
				map[g] = current;
				return current;
			}
		};

		auto setGliderColor = [](CharacterEntry& entry, ExtraResource::StreamedSarc* sarc) {
			entry.usesCustomWingColor = false;
			std::vector<u8> out_file;
			if (sarc->ReadFullFile<std::vector<u8>>(out_file, "stdWingColor.ips") && out_file.size() == sizeof(ipsColorFile)) {
				ipsColorFile* colorFile = (ipsColorFile*)out_file.data();
				if (!memcmp(colorFile->magic, ipsColorFile::MAGIC_VAL, sizeof(ipsColorFile::magic)) && 
					!memcmp(colorFile->eof, ipsColorFile::EOF_VAL, sizeof(ipsColorFile::eof))) {
						entry.stdWingColorPole = Color4(*(float*)colorFile->poleRed.color, *(float*)colorFile->poleGreen.color, *(float*)colorFile->poleBlue.color);
						entry.stdWingColorSheet = Color4(*(float*)colorFile->mainRed.color, *(float*)colorFile->mainGreen.color, *(float*)colorFile->mainBlue.color);
						entry.usesCustomWingColor = true;
					}
			}
		};

		std::string charRootDir("/CTGP-7/MyStuff/Characters");
		const std::string nameStr = std::string("name_") + Language::GetCurrLangID();

		std::shared_ptr<ExtraResource::StreamedSarc> chpack = std::make_shared<ExtraResource::StreamedSarc>(charRootDir + "/" + it, 0x4000);
		if (!chpack->processed) return;
		u64 id = chpack->GetNonSecureDataChecksum(0x20000);
		if (id == 0) return;

		TextFileParser parser;
		{
			std::string configFileData;
			if (!chpack->ReadFullFile<std::string>(configFileData, "config.ini")) return;
			if (!parser.ParseLines(configFileData)) return;
		}
		
		CharacterEntry newEntry;
		newEntry.id = id;
		newEntry.authors.clear();
		newEntry.longName = parser.getEntry(nameStr, false);
		newEntry.shortName = parser.getEntry(nameStr, true);
		newEntry.selectAllowed = true;
		std::string charID = parser.getEntry("origChar", false);
		newEntry.origChar = EDriverID::DRIVER_SIZE;
		for (int j = 0; j < EDriverID::DRIVER_SIZE; j++) {
			if (charID.compare(shortNames[j]) == 0) {
				newEntry.origChar = (EDriverID)j;
				break;
			}
		}
		if (newEntry.origChar == EDriverID::DRIVER_SIZE) return;
		if (newEntry.longName.empty() || newEntry.shortName.empty()) {
			newEntry.longName = parser.getEntry("name_ENG", false);
			newEntry.shortName = parser.getEntry("name_ENG", true);
			if (newEntry.longName.empty() || newEntry.shortName.empty()) return;
		}

		newEntry.faceRaiderOffset = 0;
		std::string faceRaiderFaceOffset = parser.getEntry("faceRaiderFaceOffset");
		if (!faceRaiderFaceOffset.empty() && TextFileParser::IsValidNumber(faceRaiderFaceOffset) != TextFileParser::NumberType::INVALID) {
			newEntry.faceRaiderOffset = std::stoi(faceRaiderFaceOffset, nullptr, 16);
			std::string base_filename = chpack->GetFileName().substr(chpack->GetFileName().find_last_of("/") + 1);
			std::string::size_type const p(base_filename.find_last_of('.'));
			std::string file_without_extension = base_filename.substr(0, p);
			newEntry.faceRainderBinDataPath = "/CTGP-7/MyStuff/Characters/" + file_without_extension + "/Faces/currFaceData.bin";
		}
		
		newEntry.creditsAllowed = parser.getEntry("allowThankYou") == "true";
		newEntry.rotateChara = parser.getEntry("rotate") == "true";

		newEntry.achievementLevel = 0;
		std::string achievementLevel = parser.getEntry("achievementLevel");
		if (!achievementLevel.empty()) {
			int level = 0;
			if (Utils::ToInteger(achievementLevel, level) == Utils::ConvertResult::SUCCESS && level > 0)
				newEntry.achievementLevel = level;
		}
		newEntry.specialAchievement = SaveHandler::SpecialAchievements::NONE;
		std::string specialAchievement = parser.getEntry("specialAchievement");
		if (!specialAchievement.empty()) {
			int level = -1;
			if (Utils::ToInteger(specialAchievement, level) == Utils::ConvertResult::SUCCESS && level >= 0 && level < SaveHandler::TOTAL_SPECIAL_ACHIEVEMENTS)
				newEntry.specialAchievement = (SaveHandler::SpecialAchievements)level;
		}

		newEntry.disableAngry = newEntry.origChar == EDriverID::DRIVER_WIGGLER && parser.getEntry("disableAngry", false) == "true";
		
		auto authorList = parser.getEntries("authors");
		for (auto it = authorList.cbegin(); it != authorList.cend(); it++) {
			newEntry.authors.push_back(InsertAuthor(*it));
		}
		
		setGliderColor(newEntry, chpack.get());
		chpack->SetEnabled(false);

		std::string group = parser.getEntry("group");

		{
			Lock lock(g_processMutex);
			newEntry.groupID = getIDFromGroup(g_groupMap, group, g_currentGroup++);
			CharacterEntry& ent = charEntries[id];
			ent = newEntry;
			ent.chPack = chpack;
		}

	}

    void CharacterHandler::Initialize() {
		if (File::Exists("/CTGP-7/config/forcePatch.flag"))
		{
			File::Remove("/CTGP-7/config/forcePatch.flag");
			Directory::Remove("/CTGP-7/gamefs/Driver");
			Directory::Remove("/CTGP-7/gamefs/Kart");
			for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
				std::string shName = shortNames[i];
				std::transform(shName.begin(), shName.end(), shName.begin(), toupper);
				File::Remove("/CTGP-7/gamefs/Sound/extData/GRP_VO_" + shName + "_GOL.bcgrp");
			}
			File::Remove("/CTGP-7/gamefs/Sound/ctr_dash.bcsar");
			File::Remove("/CTGP-7/gamefs/Sound/extData/GRP_VO_MENU.bcgrp");
			File::Remove("/CTGP-7/gamefs/Driver/common/wing_std_color.bcmcla");
		}		

		
		TaskFunc func = [](void* arg) -> s32 {
			for (int i = *(int*)arg; i < pendingDirs.size(); i+=2, BootSceneHandler::Progress(progressHandle)) {
				ProcessChPack(pendingDirs.at(i));
			}
			return 0;
		};

		int task1arg = 0, task2arg = 1;
		#if CITRA_MODE == 0
		Task::Affinity task1Aff = Task::Affinity::AppCores, task2Aff = Task::Affinity::AppCores;
		#else
		Task::Affinity task1Aff = Task::Affinity::SysCore, task2Aff = Task::Affinity::AppCore;
		#endif

		Task task1(func, &task1arg, task1Aff);
		Task task2(func, &task2arg, task2Aff);
		task1.Start();
		task2.Start();
		task1.Wait();
		task2.Wait();

		pendingDirs.clear();
		SaveHandler::UpdateCharacterIterator();
		LoadDisabledChars();
		PopulateAvailableCharacters();
		LightEvent_Init(&onlineCharacterFetchedEvent, RESET_STICKY);
    }

	void CharacterHandler::PopulateAvailableCharacters() {
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			charEntriesPerDriverID[i].clear();
		}
		for (auto it = charEntries.begin(); it != charEntries.end(); it++) {
			CharacterEntry& entry = it->second;
			if (entry.selectAllowed && (entry.achievementLevel == 0 || entry.achievementLevel <= SaveHandler::saveData.GetCompletedAchievementCount())
									&& (entry.specialAchievement == SaveHandler::SpecialAchievements::NONE || SaveHandler::saveData.IsSpecialAchievementCompleted(entry.specialAchievement)))
				charEntriesPerDriverID[entry.origChar].push_back(&entry);
		}
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			std::sort(charEntriesPerDriverID[i].begin(), charEntriesPerDriverID[i].end(), [](const CharacterEntry* e1, const CharacterEntry* e2) {
				return e1->groupID < e2->groupID;
			});
		}
		LoadCharacterChoices();
	}

	void CharacterHandler::UpdateMsbtPatches() {
		if (!Language::msbtReady)
			return;
		if (origCharNames[0].empty()) for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			origCharNames[i] = Language::MsbtHandler::GetString(1000 + msbtOrder[i]);
			Language::MsbtHandler::SetString(1000 + msbtOrder[i], 
				Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(245, 245, 245)) + 
				Language::MsbtHandler::DashTextWithTagsToString(Language::MsbtHandler::GetText(1000 + msbtOrder[i]))
			);
		}
	}

	u64 CharacterHandler::GetSelectedCharacter(int playerID) {
		if (MarioKartFramework::isLoadingAward) {
			CRaceInfo& raceInfo = *MarioKartFramework::getRaceInfo(false);
			if (playerID < 3) {
				for (int i = 0; i < raceInfo.playerAmount; i++) {
					if (raceInfo.kartInfos[i].totalUniqueRaceRank - 1 == playerID) {
						return selectedCharaceters[i];
					}
				}
			}
		}
		return selectedCharaceters[playerID];
	}

	u64 CharacterHandler::SelectRandomCharacter(EDriverID driverID, bool includeVanilla) {
		auto& entries = charEntriesPerDriverID[(int)driverID];
		if (entries.size() == 0)
			return 0;
		std::map<int, std::vector<CharacterEntry*>> entriesPerGroup;
		for (auto it = entries.begin(); it != entries.end(); it++) {
			entriesPerGroup[(*it)->groupID].push_back(*it);
		}
		u32 max = entriesPerGroup.size() - (includeVanilla ? 0 : 1);
		u32 choice = max == 0 ? 0 : Utils::Random(0, max);
		if (includeVanilla) {
			if (choice == 0) {
				return 0;
			}
			choice--;
		}
		auto it = entriesPerGroup.begin();
		std::advance(it, choice);
		max = it->second.size() - 1;
		u32 choice2 = max == 0 ? 0 : Utils::Random(0, max);
		return it->second[choice2]->id;
	}

	static bool confirmcharacters_wasdemo = false;
	void CharacterHandler::ConfirmCharacters() {
		MarioKartFramework::allowWigglerAngry = true;
		MarioKartFramework::rotateCharacterID = 0;
		if (!MarioKartFramework::isLoadingAward) do {
			if (g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD) {
				ApplyOnlineSelectedCharacters();
			} else {
				CRaceInfo& raceInfo = *MarioKartFramework::getRaceInfo(true);

				if (MissionHandler::isMissionMode || raceInfo.raceMode.mode == 4 || MarioKartFramework::isWatchRaceMode) return;

				if (raceInfo.raceMode.mode == 0 && confirmcharacters_wasdemo) {
					confirmcharacters_wasdemo = false;
					break;
				}
				if (raceInfo.raceMode.mode == 5) {
					confirmcharacters_wasdemo = true;
				}

				bool isTitleDemo = raceInfo.raceMode.type == 3 && (raceInfo.raceMode.mode == 6 || raceInfo.raceMode.mode == 7);

				if (!isTitleDemo)
					selectedCharaceters[MarioKartFramework::masterPlayerID] = selectedMenuCharacter;
				if (MarioKartFramework::isFirstRace() && raceInfo.raceMode.mode != 1) {
					for (int i = 0; i < raceInfo.playerAmount; i++) {
						if (i == MarioKartFramework::masterPlayerID && !isTitleDemo)
							continue;
						if (raceInfo.raceMode.type == 0 || (raceInfo.raceMode.mode >= 5 && raceInfo.raceMode.mode <= 7)) {
							u64 choice = SelectRandomCharacter(raceInfo.kartInfos[i].driverID, true);
							if (choice > 0) {
								selectedCharaceters[i] = choice;
								updateRaceCharaNamePending = true;
							}
							else
								selectedCharaceters[i] = 0;
						}
					}
				}
			}
		} while (false);
		for (int i = 0; i < 8; i++) {
			auto it = charEntries.find(selectedCharaceters[i]);
			if (it == charEntries.end())
				continue;
			if (it->second.disableAngry)
				MarioKartFramework::allowWigglerAngry = false;
			if (it->second.rotateChara && MarioKartFramework::rotateCharacterID == 0 && (i == MarioKartFramework::masterPlayerID || MarioKartFramework::isLoadingAward))
				MarioKartFramework::rotateCharacterID = selectedCharaceters[i];
		}
		DisableOnlineLock();
	}

	void CharacterHandler::ResetCharacters() {
		updateRaceCharaNamePending = false;
		for (int i = 0; i < 8; i++)
			selectedCharaceters[i] = 0;
	}

	void CharacterHandler::EnableOnlineLock() {
		if (!onlineLockUsed) {
			onlineLockUsed = true;
			LightEvent_Clear(&onlineCharacterFetchedEvent);
		}
	}

	void CharacterHandler::DisableOnlineLock() {
		if (onlineLockUsed) {
			onlineLockUsed = false;
			LightEvent_Signal(&onlineCharacterFetchedEvent);
		}
	}

	void CharacterHandler::WaitOnlineLock() {
		if (onlineLockUsed)
			LightEvent_Wait(&onlineCharacterFetchedEvent);
	}

	void CharacterHandler::SaveCharacterChoices() {
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			u64 selectedDriver = MenuPageHandler::MenuCharaBasePage::GetSelectedCustomCharacterID((EDriverID)i);
			SaveHandler::saveData.driverChoices[i] = selectedDriver;
		}
		SaveHandler::SaveSettingsAll();
	}

	void CharacterHandler::LoadCharacterChoices() {
		memset(MenuPageHandler::MenuCharaBasePage::currentChoices.data(), 0, sizeof(MenuPageHandler::MenuCharaBasePage::currentChoices));
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			u64 savedCharacter = SaveHandler::saveData.driverChoices[i];
			if (charEntries.find(savedCharacter) != charEntries.end()) {
				auto it = charEntriesPerDriverID[i].begin();
				for (; it != charEntriesPerDriverID[i].end(); it++) {
					if ((*it)->id == savedCharacter)
						break;
				}
				if (it != charEntriesPerDriverID[i].end())
					MenuPageHandler::MenuCharaBasePage::currentChoices[i] = (it - charEntriesPerDriverID[i].begin()) + 1;
			}
		}
	}

	void CharacterHandler::LoadDisabledChars() {
		for (auto it = SaveHandler::saveData.disabledChars.begin(); it != SaveHandler::saveData.disabledChars.end(); it++) {
			auto it2 = charEntries.find(*it);
			if (it2 != charEntries.end())
				it2->second.selectAllowed = false;
		}
	}

	void CharacterHandler::SaveDisabledChars() {
		SaveHandler::saveData.disabledChars.clear();
		for (auto it = charEntries.begin(); it != charEntries.end(); it++) {
			if (!it->second.selectAllowed)
				SaveHandler::saveData.disabledChars.push_back(it->first);
		}
	}

    u16 CharacterHandler::InsertAuthor(const std::string& author) {
        auto it = std::find(authorNames.begin(), authorNames.end(), author);
        if (it == authorNames.end())
            it = authorNames.insert(it, author);
        return it - authorNames.begin();
    }

	std::string CharacterHandler::GetAuthor(u16 index) {
		if (index < authorNames.size())
			return authorNames[index];
		return "";
	}

	std::vector<std::string> CharacterHandler::GetAllAuthors() {
		std::vector<int> freq(authorNames.size(), 0);
        for (auto it = charEntries.cbegin(); it != charEntries.cend(); it++) {
            for (auto it2 = it->second.authors.cbegin(); it2 != it->second.authors.cend(); it2++) {
                freq[*it2]++;
            }
        }

        std::vector<std::pair<std::string, int>> pairs;
        for (int i = 0; i < authorNames.size(); i++) {
			if (authorNames[i].empty())
                continue;
            pairs.push_back(std::make_pair(authorNames[i], freq[i]));
        }

        std::sort(pairs.begin(), pairs.end(), [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            return a.second > b.second;
        });

        std::vector<std::string> sorted_strings;
        for (const auto& pair : pairs) {
            sorted_strings.push_back(pair.first);
        }
        return sorted_strings;
	}

    void CharacterHandler::OnGetCharaTextureName(FixedStringBase<char, 64>& outName, ECharaIconType iconType, EDriverID* driverID, int playerID) {
        ((void(*)(FixedStringBase<char, 64>&, ECharaIconType, EDriverID*, int))getCharaTextureNameHook.callCode)(outName, iconType, driverID, playerID);
		if (iconType != ECharaIconType::CHARAICONTYPE_SELECT) {
			struct AppendedInfo {
				u8 iconTypePlusOne;
				u8 driverIDPlusOne;
				u8 playerIDPlusOne;
				u8 alwaysFF;
				u8 always00;
			};
			AppendedInfo info;
			info.iconTypePlusOne = iconType + 1;
			info.driverIDPlusOne = *driverID + 1;
			info.playerIDPlusOne = playerID + 1;
			info.alwaysFF = 0xFF;
			info.always00 = 0x00;
			strcat(outName.strData, (char*)&info);
		}
    }

	u8* CharacterHandler::OnGetCharaTexture(SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo) {
		std::string_view fileName((char*)file->data);
		struct AppendedInfo {
            u8 iconTypePlusOne;
            u8 driverIDPlusOne;
            u8 playerIDPlusOne;
            u8 alwaysFF;
        };
		if (fileName.size() < sizeof(AppendedInfo)) {
			return nullptr;
		}
        AppendedInfo* info = (AppendedInfo*)(fileName.data() + (fileName.size() - sizeof(AppendedInfo)));
		if (info->alwaysFF != 0xFF) {
			return nullptr;
		}

		WaitOnlineLock();

		ECharaIconType iconType = (ECharaIconType)(info->iconTypePlusOne - 1);
		EDriverID driverID = (EDriverID)(info->driverIDPlusOne - 1);
		int playerID = info->playerIDPlusOne - 1;

		u64 selectedID = (iconType == ECharaIconType::CHARAICONTYPE_RANKMENU && playerID == MarioKartFramework::masterPlayerID) ? GetSelectedMenuCharacter() : GetSelectedCharacter(playerID);
		auto it = charEntries.find(selectedID);
		if (it != charEntries.end() && it->second.origChar == driverID) {
			Lock l(characterHandlerMutex);
			CharacterEntry& entry = it->second;
			entry.chPack->SetEnabled(true);
			const char* fName = GetCharaIconBCLIMName(iconType);
			ExtraResource::SARC::FileInfo dstFileInfo;
			dstFileInfo.fileSize = GetCharaIconTextureSize(iconType) + CryptoResource::cryptoResourceHeaderSize;
			u8* dst = GetTexturePointer(iconType, playerID);
			bool good = entry.chPack->ReadFile(dst, dstFileInfo, fName);
			entry.chPack->SetEnabled(false);
			if (good) {
				if (fileInfo) fileInfo->fileSize = dstFileInfo.fileSize;
				return dst;
			}
		}
		// Set iconTypePlusOne to NULL to terminate the string
		info->iconTypePlusOne = '\0';
		return nullptr;
	}

	void CharacterHandler::setupExtraResource() {
		soundHeap = GameAlloc::BasicHeap(3'500'000);
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 4; j++) {
				GetTexturePointer((ECharaIconType)j, i);
			}
		}
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			GetMenuSelectTexturePointer((EDriverID)i);
		}
	}

	void CharacterHandler::applySarcPatches() {
		if (customKartsEnabled) {
			File kartFile("/CTGP-7/MyStuff/Karts/UI.sarc");
			ExtraResource::SARC* kartSARC = new ExtraResource::SARC(kartFile);
			if (!kartSARC->processed) {
				delete kartSARC;
			}
			else {
				ExtraResource::mainSarc->Append(kartSARC, true);
			}
		}
	}

	void CharacterHandler::UpdateMenuCharaText(EDriverID driverID, u64 characterID, bool isBlocked) {
		auto it = charEntries.find(characterID);
		Color defaultColor = isBlocked ? Color(255, 0, 0) : Color(245, 245, 245);
		if (it != charEntries.end() && it->second.origChar == driverID) {
			std::u16string longStr16;
			Utils::ConvertUTF8ToUTF16(longStr16, it->second.longName);
			if (!isBlocked && it->second.achievementLevel > 0 && it->second.achievementLevel <= SaveHandler::saveData.GetCompletedAchievementCount()) {
				if (it->second.achievementLevel >= 5)
					longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 0, 255)) + longStr16;
				else
					longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 255, 0)) + longStr16;
			} else if (!isBlocked && it->second.specialAchievement != SaveHandler::SpecialAchievements::NONE && SaveHandler::saveData.IsSpecialAchievementCompleted(it->second.specialAchievement)) {
				longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 255, 0)) + longStr16;
			} else {
				longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, defaultColor) + longStr16;
			}
			Language::MsbtHandler::SetString(1000 + msbtOrder[(int)driverID], longStr16);		
		} else {
			Language::MsbtHandler::RemoveAllString(1000 + msbtOrder[(int)driverID]);
			Language::MsbtHandler::SetString(1000 + msbtOrder[(int)driverID],
				Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, defaultColor) + 
				Language::MsbtHandler::DashTextWithTagsToString(Language::MsbtHandler::GetText(1000 + msbtOrder[(int)driverID]))
			);
		}
	}

	void CharacterHandler::UpdateRaceCharaText() {
		if (!updateRaceCharaNamePending)
			return;
		CRaceInfo& raceInfo = *MarioKartFramework::getRaceInfo(true);
		FixedStringBase<char16_t, 0x20>* playerNames = MarioKartFramework::getPlayerNames();
		for (int i = 0; i < raceInfo.playerAmount; i++) {
			if (raceInfo.kartInfos[i].playerType == EPlayerType::TYPE_CPU) {
				u64 selectedChar = GetSelectedCharacter(i);
				if (selectedChar) {
					auto it = charEntries.find(selectedChar);
					if (it == charEntries.end()) continue;
					std::u16string name;
					Utils::ConvertUTF8ToUTF16(name, it->second.shortName);
					strcpy16n(playerNames[i].strData, name.c_str(), playerNames[i].bufferSize * sizeof(u16));
				}
			}
		}
	}

	void CharacterHandler::UpdateMenuCharaTextures(EDriverID driverID, u8* texture, u8* texture_sh, bool forceReset) {
		u64 selectedID = GetSelectedMenuCharacter();
		auto it = forceReset ? charEntries.end() : charEntries.find(selectedID);
		bool pendingDefaultBclimCopy = true;
		constexpr u32 selectTextureSize = GetCharaIconTextureSize(ECharaIconType::CHARAICONTYPE_SELECT) - 0x28;
		constexpr u32 selectTextureFileSize = GetCharaIconTextureSize(ECharaIconType::CHARAICONTYPE_SELECT);
		constexpr u32 selectTextureFileSizeCrypto = GetCharaIconTextureSize(ECharaIconType::CHARAICONTYPE_SELECT) + CryptoResource::cryptoResourceHeaderSize;
		if (it != charEntries.end() && it->second.origChar == driverID) {
			Lock l(characterHandlerMutex);
			CharacterEntry& entry = it->second;
			entry.chPack->SetEnabled(true);
			ExtraResource::SARC::FileInfo fInfo;
			bool foundFile = entry.chPack->GetFileInfo(fInfo, GetCharaIconBCLIMName(ECharaIconType::CHARAICONTYPE_SELECT));
			if (foundFile) {
				if (fInfo.fileSize != selectTextureFileSize && fInfo.fileSize != selectTextureFileSizeCrypto) {
					panic(Utils::Format("Invalid select bclim size: %d", fInfo.fileSize).c_str());
				}
				ExtraResource::SARC::FileInfo dstFInfo = fInfo;
				pendingDefaultBclimCopy = !entry.chPack->ReadFileDirectly(GetMenuSelectTexturePointer(driverID), dstFInfo, fInfo, true);
				if (!pendingDefaultBclimCopy) {
					memcpy(texture, GetMenuSelectTexturePointer(driverID), dstFInfo.fileSize);
					if (texture != texture_sh) memcpy(texture_sh, GetMenuSelectTexturePointer(driverID), dstFInfo.fileSize);
				}
			}
			entry.chPack->SetEnabled(false);
		}
		if (pendingDefaultBclimCopy) {
			u8* dest = texture;
			memcpy(dest, originalSelectUIfiles[(int)driverID], selectTextureSize);
			svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)dest & ~0xFFF), (selectTextureSize & ~0xFFF) + 0x2000);
			if (texture != texture_sh) {
				dest = texture_sh;
				memcpy(dest, originalSelectUIfiles[(int)driverID], selectTextureSize);
				svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)dest & ~0xFFF), (selectTextureSize & ~0xFFF) + 0x2000);
			}
		}
	}


	void* CharacterHandler::OnKartUnitConstructor(void* own, int playerID, CRaceInfo& raceInfo, bool forceHiRes) {
		/*bool isHiRes = forceHiRes || 
			(raceInfo.kartInfos[playerID].playerType == EPlayerType::TYPE_USER || raceInfo.kartInfos[playerID].playerType == EPlayerType::TYPE_UNKNOWN) ||
			raceInfo.masterID == playerID || raceInfo.raceMode.mode == 4;*/
		
		lastFileLoadInfo.allow = true;
		lastFileLoadInfo.isMenu = false;
		lastFileLoadInfo.playerID = playerID;

		bool restoreColors = false;
		Color4 backupPole;
		Color4 backupSheet;
		Color4* poleColors = raceStdWingPoleColors;
		Color4* sheetColors = raceStdWingSheetColors;
		u8 colorOffset = wingColorOffset[(int)raceInfo.kartInfos[playerID].driverID];
		
		u64 selectedID = GetSelectedCharacter(playerID);
		auto it = charEntries.find(selectedID);
		if (it != charEntries.end()) {
			if (it->second.usesCustomWingColor) {
				restoreColors = true;
				backupPole = poleColors[colorOffset];
				backupSheet = sheetColors[colorOffset];
				poleColors[colorOffset] = it->second.stdWingColorPole;
				sheetColors[colorOffset] = it->second.stdWingColorSheet;
			}
			potentialCrashReason = it->second.longName;
		} else {
			potentialCrashReason = shortNames[(int)raceInfo.kartInfos[playerID].driverID];
		}
		void* res = ((void*(*)(void*, int, CRaceInfo&, bool))kartUnitConstructorHook.callCode)(own, playerID, raceInfo, forceHiRes);
		SetupRaceVoices(((u32**)own)[0x2C/4], raceInfo);
		potentialCrashReason = "";
		lastFileLoadInfo.allow = false;
		if (restoreColors) {
			poleColors[colorOffset] = backupPole;
			sheetColors[colorOffset] = backupSheet;
		}
		return res;
	}

	void CharacterHandler::OnMenu3DLoadResourceTask(void* own) {
		SafeStringBase* loadName = (SafeStringBase*)((u32)own + 0x10);
		EDriverID outDriverID;
		bool isLod = false;
		ResourceInfo info = GetFileInfoFromPath(loadName->c_str());
		switch (info.kind)
		{
		case ResourceInfo::Kind::DRIVER_MODEL:
		case ResourceInfo::Kind::BODY_MODEL:
		case ResourceInfo::Kind::TIRE_MODEL:
		case ResourceInfo::Kind::WING_MODEL:
		case ResourceInfo::Kind::SCREW_MODEL:
		case ResourceInfo::Kind::EMBLEM_MODEL:
		{
			u64 selectedID = GetSelectedMenuCharacter();
			auto it = charEntries.find(selectedID);
			if (it != charEntries.end()) {
				potentialCrashReason = it->second.longName;
			} else {
				potentialCrashReason = "Unknown";
			}
			lastFileLoadInfo.allow = true;
			lastFileLoadInfo.isMenu = true;
			((void(*)(void*))menu3DLoadResourceTaskHook.callCode)(own);
			lastFileLoadInfo.allow = false;
			return;
		}
		default:
		{
			((void(*)(void*))menu3DLoadResourceTaskHook.callCode)(own);
			return;
		}
		}
	}

	void CharacterHandler::OnMenuCharaLoadSelectUI(u32* archive, SafeStringBase* file) {
		ResourceInfo info = GetFileInfoFromPath(file->c_str());
		if (info.kind != ResourceInfo::Kind::SELECT_UI || info.id < 0)
			return;
		
		ExtraResource::SARC::FileInfo newfInfo{0};
		u8* sarc = (u8*)(archive[0xE] - 0x14);
		ExtraResource::SARC menuSarc(sarc, false);
		u8* filePtr = menuSarc.GetFile(*file, &newfInfo);
		originalSelectUIfiles[info.id] = filePtr;
		size_t selectUISize = GetCharaIconTextureSize(ECharaIconType::CHARAICONTYPE_SELECT);
		size_t selectUISizeCrypto = selectUISize + CryptoResource::cryptoResourceHeaderSize;
		if (newfInfo.fileSize != selectUISize && newfInfo.fileSize != selectUISizeCrypto) {
			panic(Utils::Format("Invalid %s size: %d", file->c_str(), newfInfo.fileSize).c_str());
		}
	}

	static char onMenuCharaLoadKartUI_tmpbuf[0x100];
	void CharacterHandler::OnMenuCharaLoadKartUI(u32* archive, SafeStringBase* file) {
		ResourceInfo info = GetFileInfoFromPath(file->c_str());
		if (info.kind == ResourceInfo::Kind::BODY_UI) {
			if (bodyUIHasDriver[info.id]) {
				sprintf(onMenuCharaLoadKartUI_tmpbuf, "b_%s.bclim", bodyUINames[info.id]);
				file->data = onMenuCharaLoadKartUI_tmpbuf;
			}
		} else if (info.kind == ResourceInfo::Kind::WING_UI) {
			if (wingUIHasDriver[info.id]) {
				sprintf(onMenuCharaLoadKartUI_tmpbuf, "g_%s.bclim", wingUINames[info.id]);
				file->data = onMenuCharaLoadKartUI_tmpbuf;
			}
		}
	}

	void CharacterHandler::OnKartDirectorCreateBeforeStruct(u32* own, u32* args) {
		ClearAllVoices();
		heapVoiceMode = HeapVoiceMode::RACE;
		((void(*)(u32*, u32*))kartDirectorCreateBeforeStructHook.callCode)(own, args);
		CRaceInfo* raceInfo = (CRaceInfo*)(args[1]);
	}

	void CharacterHandler::OnPreviewPartsWingReplaceColor(void* own) {
		EDriverID driverID = (EDriverID)((u32*)own)[0x10/4];
		u64 selectedID = GetSelectedMenuCharacter();
		auto it = charEntries.find(selectedID);
		if (it != charEntries.end() && it->second.origChar == driverID && it->second.usesCustomWingColor) {
			u8 offset = wingColorOffset[(int)driverID];
			Color4* poleColors = menuStdWingPoleColors[1];
			Color4* sheetColors = menuStdWingSheetColors[1];
			Color4 backupPole = poleColors[offset];
			Color4 backupSheet = sheetColors[offset];
			poleColors[offset] = it->second.stdWingColorPole;
			sheetColors[offset] = it->second.stdWingColorSheet;
			((void(*)(void*))previewPartsWingReplaceColorHook.callCode)(own);
			poleColors[offset] = backupPole;
			sheetColors[offset] = backupSheet;
		} else
		{
			((void(*)(void*))previewPartsWingReplaceColorHook.callCode)(own);
		}
	}

	void* CharacterHandler::OnLoadResGraphicsFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs) {
		if (!lastFileLoadInfo.allow)
			return nullptr;
		
		ResourceInfo info = GetFileInfoFromPath(path->c_str(), loadArgs);
		switch (info.kind)
		{
		case ResourceInfo::Kind::DRIVER_MODEL:
		case ResourceInfo::Kind::BODY_MODEL:
		case ResourceInfo::Kind::TIRE_MODEL:
		case ResourceInfo::Kind::WING_MODEL:
		case ResourceInfo::Kind::SCREW_MODEL:
		case ResourceInfo::Kind::EMBLEM_MODEL:
		case ResourceInfo::Kind::THANK_YOU_ANIM:
			return OnLoadKartFile(resourceLoader, path, loadArgs, info);
		default:
			return nullptr;
		}
	}

	void CharacterHandler::OnThankYouScreen(bool isBefore) {
		if (!MenuPageHandler::MenuEndingPage::loadCTGPCredits)
			return;
		if (isBefore) {
			lastFileLoadInfo.allow = true;
			lastFileLoadInfo.isCredits = true;
		} else {
			lastFileLoadInfo.allow = false;
			lastFileLoadInfo.isCredits = false;
		}
	}

	static bool g_watchAchievCharsBackup[SaveHandler::TOTAL_ACHIEVEMENTS];
	static bool g_watchSpecialAchievCharsBackup[SaveHandler::TOTAL_SPECIAL_ACHIEVEMENTS];
	static bool g_watchAchievCharsPopulated = false;
	void CharacterHandler::OnWatchRaceEnabled(bool enabled) {
		if (enabled && !g_watchAchievCharsPopulated) {
			g_watchAchievCharsPopulated = true;
			for (int i = 0; i < SaveHandler::TOTAL_ACHIEVEMENTS; i++) {
				CryptoResource::KnownFileID id = (CryptoResource::KnownFileID)(0x1000 | i);
				g_watchAchievCharsBackup[i] = CryptoResource::GetKnownFileIDAllowed(id);
				CryptoResource::AllowKnownFileID(id, true);
			}
			g_watchSpecialAchievCharsBackup[(int)SaveHandler::SpecialAchievements::ALL_BLUE_COINS] = CryptoResource::GetKnownFileIDAllowed(CryptoResource::KnownFileID::ACHIEVEMENT_BLUE_COIN);
			CryptoResource::AllowKnownFileID(CryptoResource::KnownFileID::ACHIEVEMENT_BLUE_COIN, true);
		} else if (!enabled && g_watchAchievCharsPopulated) {
			g_watchAchievCharsPopulated = false;
			for (int i = 0; i < SaveHandler::TOTAL_ACHIEVEMENTS; i++) {
				CryptoResource::KnownFileID id = (CryptoResource::KnownFileID)(0x1000 | i);
				CryptoResource::AllowKnownFileID(id, g_watchAchievCharsBackup[i]);
				g_watchAchievCharsBackup[i] = false;
			}
			CryptoResource::AllowKnownFileID(CryptoResource::KnownFileID::ACHIEVEMENT_BLUE_COIN, g_watchSpecialAchievCharsBackup[(int)SaveHandler::SpecialAchievements::ALL_BLUE_COINS]);
			g_watchSpecialAchievCharsBackup[(int)SaveHandler::SpecialAchievements::ALL_BLUE_COINS] = false;
		}
	}

	static u64 g_ThankYouChosenCharacter = 0;
	void* CharacterHandler::OnLoadKartFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs, ResourceInfo& info) {
		
		const char* fileNames[7][3] = {
			{
				"driver.bcmdl",
				"driver_lod.bcmdl",
				""
			},
			{
				"body_%s.bcmdl",
				"body_%s_lod.bcmdl",
				"body_%s_shadow.bcmdl",
			},
			{
				"tire_%s.bcmdl",
				"tire_%s_lod.bcmdl",
				"tire_%s_shadow.bcmdl",
			},
			{
				"wing_%s.bcmdl",
				"wing_%s_lod.bcmdl",
				"",
			},
			{
				"screw_%s.bcmdl",
				"screw_%s_lod.bcmdl",
				"",
			},
			{
				"emblem.bcmdl",
				"emblem_lod.bcmdl",
				""
			},
			{
				"thankyou_anim.bcmdl",
				"",
				""
			}
		};
		const char** tables[7] = {
			nullptr,
			bodyNames,
			tireNames,
			wingNames,
			screwNames,
			nullptr,
			nullptr,
		};

		const char** currFileNames;
		const char** currTable;

		switch (info.kind)
		{
		case ResourceInfo::Kind::DRIVER_MODEL:
			currFileNames = fileNames[0];
			currTable = tables[0];
			break;
		case ResourceInfo::Kind::BODY_MODEL:
			currFileNames = fileNames[1];
			currTable = tables[1];
			break;
		case ResourceInfo::Kind::TIRE_MODEL:
			currFileNames = fileNames[2];
			currTable = tables[2];
			break;
		case ResourceInfo::Kind::WING_MODEL:
			currFileNames = fileNames[3];
			currTable = tables[3];
			break;
		case ResourceInfo::Kind::SCREW_MODEL:
			currFileNames = fileNames[4];
			currTable = tables[4];
			break;
		case ResourceInfo::Kind::EMBLEM_MODEL:
			currFileNames = fileNames[5];
			currTable = tables[5];
			break;
		case ResourceInfo::Kind::THANK_YOU_ANIM:
			currFileNames = fileNames[6];
			currTable = tables[6];
			break;
		default:
			return nullptr;
		}

		if (lastFileLoadInfo.isCredits && info.kind == ResourceInfo::Kind::EMBLEM_MODEL) {
			g_ThankYouChosenCharacter = SelectRandomCharacter((EDriverID)info.id, false);
		}

		WaitOnlineLock();

		u64 selectedID = lastFileLoadInfo.isCredits ? g_ThankYouChosenCharacter : (lastFileLoadInfo.isMenu ? GetSelectedMenuCharacter() : GetSelectedCharacter(lastFileLoadInfo.playerID));
		auto it = charEntries.find(selectedID);
		u8* buffer = nullptr;
		if (it != charEntries.end() && info.id >= 0) {
			Lock l(characterHandlerMutex);
			CharacterEntry& entry = it->second;
			entry.chPack->SetEnabled(true);
			ExtraResource::SARC::FileInfo fInfo {0};
			const char* fileName;
			if (info.attribute == ResourceInfo::Attribute::NONE)
				fileName = currFileNames[0];
			else if (info.attribute == ResourceInfo::Attribute::LOD)
				fileName = currFileNames[1];
			else if (info.attribute == ResourceInfo::Attribute::SHADOW)
				fileName = currFileNames[2];
			else
				fileName = "";
			bool alreadyFound = false;
			if (lastFileLoadInfo.isMenu && info.kind == ResourceInfo::Kind::DRIVER_MODEL) {
				alreadyFound = entry.chPack->GetFileInfo(fInfo, "driver_menu.bcmdl");
			}
			if ((alreadyFound || entry.chPack->GetFileInfo(fInfo, currTable ? Utils::Format(fileName, currTable[info.id]) : std::string(fileName))) && fInfo.fileSize != 0) {
				u32 align = loadArgs->alignment ? loadArgs->alignment : 4;
				void* heap = loadArgs->heap ? loadArgs->heap : GameAlloc::get_current_heap();
				buffer = (u8*)GameAlloc::game_operator_new(fInfo.fileSize, (u32*)loadArgs->heap, align);
				if (!entry.chPack->ReadFileDirectly(buffer, fInfo, fInfo)) {
					GameAlloc::game_operator_delete(buffer);
					buffer = nullptr;
				}
				if (info.kind == ResourceInfo::Kind::DRIVER_MODEL && info.attribute == ResourceInfo::Attribute::NONE &&
					entry.faceRaiderOffset && !entry.faceRainderBinDataPath.empty()) {
					File faceRaiderBin(entry.faceRainderBinDataPath, File::READ);
					if (faceRaiderBin.IsOpen() && faceRaiderBin.GetSize() == 0x8000)
						faceRaiderBin.Read(buffer + entry.faceRaiderOffset, 0x8000);
				}
			}
			entry.chPack->SetEnabled(false);
		}
		return buffer;
	}

	void CharacterHandler::ClearAllVoices() {
		heapVoiceMode = HeapVoiceMode::NONE;
		soundHeap.Clear();

		// Race
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			for (int isCannon = 0; isCannon < 2; isCannon++) {
				for (int j = 0; j < GetSoundWARCCount((EDriverID)i, isCannon); j++) {
					CwavReplace::SetReplacementCwav({GetSoundWARCID((EDriverID)i, isCannon), j}, nullptr);
				}
			}
			for (int j = 0; j < GetSoundGolWARCCount((EDriverID)i); j++) {
				CwavReplace::SetReplacementCwav({GetSoundGolWARCID((EDriverID)i), j}, nullptr);
			}
		}

		// Menu
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
            u32 archive = CharacterHandler::GetSoundMenuWARCID((EDriverID)i);
            u32 file1 = CharacterHandler::GetSoundMenuFileID((EDriverID)i, false);
            u32 file2 = CharacterHandler::GetSoundMenuFileID((EDriverID)i, true);
            if (file1 < 0)
                continue;
            CwavReplace::SetReplacementCwav({archive, file1}, nullptr);
            CwavReplace::SetReplacementCwav({archive, file2}, nullptr);
        }
	}

	void CharacterHandler::SetupRaceVoices(u32* vehicle, CRaceInfo& raceInfo) {
		WaitOnlineLock();
        int playerID = ((u32*)vehicle)[0x84/4];
		u64 selectedID = GetSelectedCharacter(playerID);
		auto it = charEntries.find(selectedID);
		if (it != charEntries.end()) {
			Lock l(characterHandlerMutex);
			it->second.chPack->SetEnabled(true);
			for (int isCannon = 0; isCannon < 2; isCannon++) {
				u32 archiveID = GetSoundWARCID(it->second.origChar, isCannon);
				if (archiveID == 0)
					continue;
				for (int j = 0; j < GetSoundWARCCount(it->second.origChar, isCannon); j++) {
					std::string soundFile = Utils::Format("SND_%02d_%02d.bcwav", archiveID & 0xFF, j);
					ExtraResource::SARC::FileInfo fInfo {0};
					if (it->second.chPack->GetFileInfo(fInfo, soundFile) && fInfo.fileSize != 0) { 
						u8* buffer = soundHeap.Alloc(fInfo.fileSize, 0x80);
						if (!buffer) {
							panic("Not enough memory for voices!");
						}
						if (it->second.chPack->ReadFileDirectly(buffer, fInfo, fInfo)) {
							// Sound handle is not available yet, store pointer to it instead
							u32* soundHandle = &((u32**)vehicle)[0x37][0xF4/4];
							CwavReplace::SetReplacementCwav({archiveID, j}, buffer, 0, 0, soundHandle, true);
						}
					}
				}
			}
			if (playerID == MarioKartFramework::masterPlayerID) {
				u32 archiveID = GetSoundGolWARCID(it->second.origChar);
				for (int j = 0; j < GetSoundGolWARCCount(it->second.origChar); j++) {
					std::string soundFile = Utils::Format("SND_%02d_%02d.bcwav", archiveID & 0xFF, j);
					ExtraResource::SARC::FileInfo fInfo {0};
					if (it->second.chPack->GetFileInfo(fInfo, soundFile) && fInfo.fileSize != 0) { 
						u8* buffer = soundHeap.Alloc(fInfo.fileSize, 0x80);
						if (!buffer) {
							panic("Not enough memory for voices!");
						}
						if (it->second.chPack->ReadFileDirectly(buffer, fInfo, fInfo)) {
							CwavReplace::SetReplacementCwav({archiveID, j}, buffer);
						}
					}
				}
			}
			it->second.chPack->SetEnabled(false);
		}
	}

	void CharacterHandler::SetupMenuVoices(u64 selectedEntry) {
		ClearAllVoices();
		heapVoiceMode = HeapVoiceMode::MENU;
		if (!selectedEntry)
			return;
		auto it = charEntries.find(selectedEntry);
		if (it != charEntries.end()) {
			u32 archive = CharacterHandler::GetSoundMenuWARCID(it->second.origChar);
            s32 selectFile = CharacterHandler::GetSoundMenuFileID(it->second.origChar, false);
            s32 goFile = CharacterHandler::GetSoundMenuFileID(it->second.origChar, true);
			if (selectFile >= 0) {
				Lock l(characterHandlerMutex);
				it->second.chPack->SetEnabled(true);
				std::string selectName = "SND_select.bcwav";
				std::string goName = "SND_go.bcwav";
				ExtraResource::SARC::FileInfo fInfo {0};
				if (it->second.chPack->GetFileInfo(fInfo, selectName) && fInfo.fileSize != 0) { 
					u8* buffer = soundHeap.Alloc(fInfo.fileSize, 0x80);
					if (!buffer) {
						panic("Not enough memory for voices!");
					}
					if (it->second.chPack->ReadFileDirectly(buffer, fInfo, fInfo)) {
						CwavReplace::SetReplacementCwav({archive, (u32)selectFile}, buffer);
					}
				}
				fInfo = ExtraResource::SARC::FileInfo{};
				if (it->second.chPack->GetFileInfo(fInfo, goName) && fInfo.fileSize != 0) { 
					u8* buffer = soundHeap.Alloc(fInfo.fileSize, 0x80);
					if (!buffer) {
						panic("Not enough memory for voices!");
					}
					if (it->second.chPack->ReadFileDirectly(buffer, fInfo, fInfo)) {
						CwavReplace::SetReplacementCwav({archive, (u32)goFile}, buffer);
					}
				}
				it->second.chPack->SetEnabled(false);
			}
		}
	}

	void CharacterHandler::StartFetchOnlineSelectedCharacters() {
		if (netRequestHandler.GetStatus() != NetHandler::Session::Status::IDLE)
			return;
		minibson::document reqDoc;
		reqDoc.set<u64>("gatherID", MarioKartFramework::currGatheringID);

		netRequestHandler.AddRequest(NetHandler::RequestHandler::RequestType::ROOM_CHAR_IDS, reqDoc);
		netRequestHandler.Start();
	}

	void CharacterHandler::ApplyOnlineSelectedCharacters() {
		ResetCharacters();
		CRaceInfo& raceInfo = *MarioKartFramework::getRaceInfo(true);
		
		if (!MarioKartFramework::isWatchRaceMode)
			selectedCharaceters[MarioKartFramework::masterPlayerID] = selectedMenuCharacter;

		while (!netRequestHandler.HasFinished())
			netRequestHandler.WaitTimeout(Seconds(0.1f));

		minibson::document resDoc;
		if (netRequestHandler.GetResult(NetHandler::RequestHandler::RequestType::ROOM_CHAR_IDS, &resDoc) == 0) {
			const minibson::document& charIDs = resDoc.get("charIDs", minibson::document());
			for (int i = 0; i < 8; i++) {
				if (!MarioKartFramework::isWatchRaceMode && i == MarioKartFramework::masterPlayerID)
					continue;
				u16 key = '0' + i;
				u64 customChar = charIDs.get_numerical((char*)&key);
				if (customChar == 0)
					continue;
				decltype(charEntries)::iterator it;
				if (((it = charEntries.find(customChar)) != charEntries.end()) && it->second.origChar == raceInfo.kartInfos[i].driverID)
					selectedCharaceters[i] = customChar;
			}
			if (VoiceChatHandler::Initialized) {
				const minibson::document& names = resDoc.get("names", minibson::document());
				for (int i = 0; i < 8; i++) {
					u16 key = '0' + i;
					Net::othersServerNames[i] = names.get((char*)&key, "");
				}
				VoiceChatHandler::UpdatePlayerNames();
			}
			const minibson::document& badgeIDs = resDoc.get("badgeIDs", minibson::document());
			for (int i = 0; i < 8; i++) {
				u16 key = '0' + i;
				Net::othersBadgeIDs[i] = badgeIDs.get_numerical((char*)&key, 0);
			}
			BadgeManager::UpdateOnlineBadges();
		}
		netRequestHandler.Cleanup();
	}

	CharacterHandler::ResourceInfo CharacterHandler::GetFileInfoFromPath(const char* path, ResourceLoaderLoadArg* loadArgs) {
		std::string_view p(path);
		constexpr const char* driver_pattern = "Driver/";
		constexpr const u32 driver_pattern_len = strlen(driver_pattern);
		constexpr const char* body_pattern = "Body/body_";
		constexpr const u32 body_pattern_len = strlen(body_pattern);
		constexpr const char* tire_pattern = "Tire/tire_";
		constexpr const u32 tire_pattern_len = strlen(tire_pattern);
		constexpr const char* wing_pattern = "Wing/wing_";
		constexpr const u32 wing_pattern_len = strlen(wing_pattern);
		constexpr const char* screw_pattern = "Screw/screw_";
		constexpr const u32 screw_pattern_len = strlen(screw_pattern);
		constexpr const char* select_pattern = "select_";
		constexpr const u32 select_pattern_len = strlen(select_pattern);
		constexpr const char* emblem_pattern = "Emblem/emblem_";
		constexpr const u32 emblem_pattern_len = strlen(emblem_pattern);
		constexpr const char* lod_pattern = "_lod";
		constexpr const u32 lod_pattern_len = strlen(lod_pattern);
		constexpr const char* ui_body_pattern = "b_";
		constexpr const u32 ui_body_pattern_len = strlen(ui_body_pattern);
		constexpr const char* ui_tire_pattern = "t_";
		constexpr const u32 ui_tire_pattern_len = strlen(ui_tire_pattern);
		constexpr const char* ui_wing_pattern = "g_";
		constexpr const u32 ui_wing_pattern_len = strlen(ui_wing_pattern);
		
		ResourceInfo res{};
		{
			// Driver folder
			auto driv_it = p.find(driver_pattern);
			if (driv_it != p.npos) {
				driv_it += driver_pattern_len;
				auto common_it = p.find("comm", driv_it);
				auto common_mimui_it = p.find("mim_s", driv_it);
				if (common_it == p.npos && common_mimui_it == p.npos) {
					std::string_view rem(p.data() + driv_it);
					for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
						if (rem.starts_with(shortNames[i])) {
							res.id = i;
							std::string_view rem2(rem.data() + strlen(shortNames[i]));
							res.attribute = rem2.starts_with("_lod") ? ResourceInfo::Attribute::LOD : ResourceInfo::Attribute::NONE;
							break;
						}
					}
					res.kind = ResourceInfo::Kind::DRIVER_MODEL;
					return res;
				}
			}
		}
		{
			// Body folder
			auto body_it = p.find(body_pattern);
			if (body_it != p.npos) {
				body_it += body_pattern_len;
				std::string_view rem(p.data() + body_it);
				for (int i = 0; i < EBodyID::BODY_SIZE; i++) {
					if (rem.starts_with(bodyNames[i])) {
						res.id = i;
						std::string_view rem2(rem.data() + strlen(bodyNames[i]));
						if (rem2.starts_with("_lod"))
							res.attribute = ResourceInfo::Attribute::LOD;
						else if (rem2.starts_with("_shadow"))
							res.attribute = ResourceInfo::Attribute::SHADOW;
						else
							res.attribute = ResourceInfo::Attribute::NONE;
						break;
					}
				}
				res.kind = ResourceInfo::Kind::BODY_MODEL;
				return res;
			}
		}
		{
			// Tire folder
			auto tire_it = p.find(tire_pattern);
			if (tire_it != p.npos) {
				tire_it += tire_pattern_len;
				std::string_view rem(p.data() + tire_it);
				for (int i = 0; i < ETireID::TIRE_SIZE; i++) {
					if (rem.starts_with(tireNames[i])) {
						res.id = i;
						std::string_view rem2(rem.data() + strlen(tireNames[i]));
						if (rem2.starts_with("_lod"))
							res.attribute = ResourceInfo::Attribute::LOD;
						else if (rem2.starts_with("_shadow"))
							res.attribute = ResourceInfo::Attribute::SHADOW;
						else
							res.attribute = ResourceInfo::Attribute::NONE;
						break;
					}
				}
				res.kind = ResourceInfo::Kind::TIRE_MODEL;
				return res;
			}
		}
		{
			// Wing folder
			auto wing_it = p.find(wing_pattern);
			if (wing_it != p.npos) {
				wing_it += wing_pattern_len;
				std::string_view rem(p.data() + wing_it);
				for (int i = 0; i < EWingID::WING_SIZE; i++) {
					if (rem.starts_with(wingNames[i])) {
						res.id = i;
						std::string_view rem2(rem.data() + strlen(wingNames[i]));
						if (rem2.starts_with("_lod"))
							res.attribute = ResourceInfo::Attribute::LOD;
						else
							res.attribute = ResourceInfo::Attribute::NONE;
						break;
					}
				}
				res.kind = ResourceInfo::Kind::WING_MODEL;
				return res;
			}
		}
		{
			// Screw folder
			auto screw_it = p.find(screw_pattern);
			if (screw_it != p.npos) {
				std::string_view rem(p.data() + screw_it);
				res.id = 0;
				res.attribute = rem.find("_lod") != rem.npos ? ResourceInfo::Attribute::LOD : ResourceInfo::Attribute::NONE;
				res.kind = ResourceInfo::Kind::SCREW_MODEL;
				return res;
			}
		}
		{
			// Emblem folder
			auto emblem_it = p.find(emblem_pattern);
			if (emblem_it != p.npos) {
				emblem_it += emblem_pattern_len;
				auto dummy_it = p.find("dummy", emblem_it);
				if (dummy_it == p.npos) {
					std::string_view rem(p.data() + emblem_it);
					for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
						if (rem.starts_with(shortNames[i])) {
							res.id = i;
							res.attribute = rem.find("_lod") != rem.npos ? ResourceInfo::Attribute::LOD : ResourceInfo::Attribute::NONE;
							break;
						}
					}
					res.kind = ResourceInfo::Kind::EMBLEM_MODEL;
					return res;
				}
			}
		}
		{
			// select_(driver).bclim
			bool select_starts = p.starts_with(select_pattern);
			if (select_starts) {
				std::string_view rem(p.data() + select_pattern_len);
				for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
					if (selectBclimNames[i][0] != '\0' && rem.starts_with(selectBclimNames[i])) {
						std::string_view rem2(rem.data() + strlen(selectBclimNames[i]));
						if (rem2 == ".bclim") {
							res.id = i;
							res.kind = ResourceInfo::Kind::SELECT_UI;
							return res;
						}
					}
				}
				res.kind = ResourceInfo::Kind::UNKNOWN;
				return res;
			}
		}
		{
			//Body ui
			bool body_starts = p.starts_with(ui_body_pattern);
			if (body_starts) {
				std::string_view rem(p.data() + ui_body_pattern_len);
				for (int i = 0; i < EBodyID::BODY_SIZE; i++) {
					if (rem.starts_with(bodyUINames[i])) {
						res.kind = ResourceInfo::Kind::BODY_UI;
						res.id = i;
						res.id2 = -1;
						switch (bodyUIHasDriver[i])
						{
						case 1:
						{
							std::string_view rem2(rem.data() + strlen(bodyUINames[i]) + 1);
							for (int j = 0; j < EDriverID::DRIVER_SIZE; j++) {
								if (j == EDriverID::DRIVER_MIIM || j == EDriverID::DRIVER_MIIF)
									continue;
								if (rem2.starts_with(shortNames[j])) {
									res.id2 = j;
									break;
								}
							}
							break;
						}
						case 2:
						{
							std::string_view rem2(rem.data() + strlen(bodyUINames[i]) + 1);
							for (int j = 0; j < sizeof(specialPartsDriver) / sizeof(EDriverID); j++) {
								if (rem2.starts_with(shortNames[specialPartsDriver[j]])) {
									res.id2 = specialPartsDriver[j];
									break;
								}
							}
							break;
						}
						
						default:
							break;
						}
						return res;
					}
				}
				res.kind = ResourceInfo::Kind::UNKNOWN;
				return res;
			}
		}
		{
			//Tire ui
			bool tire_starts = p.starts_with(ui_tire_pattern);
			if (tire_starts) {
				std::string_view rem(p.data() + ui_tire_pattern_len);
				for (int i = 0; i < ETireID::TIRE_SIZE; i++) {
					if (rem.starts_with(tireUINames[i])) {
						res.kind = ResourceInfo::Kind::TIRE_UI;
						res.id = i;
						res.id2 = -1;
						return res;
					}
				}
				res.kind = ResourceInfo::Kind::UNKNOWN;
				return res;
			}
		}
		{
			//Wing ui
			bool wing_starts = p.starts_with(ui_wing_pattern);
			if (wing_starts) {
				std::string_view rem(p.data() + ui_wing_pattern_len);
				for (int i = 0; i < EWingID::WING_SIZE; i++) {
					if (rem.starts_with(wingUINames[i])) {
						res.kind = ResourceInfo::Kind::WING_UI;
						res.id = i;
						res.id2 = -1;
						switch (wingUIHasDriver[i])
						{
						case 1:
						{
							std::string_view rem2(rem.data() + strlen(wingUINames[i]) + 1);
							for (int j = 0; j < EDriverID::DRIVER_SIZE; j++) {
								if (j == EDriverID::DRIVER_MIIM || j == EDriverID::DRIVER_MIIF)
									continue;
								if (rem2.starts_with(shortNames[j])) {
									res.id2 = j;
									break;
								}
							}
							break;
						}
						case 2:
						{
							std::string_view rem2(rem.data() + strlen(wingUINames[i]) + 1);
							for (int j = 0; j < sizeof(specialPartsDriver) / sizeof(EDriverID); j++) {
								if (rem2.starts_with(shortNames[specialPartsDriver[j]])) {
									res.id2 = specialPartsDriver[j];
									break;
								}
							}
							break;
						}
						
						default:
							break;
						}
						return res;
					}
				}
				res.kind = ResourceInfo::Kind::UNKNOWN;
				return res;
			}
		}
		{
			//Thank you animation
			if (loadArgs && loadArgs->archiveID == 0x9 && p.size() > 1) {
				std::string_view rem(p.data() + 1);
				for (int i = 0; i < (int)EDriverID::DRIVER_SIZE; i++) {
					if (rem.starts_with(shortNames[i])) {
						res.id = i;
						res.id2 = -1;
						res.kind = ResourceInfo::Kind::THANK_YOU_ANIM;
						res.attribute = ResourceInfo::Attribute::NONE;
						return res;
					}
				}
			}
		}

		res.kind = ResourceInfo::Kind::UNKNOWN;
		return res;
	}

	u8* CharacterHandler::GetTexturePointer(ECharaIconType iconType, int playerID) {
		if (iconType == ECharaIconType::CHARAICONTYPE_SELECT)
			return nullptr;
		u8* ret = textureDataPointers[iconType][playerID];
		if (!ret) {
			ret = (u8*)GameAlloc::MemAlloc(GetCharaIconTextureSize(iconType) + CryptoResource::cryptoResourceHeaderSize, 0x80);
			textureDataPointers[iconType][playerID] = ret;
		}
		return ret;
	}

	u8* CharacterHandler::GetMenuSelectTexturePointer(EDriverID driverID) {
		u8* ret = menuSelectTextureDataPointers[driverID];
		if (!ret) {
			ret = (u8*)GameAlloc::MemAlloc(GetCharaIconTextureSize(ECharaIconType::CHARAICONTYPE_SELECT) + CryptoResource::cryptoResourceHeaderSize, 0x80);
			menuSelectTextureDataPointers[driverID] = ret;
		}
		return ret;
	}

	const char* CharacterHandler::GetCharaIconBCLIMName(ECharaIconType iconType) {
		switch (iconType)
		{
		case ECharaIconType::CHARAICONTYPE_MAPRACE:
			return "maprace.bclim";
		case ECharaIconType::CHARAICONTYPE_RANKRACE:
			return "rankrace.bclim";
		case ECharaIconType::CHARAICONTYPE_RANKMENU:
			return "rankmenu.bclim";
		case ECharaIconType::CHARAICONTYPE_SELECT:
			return "select.bclim";
		default:
			return "";
		}
	}

	static u32 g_faceMenuAmountOptions = 0;
	static u32 g_faceMenuCurrentOption = 0;
	CharacterHandler::FaceRaiderImage* CharacterHandler::g_faceMenuFaceRaiderImage = nullptr;
	FS_Archive CharacterHandler::FaceRaiderImage::archive = 0;
	std::string CharacterHandler::FaceRaiderImage::facesDir = "";
	u32 CharacterHandler::FaceRaiderImage::customFacesAmount = 0;

	void CharacterHandler::OnFaceRaiderMenuEvent(Keyboard&, KeyboardEvent &event) {
		if (event.type == KeyboardEvent::SelectionChanged) {
			g_faceMenuCurrentOption = event.selectedIndex;
			if (g_faceMenuFaceRaiderImage) {
				delete g_faceMenuFaceRaiderImage;
				g_faceMenuFaceRaiderImage = nullptr;
			}
			if (g_faceMenuCurrentOption < g_faceMenuAmountOptions - 1) {
				g_faceMenuFaceRaiderImage = new CharacterHandler::FaceRaiderImage(g_faceMenuCurrentOption);
			}
		} else if (event.type == KeyboardEvent::FrameTop && g_faceMenuCurrentOption != 0 && g_faceMenuFaceRaiderImage && g_faceMenuFaceRaiderImage->isLoaded) {
			BCLIM::Header fakeBclimHeader = {0};
			fakeBclimHeader.imag.format = BCLIM::TextureFormat::RGB565;
			fakeBclimHeader.imag.height = 128;
			fakeBclimHeader.imag.width = 128;
			IntRect imageRect((400 - 128) / 2, (240 - 128) / 2, 128, 128);
			BCLIM(g_faceMenuFaceRaiderImage->pixelData, &fakeBclimHeader).Render(imageRect, BCLIM::RenderInterface(*event.renderInterface));
			event.renderInterface->DrawRect(imageRect, Color::Red, false, 2);
		}
	}

	bool CharacterHandler::openFaceRaiderMenu(const CharacterEntry& entry) {
		Keyboard faceSelector(entry.longName + " - " + NAME("face_raider_sel"));
		std::vector<std::string> options;
		std::string base_filename = entry.chPack->GetFileName().substr(entry.chPack->GetFileName().find_last_of("/") + 1);
		std::string::size_type const p(base_filename.find_last_of('.'));
		std::string file_without_extension = base_filename.substr(0, p);
		u32 faceAmount = FaceRaiderImage::Initialize("/CTGP-7/MyStuff/Characters/" + file_without_extension + "/Faces/");
		for (int i = 0; i < faceAmount; i++) {
			options.push_back( i == 0 ? NAME("default") : std::to_string(i));
		}
		options.push_back(NAME("exit"));
		g_faceMenuAmountOptions = options.size();
		g_faceMenuCurrentOption = 0;
		faceSelector.Populate(options);
		faceSelector.ChangeEntrySound(g_faceMenuAmountOptions - 1, SoundEngine::Event::CANCEL);
		faceSelector.OnKeyboardEvent(OnFaceRaiderMenuEvent);
		g_faceMenuFaceRaiderImage = new CharacterHandler::FaceRaiderImage(g_faceMenuCurrentOption);
		int res = faceSelector.Open();
		bool result = false;
		if (res >= 0 && res != g_faceMenuAmountOptions - 1 && g_faceMenuFaceRaiderImage) {
			File faceBinData(entry.faceRainderBinDataPath, File::RWC);
			if (faceBinData.IsOpen())
				if (faceBinData.Write(g_faceMenuFaceRaiderImage->pixelData, 0x8000) == 0)
					result = true;
		}
		if (g_faceMenuFaceRaiderImage) {
			delete g_faceMenuFaceRaiderImage;
			g_faceMenuFaceRaiderImage = nullptr;
		}
		FaceRaiderImage::Finalize();
		return result;
	}

	static std::vector<CharacterHandler::CharacterEntry*> g_thisCharEntries;
	static CharacterHandler::CharacterEntry* g_currentLoadEntry = nullptr;
	static Mutex g_currentLoadEntryMutex;
	static void* g_bclimBuffer = nullptr;
	static u32 g_bclimFileSize = 0;
	static bool g_imageReady = false;
	static Task* g_readETCTask = nullptr;

	s32 CharacterHandler::LoadCharacterImageTaskFunc(void* args) {
		CharacterHandler::CharacterEntry* entry;
		bool atleastone = false;
		auto loadfallbackicon = [](CharacterHandler::CharacterEntry* entry) {
			if (!ExtraResource::isValidMenuSzsArchive())
				return false;
			ExtraResource::SARC menuSzs((u8*)(ExtraResource::latestMenuSzsArchive[0xE] - 0x14), false);
			if (!menuSzs.processed)
				return false;
			ExtraResource::SARC::FileInfo fInfo;
			u8* fileData = menuSzs.GetFile("select_" + std::string(selectBclimNames[(u32)entry->origChar]) + ".bclim", &fInfo);
			if (!fileData || fInfo.fileSize > 0x2048)
				return false;
			memcpy(g_bclimBuffer, fileData, fInfo.fileSize);
			g_bclimFileSize = fInfo.fileSize;
			return true;
		};
		while (true) {
			{
				Lock lock(g_currentLoadEntryMutex);
				if (g_currentLoadEntry == nullptr) {
					g_imageReady = atleastone;
					break;
				} else {
					entry = g_currentLoadEntry;
					g_currentLoadEntry = nullptr;
				}
			}

			entry->chPack->SetEnabled(true);
			ExtraResource::SARC::FileInfo fInfo;
			bool foundFile = entry->chPack->GetFileInfo(fInfo, GetCharaIconBCLIMName(ECharaIconType::CHARAICONTYPE_SELECT));
			if (!foundFile || fInfo.fileSize > 0x2048) {
				entry->chPack->SetEnabled(false);
				atleastone = loadfallbackicon(entry);
				continue;
			}

			if (!entry->chPack->ReadFileDirectly(g_bclimBuffer, fInfo, fInfo)) {
				entry->chPack->SetEnabled(false);
				atleastone = loadfallbackicon(entry);
				continue;
			}
			entry->chPack->SetEnabled(false);

			g_bclimFileSize = fInfo.fileSize;
			atleastone = true;
		}
		return 0;
	}

	void CharacterHandler::SetCharacterOption(Keyboard& kbd, u32 option) {
		{
			Lock lock(g_currentLoadEntryMutex);
			g_currentLoadEntry = g_thisCharEntries[option];
			g_imageReady = false;
		}
		g_readETCTask->Start();

		std::string charName;
		if (g_thisCharEntries[option]->achievementLevel == 5) charName = std::string(Color(255, 0, 255) << g_thisCharEntries[option]->longName);
		else if (g_thisCharEntries[option]->achievementLevel > 0) charName = std::string(Color(255, 255, 0) << g_thisCharEntries[option]->longName);
		else if (g_thisCharEntries[option]->specialAchievement != SaveHandler::SpecialAchievements::NONE) charName = std::string(Color(255, 255, 0) << g_thisCharEntries[option]->longName);
		else charName = g_thisCharEntries[option]->longName;
		charName += ResetColor();

		std::string stateText = Utils::Format("ID: 0x%016llX ", g_thisCharEntries[option]->id) + (g_thisCharEntries[option]->selectAllowed ? ( "(" + NAME("state_mode") + ")") : (Color(64, 64, 64) << "(" + NOTE("state_mode") + ")")) + ResetColor();
		std::string text = CenterAlign(charName) + "\n" + CenterAlign(stateText);
		
		text += HorizontalSeparator();
		text += "\n\n\n\n";
		text += HorizontalSeparator();
		if (g_thisCharEntries[option]->authors.size() > 0) {
			text += CenterAlign(NAME("authors")) + "\n";
			std::string currline = "";
			for (int i = 0; i < 6 && i < g_thisCharEntries[option]->authors.size(); i++) {
				currline += GetAuthor(g_thisCharEntries[option]->authors[i]);
				if (((i + 1) % 3) != 0 && (i+1 != g_thisCharEntries[option]->authors.size()))
					currline += ", ";
				if (((i+1) % 3) == 0)
					currline += "\n";
			}
			text += CenterAlign(currline);
		}
		kbd.GetMessage() = text;
	}

	void CharacterHandler::OnCharacterManagerMenuEvent(Keyboard& kbd, KeyboardEvent &event) {
		if (event.type == KeyboardEvent::SelectionChanged && event.selectedIndex >= 0) {
			SetCharacterOption(kbd, event.selectedIndex);
		} else if (event.type == KeyboardEvent::FrameTop) {
			if (g_imageReady) {
				int startX = ((400 - 64) / 2) - 3;
				int startY = (240 - 64) / 2 - 12;
				BCLIM(g_bclimBuffer, g_bclimFileSize).Render(IntRect(startX, startY, 64, 64), BCLIM::RenderInterface(*event.renderInterface));
			}
		}
	}

	void CharacterHandler::CustomCharacterManagerMenu(MenuEntry* entry) {
		Keyboard charopt(CenterAlign(NAME("charman") + "\n" + NAME("char_select")));
		std::vector<std::string> options;
		std::vector<int> countEach;
		if (!g_bclimBuffer) {
			g_bclimBuffer = operator new(0x2048);
		}
		if (!g_readETCTask) {
			g_readETCTask = new Task(LoadCharacterImageTaskFunc, nullptr, Task::Affinity::AppCore);
		}
		options.resize(EDriverID::DRIVER_SIZE);
		countEach.resize(EDriverID::DRIVER_SIZE);
		int achievementLevel = SaveHandler::saveData.GetCompletedAchievementCount();
		for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
			options[msbtOrder[i]] = origCharNames[i];
		}
		int result = 0;
		while (result >= 0) {
			charopt.Populate(options, false);
			result = charopt.Open();
			if (result >= 0) {
				EDriverID selectedCustomChar;
				for (int i = 0; i < EDriverID::DRIVER_SIZE; i++) {
					if (result == msbtOrder[i]) { selectedCustomChar = (EDriverID)i; break; }
				}
				int result2 = 0;
				bool somethingChanged = false;
				while (result2 >= 0) {
					g_thisCharEntries.clear();
					for (auto it = charEntries.begin(); it != charEntries.end(); it++) {
						if (it->second.origChar != selectedCustomChar || it->second.achievementLevel > achievementLevel || 
						   (it->second.specialAchievement != SaveHandler::SpecialAchievements::NONE && !SaveHandler::saveData.IsSpecialAchievementCompleted(it->second.specialAchievement))) 
							continue;
						g_thisCharEntries.push_back(&it->second);
					}
					if (g_thisCharEntries.size() == 0) {
						result2 = -1;
						continue;
					}
					std::vector<std::string> customOption;
					for (int i = 0; i < g_thisCharEntries.size(); i++) {
						if (!g_thisCharEntries[i]->selectAllowed) customOption.push_back(std::string(Color(64, 64, 64) << g_thisCharEntries[i]->longName));
						else if (g_thisCharEntries[i]->achievementLevel == 5) customOption.push_back(std::string(Color(255, 0, 255) << g_thisCharEntries[i]->longName));
						else if (g_thisCharEntries[i]->achievementLevel > 0) customOption.push_back(std::string(Color(255, 255, 0) << g_thisCharEntries[i]->longName));
						else if (g_thisCharEntries[i]->specialAchievement != SaveHandler::SpecialAchievements::NONE) customOption.push_back(std::string(Color(255, 255, 0) << g_thisCharEntries[i]->longName));
						else customOption.push_back(g_thisCharEntries[i]->longName);
					}
					Keyboard customCharOpt("dummy");
					customCharOpt.Populate(customOption);
					customCharOpt.OnKeyboardEvent(OnCharacterManagerMenuEvent);
					customCharOpt.ChangeSelectedEntry(result2);
					SetCharacterOption(customCharOpt, result2);
					result2 = customCharOpt.Open();
					if (result2 >= 0) {
						if (g_thisCharEntries[result2]->faceRaiderOffset != 0) {
							Keyboard kbdFace;
							kbdFace.Populate({ g_thisCharEntries[result2]->selectAllowed ? NOTE("state_inf") : NAME("state_inf"),  NAME("change_face")});
							int result3 = kbdFace.Open();
							if (result3 == 0) {
								g_thisCharEntries[result2]->selectAllowed = !g_thisCharEntries[result2]->selectAllowed;
								somethingChanged = true;
							} else if (result3 == 1) {
								openFaceRaiderMenu(*g_thisCharEntries[result2]);
							}
						} else {
							g_thisCharEntries[result2]->selectAllowed = !g_thisCharEntries[result2]->selectAllowed;
							somethingChanged = true;
						}
					}
				}
				if (somethingChanged) {
					SaveDisabledChars();
					PopulateAvailableCharacters();
					SaveCharacterChoices();
				}
			}
		}
		g_thisCharEntries.clear();
		if (g_bclimBuffer) {
			free(g_bclimBuffer);
			g_bclimBuffer = nullptr;
		}
		if (g_readETCTask) {
			{
				Lock lock(g_currentLoadEntryMutex);
				g_currentLoadEntry = nullptr;
			}
			g_readETCTask->Wait();
			delete g_readETCTask;
			g_readETCTask = nullptr;
		}
	}

	void CharacterHandler::enableCustomKartsSettings(MenuEntry* entry)
	{
		Keyboard kbd(NAME("cuskart") + "\n\n" + NAME("state") + ": " + (SaveHandler::saveData.flags1.customKartsEnabled ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) + ResetColor() + "\n\n" + NAME("reboot_req"));
		kbd.Populate({ NAME("state_inf"), NOTE("state_inf") });
		kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = kbd.Open();
		bool changed = false;
		if (ret >= 0){
			bool newOpt = ret == 0;
			if (newOpt != SaveHandler::saveData.flags1.customKartsEnabled) {
				SaveHandler::saveData.flags1.customKartsEnabled = newOpt;
				changed = true;
				entry->Name() = NAME("cuskart") + " (" + (SaveHandler::saveData.flags1.customKartsEnabled ? NAME("state_mode") : NOTE("state_mode")) + ")";
			}
		}
		if (changed) {
			MessageBox(NAME("reboot_req"))();
		}
	}


	CharacterHandler::FaceRaiderImage::FaceRaiderImage(u32 id) {
		isLoaded = false;
		pixelData = nullptr;
		if (id <= customFacesAmount) {
			std::string filePath = facesDir + ((id == 0) ? std::string("default.bclim") : Utils::Format("%d.bclim", id));
			File defaultFile(filePath, File::READ);
			if (!defaultFile.IsOpen() || defaultFile.GetSize() != 0x8028) {
				return;
			}
			pixelData = (u16*)operator new(0x8000);
			if (R_FAILED(defaultFile.Read(pixelData, 0x8000))) {
				operator delete(pixelData);
				pixelData = nullptr;
				return;
			}
		} else {
			if (!archive)
				return;
			Handle fileHandle;
			std::string file = Utils::Format("/PhotoF%02d.dat", id - customFacesAmount - 1);
			Result res = FSUSER_OpenFile(&fileHandle, archive, fsMakePath(PATH_ASCII, file.c_str()), FS_OPEN_READ, 0);
			if (R_FAILED(res))
				return;
			u64 fileSize = 0;
			res = FSFILE_GetSize(fileHandle, &fileSize);
			if (R_FAILED(res) || fileSize != 0x8008) {
				FSFILE_Close(fileHandle);
				return;
			}
			pixelData = (u16*)operator new(0x8000);
			u32 bytesRead = 0;
			res = FSFILE_Read(fileHandle, &bytesRead, 0x4, pixelData, 0x8000);
			FSFILE_Close(fileHandle);
			if (R_FAILED(res) || bytesRead != 0x8000) {
				operator delete(pixelData);
				pixelData = nullptr;
				return;
			}
		}
		isLoaded = true;
	}

	CharacterHandler::FaceRaiderImage::~FaceRaiderImage() {
		if (isLoaded && pixelData)
			operator delete(pixelData);
	}

	u32 CharacterHandler::FaceRaiderImage::Initialize(const std::string& extraFacesPath) {
		// Faces folder
		facesDir = extraFacesPath;
		customFacesAmount = 0;
		if (!File::Exists(facesDir + "default.bclim"))
			return 0;
		for (int j = 1; j < 100; j++) {
			if (File::Exists(facesDir + Utils::Format("%d.bclim", j)))
				customFacesAmount++;
			else
				break;
		}

		// Face raiders
		const u32 faceRaiderSavedatas[3] = {0x0002020D, 0x0002021D, 0x0002022D};
		u32* saveDataIds = (u32*)operator new(0x100);
		u32 saveDataIdsWritten = 0;
		Result res = FSUSER_EnumerateSystemSaveData(&saveDataIdsWritten, 0x100 * 4, saveDataIds);
		if (R_FAILED(res)) {
			operator delete(saveDataIds);
			return customFacesAmount + 1;
		}
		u32 useSaveID = 0;
		for (int j = 0; j < saveDataIdsWritten; j++) {
			if (saveDataIds[j] == faceRaiderSavedatas[0] || 
				saveDataIds[j] == faceRaiderSavedatas[1] || 
				saveDataIds[j] == faceRaiderSavedatas[2]) {
				useSaveID = saveDataIds[j];
				break;
			}
		}
		operator delete(saveDataIds);
		if (!useSaveID)
			return customFacesAmount + 1;
		
		u32 path[2] = {MEDIATYPE_NAND, useSaveID};
		res = FSUSER_OpenArchive(&archive, ARCHIVE_SYSTEM_SAVEDATA, {PATH_BINARY, sizeof(path), path});
		if (R_FAILED(res)) {
			return customFacesAmount + 1;
		}
		u32 i;
		for (i = 0; i < 100; i++) {
			Handle fileHandle;
			std::string file = Utils::Format("/PhotoF%02d.dat", i);
			res = FSUSER_OpenFile(&fileHandle, archive, fsMakePath(PATH_ASCII, file.c_str()), FS_OPEN_READ, 0);
			if (R_FAILED(res))
				break;
			FSFILE_Close(fileHandle);
		}
		return i + customFacesAmount + 1;		
	}

	void CharacterHandler::FaceRaiderImage::Finalize() {
		facesDir = "";
		customFacesAmount = 0;
		if (archive) {
			FSUSER_CloseArchive(archive);
			archive = 0;
		}
	}
}