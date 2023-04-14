/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CharacterManager.cpp
Open source lines: 820/820 (100.00%)
*****************************************************/

#include "CharacterManager.hpp"
#include "Lang.hpp"
#include "ExtraResource.hpp"
#include "cheats.hpp"
#include "3ds.h"
#include "TextFileParser.hpp"
#include <string.h>
#include "str16utils.hpp"
#include "MenuPage.hpp"
#include "SaveHandler.hpp"

namespace CTRPluginFramework {
	extern const char* g_LangFilenames[];
	const char* CharacterManager::shortNames[CharacterManager::CharID::LAST] =	{
			"bw",
			"dk",
			"ds",
			"hq",
			"kt",
			"lg",
			"lk",
			"mii",
			"mr",
			"mtl",
			"pc",
			"rs",
			"sh",
			"td",
			"wig",
			"wr",
			"ys"
	};
	const int CharacterManager::msbtOrder[CharacterManager::CharID::LAST] = {
		4,
		5,
		9,
		13,
		7,
		1,
		10,
		15,
		0,
		14,
		2,
		11,
		16,
		6,
		12,
		8,
		3
	};
	const CharacterManager::CharID CharacterManager::driverIDOrder[EDriverID::DRIVER_SIZE] = {
		CharacterManager::BOWSER,
		CharacterManager::DAISY,
		CharacterManager::DONKEY_KONG,
		CharacterManager::HONEY_QUEEN,
		CharacterManager::KOOPA_TROOPA,
		CharacterManager::LAKITU,
		CharacterManager::LUIGI,
		CharacterManager::MARIO,
		CharacterManager::METAL_MARIO,
		CharacterManager::MII,
		CharacterManager::MII,
		CharacterManager::PEACH,
		CharacterManager::ROSALINA,
		CharacterManager::SHY_GUY,
		CharacterManager::TOAD,
		CharacterManager::WARIO,
		CharacterManager::WIGGLER,
		CharacterManager::YOSHI
	};

	CharacterManager::CharacterSave CharacterManager::currSave;

	std::vector<CharacterManager::CharacterEntry> CharacterManager::charEntries;
	std::vector<CharacterManager::CharacterEntry> CharacterManager::enabledEntries;
	std::vector<std::string> CharacterManager::origCharNames;
	std::vector<std::string> CharacterManager::authorNames;

	std::string CharacterManager::patchProgress1;
	std::string CharacterManager::patchProgress2;

	FileCopier* CharacterManager::copier = nullptr;

	bool CharacterManager::thankYouFilterCharacterFiles = false;

	FS_Archive CharacterManager::FaceRaiderImage::archive = 0;
	std::string CharacterManager::FaceRaiderImage::facesDir = "";
	u32 CharacterManager::FaceRaiderImage::customFacesAmount = 0;

	extern "C" size_t strnlen (const char *s, size_t maxlen);
	void CharacterManager::Initialize()
	{
		File configFile("/CTGP-7/MyStuff/Characters/config.bin", File::READ);
		if (configFile.IsOpen()) {
			configFile.Read(&currSave, sizeof(CharacterSave));
			configFile.Close();
		}
		if (currSave.magic != 0x56534843 || currSave.version != 2) {
			currSave = CharacterSave();
		}
		if (File::Exists("/CTGP-7/config/forcePatch.flag"))
		{
			File::Remove("/CTGP-7/config/forcePatch.flag");
			currSave.needsPatching = true;
		}
		populateCharEntries();
		int achievementLevel = SaveHandler::saveData.GetCompletedAchievementCount();
		for (int i = 0; i < CharID::LAST; i++) {
			if (strnlen(currSave.inUseNames[i], 0x20) == 0) continue;
			for (int j = 0; j < charEntries.size(); j++) {
				if (charEntries[j].origChar == i && strncmp(charEntries[j].folderName.c_str(), currSave.inUseNames[i], 0x20) == 0 && charEntries[j].achievementLevel <= achievementLevel) {						
					enabledEntries.push_back(charEntries[j]);
				}
			}
		}
	}

