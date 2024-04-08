/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Lang.cpp
Open source lines: 825/826 (99.88%)
*****************************************************/

#include "Lang.hpp"
#include "main.hpp"
#include "CTRPluginFramework.hpp"
#include "CharacterHandler.hpp"
#include "3ds.h"
#include <algorithm>
#include "CourseManager.hpp"
#include "str16utils.hpp"
#include "VersusHandler.hpp"
#include "MarioKartFramework.hpp"
#include "MissionHandler.hpp"
#include "entrystructs.hpp"
#include "foolsday.hpp"
#include "CustomTextEntries.hpp"
#include "BCLIM.hpp"
#include "BlueCoinChallenge.hpp"

namespace CTRPluginFramework
{
    TextFileParser     Language::currLanguage;
	std::vector<std::pair<int,int>> Language::queuedReferenceStr;
	Language::LangFileSettings Language::currSettings;
	int Language::currOrdinalMode = 0;
	std::vector<Language::TranslationInfo> Language::availableLang;
	bool Language::msbtReady = false;

	Language::MsbtHandler::MessageData* Language::MsbtHandler::common = nullptr;
	Language::MsbtHandler::MessageData* Language::MsbtHandler::menu = nullptr;
	Language::MsbtHandler::MessageData* Language::MsbtHandler::race = nullptr;
	Language::MsbtHandler::PluginMessageDataList* Language::MsbtHandler::allList = nullptr;

	std::unordered_map<int, Language::MessageString*> Language::customText;
	BootSceneHandler::ProgressHandle Language::progressHandle;
	std::vector<std::string> Language::pendingLangfiles;
	
	int Language::currentTranslation = -1;
	char Language::currPostfix[3] = { 0 };
	bool Language::availableSZS[NONE] = { false };

    static const char* g_LangIDs[] =
    {
        "JAP",
        "ENG",
        "FRA",
        "DEU",
        "ITA",
        "SPA",
        "ENG",
        "KOR",
        "DUT",
        "POR",
        "RUS",
        "ENG"
    };

	void	Language::loadSettings() {
		File settFile("langSetting.bin", File::READ);
		if (settFile.IsOpen() && settFile.GetSize() == sizeof(LangFileSettings))
			settFile.Read(&currSettings, sizeof(LangFileSettings));
		currSettings.langID[3] = '\0';
		if (currSettings.magic != 0x474C4553) {
			currSettings.magic = 0x474C4553;
			strcpy(currSettings.langID, "");
		}
		if (currSettings.langID[0] == '\0') {
			strcpy(currSettings.langID, g_LangIDs[static_cast<u32>(System::GetSystemLanguage())]);
			settFile.Close();
			UpdateLangSettings();
		}
	}

	const char* g_szsFileNames[] = { "common-", "menu-", "race-", "thankyou-", "trophy-" };

	static u32 GetRegFromCode(const std::string& reg) {
		if (reg == "EUR")
			return GameRegion::EUROPE;
		else if (reg == "USA")
			return GameRegion::AMERICA;
		else if (reg == "JPN")
			return GameRegion::JAPAN;
		else if (reg == "KOR")
			return GameRegion::KOREA;
		else
			return 0;
	}

