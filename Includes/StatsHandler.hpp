/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StatsHandler.hpp
Open source lines: 96/96 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Minibson.hpp"
#include "NetHandler.hpp"
#include "MissionHandler.hpp"

namespace CTRPluginFramework {
	class StatsHandler
	{
	public:
		enum class Stat
		{
			LAUNCHES = 0,

			R_START,
			RACES,
			TT,
			COIN_BATTLES,
			BALLOON_BATTLES,
			ONLINE_RACES,
			COMMUNITY_RACES,
			CTWW_RACES,
			CD_RACES,
			ONLINE_COIN_BATTLES,
			ONLINE_BALLOON_BATTLES,
			R_END,

			M_START,
			FAILED_MISSIONS,
			COMPLETED_MISSIONS,
			PERFECT_MISSIONS,
			CUSTOM_MISSIONS,
			GRADEMEAN_MISSIONS,
			GRADECOUNT_MISSIONS,
			M_END,
			
			RACE_POINTS,
			
			TRACK_FREQ
		};

		static void Initialize();
		static void CommitToFile();
		static void UploadStats();

		static minibson::document FetchSendStatus();

		static int GetStat(Stat stat, int courseID = -1);
		static void IncreaseStat(Stat stat, int courseID = -1, int amount = 1);
		static double GetMissionMean();
		static void UpdateMissionMean(double newValue, bool increaseCount = true);

		static void OnCourseFinish();
		static void OnMissionFinish(int missionGrade, bool checksumValid, int world);
		static void StatsMenu(MenuEntry* entry);
	private:
		static const char* statStr[];

		struct StatsSaveLegacy
		{
			u32 magic;
			u8 bsondata[];
		};
		
		static minibson::encdocument statsDoc;
		static minibson::document* uploadDoc;
		static Mutex statsDocMutex;
		static Task uploadStatsTask;
		static bool firstReport;

		static s32 racePointsPos;

		static const minibson::document& GetUploadedStats();
		static const minibson::document& GetPendingStats();
		static int GetSequenceID();
		static void SetSequenceID(int seqID);
		static void RemovePendingUploads();
		static s32 UploadStatsFunc(void* args);
		static int GetDocStat(const minibson::document& doc, Stat stat, int courseID = -1);
		static void IncreaseDocStat(minibson::document &doc, Stat stat, int courseID = -1, int amount = 1);
		static void ForceDocStat(minibson::document &doc, Stat stat, int courseID, int amount);
		static std::pair<double, int> GetDocMissionMean(const minibson::document& doc);
		static void UpdateDocMissionMean(minibson::document &doc, double newMean, bool increaseCount = true);
		static std::vector<std::pair<int, int>> GetMostPlayedCourses();
		static std::vector<std::pair<int, int>> GetMostPlayedArenas();
	};
}