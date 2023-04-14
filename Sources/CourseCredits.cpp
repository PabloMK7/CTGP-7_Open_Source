/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CourseCredits.cpp
Open source lines: 108/108 (100.00%)
*****************************************************/

#include "CourseCredits.hpp"
#include "CustomTextEntries.hpp"
#include "CourseManager.hpp"
#include "UserCTHandler.hpp"
#include "TextFileParser.hpp"
#include "Lang.hpp"

namespace CTRPluginFramework {

    std::vector<std::string> CourseCredits::authorNames;
    std::map<u32, std::vector<u16>> CourseCredits::trackAuthors;

    u16 CourseCredits::InsertAuthor(const std::string& author) {
        auto it = std::find(authorNames.begin(), authorNames.end(), author);
        if (it == authorNames.end())
            it = authorNames.insert(it, author);
        return it - authorNames.begin();
    }

    static const std::string g_emptyString = "";
    const std::string& CourseCredits::GetAuthor(u16 index) {
        if (index >= authorNames.size())
            return g_emptyString;
        return authorNames[index];
    }

    std::string CourseCredits::BuildString(u32 courseID) {
        auto it = trackAuthors.find(courseID);
        if (it == trackAuthors.end() || it->second.size() == 0)
            return "";
        std::string ret = "By ";
        u32 authorCount = std::min(3, (int)it->second.size());
        for (int i = 0; i < authorCount - 1; i++) {
            ret.append(GetAuthor(it->second[i]) + ", ");
        }
        ret.append(GetAuthor(it->second[authorCount - 1]));
        if (it->second.size() > 3)
            ret.append(", ...");
        return ret;
    }

    void CourseCredits::Initialize() {
        TextFileParser parser;
        if (!parser.Parse("/CTGP-7/resources/courseCredits.ini"))
            return;
        for (auto it = parser.begin(); it != parser.end(); it++) {
            const std::string& courseIDName = it->first;
            u32 courseID = CourseManager::getCourseIDFromName(courseIDName);
            if (courseID == -1)
                continue;
            std::vector<u16> authors;
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                authors.push_back(InsertAuthor(*it2));
            }
            trackAuthors[courseID] = authors;
        }
    }

    int CourseCredits::OnRaceDemoTextReset(bool isFirst, int original) {
        if (UserCTHandler::IsUsingCustomCup()) {
            return isFirst ? UserCTHandler::GetCourseTextID() : UserCTHandler::GetCupTextID();
        }
        if (isFirst) {
            u32 courseID = CourseManager::lastLoadedCourseID;
            u32 cup = CourseManager::getCupFromCourseID(courseID);
            std::string topText;
            CourseManager::getCourseText(topText, courseID, false);
            Language::MsbtHandler::SetString(CustomTextEntries::demoRaceTextTop, topText);
            Language::MsbtHandler::SetString(CustomTextEntries::demoRaceTextBot, BuildString(courseID));
        }
        return isFirst ? CustomTextEntries::demoRaceTextTop : CustomTextEntries::demoRaceTextBot;
    }

    std::vector<std::string> CourseCredits::GetAllAuthors() {
        std::vector<int> freq(authorNames.size(), 0);
        for (auto it = trackAuthors.cbegin(); it != trackAuthors.cend(); it++) {
            for (auto it2 = it->second.cbegin(); it2 != it->second.cend(); it2++) {
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
}
