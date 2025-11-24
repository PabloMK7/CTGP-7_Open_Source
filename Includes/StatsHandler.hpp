/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StatsHandler.hpp
Open source lines: 101/101 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Minibson.hpp"
#include "NetHandler.hpp"
#include "MissionHandler.hpp"
#include "BCLIM.hpp"
#include <map>
#include <vector>

namespace CTRPluginFramework {
	class StatsHandler
	{
	public:
		static constexpr u32 achievementReportVersion = 0;

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
			SCORE_ATTACK_RACES,
			WEEKLY_CHALLENGE_ATTEMPTS,
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
			
			TRACK_FREQ,
			SCORE_ATTACK_MEAN,
		};

		static void Initialize();
		static void CommitToFile();
		static minibson::document CreateBackup();
		static bool RestoreBackup(const minibson::document& doc);
		static void UploadStats(bool async = true, bool forceUpload = false);

		static minibson::document FetchSendStatus();

		static int GetStat(Stat stat, int courseID = -1);
		static std::pair<s64, s64> GetStatPair(Stat stat, int courseID = -1);
		static void IncreaseStat(Stat stat, int courseID = -1, int amount = 1);
		static double GetMissionMean();
		static void UpdateMissionMean(double newValue, bool increaseCount = true);

		static void OnCourseFinish();
		static void OnMissionFinish(int missionGrade, bool checksumValid, int world);
		static void OnScoreAttackFinish(bool isweekly, int score, int courseID);
		static void StatsMenu(MenuEntry* entry);
		static int GetSequenceID();
	private:
		static const char* statStr[];
		static minibson::document statsDoc;
		static minibson::document* uploadDoc;
		static Mutex statsDocMutex;
		static Task uploadStatsTask;

		static s32 racePointsPos;

		static minibson::document& GetUploadedStats();
		static minibson::document& GetPendingStats();
		static void SetSequenceID(int seqID);
		static void RemovePendingUploads();
		static s32 UploadStatsFunc(void* args);
		static int GetDocStat(const minibson::document& doc, Stat stat, int courseID = -1);
		static std::pair<s64, s64> GetDocStatPair(const minibson::document& doc, Stat stat, int courseID = -1);
		static void IncreaseDocStat(minibson::document &doc, Stat stat, int courseID = -1, int amount = 1);
		static void ForceDocStat(minibson::document &doc, Stat stat, int courseID, int amount);
		static std::pair<double, int> GetDocMissionMean(const minibson::document& doc);
		static void UpdateDocMissionMean(minibson::document &doc, double newMean, bool increaseCount = true);
		static std::vector<std::pair<int, int>> GetMostPlayedCourses();
		static std::vector<std::pair<int, int>> GetMostPlayedArenas();
	};
}