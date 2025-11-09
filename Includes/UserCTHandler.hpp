/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: UserCTHandler.hpp
Open source lines: 109/109 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "ExtraResource.hpp"
#include "BootSceneHandler.hpp"

#define USERCUPID 65
extern u32 USERTRACKID;

namespace CTRPluginFramework {
    class UserCTHandler
    {
    private:
        static BootSceneHandler::ProgressHandle progressHandle;
        static std::vector<std::string> pendingDirs;
    public:
        class CustomCourse {
        public:
            std::string internalName;
            std::string courseName;
            std::string fileName;
            u32 originalSlot;
            u32 lapAmount;

            CustomCourse();
            CustomCourse(const std::string& iName, const std::string& cName, const std::string& fileName, u32 origSlot, u32 lapAmount);
        };
        class CustomCup {
        public:
            std::string cupName;

            CustomCup();
            CustomCup(const std::string& cName);

            inline CustomCourse& GetCourse(u32 trackID) {return courses[trackID];};
        private:
            CustomCourse courses[4];
        };

        static void RegisterProgress();
        static void Initialize();
        static void FixNames();
        static u32 GetCustomCupAmount();
        static u32 GetSelectedCustomCup(u32 startingButtonID, u32 selectedCupIcon);
        static void UpdateCurrentCustomCup(u32 cupID, s32 startingButtonID, s32 selectedCupIcon);
        static void UpdateCurrentCourseData();
        static u32 GetCupTextID();
        static u32 GetCourseTextID();
        static u32 GetCourseInternalName();
        static u32 GetCurrentCourseOrigSlot();
        static u32 GetCurrentCourseLapAmount();
        static u32 GetCurrentTrackInCup();
        static u32 GetCurrentCupText(u32 track);
        static bool IsUsingCustomCup();
        static bool IsLastRace(u32 track);
        static void OnNextTrackLoad();
        static void CleanTextureSarc();
        static void TimeTrialsSetTrack(u32 track);
        static void GetCouseSZSPath(char16_t* dst, bool withLang);

        static u8* LoadTextureFile(u32* archive, SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo);

        class SkipToCourseConfig {
        public:
            bool enabled;
            bool useLeftToFinish;
            bool skipCoursePreview;

            int cpuAmount;
            int courseID;

            s32 driverID;
            s32 bodyID;
            s32 tireID;
            s32 wingID;
            
            int itemID;

            EEngineLevel engineLevel;
            bool mirror;
        };

        static SkipToCourseConfig skipConfig;
        static void ApplySkipToCourseConfig();
        static u32 BaseMenuPage_applySetting_TitleDemo_Race;
        static u32 BaseMenuPage_applySetting_GP;
    private:

        static void populateCups();
        static void initSkipConfig();
        static CustomCup populateCourses(const std::vector<std::string>& szsFileNames, const std::string& cupName);

        static std::vector<CustomCup> customCups;
        static u32 selectedCustomCup;
        static u32 currentTrack;
        static bool usingCustomCup;
        static ExtraResource::StreamedSarc* textureSarc;
        static u32 lastLastLoadedTrack;
        static std::vector<u32> replacedSarcFiles;
    };
}
