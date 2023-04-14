/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CourseCredits.hpp
Open source lines: 29/29 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "map"

namespace CTRPluginFramework {
    class CourseCredits {
    public:
        static void Initialize();
        static int OnRaceDemoTextReset(bool isFirst, int original);

        static std::vector<std::string> GetAllAuthors();
    private:
        static u16 InsertAuthor(const std::string& author);
        static const std::string& GetAuthor(u16 index);
        static std::string BuildString(u32 courseID);

        static std::vector<std::string> authorNames;
        static std::map<u32, std::vector<u16>> trackAuthors;
    };
}
