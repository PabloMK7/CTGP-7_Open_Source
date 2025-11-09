/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CourseManager.hpp
Open source lines: 129/133 (96.99%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include <bitset>

#define MAXCUPS 32
#define MAXTRACKS 138

// No need to change anything below here
#define INVALIDTRACK 0x29
#define ORIGINALTRACKLOWER 0x0
#define ORIGINALTRACKUPPER (ORIGINALTRACKLOWER + 0x1F)
#define BATTLETRACKLOWER (ORIGINALTRACKUPPER + 1)
#define BATTLETRACKUPPER (BATTLETRACKLOWER + 5)
#define CUSTOMTRACKLOWER 0x2A
#define CUSTOMTRACKUPPER (MAXTRACKS - 1)
#define FROMCUPTOBUTTON(a) ((a > 7) ? (a + 2) : a)
#define FROMBUTTONTOCUP(a) ((a > 7) ? (a - 2) : a)
#define MAXCUPSBUTTON (FROMCUPTOBUTTON(MAXCUPS - 1) + 1)
#define CUSTOMCUPLOWER 0xA
#define CUSTOMCUPUPPER (MAXCUPS + 1) // To compensate FROMCUPTOBUTTON
#define TOTALCUSTOMCUPS (CUSTOMCUPUPPER - CUSTOMCUPLOWER + 1)
#define TOTALALLCUPS (TOTALCUSTOMCUPS + 8)

namespace CTRPluginFramework
{
	struct CTNameFunc {
		u32 unused1;
		u32 unused2;
		u32 nullsub;
		static void nullsub_func() { return; }
	};

	struct CTNameData {
		struct CTNameEntry {
			const CTNameFunc* f;
			const char* name;
			u32 originalSlot;
			u32 lapAmount;
			u32 countdownTimerValue;
			u32 pointsModeClearScore;
		};
		CTNameFunc f;
		CTNameEntry entries[MAXTRACKS];
	};

	struct TrophyNameData {
		struct TrophyNameEntry {
			CTNameFunc* f;
			const char* name;
		};
		CTNameFunc f;
		TrophyNameEntry entries[MAXCUPS];
	};

	extern const u32 globalCustomTracksWithReplay[9];
	extern u32 alphabeticalGlobalCupData[MAXCUPS][4];
    extern const u32 globalCupData[MAXCUPS][4];
	extern const CTNameData globalNameData;
	extern const u32 globalCupTranslateTable[MAXCUPS];
	extern const u32 ctwwCupTranslateTable[MAXCUPS - 8];
	extern TrophyNameData globalTrophyData;
	extern TrophyNameData::TrophyNameEntry generalThrophy;

    class CourseManager {
        private:
        public:
			static u32 lastLoadedCourseID;
			static bool hadToDisableSpeed;
			static u8 customTracksAllowedFlag;
			static u8 originalTracksAllowedFlag;
			static bool getBGMFromNormalLap;
			static int multiCupButtonsIDs[8];
			static u32* finalGlobalCupTranslateTable;
			static u32 finalGlobalCupTranslateTableSize;
			static u32* pointsModeCupTranslateTable;
			static u32 pointsModeCupTranslateTableSize;
			static void resetGhost(MenuEntry* entry);
			static void resetCourseGhost(u32 courseID);
			static void setCustomTracksAllowed(bool mode);
			static bool getCustomTracksAllowed();
			static void setOriginalTracksAllowed(bool mode);
			static bool getOriginalTracksAllowed();
			static u32 getCupGlobalIDName(u32 cupID);
			static u32 getSingleCourseGlobalIDName(u32 courseID);
			static u32 getCourseGlobalIDName(u32 cup, int track, bool forceOrig = false);
			static u32* getGPCourseID(u32* ptr, u32 cup, int track, bool forceOriginal = false);
			static u32	getCourseName(u32* trackID);
			static u32  getCountdownCourseTime(u32 courseID);
			static int  getCourseIDFromName(const std::string& str);
			static const CTNameData::CTNameEntry* getCourseData(const std::string& str);
			static const CTNameData::CTNameEntry* getCourseData(u32 slotID);
			static u32	getOriginalCourseID(u32 courseID);
			static u32	getCourseLapAmount(u32 courseID);
			static u32 getLatestLoadedCourseID();
			static void findCupCourseID(u32 found[2], u32 courseID);
			static void replaceConsoleText(std::string& subject, const std::string search, const std::string replace, bool appendEnd);
			static u32	getCupFromCourseID(u32 courseID);
			static u32	getTrackFromCourseID(u32 courseID);
			static u32 getTrophyName(u32 cupID);
			static bool isRainbowTrack(u32 trackID);
			static u32 getTrackChannelMode();
			static u8 getCustomBeatBPM(u8 orig);
			static u32 getCustomBeatOffset(u32 orig);
			static void (*BaseMenuButtonControl_setTex)(u32 buttonObject, u32 texPtrn, u32 texID);
			static void	(*MenuSingle_CupBase_buttonHandler_OK)(u32 menuObject, u32 cupID);
			static u32 (*Sequence_GetGPCourseNameID)(u32 cup, u32 track);
			static void fixTranslatedMsbtEntries();
			static void sortTracksAlphabetically();
			static void multiCourseIDToCupTrack(u8* data);
			static void multiCupTrackToCourseID(u8* data);
			static void changeMoflex(u32 cupBaseMenu, int cupID);
			static bool handleMenuCallback(const Screen& screen);
			static void getRandomCourse(u32* result, bool isRace);
			static u32  getRandomCourseDemo(bool isRace);
			static const u32* getCupTranslatetable(u32* size, bool forceGetAll = false);
			static void initGlobalCupTranslateTable();
			static void getCourseText(std::string& out, u32 courseID, bool replaceSymbols);
			static void getCupText(std::string& out, u32 cupID);
    };
}