	void Language::PopulateLang() {
		std::string langRootDir("/CTGP-7/MyStuff/Languages");
		PluginHeader* header = reinterpret_cast<PluginHeader*>(0x07000000);
		LaunchSettings* launchSet = reinterpret_cast<LaunchSettings*>(&header->config[0]);

		int i = 0;
		int engI = -1;

		u32 sysReg = launchSet->region;
		if (System::GetSystemLanguage() == LanguageId::Japanese)
			sysReg = GameRegion::JAPAN;
		else if (System::GetSystemLanguage() == LanguageId::Korean)
			sysReg = GameRegion::KOREA;

		for (auto it = pendingLangfiles.begin(); it < pendingLangfiles.end(); it++, BootSceneHandler::Progress(progressHandle)) {
			std::string& f = *it;
			if (f.size() != 12 || f.substr(0, 5) != "Lang_" || f.substr(8, 4) != ".ini") { i++;  continue; }
			std::string fID = f.substr(5, 3);
			if (!Directory::IsExists(langRootDir + "/Lang_" + fID)) { i++;  continue; }
			TextFileParser parser;
			if (!parser.Parse(langRootDir + "/" + f)) { i++;  continue; }
			TranslationInfo currInfo;
			currInfo.ID = fID;
			currInfo.Name = parser.getEntry("name", 0);
			currInfo.EURPostfix = parser.getEntry("postfix", 0);
			currInfo.USAPostfix = parser.getEntry("postfix", 1);
			std::string ordinalStr = parser.getEntry("ordinalMode", 0);
			if (!ordinalStr.empty() && TextFileParser::IsNumerical(ordinalStr, false))
				currInfo.ordinalMode = std::stoi(ordinalStr);

			if (currInfo.Name.empty() || currInfo.EURPostfix.empty()) { i++;  continue; }
			if (currInfo.USAPostfix.empty()) currInfo.USAPostfix = currInfo.EURPostfix;
			int j = 0;
			while (true)
			{

				std::string wh = parser.getEntry("regWhiteList", j);
				if (wh.empty()) break;
				currInfo.regWhiteList.push_back(GetRegFromCode(wh));
				j++;
			}
			j= 0;
			while (true)
			{

				std::string wh = parser.getEntry("regBlackList", j);
				if (wh.empty()) break;
				currInfo.regBlackList.push_back(GetRegFromCode(wh));
				j++;
			}

			if (currInfo.IsAllowed(launchSet->region) && currInfo.IsAllowed(sysReg)) {
				availableLang.push_back(currInfo);
				if (fID == currSettings.langID) currentTranslation = i;
				if (fID == "ENG") engI = i;
				i++;
			}			
		}
		if (currentTranslation == -1) {
			if (engI == -1) panic("Failed to find translation file.");
			currentTranslation = engI;
			strcpy(currSettings.langID, "ENG");
			UpdateLangSettings();
		}
		strcpy(currPostfix, (MarioKartFramework::region == GameRegion::AMERICA) ? availableLang[currentTranslation].USAPostfix.substr(0,2).c_str() : availableLang[currentTranslation].EURPostfix.substr(0, 2).c_str());
		currOrdinalMode = availableLang[currentTranslation].ordinalMode;
		for (int i = 0; i < NONE; i++)
			availableSZS[i] = File::Exists(langRootDir + "/Lang_" + availableLang[currentTranslation].ID + "/" + g_szsFileNames[i] + currPostfix + ".szs");
		pendingLangfiles.clear();
	}

	void Language::RegisterProgress(void) {
		Directory langRootDir("/CTGP-7/MyStuff/Languages");
		if (!langRootDir.IsOpen()) return;
		langRootDir.ListFiles(pendingLangfiles, ".ini");
		progressHandle = BootSceneHandler::RegisterProgress(pendingLangfiles.size());
	}

	void	Language::Initialize(void) {
		loadSettings();
		PopulateLang();
	}

    void    Language::Import(void)
    {
		std::string langFile = std::string("/CTGP-7/MyStuff/Languages/Lang_") + currSettings.langID + "/Text.txt";
		std::string kartFile = std::string("/CTGP-7/MyStuff/Karts/Lang_") + currSettings.langID + ".txt";

        // If a language file exists, load it
        if (File::Exists(langFile))
        {
            currLanguage.Parse(langFile);
			if (!File::Exists(kartFile)) kartFile = std::string("/CTGP-7/MyStuff/Karts/Lang_ENG.txt");
			if (CharacterHandler::customKartsEnabled) currLanguage.Parse(kartFile);
		}
		else {
			strcpy(currSettings.langID, "ENG");
			UpdateLangSettings();
			langFile = std::string("/CTGP-7/MyStuff/Languages/Lang_") + currSettings.langID + "/Text.txt";
			kartFile = std::string("/CTGP-7/MyStuff/Karts/Lang_ENG.txt");

			if (File::Exists(langFile))
			{
				currLanguage.Parse(langFile);
			}
			else {
				panic("Current language files corrupted.");
			}
			if (CharacterHandler::customKartsEnabled) currLanguage.Parse(kartFile);
		}
		MsbtHandler::ApplyCustomText();
    }

    const std::string&  Language::GetName(const std::string &key)
    {
		return currLanguage.getEntry(key, 0);
    }

    const std::string&  Language::GetNote(const std::string &key)
    {
		return currLanguage.getEntry(key, 1);
    }

	const char* Language::GetCurrLangID()
	{
		return currSettings.langID;
	}