	u16 CharacterManager::InsertAuthor(const std::string& author) {
        auto it = std::find(authorNames.begin(), authorNames.end(), author);
        if (it == authorNames.end())
            it = authorNames.insert(it, author);
        return it - authorNames.begin();
    }

	std::vector<std::string> CharacterManager::GetAllAuthors() {
		std::vector<int> freq(authorNames.size(), 0);
        for (auto it = charEntries.cbegin(); it != charEntries.cend(); it++) {
            for (auto it2 = it->authors.cbegin(); it2 != it->authors.cend(); it2++) {
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

	void CharacterManager::populateCharEntries()
	{
		Directory charRootDir("/CTGP-7/MyStuff/Characters");
		if (!charRootDir.IsOpen()) return;
		std::vector<std::string> dirs;
		std::string pat = "";
		charRootDir.ListDirectories(dirs);

		std::string nameStr = std::string("name_") + Language::GetCurrLangID();
		std::string engLongName, engShortName;

		CharacterEntry newEntry;

		for (int i = 0; i < dirs.size(); i++) {
			bool origChar = false;
			TextFileParser currParser;
			bool wasParsed = currParser.Parse(charRootDir.GetFullName() + "/" + dirs[i] + "/config.ini");
			if (!wasParsed) continue;
			newEntry.sarc = nullptr;
			newEntry.longName = currParser.getEntry(nameStr, false);
			newEntry.shortName = currParser.getEntry(nameStr, true);
			std::string charID = currParser.getEntry("origChar", false);
			for (int j = 0; j < CharID::LAST; j++) {
				if (charID.compare(shortNames[j]) == 0) {
					newEntry.origChar = (CharID)j;
					origChar = true;
					if (newEntry.origChar == CharID::WIGGLER && strncmp(currSave.inUseNames[j], dirs[i].c_str(), 0x20) == 0 && currParser.getEntry("disableAngry", false) == "true")
						MarioKartFramework::allowWigglerAngry = false;
					break;
				}
			}
			if (newEntry.longName.empty() || newEntry.shortName.empty()) {
				newEntry.longName = currParser.getEntry("name_ENG", false);
				newEntry.shortName = currParser.getEntry("name_ENG", true);
				if (newEntry.longName.empty() || newEntry.shortName.empty()) continue;
			}

			newEntry.faceRaiderOffset = 0;
			std::string faceRaiderFaceOffset = currParser.getEntry("faceRaiderFaceOffset");
			if (!faceRaiderFaceOffset.empty() && TextFileParser::IsValidNumber(faceRaiderFaceOffset) != TextFileParser::NumberType::INVALID) {
				newEntry.faceRaiderOffset = std::stoi(faceRaiderFaceOffset, nullptr, 16);
			}
			std::string allowInThankYou = currParser.getEntry("allowThankYou");
			newEntry.creditsAllowed = allowInThankYou == "true";

			newEntry.achievementLevel = 0;
			std::string achievementLevel = currParser.getEntry("achievementLevel");
			if (!achievementLevel.empty()) {
				int level = 0;
				if (Utils::ToInteger(achievementLevel, level) == Utils::ConvertResult::SUCCESS && level >= 0)
					newEntry.achievementLevel = level;
			}

			if (!origChar) continue;
			newEntry.folderName = dirs[i];
			
			auto authorList = currParser.getEntries("authors");
			for (auto it = authorList.cbegin(); it != authorList.cend(); it++) {
				newEntry.authors.push_back(InsertAuthor(*it));
			}

			charEntries.push_back(newEntry);
		}
	}
	void CharacterManager::saveSettings()
	{
		File writeConfigFile("/CTGP-7/MyStuff/Characters/config.bin", File::RWC);
		if (writeConfigFile.IsOpen()) {
			writeConfigFile.Write(&currSave, sizeof(CharacterSave));
		}
	}
	static int g_charManCopyStep;
	static int g_charManFileStep;
	void CharacterManager::applyDirectoryPatches()
	{
		if (currSave.needsPatching) {
			copier = new FileCopier(FileCopy1, FileCopy2);
			patchProgress1 = "Cleaning up...";
			OSD::Run(LogPatchProgress);
			Directory::Remove("/CTGP-7/gamefs/Driver");
			Directory::Remove("/CTGP-7/gamefs/Kart");
			Directory::Create("/CTGP-7/gamefs/Driver");
			if (currSave.customKartsEnabled) {
				patchProgress1 = "Listing kart patches...";
				copyDirectory("/CTGP-7/gamefs/Kart", "/CTGP-7/MyStuff/Karts/Kart");
				copier->Run();
				delete copier;
				copier = new FileCopier(FileCopy1, FileCopy2);
				patchProgress2 = "";
			}
			patchProgress1 = "Listing character patches...";
			for (int i = 0; i < enabledEntries.size(); i++) {
				copyDirectory("/CTGP-7/gamefs/Driver", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/Driver");
				copyDirectory("/CTGP-7/gamefs/Kart", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/Kart");
			}
			copier->Run();
			patchProgress1 = "";
			patchProgress2 = "";
			delete copier;
			copier = nullptr;
			OSD::Stop(LogPatchProgress);
		}
	}
	void CharacterManager::ipsCallback(IPS::IFile& f) {
		float progress = (f.offset / (float)f.totProgress) * 100;
		char buf[0x20];
		std::sprintf(buf, "%.2f", progress);
		patchProgress1 = "Applying ips patch " + std::to_string(g_charManFileStep) + std::string(": ") << buf + std::string("%");
	}
	void CharacterManager::applySoundPatches()
	{
		if (currSave.needsPatching) {
			patchProgress1 = "Cleaning up...";
			OSD::Run(LogPatchProgress);
			g_charManFileStep = 1;
			for (int i = 0; i < CharID::LAST; i++) {
				std::string shName = shortNames[i];
				std::transform(shName.begin(), shName.end(), shName.begin(), toupper);
				File::Remove("/CTGP-7/gamefs/Sound/extData/GRP_VO_" + shName + "_GOL.bcgrp");
			}
			File::Remove("/CTGP-7/gamefs/Sound/ctr_dash.bcsar");
			File::Remove("/CTGP-7/gamefs/Sound/extData/GRP_VO_MENU.bcgrp");
			File::Remove("/CTGP-7/gamefs/Driver/common/wing_std_color.bcmcla");
			Directory::Create("/CTGP-7/gamefs/Sound");
			Directory::Create("/CTGP-7/gamefs/Sound/extData");
			Directory::Create("/CTGP-7/gamefs/Driver/common");
			bool firstBcsarPatch = !IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/ctr_dash.bcsar", "romfs:/Sound/ctr_dash.bcsar", "/CTGP-7/resources/empty.ips", ipsCallback);
			g_charManFileStep++;
			bool firstMenuPatch = true;
			bool firstWingPatch = true;
			for (int i = 0; i < enabledEntries.size(); i++) {
				if (firstBcsarPatch) firstBcsarPatch = !IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/ctr_dash.bcsar", "romfs:/Sound/ctr_dash.bcsar", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/bcsarSoundData.ips", ipsCallback);
				else IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/ctr_dash.bcsar", "", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/bcsarSoundData.ips", ipsCallback);
				g_charManFileStep++;
				std::string shName = shortNames[enabledEntries[i].origChar];
				std::transform(shName.begin(), shName.end(), shName.begin(), toupper);
				IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/extData/GRP_VO_" + shName + "_GOL.bcgrp", "romfs:/Sound/extData/GRP_VO_" + shName + "_GOL.bcgrp", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/voiceSoundData.ips", ipsCallback);
				g_charManFileStep++;
				if (firstMenuPatch) firstMenuPatch = !IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/extData/GRP_VO_MENU.bcgrp", "romfs:/Sound/extData/GRP_VO_MENU.bcgrp", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/menuSoundData.ips", ipsCallback);
				else IPS::applyIPSPatch("/CTGP-7/gamefs/Sound/extData/GRP_VO_MENU.bcgrp", "", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/menuSoundData.ips", ipsCallback);
				g_charManFileStep++;
				if (firstWingPatch) firstWingPatch = !IPS::applyIPSPatch("/CTGP-7/gamefs/Driver/common/wing_std_color.bcmcla", "romfs:/Driver/common/wing_std_color.bcmcla", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/stdWingColor.ips", ipsCallback);
				else IPS::applyIPSPatch("/CTGP-7/gamefs/Driver/common/wing_std_color.bcmcla", "", "/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/stdWingColor.ips", ipsCallback);
				g_charManFileStep++;
			}
			patchProgress1 = "Done!";
			Sleep(Seconds(0.1));
			OSD::Stop(LogPatchProgress);
		}
	}
	void CharacterManager::applySarcPatches()
	{
		if (currSave.customKartsEnabled) {
			File kartFile("/CTGP-7/MyStuff/Karts/UI.sarc");
			ExtraResource::SARC* kartSARC = new ExtraResource::SARC(kartFile);
			if (!kartSARC->processed) {
				delete kartSARC;
			}
			else {
				ExtraResource::mainSarc->Append(kartSARC, true);
			}
		}
		for (int i = 0; i < enabledEntries.size(); i++) {
			File sarcFile("/CTGP-7/MyStuff/Characters/" + enabledEntries[i].folderName + "/UI.sarc");
			ExtraResource::SARC* charSARC = new ExtraResource::SARC(sarcFile);
			if (!charSARC->processed) {
				delete charSARC;
				continue;
			}
			enabledEntries[i].sarc = charSARC;
			ExtraResource::mainSarc->Append(charSARC, true);
		}
	}
	void CharacterManager::UpdateMsbtPatches()
	{
		if (!Language::msbtReady)
			return;
		if (origCharNames.empty()) for (int i = 0; i < CharID::LAST; i++) {
			origCharNames.push_back(Language::MsbtHandler::GetString(1000 + msbtOrder[i]));
			Language::MsbtHandler::SetString(1000 + msbtOrder[i], 
				Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(245, 245, 245)) + 
				Language::MsbtHandler::DashTextWithTagsToString(Language::MsbtHandler::GetText(1000 + msbtOrder[i]))
			);
		}
		for (int i = 0; i < enabledEntries.size(); i++) {
			string16 longStr16, shortStr16;
			Utils::ConvertUTF8ToUTF16(longStr16, enabledEntries[i].longName);
			Utils::ConvertUTF8ToUTF16(shortStr16, enabledEntries[i].shortName);
			if (enabledEntries[i].achievementLevel > 0 && enabledEntries[i].achievementLevel <= SaveHandler::saveData.GetCompletedAchievementCount()) {
				if (enabledEntries[i].achievementLevel >= 5)
					longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 0, 255)) + longStr16;
				else
					longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 255, 0)) + longStr16;
			} else {
				longStr16 = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(245, 245, 245)) + longStr16;
			}
			Language::MsbtHandler::SetString(1000 + msbtOrder[enabledEntries[i].origChar], longStr16);
			Language::MsbtHandler::SetString(1050 + msbtOrder[enabledEntries[i].origChar], shortStr16);
		}
	}
	bool CharacterManager::LogPatchProgress(const Screen& screen)
	{
		if (!screen.IsTop) return false;
		std::string tmpstr = patchProgress1 + "                             ";
		screen.Draw(tmpstr, 10, 10);
		tmpstr = patchProgress2 + "                             ";
		screen.Draw(tmpstr, 10, 20);
		return true;
	}
	void CharacterManager::copyFile(u8* copyBuffer, u32 copyBufferSize, std::string &dst, std::string &ori, std::string& progStr, int currFile, int totFile)
	{
		File::Remove(dst);
		File oriFile(ori, File::READ);
		File dstFile(dst, File::RWC);
		if (!oriFile.IsOpen() || !dstFile.IsOpen()) return;
		u32 fileSize = oriFile.GetSize();
		u32 currSize = 0, totSize = fileSize;

		while (fileSize != 0) {
			u32 block = (fileSize > copyBufferSize) ? copyBufferSize : fileSize;
			oriFile.Read(copyBuffer, block);
			dstFile.Write(copyBuffer, block);
			currSize += block;
			fileSize -= block;

			progStr = Utils::Format("Copy file %d/%d: %.2f%%", currFile, totFile, (currSize / (float)totSize) * 100.f);
		}
	}

	void CharacterManager::copyDirectory(std::string dst, std::string ori)
	{
		if (!Directory::IsExists(dst)) Directory::Create(dst);
		Directory oriDir(ori);
		if (!oriDir.IsOpen()) return;
		std::vector<std::string> dirs;
		std::vector<std::string> files;
		oriDir.ListDirectories(dirs);
		oriDir.ListFiles(files);

		for (int i = 0; i < files.size(); i++) {
			std::string l = dst << "/" << files[i];
			std::string r = ori << "/" << files[i];
			std::tuple<std::string, std::string> t = std::tuple<std::string, std::string>(l, r);
			copier->AddFileTuple(t);
		}
		files.clear();

		for (int i = 0; i < dirs.size(); i++) {
			copyDirectory(dst << "/" << dirs[i], ori << "/" << dirs[i]);
		}
		
	}

	static u32 g_faceMenuAmountOptions = 0;
	static u32 g_faceMenuCurrentOption = 0;
	static CharacterManager::FaceRaiderImage* g_faceMenuFaceRaiderImage;
	static int g_faceMenuTileOrder[] =
	{
		0,  1,   4,  5,
		2,  3,   6,  7,

		8,  9,   12, 13,
		10, 11,  14, 15
	};

	void CharacterManager::OnFaceRaiderMenuEvent(Keyboard&, KeyboardEvent &event) {
		if (event.type == KeyboardEvent::SelectionChanged) {
			g_faceMenuCurrentOption = event.selectedIndex;
			if (g_faceMenuFaceRaiderImage) {
				delete g_faceMenuFaceRaiderImage;
				g_faceMenuFaceRaiderImage = nullptr;
			}
			if (g_faceMenuCurrentOption < g_faceMenuAmountOptions - 1) {
				g_faceMenuFaceRaiderImage = new CharacterManager::FaceRaiderImage(g_faceMenuCurrentOption);
			}
		} else if (event.type == KeyboardEvent::FrameTop && g_faceMenuCurrentOption != 0 && g_faceMenuFaceRaiderImage && g_faceMenuFaceRaiderImage->isLoaded) {
			int startX = (400 - 128) / 2;
			int startY = (240 - 128) / 2;
			int offs = 0;
			for (int y = 0; y < 128; y+=8) {
				for (int x = 0; x < 128; x+=8) {
					for (int i = 0; i < 64; i++) {
						int x2 = i % 8;
						if (x + x2 >= 128) continue;
						int y2 = i / 8;
						if (y + y2 >= 128) continue;
						int pos = g_faceMenuTileOrder[x2 % 4 + y2 % 4 * 4] + 16 * (x2 / 4) + 32 * (y2 / 4);
						u16 pixel = g_faceMenuFaceRaiderImage->pixelData[offs + pos];
						u8 b = (u8)((pixel & 0x1F) << 3);
						u8 g = (u8)((pixel & 0x7E0) >> 3);
						u8 r = (u8)((pixel & 0xF800) >> 8);
						event.renderInterface->DrawPixel(startX + x + x2, startY + y + y2, Color(r, g, b));
					}
					offs += 64;
				}
			}
			event.renderInterface->DrawRect(IntRect(startX, startY, 128, 128), Color::Red, false, 2);
		}
	}

	bool CharacterManager::openFaceRaiderMenu(const CharacterEntry& entry) {
		Keyboard faceSelector(entry.longName + " - " + NAME("face_raider_sel"));
		StringVector options;
		u32 faceAmount = FaceRaiderImage::Initialize("/CTGP-7/MyStuff/Characters/" + entry.folderName + "/Faces/");
		for (int i = 0; i < faceAmount; i++) {
			options.push_back( i == 0 ? NAME("default") : std::to_string(i));
		}
		options.push_back(NAME("exit"));
		g_faceMenuAmountOptions = options.size();
		g_faceMenuCurrentOption = 0;
		faceSelector.Populate(options);
		faceSelector.ChangeEntrySound(g_faceMenuAmountOptions - 1, SoundEngine::Event::CANCEL);
		faceSelector.OnKeyboardEvent(OnFaceRaiderMenuEvent);
		g_faceMenuFaceRaiderImage = new CharacterManager::FaceRaiderImage(g_faceMenuCurrentOption);
		int res = faceSelector.Open();
		bool result = false;
		if (res >= 0 && res != g_faceMenuAmountOptions - 1 && g_faceMenuFaceRaiderImage) {
			std::string modelFilePath = "/CTGP-7/MyStuff/Characters/" + entry.folderName + "/Driver/" + shortNames[entry.origChar] + "/" + shortNames[entry.origChar];
			if (entry.origChar == SHY_GUY)
				modelFilePath += "_red";
			modelFilePath += ".bcmdl";
			File modelFile(modelFilePath, File::READ | File::WRITE );
			if (modelFile.IsOpen()) {
				modelFile.Seek(entry.faceRaiderOffset);
				modelFile.Write(g_faceMenuFaceRaiderImage->pixelData, 0x8000);
			}
			result = true;
		}
		if (g_faceMenuFaceRaiderImage) {
			delete g_faceMenuFaceRaiderImage;
			g_faceMenuFaceRaiderImage = nullptr;
		}
		FaceRaiderImage::Finalize();
		return result;
	}

	void CharacterManager::characterManagerSettings(MenuEntry* entry) {
		Keyboard charopt(NAME("charman") + "\n\n" + NAME("char_select") + "\n" + NAME("reboot_req"));
		std::vector<std::string> options;
		std::vector<int> countEach;
		options.resize(CharID::LAST);
		countEach.resize(CharID::LAST);
		int achievementLevel = SaveHandler::saveData.GetCompletedAchievementCount();
		for (int i = 0; i < CharID::LAST; i++) {
			u32 count = 1;
			bool found = false;
			for (int j = 0; j < charEntries.size(); j++) {
				if (charEntries[j].origChar == i && charEntries[j].achievementLevel <= achievementLevel) {
					count++;
					if (!found && strncmp(currSave.inUseNames[i], charEntries[j].folderName.c_str(), 0x20) == 0) {
						found = true;
						options[msbtOrder[i]] = charEntries[j].longName;
					}
				}
			}
			if (!found) options[msbtOrder[i]] = origCharNames[i];
			options[msbtOrder[i]] += " (" + std::to_string(count) + ")";
			countEach[i] = count;
		}
		int result = 0;
		while (result >= 0) {
			charopt.Populate(options, false);
			result = charopt.Open();
			if (result >= 0) {
				CharID selectedCustomChar;
				for (int i = 0; i < CharID::LAST; i++) {
					if (result == msbtOrder[i]) { selectedCustomChar = (CharID)i; break; }
				}
				CharacterEntry origEntry;
				int greenEntry = 0;
				origEntry.folderName = "";
				origEntry.origChar = selectedCustomChar;
				origEntry.longName = origCharNames[selectedCustomChar];
				origEntry.achievementLevel = 0;
				origEntry.creditsAllowed = true;
				origEntry.faceRaiderOffset = 0;
				std::vector<CharacterEntry> thisCharEntries;
				thisCharEntries.push_back(origEntry);
				int currCusEntry = 1;
				for (int i = 0; i < charEntries.size(); i++) {
					if (charEntries[i].origChar != selectedCustomChar || charEntries[i].achievementLevel > achievementLevel) continue;
					if (strncmp(currSave.inUseNames[selectedCustomChar], charEntries[i].folderName.c_str(), 0x20) == 0) {
						greenEntry = currCusEntry;
					}
					thisCharEntries.push_back(charEntries[i]);
					currCusEntry++;
				}
				std::vector<std::string> customOption;
				currCusEntry = 0;
				for (int i = 0; i < thisCharEntries.size(); i++) {
					if (greenEntry == currCusEntry) customOption.push_back(std::string(Color::Lime << thisCharEntries[i].longName));
					else if (thisCharEntries[i].achievementLevel == 5) customOption.push_back(std::string(Color(255, 0, 255) << thisCharEntries[i].longName));
					else if (thisCharEntries[i].achievementLevel > 0) customOption.push_back(std::string(Color(255, 255, 0) << thisCharEntries[i].longName));
					else customOption.push_back(thisCharEntries[i].longName);
					currCusEntry++;
				}
				Keyboard customCharOpt(NAME("char_replace") + options[result] + std::string(Color::Lime << "\n\n" + NAME("green_info")));
				customCharOpt.Populate(customOption);
				customCharOpt.ChangeSelectedEntry(greenEntry);
				int result2 = customCharOpt.Open();
				if (result2 >= 0) {
					bool forceSave = false;
					if (thisCharEntries[result2].faceRaiderOffset != 0) forceSave = openFaceRaiderMenu(thisCharEntries[result2]);
					if (result2 != greenEntry || forceSave) {
						memset(currSave.inUseNames[selectedCustomChar], 0, 0x20);
						strncpy(currSave.inUseNames[selectedCustomChar], thisCharEntries[result2].folderName.c_str(), 0x20);
						options[msbtOrder[selectedCustomChar]] = thisCharEntries[result2].longName + " (" + std::to_string(countEach[selectedCustomChar]) + ")";
						currSave.needsPatching = true;
						saveSettings();
					}
				}
			}
		}
	}
	void CharacterManager::enableCustomKartsSettings(MenuEntry* entry)
	{
		Keyboard kbd(NAME("cuskart") + "\n\n" + NAME("state") + ": " + (currSave.customKartsEnabled ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) + ResetColor() + "\n\n" + NAME("reboot_req"));
		kbd.Populate({ NAME("state_inf"), NOTE("state_inf") });
		kbd.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = kbd.Open();
		if (ret < 0) return;
		bool newOpt = ret == 0;
		if (newOpt != currSave.customKartsEnabled) {
			currSave.customKartsEnabled = newOpt;
			currSave.needsPatching = true;
			saveSettings();
			entry->Name() = NAME("cuskart") + " (" + (currSave.customKartsEnabled ? NAME("state_mode") : NOTE("state_mode")) + ")";
		}
	}

	static std::vector<u8> g_thankyoublacklistedchars;
	void CharacterManager::OnThankYouLoad(bool isBefore) {
		thankYouFilterCharacterFiles = isBefore;

		if (isBefore) for (u8 i = CharID::BOWSER; i < CharID::LAST; i++) {
			g_thankyoublacklistedchars.push_back(i);
		}
		for (auto it = enabledEntries.begin(); it != enabledEntries.end(); it++) {
			if (it->sarc && (!MenuPageHandler::MenuEndingPage::loadCTGPCredits || !it->creditsAllowed))
				it->sarc->SetEnabled(!isBefore);
			if (isBefore && it->creditsAllowed) {
				auto it2 = std::find(g_thankyoublacklistedchars.begin(), g_thankyoublacklistedchars.end(), it->origChar);
				if (it2 != g_thankyoublacklistedchars.end());
					g_thankyoublacklistedchars.erase(it2);
			}
		}
		if (!isBefore) {
			g_thankyoublacklistedchars.clear();
		}
	}

	bool CharacterManager::filterChararacterFileThankYou(u16* filename) {
		if (!MenuPageHandler::MenuEndingPage::loadCTGPCredits) {
			filename[0] = '\0';
			return true;
		}
		string16 file = string16(filename);
		for (auto it = g_thankyoublacklistedchars.begin(); it != g_thankyoublacklistedchars.end(); it++) {
			string16 out;
			Utils::ConvertUTF8ToUTF16(out, shortNames[*it]);
			string16 kind1 = (u16*)u"_" + out + (u16*)u".";
			string16 kind2 = (u16*)u"/" + out;
			if (file.find(kind1) != file.npos || file.find(kind2) != file.npos) {
				filename[0] = '\0';
				return true;
			}
		}
		return false;
	}

	s32 CharacterManager::FileCopy1(void* arg)
	{
		return FileCopy(static_cast<FileCopier*>(arg), 0);
	}

	s32 CharacterManager::FileCopy2(void* arg)
	{
		return FileCopy(static_cast<FileCopier*>(arg), 1);
	}

	s32 CharacterManager::FileCopy(FileCopier* cop, int ID)
	{
		u32 totalFiles = cop->GetTotalFiles();
		u8* copyBuf = (u8*)memalign(0x1000, 0x4000);
		while (true) {
			std::tuple<std::string, std::string> tuple;
			u32 currentFile = (totalFiles - cop->GetRemainingFiles()) + 1;
			if (!cop->GetNextTuple(tuple)) break;
			copyFile(copyBuf, 0x4000, std::get<0>(tuple), std::get<1>(tuple), ID == 0 ? patchProgress1 : patchProgress2, currentFile, totalFiles);
		}
		free(copyBuf);
		return 0;
	}

	FileCopier::FileCopier(TaskFunc fileFunction1, TaskFunc fileFunction2)
	{
		#if CITRA_MODE == 0
		Tasks[0] = new Task(fileFunction1, (void*)this, Task::Affinity::AppCores);
		Tasks[1] = new Task(fileFunction2, (void*)this, Task::Affinity::AppCores);
		#else
		Tasks[0] = new Task(fileFunction1, (void*)this, Task::Affinity::AppCore);
		#endif
	}

	FileCopier::~FileCopier()
	{
		Files.clear();
		delete Tasks[0];
		#if CITRA_MODE == 0
		delete Tasks[1];
		#endif
	}

	void FileCopier::AddFileTuple(std::tuple<std::string, std::string>& tuple)
	{
		Lock lock(GetMutex);
		Files.push_back(tuple);
		TotalFiles++;
	}

	bool FileCopier::GetNextTuple(std::tuple<std::string, std::string>& outTuple)
	{
		Lock lock(GetMutex);
		if (Files.empty()) return false;
		outTuple = Files.back();
		Files.pop_back();
		return true;
	}

	int FileCopier::GetTotalFiles() {
		return TotalFiles;
	}

	int FileCopier::GetRemainingFiles()
	{
		Lock lock(GetMutex);
		return Files.size();
	}

	void FileCopier::Run()
	{
		Tasks[0]->Start();
		#if CITRA_MODE == 0
		Tasks[1]->Start();
		#endif

		Tasks[0]->Wait();
		#if CITRA_MODE == 0
		Tasks[1]->Wait();
		#endif
	}

	CharacterManager::FaceRaiderImage::FaceRaiderImage(u32 id) {
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

	CharacterManager::FaceRaiderImage::~FaceRaiderImage() {
		if (isLoaded && pixelData)
			operator delete(pixelData);
	}

	u32 CharacterManager::FaceRaiderImage::Initialize(const std::string& extraFacesPath) {
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

	void CharacterManager::FaceRaiderImage::Finalize() {
		facesDir = "";
		customFacesAmount = 0;
		if (archive) {
			FSUSER_CloseArchive(archive);
			archive = 0;
		}
	}
}