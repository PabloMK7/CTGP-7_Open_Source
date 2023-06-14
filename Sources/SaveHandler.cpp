/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveHandler.cpp
Open source lines: 467/522 (89.46%)
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
#include "CharacterManager.hpp"

namespace CTRPluginFramework {
	
	SaveHandler::CTGP7Save SaveHandler::saveData;
	Task SaveHandler::saveSettinsTask(SaveHandler::SaveSettingsTaskFunc, nullptr, Task::Affinity::AppCore);
	int SaveHandler::lastAchievementCount = 0;

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
		MarioKartFramework::changeNumberRounds(saveData.numberOfRounds);

		bool* isEnabled; u32* hotkey;
		PluginMenu::ScreenshotSettings(&isEnabled, &hotkey);
		*isEnabled = saveData.flags1.screenshotEnabled;
		*hotkey = saveData.screenshotHotkey;

		if (saveData.ctVR < 1 || saveData.ctVR > 99999)
			saveData.ctVR = 1000;
		if (saveData.cdVR < 1 || saveData.cdVR > 99999)
			saveData.cdVR = 1000;

		UpdateAchievementCryptoFiles();
		lastAchievementCount = saveData.GetCompletedAchievementCount();
	}

	void SaveHandler::DefaultSettings() {
		saveData = CTGP7Save();
	}

	void SaveHandler::UpdateAchievementCryptoFiles() {
		for (int i = 0; i < saveData.GetCompletedAchievementCount(); i++) {
			CryptoResource::AllowKnownFileID((CryptoResource::KnownFileID)(0x1000 | i), true);
		}
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
			if (MissionHandler::SaveData::GetAllFullGradeFlag()) {
				SaveHandler::saveData.SetAchievementPending(Achievements::ALL_MISSION_TEN, true);
				justGranted = true;
			}
		}
		if (!SaveHandler::saveData.IsAchievementCompleted(Achievements::VR_5000) && !SaveHandler::saveData.IsAchievementPending(Achievements::VR_5000)) {
		#if CITRA_MODE == 0
			if (saveData.ctVR >= 5000 || saveData.cdVR >= 5000) {
				SaveHandler::saveData.SetAchievementPending(Achievements::VR_5000, true);
				justGranted = true;
			}
		#else
			if (justGranted) {
				SaveHandler::saveData.SetAchievementPending(Achievements::VR_5000, true);
			}
		#endif
		}
		if (justGranted) {
			SaveHandler::SaveSettingsAll();
		}
	}
	
	static u32 g_lastCharShown = 0;
	static u32 g_pendingFlagOpen = 0;
	bool SaveHandler::CheckAndShowAchievementMessages() {
		if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_GOLD)) {
			if (MarioKartFramework::isDialogOpened()) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_gold"));
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_GOLD, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_GOLD, true);
			SaveHandler::SaveSettingsAll();
			return true;
		} if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_ONE_STAR)) {
			if (MarioKartFramework::isDialogOpened()) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_1star"));
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_ONE_STAR, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_ONE_STAR, true);
			SaveHandler::SaveSettingsAll();
			return true;
		} if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_THREE_STAR)) {
			if (MarioKartFramework::isDialogOpened()) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_3star"));
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_THREE_STAR, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_THREE_STAR, true);
			SaveHandler::SaveSettingsAll();
			return true;
		} if (SaveHandler::saveData.IsAchievementPending(Achievements::ALL_MISSION_TEN)) {
			if (MarioKartFramework::isDialogOpened()) return true;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_all_mission"));
			SaveHandler::saveData.SetAchievementPending(Achievements::ALL_MISSION_TEN, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::ALL_MISSION_TEN, true);
			SaveHandler::SaveSettingsAll();
			return true;
		} if (SaveHandler::saveData.IsAchievementPending(Achievements::VR_5000)) {
			if (MarioKartFramework::isDialogOpened()) return true;
			#if CITRA_MODE == 0
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("achiev_5000_vr"));
			#endif
			SaveHandler::saveData.SetAchievementPending(Achievements::VR_5000, false);
			SaveHandler::saveData.SetAchievementCompleted(Achievements::VR_5000, true);
			SaveHandler::SaveSettingsAll();
			return true;
		} else {
			if (MarioKartFramework::isDialogOpened()) {
				if (g_pendingFlagOpen) {
					g_pendingFlagOpen--;
				}
				if (g_pendingFlagOpen == 1) {
					Snd::PlayMenu(Snd::SoundID::FLAG_OPEN);
				}
				return true;
			}
			u32 newCount = SaveHandler::saveData.GetCompletedAchievementCount();
			if (lastAchievementCount < newCount) {
				for (int i = g_lastCharShown; i < CharacterManager::charEntries.size(); i++) {
					if (CharacterManager::charEntries[i].achievementLevel > 0 && CharacterManager::charEntries[i].achievementLevel == (lastAchievementCount + 1)) {
						string16 text;
						Utils::ConvertUTF8ToUTF16(text, NAME("unlocked_item"));
						text.append(
							Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM,
								CharacterManager::charEntries[i].achievementLevel < 5 ? Color(255, 180, 0) : Color(180, 0, 180)
							)
						);
						Utils::ConvertUTF8ToUTF16(text, CharacterManager::charEntries[i].longName);
						Language::MsbtHandler::SetString(CustomTextEntries::dialog, text);
						MarioKartFramework::openDialog(DialogFlags::Mode::OK, "", nullptr, true);
						g_pendingFlagOpen = 30;
						g_lastCharShown = i+1;
						return true;
					}
				}
				g_lastCharShown = 0;
				lastAchievementCount++;
				return true;
			}
		}
		return false;
	}

	u32 SaveHandler::calculateChecksumLegacy(SaveHandler::CTGP7SaveFileLegacy* saveFile, u32 fileSize)
	{
	}

	s32 SaveHandler::SaveSettingsTaskFunc(void* args) {
		SaveSettings();
		StatsHandler::CommitToFile();
		return 0;
	}

	void SaveHandler::LoadSettingsLegacy() {
		static constexpr const char* OptionsSaveFilePathLegacy = "/CTGP-7/savefs/mod/options.sav";
		static constexpr u32 OptionsSaveFileMagicLegacy = 0x53375443;

		bool loaded = false;
		do {
			File savefile(OptionsSaveFilePathLegacy);
			if (!savefile.IsOpen()) break;

			u32 saveFileSize = savefile.GetSize();
			if (saveFileSize < 0xC || saveFileSize > 0x8000) break;

			CTGP7SaveFileLegacy* filedata = (CTGP7SaveFileLegacy*)::memalign(0x1000, saveFileSize);
			savefile.Read(filedata, saveFileSize);
			if (filedata->magic == OptionsSaveFileMagicLegacy) {
				if (calculateChecksumLegacy(filedata, saveFileSize) != filedata->checksum) {
					free(filedata);
					break;
				}
				minibson::document bsonDoc(filedata->bsondata, saveFileSize - offsetof(CTGP7SaveFileLegacy, bsondata));
				saveData = CTGP7Save(bsonDoc);
				loaded = true;
			}
			free(filedata);
		} while (false);

		if (!loaded)
			DefaultSettings();
	}

	void SaveHandler::LoadSettings() {
		SaveFile::LoadStatus status;
		minibson::encdocument doc = SaveFile::Load(SaveFile::SaveType::OPTIONS, status);
		if (status == SaveFile::LoadStatus::SUCCESS) {
			saveData = CTGP7Save(doc);
		} else if (status == SaveFile::LoadStatus::MAGIC_MISMATCH) {
			LoadSettingsLegacy();
		} else DefaultSettings();

		CupRankSave::Load();
		MissionHandler::SaveData::Load();
	}

	void SaveHandler::SaveSettings() {

		saveData.cc_settings = ccsettings[0];
		bool* isEnabled; u32* hotkey;
		PluginMenu::ScreenshotSettings(&isEnabled, &hotkey);
		saveData.screenshotHotkey = *hotkey;
		saveData.flags1.screenshotEnabled = *isEnabled;

		minibson::encdocument saveDoc;
		saveData.serialize(saveDoc);
		
		SaveFile::Save(SaveFile::SaveType::OPTIONS, saveDoc);

		CupRankSave::Save();
	}
	
	std::map<u32, u8> SaveHandler::CupRankSave::cupData; 

	u32 SaveHandler::CupRankSave::calculateCheckSumLegacy(SaveHandler::CupRankSave::LegacyCupRankFormat &data)
	{
	}
	
	
	void SaveHandler::CupRankSave::legacyLoad()
	{
	}

	void SaveHandler::CupRankSave::Load() {
		legacyLoad();
		SaveFile::LoadStatus status;
		minibson::encdocument doc = SaveFile::Load(SaveFile::SaveType::RACES, status);
		if (status == SaveFile::LoadStatus::SUCCESS) {
			if (doc.get<u64>("cID", 0ULL) == NetHandler::GetConsoleUniqueHash()) {
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

	void SaveHandler::CupRankSave::getGrandPrixData(u32 saveData, GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror) {
		if (*GPID == USERCUPID) {
			out->isCompleted = false;
			return;
		}
		if (MissionHandler::isMissionMode && MenuPageHandler::MenuSingleCupGPPage::GetInstace()->isInPage) {
			MissionHandler::OnGetGrandPrixData(out, GPID, engineLevel, isMirror);
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
		if (*GPID == USERCUPID || VersusHandler::IsVersusMode) return;
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
		"mission"
	};

	minibson::encdocument SaveHandler::SaveFile::Load(SaveType type, LoadStatus& status) {
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s.sav", SaveNames[(u32)type]);
		File savefile(path);
		if (!savefile.IsOpen()) {status = LoadStatus::FILE_NOT_FOUND; return minibson::encdocument();}

		u32 saveFileSize = savefile.GetSize();
		if (saveFileSize < 0xC || saveFileSize > 0x10000) {status = LoadStatus::CORRUPTED_FILE; return minibson::encdocument();}

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
		std::string path = Utils::Format("/CTGP-7/savefs/mod/%s.sav", SaveNames[(u32)type]);
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
		inData.serialize(fileData->bsondata, serializedSize - offsetof(CTGP7SaveFile, bsondata));
		
		savefile.Write(fileData, serializedSize);
		free((u8*)fileData);

		return;
	}
}