	std::string Language::GenerateOrdinal(u32 number) {
		if (number == 0)
			return std::to_string(number);
		switch (currOrdinalMode)
		{
		case 0: // English style
		{
			static const char* suffixes [] = {"th", "st", "nd", "rd"};
			auto ord = number % 100;
			if (ord / 10 == 1) { ord = 0; }
			ord = ord % 10;
			if (ord > 3) { ord = 0; }
			return std::to_string(number) + suffixes[ord];
		}
		case 1: // Spanish style
		{
			return std::to_string(number) + ".º";
		}
		case 2: // Catalan style
		{
			static const char* suffixes [] = {".è", ".r", ".n", ".r", ".t"};
			const char* chosen;
			if (number < 5) {
				chosen = suffixes[number];
			} else {
				chosen = suffixes[0];
			}
			return std::to_string(number) + chosen;
		}
		case 3: // German/Hungarian style
		{
			return std::to_string(number) + ".";
		}
		case 4: // Dutch style
		{
			return std::to_string(number) + "e";
		}
		case 5: // Italian/Portuguese style
		{
			return std::to_string(number) + "º";
		}
		case 6: // French style
		{
			return std::to_string(number) + ((number == 1) ? "er" : "e");
		}
		case 7: // Japanese style
		{
			return std::to_string(number) + "位";
		}
		case 8: // Korean style
		{
			return "제" + std::to_string(number);
		}
		default:
			return "";
		}
	}

	void Language::GetLanguagePostfix(SafeStringBase* mem)
	{
		new(mem) SafeStringBase(currPostfix);
	}

	const char* Language::GetLanguagePostfixChar() {
		return currPostfix;
	}

	void Language::UpdateLangSettings()
	{
		File settFile("langSetting.bin", File::RWC | File::TRUNCATE);
		settFile.Write(&currSettings, sizeof(LangFileSettings));
	}

	static u8* peaceIconData = nullptr;
	static u8* peaceDoveIconData = nullptr;
	static u32 peaceIconDataSize = 0;
	static u32 peaceDoveIconDataSize = 0;
	static u8 peaceFrame = 0;
	static void LanguageKeyboardCallback(Keyboard& k, KeyboardEvent& event) {
		auto transblendfunc = [](const Color& dst, const Color& src) -> Color {
			float prog;
			
			if (peaceFrame > 128 + 64) {
				prog = 1.f - (((float)peaceFrame - (128.f + 64.f)) / 64.f);
			} else if (peaceFrame > 128)
				prog = 1.f;
			else if (peaceFrame > 64) {
				prog = ((float)peaceFrame - 64.f) / 64.f;
			} else
				prog = 0.f;
			Color copy = dst;
			copy.a = copy.a * prog;
			return src.Blend(copy, Color::BlendMode::Alpha);
		};
		if (event.type == KeyboardEvent::EventType::FrameTop) {
			peaceFrame++;
			if (peaceIconData) BCLIM(peaceIconData, peaceIconDataSize).Render(IntRect(339, 189, 26, 26), BCLIM::RenderInterface(*event.renderInterface));
			if (peaceDoveIconData) BCLIM(peaceDoveIconData, peaceDoveIconDataSize).Render(IntRect(339, 189, 26, 26), BCLIM::RenderInterface(*event.renderInterface), Rect<int>(0, 0, INT32_MAX, INT32_MAX), Rect<int>(0, 0, 400, 240), std::make_pair(true, transblendfunc));
		}
	}
	
	void Language::ShowLangMenu(MenuEntry* entry)
	{
		if (!peaceIconData) {
			ExtraResource::SARC::FileInfo finfo;
			peaceIconData = ExtraResource::mainSarc->GetFile("Plugin/peace.bclim", &finfo);
			peaceIconDataSize = finfo.fileSize;
			peaceDoveIconData = ExtraResource::mainSarc->GetFile("Plugin/peace_dove.bclim", &finfo);
			peaceDoveIconDataSize = finfo.fileSize;
		}
		std::vector<std::string> langs;
		for (int i = 0; i < availableLang.size(); i++) {
			langs.push_back(availableLang[i].Name);
		}
		Keyboard kbd("Please select language.");
		peaceFrame = 0;
		kbd.OnKeyboardEvent(LanguageKeyboardCallback);
		kbd.Populate(langs);
		int opt = kbd.Open();
		if (opt >= 0) {
			bool changed = strcmp(currSettings.langID, availableLang[opt].ID.c_str()) != 0;
			if (changed) {
				strcpy(currSettings.langID, availableLang[opt].ID.c_str());
				entry->Name() = "Language (" + availableLang[opt].Name + ")";
				UpdateLangSettings();
				MessageBox("Reboot required to apply changes.")();
			}
		}
	}

