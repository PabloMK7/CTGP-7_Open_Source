/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveHandler.cpp
Open source lines: 579/582 (99.48%)
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

namespace CTRPluginFramework {
	
	SaveHandler::CTGP7Save SaveHandler::saveData;
	Task SaveHandler::saveSettinsTask(SaveHandler::SaveSettingsTaskFunc, nullptr, Task::Affinity::AppCore);
	int SaveHandler::lastAchievementCount = 0;
	u32 SaveHandler::lastSpecialAchievements = 0;

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

	s32 SaveHandler::SaveSettingsTaskFunc(void* args) {
		SaveSettings();
		StatsHandler::CommitToFile();
		PointsModeHandler::SaveSaveData();
		return 0;
	}

	void SaveHandler::LoadSettings() {
		SaveFile::LoadStatus status;
		minibson::encdocument doc = SaveFile::Load(SaveFile::SaveType::OPTIONS, status);
		u64 scID = doc.get<u64>("cID", 0);
		if (status == SaveFile::LoadStatus::SUCCESS && (scID == NetHandler::GetConsoleUniqueHash()
		#if CITRA_MODE == 1
		|| scID == 0x5AFF5AFF5AFF5AFF
		#endif
		#ifdef ALLOW_SAVES_FROM_OTHER_CID
		|| true
		#endif
		)) {
			saveData = std::move(CTGP7Save(doc));
		} else DefaultSettings();

		CupRankSave::Load();
		MissionHandler::SaveData::Load();
		PointsModeHandler::LoadSaveData();
	}

	void SaveHandler::SaveSettings() {

		saveData.cc_settings = ccsettings[0];

		minibson::encdocument saveDoc;
		saveDoc.set<u64>("cID", NetHandler::GetConsoleUniqueHash());
		saveData.serialize(saveDoc);
		
		SaveFile::Save(SaveFile::SaveType::OPTIONS, saveDoc);

		CupRankSave::Save();
	}
	
	std::map<u32, u8> SaveHandler::CupRankSave::cupData; 

	void SaveHandler::CupRankSave::Load() {
		SaveFile::LoadStatus status;
		minibson::encdocument doc = SaveFile::Load(SaveFile::SaveType::RACES, status);
		if (status == SaveFile::LoadStatus::SUCCESS) {
			u64 scID = doc.get<u64>("cID", 0ULL);
			if (scID == NetHandler::GetConsoleUniqueHash()
			#if CITRA_MODE == 1
			|| scID == 0x5AFF5AFF5AFF5AFF
			#endif
			#ifdef ALLOW_SAVES_FROM_OTHER_CID
			|| true
			#endif
			) {
				const minibson::document& cuprankdoc = doc.get("cuprank", minibson::document());
				for (auto it = cuprankdoc.cbegin(); it != cuprankdoc.cend(); it++) {
					if (it->second->get_node_code() == minibson::bson_node_type::null_node)
					{
						u32 value = std::stoi(it->first);
						u32 cupId = value >> 8;
						u8 data = value & 0xFF;
						cupData[cupId] = data;
					}
				}
			}
		}
	}

	void SaveHandler::CupRankSave::Save() {

		minibson::document data;
		for (auto it = cupData.cbegin(); it != cupData.cend(); it++) {
			if (it->second) {
				data.set(std::to_string((it->first << 8) | it->second).c_str());
			}
		}

		minibson::encdocument saveDoc;
		saveDoc.set("cID", NetHandler::GetConsoleUniqueHash());
		saveDoc.set("cuprank", data);
		
		SaveFile::Save(SaveFile::SaveType::RACES, saveDoc);
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
		GrandPrixData current;
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

	minibson::encdocument SaveHandler::SaveFile::Load(SaveType type, LoadStatus& status) {
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s.sav", SaveNames[(u32)type]);
		File savefile(path);
		if (!savefile.IsOpen()) {status = LoadStatus::FILE_NOT_FOUND; return minibson::encdocument();}

		u32 saveFileSize = savefile.GetSize();
		if (saveFileSize < 0xC || saveFileSize > 0x40000) {status = LoadStatus::CORRUPTED_FILE; return minibson::encdocument();}

		CTGP7SaveFile* filedata = (CTGP7SaveFile*)::memalign(0x1000, saveFileSize);
		savefile.Read(filedata, saveFileSize);
		if (filedata->magic != SaveMagic) {free(filedata); status = LoadStatus::MAGIC_MISMATCH; return minibson::encdocument();}
		if (filedata->type != type) {free(filedata); status = LoadStatus::TYPE_MISMATCH; return minibson::encdocument();}

		minibson::encdocument bsonDoc(filedata->bsondata, saveFileSize - offsetof(CTGP7SaveFile, bsondata));
		if (!bsonDoc.valid) { free(filedata); status = LoadStatus::CORRUPTED_FILE; return minibson::encdocument();}
		free(filedata);
		status = LoadStatus::SUCCESS;
		return bsonDoc;
	}
	void SaveHandler::SaveFile::Save(SaveType type, const minibson::encdocument& inData) {
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
		
		u32 docSize = inData.get_serialized_size();
		u32 serializedSize = (docSize + offsetof(CTGP7SaveFile, bsondata) + 0xFFF) & ~0xFFF;
		if (serializedSize < savefile.GetSize()) {
			savefile.Close();
			File::Open(savefile, path, File::RWC | File::TRUNCATE);
			if (!savefile.IsOpen()) return;
		}
			
		CTGP7SaveFile* fileData = (CTGP7SaveFile*)::memalign(0x1000, serializedSize);
		memset(fileData, 0, serializedSize);
		fileData->magic = SaveMagic;
		fileData->type = type;
	#ifdef SAVE_DATA_UNENCRYPTED
		inData.minibson::document::serialize(fileData->bsondata, serializedSize - offsetof(CTGP7SaveFile, bsondata));
	#else
		inData.serialize(fileData->bsondata, serializedSize - offsetof(CTGP7SaveFile, bsondata));
	#endif
		
		savefile.Write(fileData, serializedSize);
		free((u8*)fileData);

		return;
	}
}