/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MusicSlotMngr.cpp
Open source lines: 110/110 (100.00%)
*****************************************************/

#include "MusicSlotMngr.hpp"
#include "CourseManager.hpp"
#include "TextFileParser.hpp"

namespace CTRPluginFramework {
	std::map<u32, MusicSlotMngr::MusicSlotEntry> MusicSlotMngr::entryMap;
    std::vector<std::pair<bool,std::string>> MusicSlotMngr::authorNames;

	void MusicSlotMngr::initMusicSlotData(const std::string& filename, bool isUserData, std::map<u32, MusicSlotMngr::MusicSlotEntry>& musicMap) {

		TextFileParser parser;
		if (!parser.Parse(filename)) return;

		for (auto it = parser.cbegin(); it != parser.cend(); it++) {
			if (it->second.size() < 6 || it->second[0].size() > 49) continue;
			bool allNumerical = true;
			for (int i = 2; i < 6; i++) {
				allNumerical = allNumerical && TextFileParser::IsNumerical(it->second[i], false);
			}
			if (!allNumerical) continue;

			int courseID = CourseManager::getCourseIDFromName(it->first);
			if (courseID == -1) continue;

			BgmMode mode;
			if (it->second[1].compare("SINGLE") == 0) mode = BgmMode::SINGLE;
			else if (it->second[1].compare("MULTI_WATER") == 0) mode = BgmMode::MULTI_WATER;
			else if (it->second[1].compare("MULTI_AREA") == 0) mode = BgmMode::MULTI_AREA;
			else continue;

			MusicSlotEntry newEntry(it->second[0], mode, std::stoi(it->second[2]), std::stoi(it->second[3]), std::stoi(it->second[4]), std::stoi(it->second[5]), isUserData);

			if (it->second.size() > 6) {
				newEntry.musicName = it->second[6];
				for (int i = 7; i < it->second.size(); i++) {
					newEntry.authors.push_back(InsertAuthor(it->second[i], isUserData));
				}
			}

			auto it2 = musicMap.find(courseID);
			if (it2 != musicMap.end()) it2->second = std::move(newEntry);
			else musicMap[courseID] = std::move(newEntry);
		}
	}
	void MusicSlotMngr::Initialize()
	{
		initMusicSlotData("/CTGP-7/resources/musicSlots.ini", false, entryMap);
		initMusicSlotData("/CTGP-7/MyStuff/musicSlotsUser.ini", true, entryMap);
	}

	std::vector<std::string> MusicSlotMngr::GetAllAuthors() {
		std::vector<int> freq(authorNames.size(), 0);
        for (auto it = entryMap.cbegin(); it != entryMap.cend(); it++) {
            for (auto it2 = it->second.authors.cbegin(); it2 != it->second.authors.cend(); it2++) {
                freq[*it2]++;
            }
        }

        std::vector<std::pair<std::string, int>> pairs;
        for (int i = 0; i < authorNames.size(); i++) {
			if (authorNames[i].second.empty())
                continue;
            pairs.push_back(std::make_pair(authorNames[i].second, freq[i]));
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

	std::string MusicSlotMngr::MusicSlotEntry::GetAuthorString() {
		std::string ret = "";
		if (authors.empty())
			return ret;
		ret = GetAuthor(authors[0]);
		for (int i = 1; i < authors.size(); i++) {
			ret += ", " + GetAuthor(authors[i]);
		}
		return ret;
	}

	u16 MusicSlotMngr::InsertAuthor(const std::string& author, bool isUserData) {
        auto it = std::find(authorNames.begin(), authorNames.end(), std::make_pair(isUserData, author));
        if (it == authorNames.end())
            it = authorNames.insert(it, std::make_pair(isUserData, author));
        return it - authorNames.begin();
    }

	static const std::string g_emptyString = "";
    const std::string& MusicSlotMngr::GetAuthor(u16 index) {
        if (index >= authorNames.size())
            return g_emptyString;
        return std::get<1>(authorNames[index]);
    }
}