	void Language::GetLangSpecificFile(u16* filename, SZSID id, bool isPatch)
	{
		if (isPatch) {
			if (MarioKartFramework::region == JAPAN && MarioKartFramework::revision == REV2) {
				if (strfind16(filename, (u16*)u"dash")) {
					strcpy16(filename, (u16*)u"pat1:/Patch/UI/common-jp/dash_ntlg16b.bcfnt");
				}
			}
			return;
		}
		if (!availableSZS[id]) {return;}
		std::string file("ram:/CTGP-7/MyStuff/Languages/Lang_" + availableLang[currentTranslation].ID + "/" + g_szsFileNames[id] + currPostfix + ".szs");
		memset(filename, 0, 0x200);
		utf8_to_utf16(filename, (u8*)file.c_str(), 256);
	}

	void Language::FixRegionSpecificFile(u16* filename) {
		int filepos;
		strfind16(filename, (u16*)u"ve-", &filepos);
		filepos += 5;
		int extensionpos;
		strfind16(filename, (u16*)u".", &extensionpos);
		int fileEndPos = extensionpos - 3;
		// Assuming the new name is always shorter
		strcpy16(filename + filepos - 3, filename + filepos); // Remove the extension from the folder.
		strcpy16(filename + fileEndPos - 3, filename + extensionpos - 3);
	}

	void Language::doTextPatchesAfterMsbtReady() {
		msbtReady = true;
		CourseManager::fixTranslatedMsbtEntries();
		CourseManager::sortTracksAlphabetically();
		CharacterHandler::UpdateMsbtPatches();
		VersusHandler::InitializeText();
		MissionHandler::InitializeText();
		BlueCoinChallenge::InitializeLanguage();
		CCSettings* settings = static_cast<CCSettings*>(ccselectorentry->GetArg());
		if (settings->enabled) {
			std::string cc_name = std::to_string((int)settings->value) + "cc";
			for (int i = 1812; i >= 1810; i--) MsbtHandler::SetString(i, cc_name);
		}
		MsbtHandler::SetString(6001, NAME("server_ctgp7_conn"));
		MsbtHandler::SetString(6002, NAME("server_ctgp7_dconn"));
		MsbtHandler::SetString(6334, NAME("server_ctgp7_nocomm"));
	}

    static void ReplaceStringInPlace(std::string& subject, const std::string search, const std::string replace) {
        size_t pos = 0;

        while ((pos = subject.find(search, pos)) != std::string::npos) {
            subject.replace(pos, search.length(), replace);
            pos += replace.length();
        }
    }

    int utf8len(const char* str)
    {
        u32 len = 0;
        u32 tmp = 0;
        u16 tmp2[2];
        const char* s = str;
        while (*s) {
            len++;
            int codelen = decode_utf8(&tmp, (u8*)s);
            if (codelen == -1) break;
            if (encode_utf16(tmp2, tmp) > 1) return -len;
            s += codelen;
        }
        return len;
    }
	
	bool Language::MsbtHandler::ControlString::GetChoice(u16* out, int choice) const {
		if (header.group == Group::STRFORMAT && header.type == 6) {
			const u8* c = args;
			int counter = 0;
			u16 currSize;
			while (true) {
				currSize = *(u16*)c;
				c += 2;
				if (currSize == 0)
					return false;
				if (choice == counter)
					break;
				c += currSize;
				counter++; 
			}
			memcpy(out, c, currSize);
			out[currSize / 2] = '\0';
			return true;
		}
		return false;
	}

	string16 Language::MsbtHandler::ControlString::GenSizeControlString(s16 percentage) {
		string16 ret((sizeof(ControlString) / 2) + 1, '\0');
		ControlString* control = (ControlString*)ret.data();
		control->header.magic = 0xE;
		control->header.group = Group::RENDERING;
		control->header.type = (u16)RenderingType::FONT_SIZE;
		control->header.argAmount = 2;
		((s16*)control->args)[0] = (s16)percentage;
		return ret;
	}

