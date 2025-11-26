/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveHandler.cpp
Open source lines: 689/692 (99.57%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "SaveHandler.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "VersusHandler.hpp"
#include "Minibson.hpp"
#include "MissionHandler.hpp"
#include "malloc.h"
#include <string>
#include "UserCTHandler.hpp"
#include "MenuPage.hpp"
#include "StatsHandler.hpp"
#include "CustomTextEntries.hpp"
#include "CharacterHandler.hpp"
#include "BlueCoinChallenge.hpp"
#include "PointsModeHandler.hpp"
#include "SaveBackupHandler.hpp"
#include "BadgeManager.hpp"
#include "plgldr.h"

namespace CTRPluginFramework {
	
	bool SaveHandler::disableSaving = false;
	SaveHandler::CTGP7Save SaveHandler::saveData{};
	Task SaveHandler::saveSettinsTask(SaveHandler::SaveSettingsTaskFunc, nullptr, Task::Affinity::AppCore);
	int SaveHandler::lastAchievementCount = 0;
	u32 SaveHandler::lastSpecialAchievements = 0;
	Mutex SaveHandler::saveSettingsMutex;

	void SaveHandler::ApplySettings() {
		//
		ccsettings[0] = saveData.cc_settings;
		ccselectorentry->SetArg(&ccsettings[0]);
		//

		ccselector_apply(ccselectorentry);
		speedometer_apply();
		backwardscam_apply(saveData.flags1.backCamEnabled);
		warnItemUse_apply(saveData.flags1.warnItemEnabled);
		courseOrder_apply(saveData.flags1.isAlphabeticalEnabled);
		improvedTricks_apply(saveData.flags1.improvedTricks);
		renderImprovements_apply(saveData.flags1.renderOptimization);
		autoAccel_apply(saveData.flags1.autoacceleration);
		brakedrift_apply(saveData.flags1.brakedrift);
		automaticdelaydrift_apply(saveData.flags1.automaticDelayDrift);
		improvedhorn_apply(saveData.flags1.improvedHonk);
		bluecoin_apply(saveData.flags1.blueCoinsEnabled);
		MarioKartFramework::changeNumberRounds(saveData.numberOfRounds);

		if (saveData.ctVR < 1 || saveData.ctVR > 999999)
			saveData.ctVR = 1000;
		if (saveData.cdVR < 1 || saveData.cdVR > 999999)
			saveData.cdVR = 1000;

		UpdateAchievementCryptoFiles();
		lastAchievementCount = saveData.GetCompletedAchievementCount();
		lastSpecialAchievements = saveData.specialAchievements;
	}

	void SaveHandler::DefaultSettings() {
		saveData = std::move(CTGP7Save());
		saveData.GenerateNewSaveID();
		saveData.newlyGeneratedID = true;
	}

	void SaveHandler::UpdateAchievementCryptoFiles() {
		for (int i = 0; i < saveData.GetCompletedAchievementCount(); i++) {
			CryptoResource::AllowKnownFileID((CryptoResource::KnownFileID)(0x1000 | i), true);
		}
		if (saveData.IsSpecialAchievementCompleted(SpecialAchievements::ALL_BLUE_COINS))
			CryptoResource::AllowKnownFileID(CryptoResource::KnownFileID::ACHIEVEMENT_BLUE_COIN, true);
		if (saveData.IsSpecialAchievementCompleted(SpecialAchievements::DODGED_BLUE_SHELL))
			CryptoResource::AllowKnownFileID(CryptoResource::KnownFileID::ACHIEVEMENT_DODGED_BLUE, true);
	}

	void SaveHandler::UpdateAchievementsConditions() {
		bool justGranted = false;
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::ALL_GOLD) && !SaveHandler::saveData.IsAchievementPending(Achievements::ALL_GOLD)) {
			if (SaveHandler::CupRankSave::CheckModAllSatisfy(SaveHandler::CupRankSave::SatisfyCondition::GOLD)) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_GOLD, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::ALL_ONE_STAR) && !SaveHandler::saveData.IsAchievementPending(Achievements::ALL_ONE_STAR)) {
			if (SaveHandler::CupRankSave::CheckModAllSatisfy(SaveHandler::CupRankSave::SatisfyCondition::ONE_STAR)) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_ONE_STAR, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::ALL_THREE_STAR) && !SaveHandler::saveData.IsAchievementPending(Achievements::ALL_THREE_STAR)) {
			if (SaveHandler::CupRankSave::CheckModAllSatisfy(SaveHandler::CupRankSave::SatisfyCondition::THREE_STAR)) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_THREE_STAR, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::ALL_MISSION_TEN) && !SaveHandler::saveData.IsAchievementPending(Achievements::ALL_MISSION_TEN)) {
			auto prog = MissionHandler::SaveData::GetAllFullGradeFlag();
			if (prog.second >= prog.first) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_MISSION_TEN, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::VR_5000) && !SaveHandler::saveData.IsAchievementPending(Achievements::VR_5000)) {
			if (saveData.ctVR >= 5000 || saveData.cdVR >= 5000) {
				SaveHandler::saveData.SetAchievementPending(Achievements::VR_5000, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::ALL_SCORE_COMPLETED) && !SaveHandler::saveData.IsAchievementPending(Achievements::ALL_SCORE_COMPLETED)) {
			if (PointsModeHandler::HasCompletedAllTracks()) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_SCORE_COMPLETED, true);
				justGranted = true;
			}
		}
		
		if (!SaveHandler::saveData.IsSpecialAchievementCompleted(SpecialAchievements::ALL_BLUE_COINS) && !SaveHandler::saveData.IsSpecialAchievementPending(SpecialAchievements::ALL_BLUE_COINS)) {
			if (BlueCoinChallenge::GetCollectedCoinCount() >= BlueCoinChallenge::GetTotalCoinCount()) {
				SaveHandler::saveData.SetSpecialAchievementPending(SpecialAchievements::ALL_BLUE_COINS, true);
				justGranted = true;
			}
		}

		if (!SaveHandler::saveData.IsSpecialAchievementCompleted(SpecialAchievements::DODGED_BLUE_SHELL) && !SaveHandler::saveData.IsSpecialAchievementPending(SpecialAchievements::DODGED_BLUE_SHELL)) {
			if (saveData.blueShellDodgeAmount >= BLUE_SHELL_DODGE_COUNT_ACHIEVEMENT) {
				SaveHandler::saveData.SetSpecialAchievementPending(SpecialAchievements::DODGED_BLUE_SHELL, true);
				justGranted = true;
			}
		}

		if (justGranted) {
			SaveHandler::SaveSettingsAll();
		}
	}
	
	static std::unordered_map<u64, CharacterHandler::CharacterEntry>::iterator g_lastCharShown;
	static bool g_processing_achievements = false;
	void SaveHandler::UpdateCharacterIterator() {
		g_lastCharShown = CharacterHandler::GetCharEntries().begin();
	}
	bool SaveHandler::CheckAndShowAchievementMessages() {
		if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_GOLD)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_gold"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_GOLD, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_GOLD, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_ONE_STAR)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_1star"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_ONE_STAR, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_ONE_STAR, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_THREE_STAR)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_3star"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_THREE_STAR, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_THREE_STAR, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_MISSION_TEN)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_mission"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_MISSION_TEN, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_MISSION_TEN, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsAchievementPending(Achievements::VR_5000)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_5000_vr"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::VR_5000, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::VR_5000, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_SCORE_COMPLETED)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_score_completed"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_SCORE_COMPLETED, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_SCORE_COMPLETED, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		}
		
		else if (SaveHandler::saveData.IsSpecialAchievementPending(SpecialAchievements::ALL_BLUE_COINS)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_blue_coin"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetSpecialAchievementPending(SpecialAchievements::ALL_BLUE_COINS, false);
			SaveHandler::saveData.SetSpecialAchievementCompleted(SpecialAchievements::ALL_BLUE_COINS, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		} else if (SaveHandler::saveData.IsSpecialAchievementPending(SpecialAchievements::DODGED_BLUE_SHELL)) {
			if (MarioKartFramework::isDialogOpened() && g_processing_achievements) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_dodged_blue"));
			g_processing_achievements = true;
			SaveHandler::saveData.SetSpecialAchievementPending(SpecialAchievements::DODGED_BLUE_SHELL, false);
			SaveHandler::saveData.SetSpecialAchievementCompleted(SpecialAchievements::DODGED_BLUE_SHELL, true);
			UpdateAchievementCryptoFiles();
			CharacterHandler::PopulateAvailableCharacters();
			SaveHandler::SaveSettingsAll();
			return true;
		}

		else {
			if (g_processing_achievements && MarioKartFramework::isDialogOpened()) return true;
			u32 newCount = SaveHandler::saveData.GetCompletedAchievementCount();
			if (lastAchievementCount < newCount) {
				for (auto it = g_lastCharShown; it != CharacterHandler::GetCharEntries().end(); it++) {
					if (it->second.achievementLevel > 0 && it->second.achievementLevel == (lastAchievementCount + 1)) {
						std::u16string text;
						Utils::ConvertUTF8ToUTF16(text, NAME("unlocked_item"));
						text.append(
							Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM,
								it->second.achievementLevel < 5 ? Color(255, 180, 0) : Color(180, 0, 180)
							)
						);
						Utils::ConvertUTF8ToUTF16(text, it->second.longName);
						Language::MsbtHandler::SetString(CustomTextEntries::dialog, text);
						MarioKartFramework::openDialog(DialogFlags::Mode::OK, "", nullptr, true);
						g_processing_achievements = true;
						MarioKartFramework::playTitleFlagOpenTimer = 30;
						g_lastCharShown = it;
						g_lastCharShown++;
						return true;
					}
				}
				g_lastCharShown = CharacterHandler::GetCharEntries().begin();
				lastAchievementCount++;
				return true;
			}
			if (lastSpecialAchievements != SaveHandler::saveData.specialAchievements) {
				SpecialAchievements next = SpecialAchievements::NONE;
				for (int i = 0; i < 32; i++) {
					if (((SaveHandler::saveData.specialAchievements & (1 << i)) != 0) && ((lastSpecialAchievements & (1 << i)) == 0)) {
						next = (SpecialAchievements)i;
						break;
					}
				}
				for (auto it = g_lastCharShown; it != CharacterHandler::GetCharEntries().end(); it++) {
					if (it->second.specialAchievement != SpecialAchievements::NONE && it->second.specialAchievement == next) {
						std::u16string text;
						Utils::ConvertUTF8ToUTF16(text, NAME("unlocked_item"));
						text.append(
							Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM,
								Color(255, 180, 0)
							)
						);
						Utils::ConvertUTF8ToUTF16(text, it->second.longName);
						Language::MsbtHandler::SetString(CustomTextEntries::dialog, text);
						MarioKartFramework::openDialog(DialogFlags::Mode::OK, "", nullptr, true);
						g_processing_achievements = true;
						MarioKartFramework::playTitleFlagOpenTimer = 30;
						g_lastCharShown = it;
						g_lastCharShown++;
						return true;
					}
				}
				g_lastCharShown = CharacterHandler::GetCharEntries().begin();
				if (next != SpecialAchievements::NONE)
					lastSpecialAchievements |= (1 << (u32)next);
				return true;
			}
		}
		g_processing_achievements = false;
		return false;
	}

    bool SaveHandler::CheckAndShowServerCommunicationDisabled()
    {
        if (SaveHandler::saveData.flags1.serverCommunication)
			return true;

		Keyboard kbd(NOTE("serv_communic"));
		kbd.Populate({NAME("exit")});
		kbd.Open();
		return false;
    }

    s32 SaveHandler::SaveSettingsTaskFunc(void* args) {
		Lock lock(saveSettingsMutex);
		if (disableSaving) return 0;
		SaveOptions();
		CupRankSave::Save();
		StatsHandler::CommitToFile();
		MissionHandler::SaveSaveData();
		PointsModeHandler::SaveSaveData();
		return 0;
	}

	void SaveHandler::LoadSettings() {
		SaveBackupHandler::AddDoBackupHandler("options", SaveSettingsBackup);
		SaveBackupHandler::AddRestoreBackupHandler("options", RestoreSettingsBackup);

		SaveFile::LoadStatus status;
		minibson::document doc = SaveFile::Load(SaveFile::SaveType::OPTIONS, status);
		u64 scID = doc.get<u64>("_cID", 0); if (scID == 0) scID = doc.get<u64>("cID", 0);
		if (status == SaveFile::LoadStatus::SUCCESS) {
			saveData = std::move(CTGP7Save(doc, true));
		} else {
			SaveFile::HandleError(SaveFile::SaveType::OPTIONS, status);
			DefaultSettings();
		}

		CupRankSave::Load();
		MissionHandler::SaveData::Load();
		PointsModeHandler::LoadSaveData();
		StatsHandler::Initialize();
		BadgeManager::Initialize();

		if (saveData.newlyGeneratedID) {
			SaveOptions();
			saveData.newlyGeneratedID = false;
		}
	}

	void SaveHandler::SaveOptions() {

		saveData.cc_settings = ccsettings[0];

		minibson::document saveDoc;
		saveDoc.set<u64>("_cID", NetHandler::GetConsoleUniqueHash());
		saveDoc.set("sID0", (s64)saveData.saveID[0]);
		saveDoc.set("sID1", (s64)saveData.saveID[1]);
		saveData.serialize(saveDoc);
		
		SaveFile::Save(SaveFile::SaveType::OPTIONS, saveDoc);
	}

	minibson::document SaveHandler::SaveSettingsBackup(void) {
		saveData.cc_settings = ccsettings[0];

		minibson::document saveDoc;
		saveData.serialize(saveDoc);

		return saveDoc;
	}

    bool SaveHandler::RestoreSettingsBackup(const minibson::document &doc)
    {
		saveData = std::move(CTGP7Save(doc, false));
        return true;
    }

    std::map<u32, u8> SaveHandler::CupRankSave::cupData; 

	void SaveHandler::CupRankSave::Load() {
		SaveBackupHandler::AddDoBackupHandler("races", SaveBackup);
		SaveBackupHandler::AddRestoreBackupHandler("races", RestoreBackup);

		SaveFile::LoadStatus status;
		minibson::document doc = SaveFile::Load(SaveFile::SaveType::RACES, status);
		if (status == SaveFile::LoadStatus::SUCCESS) {
			FromDocument(doc.get("cuprank", minibson::document()));	
		} else {
			SaveFile::HandleError(SaveFile::SaveType::RACES, status);
		}
	}

	minibson::document SaveHandler::CupRankSave::ToDocument() {
		minibson::document data;
		for (auto it = cupData.cbegin(); it != cupData.cend(); it++) {
			if (it->second) {
				data.set(std::to_string((it->first << 8) | it->second));
			}
		}
		return data;
	}

    void SaveHandler::CupRankSave::FromDocument(const minibson::document &doc)
    {
		for (auto it = doc.cbegin(); it != doc.cend(); it++) {
			if (it->second->get_node_code() == minibson::bson_node_type::null_node)
			{
				u32 value = std::stoul(it->first);
				u32 cupId = value >> 8;
				u8 data = value & 0xFF;
				cupData[cupId] = data;
			}
		}
    }

    void SaveHandler::CupRankSave::Save() {
		minibson::document saveDoc;
		saveDoc.set("_cID", NetHandler::GetConsoleUniqueHash());
		saveDoc.set<s64>("sID0", SaveHandler::saveData.saveID[0]);
		saveDoc.set<s64>("sID1", SaveHandler::saveData.saveID[1]);
		saveDoc.set("cuprank", ToDocument());
		
		SaveFile::Save(SaveFile::SaveType::RACES, saveDoc);
	}

    minibson::document SaveHandler::CupRankSave::SaveBackup()
    {
		minibson::document saveDoc;
		saveDoc.set("cuprank", ToDocument());

		return saveDoc;
    }

    bool SaveHandler::CupRankSave::RestoreBackup(const minibson::document &doc)
    {
		FromDocument(doc.get("cuprank", minibson::document()));
        return true;
    }

    bool SaveHandler::CupRankSave::CheckModAllSatisfy(SatisfyCondition condition) {
		bool satisfy;
		u32 systemSaveData = MarioKartFramework::getSystemSaveData();
		for (u32 i = 0; i < 4; i++) {
			satisfy = true;
			for (u32 cupID = CUSTOMCUPLOWER; cupID <= CUSTOMCUPUPPER; cupID++) {
				GrandPrixData data;
				u32 realLevel = (i == 3) ? 2 : i;
				bool isMirror = (i == 3);
				getGrandPrixData(systemSaveData, &data, &cupID, &realLevel, isMirror);
				if (condition == SatisfyCondition::COMPLETED && !data.isCompleted) {
					satisfy = false;
					break;
				}
				if (condition == SatisfyCondition::GOLD && (!data.isCompleted || data.trophyType != 3)) {
					satisfy = false;
					break;
				}
				if (condition == SatisfyCondition::ONE_STAR && (!data.isCompleted || data.starRank < 4)) {
					satisfy = false;
					break;
				}
				if (condition == SatisfyCondition::THREE_STAR && (!data.isCompleted || data.starRank < 6)) {
					satisfy = false;
					break;
				}
			}
			if (satisfy) {
				return true;
			}
		}		
		return false;
	}

	std::pair<int, std::array<int, 4>> SaveHandler::CupRankSave::CheckModSatisfyProgress(SatisfyCondition condition) {
		constexpr int total = TOTALCUSTOMCUPS;
		std::array<int, 4> current = {0};
		u32 systemSaveData = MarioKartFramework::getSystemSaveData();
		for (u32 i = 0; i < 4; i++) {
			for (u32 cupID = CUSTOMCUPLOWER; cupID <= CUSTOMCUPUPPER; cupID++) {
				GrandPrixData data;
				u32 realLevel = (i == 3) ? 2 : i;
				bool isMirror = (i == 3);
				getGrandPrixData(systemSaveData, &data, &cupID, &realLevel, isMirror);
				if (condition == SatisfyCondition::COMPLETED && data.isCompleted) {
					current[i]++;
				}
				if (condition == SatisfyCondition::GOLD && (data.isCompleted && data.trophyType == 3)) {
					current[i]++;
				}
				if (condition == SatisfyCondition::ONE_STAR && (data.isCompleted && data.starRank >= 4)) {
					current[i]++;
				}
				if (condition == SatisfyCondition::THREE_STAR && (data.isCompleted && data.starRank == 6)) {
					current[i]++;
				}
			}
		}		
		return std::make_pair(total, current);
	}

	void SaveHandler::CupRankSave::getGrandPrixData(u32 saveData, GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror) {
		if (*GPID == USERCUPID || *GPID == POINTSRANDOMCUPID || *GPID == POINTSWEEKLYCHALLENGECUPID) {
			out->isCompleted = false;
			return;
		}
		if (MissionHandler::isMissionMode && MenuPageHandler::MenuSingleCupGPPage::GetInstace()->isInPage) {
			MissionHandler::OnGetGrandPrixData(out, GPID, engineLevel, isMirror);
			return;
		}
		if (PointsModeHandler::isPointsMode && MenuPageHandler::MenuSingleCupGPPage::GetInstace()->isInPage) {
			PointsModeHandler::OnGetGrandPrixData(out, GPID, engineLevel, isMirror);
			return;
		}
		u32 engineLvl = *engineLevel;
		if (engineLvl == 2 && isMirror) engineLvl++;
		if (*GPID < 8) {
			u8* packedSavedData = (u8*)(saveData + 1804 + *GPID * 4 + engineLvl);
			fromPackedToGP(out, *packedSavedData);
		}
		else {
			u8 packed = 0;
			auto it = cupData.find(*GPID * 4 + engineLvl);
			if (it != cupData.end()) packed = it->second;
			fromPackedToGP(out, packed);
		}
		if (!out->isCompleted) {
			out->starRank = out->trophyType = 0;
		}
	}

	static bool g_setGrandPrixDataPreventRecursion = false;
	void SaveHandler::CupRankSave::setGrandPrixData(u32 saveData, GrandPrixData* in, u32* GPID, u32* engineLevel, bool isMirror) {
		if (*GPID == USERCUPID || *GPID == POINTSRANDOMCUPID || *GPID == POINTSWEEKLYCHALLENGECUPID || VersusHandler::IsVersusMode) return;
		u32 engineLvl = *engineLevel;
		GrandPrixData current{};
		if (engineLvl == 2 && isMirror) engineLvl++;
		if (*GPID < 8) {
			u8* packedSavedData = (u8*)(saveData + 1804 + *GPID * 4 + engineLvl);
			fromPackedToGP(&current, *packedSavedData);
			if (!in) {
				current.isCompleted = false;
			} else {
				current.isCompleted = true;
				if (in->starRank < 8) current.starRank = in->starRank;
				if (in->trophyType < 4)	current.trophyType = in->trophyType;
			}
			fromGPToPacked(packedSavedData, &current);
		}
		else {
			u8 packed = 0;
			auto it = cupData.find(*GPID * 4 + engineLvl);
			if (it != cupData.end()) packed = it->second;
			fromPackedToGP(&current, packed);
			if (!in) {
				current.isCompleted = false;
			}
			else {
				current.isCompleted = true;
				if (in->starRank < 8) current.starRank = in->starRank;
				if (in->trophyType < 4) current.trophyType = in->trophyType;
			}
			fromGPToPacked(&packed, &current);
			cupData[*GPID * 4 + engineLvl] = packed;
		}
		if (!g_setGrandPrixDataPreventRecursion && !isMirror && engineLvl != 0) {
			g_setGrandPrixDataPreventRecursion = true;
			while (true) {
				engineLvl--;
				GrandPrixData tmpData;
				getGrandPrixData(saveData, &tmpData, GPID, &engineLvl, isMirror);
				if (!tmpData.isCompleted || (tmpData.trophyType < in->trophyType || (tmpData.trophyType == in->trophyType && tmpData.starRank < in->starRank)))
					setGrandPrixData(saveData, in, GPID, &engineLvl, isMirror);
				if (engineLvl == 0)
					break;
			}
			g_setGrandPrixDataPreventRecursion = false;
		}
	}

	const char* SaveHandler::SaveFile::SaveNames[(u32)SaveHandler::SaveFile::SaveType::MAX_TYPE] = {
		"options",
		"stats",
		"races",
		"mission",
		"badges",
		"point",
	};

	minibson::document SaveHandler::SaveFile::Load(SaveType type, LoadStatus& status) {
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s.sav", SaveNames[(u32)type]);
		File savefile(path);
		if (!savefile.IsOpen()) {status = LoadStatus::FILE_NOT_FOUND; return minibson::document();}

		u32 saveFileSize = savefile.GetSize();
		if (saveFileSize < 0xC || saveFileSize > 0x40000) {status = LoadStatus::CORRUPTED_FILE; return minibson::document();}

		MiscUtils::Buffer buf(saveFileSize);

		CTGP7SaveFile* filedata = (CTGP7SaveFile*)buf.Data();
		savefile.Read(filedata, saveFileSize);

		if (filedata->magic != SaveMagic) {status = LoadStatus::MAGIC_MISMATCH; return minibson::document();}
		if (filedata->type != type) {status = LoadStatus::TYPE_MISMATCH; return minibson::document();}

		auto ret = minibson::crypto::decrypt(filedata->bsondata, saveFileSize - offsetof(CTGP7SaveFile, bsondata));

		if (!ret.has_value()) {status = LoadStatus::CORRUPTED_FILE; return minibson::document();}

		minibson::document& doc = ret.value();

		u64 scID = doc.get<u64>("_cID", 0); if (scID == 0) scID = doc.get<u64>("cID", 0);
		if (scID != NetHandler::GetConsoleUniqueHash()
			#ifdef ALLOW_SAVES_FROM_OTHER_CID
			&& false
			#endif
		) {
			status = LoadStatus::INCORRECT_CID;
			return minibson::document();
		}
		doc.remove("cID");

		if (type != SaveType::OPTIONS) {
			s64 saveID[2];
            saveID[0] = doc.get<s64>("sID0", 0);
            saveID[1] = doc.get<s64>("sID1", 0);
            if ((saveID[0] != 0 && SaveHandler::saveData.saveID[0] != 0 && saveID[0] != SaveHandler::saveData.saveID[0]) ||
                (saveID[1] != 0 && SaveHandler::saveData.saveID[1] != 0 && saveID[1] != SaveHandler::saveData.saveID[1])
			     #ifdef ALLOW_SAVES_FROM_OTHER_CID
				&& false
				#endif
				) {
					status = LoadStatus::INCORRECT_SID;
					return minibson::document();
				}
		}

		status = LoadStatus::SUCCESS;
		return ret.value();
	}
	void SaveHandler::SaveFile::Save(SaveType type, const minibson::document& inData) {
	#if STRESS_MODE == 1
		return;
	#endif
	#ifdef SAVE_DATA_UNENCRYPTED
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s_dec.sav", SaveNames[(u32)type]);
	#else
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s.sav", SaveNames[(u32)type]);
	#endif
		File savefile(path, File::RWC);
		if (!savefile.IsOpen()) return;
		
		u32 docSize = minibson::crypto::get_serialized_size(inData);
		u32 serializedSize = (docSize + offsetof(CTGP7SaveFile, bsondata) + 0xFFF) & ~0xFFF;
		if (serializedSize < savefile.GetSize()) {
			savefile.Close();
			File::Open(savefile, path, File::RWC | File::TRUNCATE);
			if (!savefile.IsOpen()) return;
		}
			
		MiscUtils::Buffer buf(serializedSize);
		CTGP7SaveFile* fileData = (CTGP7SaveFile*)buf.Data();

		memset(fileData, 0, serializedSize);
		fileData->magic = SaveMagic;
		fileData->type = type;
	#ifdef SAVE_DATA_UNENCRYPTED
		inData.serialize(fileData->bsondata, serializedSize - offsetof(CTGP7SaveFile, bsondata));
	#else
		minibson::crypto::encrypt(inData, fileData->bsondata, docSize);
	#endif
		
		savefile.Write(fileData, serializedSize);

		return;
	}

    void SaveHandler::SaveFile::HandleError(SaveHandler::SaveFile::SaveType type, SaveHandler::SaveFile::LoadStatus status)
    {
		if (status == SaveHandler::SaveFile::LoadStatus::FILE_NOT_FOUND)
			return;

		disableSaving = true;

		u32 error = 0x80000000 | (((u8)type) << 8) | ((u8)status);
		if (R_SUCCEEDED(plgLdrInit())) {
			PLGLDR__DisplayErrMessage("CTGP-7", "Failed to load CTGP-7 save data.\n\nCopy the error code below and ask\nfor support or reset your save data.", error);
			plgLdrExit();
		}

		Process::ReturnToHomeMenu();
		for (;;);
    }
}