	string16 Language::MsbtHandler::ControlString::GenColorControlString(DashColor color, const Color& customColor) {
		s16 realColor;
		if (color == DashColor::CUSTOM) {
			// NOTE: Pure white is not possible as it generates DashColor::RESET
			u8 r = ((u32)customColor.r) * 0x1F / 255;
			u8 g = ((u32)customColor.g) * 0x1F / 255;
			u8 b = ((u32)customColor.b) * 0x1F / 255;
			realColor = (0x8000 | r | (s16)g << 5 | (s16)b << 10);
		} else {
			realColor = (s16)color;
		}
		string16 ret((sizeof(ControlString) / 2) + 1, '\0');
		ControlString* control = (ControlString*)ret.data();
		control->header.magic = 0xE;
		control->header.group = Group::RENDERING;
		control->header.type = (u16)RenderingType::FONT_COLOR;
		control->header.argAmount = 2;
		((s16*)control->args)[0] = (s16)realColor;
		return ret;
	}

	void Language::MsbtHandler::ControlString::GetColor(u32* abgr, u32 messageWritter, u16 color) {
		switch (color)
		{
		case 0: // Red
			*abgr = 0xFF0F0FEB;
			break;
		case 1: // Blue
			*abgr = 0xFFE17D0F;
			break;
		case 2: // Green
			*abgr = 0xFF19A514;
			break;
		default:
			if (color & 0x8000) {
				u8 r = (u8)(((u32)((color) & 0x1F)) * 255 / 0x1F);
				u8 g = (u8)(((u32)((color >> 5) & 0x1F)) * 255 / 0x1F);
				u8 b = (u8)(((u32)((color >> 10) & 0x1F)) * 255 / 0x1F);
				*abgr = ((u32)r | (u32)g << 8 | (u32)b << 16 | 0xFF << 24);
			} else {
				*abgr = 0xFFFFFFFF;
			}
			break;
		}
	}

	static VisualControl::Message defaultMsg((u16*)u"");
	void Language::MsbtHandler::GetMessage(VisualControl::Message& out, MessageData* data, u32 id) {
		if (g_isFoolActive && id != CustomTextEntries::dialog) {out = VisualControl::Message(getFoolsText()); return;}
		auto it = customText.find(id);
		const VisualControl::Message* customMsg;
		if (it != customText.end() && (customMsg = it->second->GetMessage()))
		{
			out = *customMsg;
			return;
		}
		if (!data || !data->tag)
		{
			out = defaultMsg;
			return;
		}
		u32 entry;
		if ((entry = data->tag->GetNLI1()->GetTxt2Entry(id)) != 0xFFFFFFFF) {
			u16* ptr = data->tag->GetTXT2()->GetText(entry);
			out = (ptr) ? VisualControl::Message(ptr) : defaultMsg;
		} else out = defaultMsg;
	}

	void Language::MsbtHandler::GetMessageFromList(VisualControl::Message& out, MessageDataList* list, u32 id) {
		if (!list->size) out = defaultMsg;
		else GetMessage(out, list->GetDataForID(id), id);
	}

	static bool g_firstAllLoad = false;

	void Language::MsbtHandler::RecalculateCrossMsbtPtr(u32 msbtIndex, void* newStart) {
		for (auto it = customText.cbegin(); it != customText.cend(); it++) {
			if (it->second->didCopy) continue;
			const u16* text = it->second->message.data;
			if (text >= allList->infos[msbtIndex].txt2StartPtr && text <= allList->infos[msbtIndex].txt2EndPtr)
			{
				text = (u16*)((u32)text + ((u32)newStart - (u32)allList->infos[msbtIndex].txt2StartPtr));
				it->second->message.data = text;
			}
		}
	}

	void Language::MsbtHandler::OnMessageDataConstruct(MessageData* data) {
		
		if (!allList) allList = new PluginMessageDataList(3);

		if (strcmp(data->name, "Common") == 0) {
			if(common && allList->infos[0].txt2StartPtr != data->tag->GetTXT2())
				RecalculateCrossMsbtPtr(0, data->tag->GetTXT2());
			common = data;
			allList->msg.list[0] = data;
			allList->infos[0].txt2StartPtr = data->tag->GetTXT2();
			allList->infos[0].txt2EndPtr = (u8*)allList->infos[0].txt2StartPtr + data->tag->GetTXT2Size();
		} else if (strcmp(data->name, "Menu") == 0) {
			if(menu && allList->infos[1].txt2StartPtr != data->tag->GetTXT2())
				RecalculateCrossMsbtPtr(1, data->tag->GetTXT2());
			menu = data;
			allList->msg.list[1] = data;
			allList->infos[1].txt2StartPtr = data->tag->GetTXT2();
			allList->infos[1].txt2EndPtr = (u8*)allList->infos[1].txt2StartPtr + data->tag->GetTXT2Size();
			#if CITRA_MODE == 1
			MsbtHandler::SetString(2092, NOTE("server_name"));
			#endif
		} else if (strcmp(data->name, "Race") == 0) {
			if(race && allList->infos[2].txt2StartPtr != data->tag->GetTXT2())
				RecalculateCrossMsbtPtr(2, data->tag->GetTXT2());
			race = data;
			allList->msg.list[2] = data;
			allList->infos[2].txt2StartPtr = data->tag->GetTXT2();
			allList->infos[2].txt2EndPtr = (u8*)allList->infos[2].txt2StartPtr + data->tag->GetTXT2Size();
		}

		if (!g_firstAllLoad && common && menu && race) {
			g_firstAllLoad = true;
			ApplyQueuedReferenceStr();
			doTextPatchesAfterMsbtReady();
		}
	}

	void Language::MsbtHandler::ApplyCustomText() {
		for (auto it = currLanguage.cbegin(); it != currLanguage.cend();)
        {
            if ((*it).first[0] == '$')
            {
                std::string&& textid = (*it).first.substr(1);
                if (!TextFileParser::IsNumerical(textid, false)) {it++; continue;}
				if ((*it).second.size() != 1) {it++; continue;}
				std::string name = (*it).second[0];
				if (name.length() > 2 && name[0] == '\\' && name[1] == 'r') { //Reference string
					std::string other = name.substr(2);
					if (!TextFileParser::IsNumerical(other, false)) {it++; continue;}
					queuedReferenceStr.push_back(std::make_pair(std::stoi(textid),std::stoi(other)));
				}
				else {
					SetString(std::stoi(textid), name);
				}
				it = currLanguage.erase(it);
            }
            else
            {
                ++it;
            }
        }
	}

	void Language::MsbtHandler::ApplyQueuedReferenceStr() {
		for (auto it = queuedReferenceStr.begin(); it != queuedReferenceStr.end(); it++)
			SetText(it->first, GetText(it->second));
		queuedReferenceStr.clear();
	}

	u32 Language::MsbtHandler::GetTextLenNoFormatting(const u16* text) {
		const u16* p = text;
		u32 count = 0;
		while (*p) {
			if (*p == 0xE) { // Control String
				u32 csSize = ((ControlString*)p)->GetSizeBytes();
				p += csSize / 2;
			} else {
				count++;
			}
		}
		return count;
	}

	void Language::MsbtHandler::SkipTextControlString(u16* dst, const u16* src) {
		while (*src)
		{
			if (*src == 0xE) { // Control String
				u32 csSize = ((ControlString*)src)->GetSizeBytes();
				src += csSize / sizeof(u16);
			} else {
				*dst++ = *src++;
			}
		}
		*dst = '\0';
	}

	string16 Language::MsbtHandler::DashTextWithTagsToString(const u16* src) {
		string16 ret;
		while (*src)
		{
			if (*src == 0xE) { // Control String
				u32 csSize = ((const ControlString*)src)->GetSizeBytes() / sizeof(u16);
				while (csSize--) {
					ret.push_back(*src++);
				}
			} else {
				ret.push_back(*src++);
			}
		}
		return ret;
	}

	const Language::MsbtHandler::ControlString* Language::MsbtHandler::FindControlString(const u16* text, u16 group, u16 type, const u16** nextChar) {
		while (*text)
		{
			if (*text == 0xE) { // Control String
				ControlString* cstr = (ControlString*)text;
				if ((u16)cstr->header.group == group && cstr->header.type == type) {
					if (nextChar) *nextChar = text + cstr->GetSizeBytes() / sizeof(u16);
					return cstr;
				}
			}
			text++;
		}
		if (nextChar) *nextChar = text;
		return nullptr;
	}

	const u16* Language::MsbtHandler::GetText(u32 id) {
		VisualControl::Message msg;
		GetMessageFromList(msg, &allList->msg, id);
		return msg.data;
	}

	static u16* g_convBuff1 = nullptr;
	std::string Language::MsbtHandler::GetString(const u16* text) {
		if (!g_convBuff1)
			g_convBuff1 = (u16*)operator new(0x200);

		SkipTextControlString(g_convBuff1, text);
		
		std::string ret;
		Utils::ConvertUTF16ToUTF8(ret, g_convBuff1);

		return ret;
	}

	std::string Language::MsbtHandler::GetString(u32 id) {
		return GetString(GetText(id));
	}
	
	void Language::MsbtHandler::SetText(u32 id, const u16* text, bool insertFront, u32 textSize) {
		auto it = customText.find(id);
		MessageString* m =  new MessageString((u16*)text, false, textSize);
		if (it != customText.end()) {
			if (insertFront) {
				MessageString* old = it->second;
				customText[id] = m;
				m->next = old;
			} else {
				MessageString* oldNext = it->second->next;
				delete it->second;
				customText[id] = m;
				if (oldNext) m->next = oldNext;
			}
		} else customText[id] = m;
	}

	void Language::MsbtHandler::SetString(u32 id, const char* text, bool inserFront) {
		auto it = customText.find(id);
		MessageString* m =  new MessageString(text);
		if (it != customText.end()) {
			if (inserFront) {
				MessageString* old = it->second;
				customText[id] = m;
				m->next = old;
			} else {
				MessageString* oldNext = it->second->next;
				delete it->second;
				customText[id] = m;
				if (oldNext) m->next = oldNext;
			}
		} else customText[id] = m;
	}

	void Language::MsbtHandler::RemoveAllString(u32 id) {
		auto it = customText.find(id);
		if (it != customText.end()) {
			MessageString* str = it->second;
			while (str) {
				MessageString* next = str->next;
				delete str;
				str = next;
			}
			customText.erase(id);
		}
	}
	
	void Language::MsbtHandler::SetTextEnabled(u32 id, bool enabled) {
		auto it = customText.find(id);
		if (it != customText.end()) {
			it->second->isEnabled = enabled;
		}
	}

	Language::MessageString::MessageString(u16* src, bool makeCopy, u32 size) {
		if (size != 0xFFFFFFFF) makeCopy = true;
		didCopy = makeCopy;
		isEnabled = true;
		next = nullptr;
        if (makeCopy)
        {
            u32 sizeBytes = (size == 0xFFFFFFF) ? ((strsize16(src) + 1) * 2) : ((size + 1) * 2);
            message.data = (u16*)::operator new(sizeBytes);
			memcpy((u16*)message.data, src, sizeBytes - 2);
			((u16*)message.data)[sizeBytes / 2 - 1] = 0;
        }
        else message.data = src;
    }

	static ssize_t utf8_to_utf16_special_null(uint16_t *out, const uint8_t *in, size_t len) {
		ssize_t  rc = 0;
		ssize_t  units;
		uint32_t code;
		uint16_t encoded[2];

		do
		{
			units = decode_utf8(&code, in);
			if(units == -1)
			return -1;

			if(code > 0)
			{
			in += units;

			units = encode_utf16(encoded, code);
			if(units == -1)
				return -1;

			if(out != NULL)
			{
				if(rc + units <= len)
				{
					if (code == 0xF) // Special case
						*out++ = 0;
					else
					{
						*out++ = encoded[0];
						if(units > 1)
							*out++ = encoded[1];
					}
				}
			}

			if(SSIZE_MAX - units >= rc)
				rc += units;
			else
				return -1;
			}
		} while(code > 0);

		return rc;
	}

    Language::MessageString::MessageString(const char* src) {
        int size = utf8len(src);
		isEnabled = true;
		didCopy = true;
		next = nullptr;
		if (size < 0)
			panic("Invalid character while decoding utf16");
        u32 sizeBytes = (size + 1) * 2;

        message.data = (u16*)::operator new(sizeBytes);

        int units = utf8_to_utf16_special_null((u16*)message.data, (u8*)src, size);
        if (units >= 0)
            ((u16*)message.data)[units] = 0;
    }
}
