/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StatsHandler.cpp
Open source lines: 716/716 (100.00%)
*****************************************************/

#include "StatsHandler.hpp"
#include "BadgeManager.hpp"
#include "SaveHandler.hpp"
#include "CourseManager.hpp"
#include "Unicode.h"
#include "ExtraResource.hpp"
#include "main.hpp"
#include "malloc.h"
#include "UserCTHandler.hpp"
#include "set"
#include "time.h"
#include "SaveBackupHandler.hpp"

namespace CTRPluginFramework {

	const char* StatsHandler::statStr[] = { // Append #number to increase the version number
		"launches",
		"",
		"races",
		"ttrials",
		"coin_battles",
		"balloon_battles",
		"online_races",
		"comm_races",
		"ctww_races",
		"cd_races",
		"online_coin_battles",
		"online_balloon_battles",
		"score_attack_races",
		"weekly_challenge_attempts",
		"",
		"",
		"failed_mission#2",
		"completed_mission#2",
		"perfect_mission#2",
		"custom_mission#2",
		"grademean_mission#2",
		"gradecount_mission#2",
		"",
		"race_points",
		"played_tracks",
		"score_track_mean",
	};
	minibson::document StatsHandler::statsDoc;
	minibson::document* StatsHandler::uploadDoc;
	Mutex StatsHandler::statsDocMutex{};
	#if CITRA_MODE == 0
	Task StatsHandler::uploadStatsTask(StatsHandler::UploadStatsFunc, nullptr, Task::Affinity::AppCore);
	#else
	Task StatsHandler::uploadStatsTask(StatsHandler::UploadStatsFunc, nullptr, Task::Affinity::AppCore);
	#endif
	s32 StatsHandler::racePointsPos = -1;

    void StatsHandler::Initialize()
    {
		SaveBackupHandler::AddDoBackupHandler("stats", CreateBackup);
		SaveBackupHandler::AddRestoreBackupHandler("stats", RestoreBackup);

		{
			SaveHandler::SaveFile::LoadStatus status;
			minibson::document doc = SaveHandler::SaveFile::Load(SaveHandler::SaveFile::SaveType::STATS, status);
			if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS) {
				statsDoc = std::move(doc);
			} else {
				SaveHandler::SaveFile::HandleLoadError(SaveHandler::SaveFile::SaveType::STATS, status);
			}
		}

		if (!statsDoc.contains<minibson::document>("Uploaded")) {
			statsDoc.set("Uploaded", minibson::document());
		}
		if (!statsDoc.contains<minibson::document>("Pending")) {
			statsDoc.set("Pending", minibson::document());
		}
		IncreaseStat(Stat::LAUNCHES);

		// Fix mission mode stats being reset
		{
			minibson::document& uploadedStats = GetUploadedStats();

			int failed_mission = uploadedStats.get<int>("failed_mission", 0);
			int completed_mission = uploadedStats.get<int>("completed_mission", 0);
			int perfect_mission = uploadedStats.get<int>("perfect_mission", 0);
			double grademean_mission = uploadedStats.get<double>("grademean_mission", 0.0);
			int gradecount_mission = uploadedStats.get<int>("gradecount_mission", 0);

			uploadedStats
				.remove("failed_mission")
				.remove("completed_mission")
				.remove("perfect_mission")		
				.remove("grademean_mission")
				.remove("gradecount_mission");

			IncreaseDocStat(uploadedStats, Stat::FAILED_MISSIONS, -1, failed_mission);
			IncreaseDocStat(uploadedStats, Stat::COMPLETED_MISSIONS, -1, completed_mission);
			IncreaseDocStat(uploadedStats, Stat::PERFECT_MISSIONS, -1, perfect_mission);
			UpdateDocMissionMean(uploadedStats, grademean_mission, false);
			IncreaseDocStat(uploadedStats, Stat::GRADECOUNT_MISSIONS, -1, gradecount_mission);
		}
		//
	}

	void StatsHandler::CommitToFile()
	{
		{
			Lock lock(statsDocMutex);
			statsDoc.set("_cID", NetHandler::GetConsoleUniqueHash());
        	statsDoc.set<s64>("sID0", SaveHandler::saveData.saveID[0]);
			statsDoc.set<s64>("sID1", SaveHandler::saveData.saveID[1]);
			auto res = SaveHandler::SaveFile::Save(SaveHandler::SaveFile::SaveType::STATS, statsDoc);
			if (res != SaveHandler::SaveFile::SaveStatus::SUCCESS) SaveHandler::SaveFile::HandleSaveError(SaveHandler::SaveFile::SaveType::STATS, res);
		}
		BadgeManager::CommitToFile();
	}

    minibson::document StatsHandler::CreateBackup()
    {
		Lock lock(statsDocMutex);
		minibson::document copy = statsDoc;
		copy.remove("_cID");
		copy.remove("seqID");
        copy.remove("sID0");
        copy.remove("sID1");
        return copy;
    }

    bool StatsHandler::RestoreBackup(const minibson::document &doc)
    {
		Lock lock(statsDocMutex);
		statsDoc = doc;
        return true;
    }

    void StatsHandler::UploadStats(bool async, bool forceUpload)
	{
		Lock lock(statsDocMutex);
		if (uploadDoc != nullptr || !SaveHandler::saveData.flags1.serverCommunication)
			return;
		int seqID = GetSequenceID();
		if (seqID == 0) {
			uploadDoc = new minibson::document();
			uploadDoc->set<int>("seqID", seqID);
			if (async) uploadStatsTask.Start(); else UploadStatsFunc(nullptr);
		}
		else {
			const minibson::document& pending = GetPendingStats();
			if (!pending.empty() || forceUpload) {
				uploadDoc = new minibson::document(pending);
				uploadDoc->set<int>("seqID", seqID);
				uploadDoc->set<minibson::document>("status", FetchSendStatus());
				if (async) uploadStatsTask.Start(); else UploadStatsFunc(nullptr);
			}
		}
	}

	minibson::document StatsHandler::FetchSendStatus() {
		minibson::document ret;
		
		ret.set<bool>("allgold", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_GOLD));
		ret.set<bool>("all1star", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_ONE_STAR));
		ret.set<bool>("all3star", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_THREE_STAR));
		ret.set<bool>("all10pts", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_MISSION_TEN));
		ret.set<bool>("vr5000", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::VR_5000));
		ret.set<bool>("scoreat", SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_SCORE_COMPLETED));
		ret.set<bool>("bluecoin", SaveHandler::saveData.IsSpecialAchievementCompleted(SaveHandler::SpecialAchievements::ALL_BLUE_COINS));
		ret.set<bool>("dodgedblue", SaveHandler::saveData.IsSpecialAchievementCompleted(SaveHandler::SpecialAchievements::DODGED_BLUE_SHELL));
		ret.set<bool>("watchedcredits", SaveHandler::saveData.flags1.creditsWatched);

		ret.set<int>("version", achievementReportVersion);
		return ret;
	}
	
	static minibson::document g_defaultDoc;
	minibson::document& StatsHandler::GetUploadedStats()
	{
		return statsDoc.get_noconst("Uploaded", g_defaultDoc);
	}

	minibson::document& StatsHandler::GetPendingStats()
	{
		return statsDoc.get_noconst("Pending", g_defaultDoc);
	}

	int StatsHandler::GetSequenceID()
	{
		Lock lock(statsDocMutex);
		return statsDoc.get<int>("seqID", 0);
	}

	void StatsHandler::SetSequenceID(int seqID)
	{
		Lock lock(statsDocMutex);
		statsDoc.set<int>("seqID", seqID);
	}

	static void addDocumentValues(minibson::document& target, const minibson::document& doc1, const minibson::document& doc2)
	{
		for (auto it = doc1.cbegin(); it != doc1.cend(); it++) {
			const std::string& key = (*it).first;
			minibson::node* value = (*it).second.get();
			if (value->get_node_code() == minibson::bson_node_type::int32_node)
			{
				int doc1Val = static_cast<const minibson::int32*>(value)->get_value();
				target.set<int>(key.c_str(), doc1Val);
			} else if (value->get_node_code() == minibson::bson_node_type::double_node)
			{
				double doc1Val = static_cast<const minibson::Double*>(value)->get_value();
				target.set<double>(key.c_str(), doc1Val);
			} else if (value->get_node_code() == minibson::bson_node_type::binary_node)
			{
				auto& doc1Val = static_cast<const minibson::binary*>(value)->get_value();
				target.set(key.c_str(), doc1Val);
			}
		}
		for (auto it = doc2.cbegin(); it != doc2.cend(); it++) {
			const std::string& key = (*it).first;
			minibson::node* value = (*it).second.get();
			if (value->get_node_code() == minibson::bson_node_type::int32_node)
			{
				int doc2Val = static_cast<const minibson::int32*>(value)->get_value();
				int targetVal = target.get<int>(key.c_str(), 0);
				target.set<int>(key.c_str(), doc2Val + targetVal);
			} else if (value->get_node_code() == minibson::bson_node_type::double_node)
			{
				double doc2Val = static_cast<const minibson::Double*>(value)->get_value();
				double targetVal = target.get<double>(key.c_str(), 0.0);
				target.set<double>(key.c_str(), doc2Val + targetVal);
			} else if (value->get_node_code() == minibson::bson_node_type::binary_node)
			{
				auto& doc2Val = static_cast<const minibson::binary*>(value)->get_value();
				if (doc2Val.Size() == 0xC) {
					const auto& targetVal = target.get_binary(key.c_str());
					if (targetVal.Size() == 0xC) {
						s64 score[2]; int count[2];
						u8 buf[sizeof(s64) + sizeof(int)];
						memcpy(&score[0], targetVal.Data(), sizeof(s64));
						memcpy(&count[0], (u8*)targetVal.Data() + sizeof(s64), sizeof(int));
						memcpy(&score[1], doc2Val.Data(), sizeof(s64));
						memcpy(&count[1], (u8*)doc2Val.Data() + sizeof(s64), sizeof(int));
						score[0] += score[1];
						count[0] += count[1];
						memcpy(buf, &score[0], sizeof(s64));
						memcpy(buf + sizeof(s64), &count[0], sizeof(int));
						target.set(key.c_str(), MiscUtils::Buffer(buf, sizeof(buf)));
					}
				}
			}
		}
	}

	void StatsHandler::RemovePendingUploads()
	{
		Lock lock(statsDocMutex);
		minibson::document tempDocument;
		minibson::document emptyDoc;
		const minibson::document& uploaded = GetUploadedStats();
		const minibson::document& pending = GetPendingStats();

		addDocumentValues(tempDocument, pending, uploaded);

		minibson::document tempDocument2;
		addDocumentValues(tempDocument2, pending.get(statStr[(int)Stat::TRACK_FREQ], emptyDoc), uploaded.get(statStr[(int)Stat::TRACK_FREQ], emptyDoc));
		tempDocument.set(statStr[(int)Stat::TRACK_FREQ], tempDocument2);

		tempDocument2.clear();
		addDocumentValues(tempDocument2, pending.get(statStr[(int)Stat::SCORE_ATTACK_MEAN], emptyDoc), uploaded.get(statStr[(int)Stat::SCORE_ATTACK_MEAN], emptyDoc));
		tempDocument.set(statStr[(int)Stat::SCORE_ATTACK_MEAN], tempDocument2);
		
		statsDoc.set("Pending", emptyDoc);
		statsDoc.set("Uploaded", tempDocument);
	}

	s32 StatsHandler::UploadStatsFunc(void* args) {
		if (uploadDoc == nullptr)
			return -1;

		NetHandler::RequestHandler reqHandler;
		reqHandler.AddRequest(NetHandler::RequestHandler::RequestType::GENERAL_STATS, *uploadDoc);

		reqHandler.Start();
		reqHandler.Wait();

		minibson::document outdoc;
		int resultCode = reqHandler.GetResult(NetHandler::RequestHandler::RequestType::GENERAL_STATS, &outdoc);
		int sequenceID = outdoc.get_numerical("seqID", 0);
		if (resultCode == 0) { // Sequence ID correct and stats stored
			RemovePendingUploads();
			s64 racePoints = outdoc.get_numerical("points", -1);
			if (racePoints >= 0) {
				Lock lock(statsDocMutex);
				ForceDocStat(GetUploadedStats(), Stat::RACE_POINTS, -1, racePoints);
			}
			racePointsPos = outdoc.get<int>("pointsPos", -1);
			SetSequenceID(sequenceID);
			CommitToFile();

			auto& badges_bin = outdoc.get_binary("badges");
			std::vector<u64> badges(badges_bin.Size() / sizeof(u64));
			memcpy(badges.data(), badges_bin.Data(), badges_bin.Size());
			auto& times_bin = outdoc.get_binary("badge_times");
			std::vector<u64> times(times_bin.Size() / sizeof(u64));
			memcpy(times.data(), times_bin.Data(), times_bin.Size());
			BadgeManager::ProcessMyBadges(badges, times);
		}
		else if (resultCode == 1) // Sequence ID request successful
		{
			SetSequenceID(sequenceID);
			CommitToFile();
		}
		else if (resultCode == 2) // Sequence ID incorrect, new sequence ID assigned
		{
			RemovePendingUploads();
			SetSequenceID(sequenceID);
			CommitToFile();
		}
		
		delete uploadDoc;
		uploadDoc = nullptr;
		return 0;
	}

	int StatsHandler::GetDocStat(const minibson::document& doc, Stat stat, int courseID)
	{
		if (stat == Stat::TRACK_FREQ) {
			if (courseID >= MAXTRACKS)
				return 0;
			minibson::document defDoc;
			const char* courseName = globalNameData.entries[courseID].name;
			const minibson::document& races = doc.get(statStr[(int)stat], defDoc);
			return races.get<int>(courseName, 0);
		}
		else {
			return doc.get<int>(statStr[(int)stat], 0);
		}
	}

	std::pair<s64, s64> StatsHandler::GetDocStatPair(const minibson::document& doc, Stat stat, int courseID) {
		std::pair<s64, s64> ret;
		if (stat == Stat::SCORE_ATTACK_MEAN) {
			if (courseID >= MAXTRACKS)
				return ret;
			minibson::document defDoc;
			const char* courseName = globalNameData.entries[courseID].name;
			const minibson::document& scores = doc.get(statStr[(int)stat], defDoc);
			const auto& buff = scores.get_binary(courseName);
			if (buff.Size() == sizeof(s64) + sizeof(int)) {
				s64 score_val; int count_val;
				memcpy(&score_val, buff.Data(), sizeof(s64));
				memcpy(&count_val, (u8*)buff.Data() + sizeof(s64), sizeof(int));
				ret = std::make_pair(score_val, (s64)count_val);
			}
		}
		return ret;
	}

	void StatsHandler::IncreaseDocStat(minibson::document& doc, Stat stat, int courseID, int amount)
	{
		int prevVal = GetDocStat(doc, stat, courseID);
		if (stat == Stat::TRACK_FREQ) {
			if (courseID >= MAXTRACKS)
				return;
			const char* courseName = globalNameData.entries[courseID].name;
			minibson::document defDoc;
			if (!doc.contains<minibson::document>(statStr[(int)stat]))
			{
				doc.set(statStr[(int)stat], defDoc);
			}
			minibson::document& races = doc.get_noconst(statStr[(int)stat], defDoc);
			races.set<int>(courseName, prevVal + amount);
		} else if (stat == Stat::SCORE_ATTACK_MEAN) {
			if (courseID >= MAXTRACKS)
				return;
			auto prevPair = GetDocStatPair(doc, stat, courseID);
			const char* courseName = globalNameData.entries[courseID].name;
			minibson::document defDoc;
			if (!doc.contains<minibson::document>(statStr[(int)stat]))
			{
				doc.set(statStr[(int)stat], defDoc);
			}
			minibson::document& scores = doc.get_noconst(statStr[(int)stat], defDoc);
			u8 storeBuf[sizeof(s64) + sizeof(int)];
			s64 score_val = prevPair.first + amount;
			int count_val = prevPair.second + 1;
			memcpy(storeBuf, &score_val, sizeof(s64));
			memcpy(storeBuf + sizeof(s64), &count_val, sizeof(int));
			scores.set(courseName, MiscUtils::Buffer(storeBuf, sizeof(storeBuf)));
		}
		else {
			doc.set<int>(statStr[(int)stat], prevVal + amount);
		}
	}

	void StatsHandler::ForceDocStat(minibson::document &doc, Stat stat, int courseID, int amount) {
		if (stat == Stat::TRACK_FREQ) {
			if (courseID >= MAXTRACKS)
				return;
			const char* courseName = globalNameData.entries[courseID].name;
			minibson::document defDoc;
			if (!doc.contains<minibson::document>(statStr[(int)stat]))
			{
				doc.set(statStr[(int)stat], defDoc);
			}
			minibson::document& races = doc.get_noconst(statStr[(int)stat], defDoc);
			races.set<int>(courseName, amount);
		} else {
			doc.set<int>(statStr[(int)stat], amount);
		}
	}

	std::pair<double, int> StatsHandler::GetDocMissionMean(const minibson::document& doc) {
		double mean = doc.get<double>(statStr[(int)Stat::GRADEMEAN_MISSIONS], 0.0);
		int storedAmount = doc.get<int>(statStr[(int)Stat::GRADECOUNT_MISSIONS], 0);
		return std::make_pair(mean, storedAmount);
	}

	void StatsHandler::UpdateDocMissionMean(minibson::document &doc, double newMean, bool increaseCount) {
		double prevMean = doc.get<double>(statStr[(int)Stat::GRADEMEAN_MISSIONS], 0.0);
		doc.set<double>(statStr[(int)Stat::GRADEMEAN_MISSIONS], prevMean + newMean);
		if (increaseCount) IncreaseDocStat(doc, Stat::GRADECOUNT_MISSIONS);
	}

	std::vector<std::pair<int, int>> StatsHandler::GetMostPlayedCourses()
	{
		std::vector<std::pair<int, int>> trackFreq;
		for (int i = ORIGINALTRACKLOWER; i <= ORIGINALTRACKUPPER; i++) {
			trackFreq.push_back(std::make_pair(i, GetStat(Stat::TRACK_FREQ, i) + GetStatPair(Stat::SCORE_ATTACK_MEAN, i).second));
		}
		for (int i = CUSTOMTRACKLOWER; i <= CUSTOMTRACKUPPER; i++) {
			trackFreq.push_back(std::make_pair(i, GetStat(Stat::TRACK_FREQ, i) + GetStatPair(Stat::SCORE_ATTACK_MEAN, i).second));
		}
		std::sort(trackFreq.begin(), trackFreq.end(), [](std::pair<int, int> a, std::pair<int, int> b) {
			return a.second > b.second;
		});
		return trackFreq;
	}

	std::vector<std::pair<int, int>> StatsHandler::GetMostPlayedArenas()
	{
		std::vector<std::pair<int, int>> trackFreq;
		for (int i = BATTLETRACKLOWER; i <= BATTLETRACKUPPER; i++) {
			trackFreq.push_back(std::make_pair(i, GetStat(Stat::TRACK_FREQ, i)));
		}
		std::sort(trackFreq.begin(), trackFreq.end(), [](std::pair<int, int> a, std::pair<int, int> b) {
			return a.second > b.second;
		});
		return trackFreq;
	}

	void StatsHandler::OnCourseFinish()
	{
		u32 lastTrack = CourseManager::lastLoadedCourseID;
		if (lastTrack == USERTRACKID || (!ExtraResource::lastTrackFileValid && !(lastTrack == 0x7 || lastTrack == 0x8 || lastTrack == 0x9 || lastTrack == 0x1D)))
			return;
		MarioKartFramework::CRaceMode& raceMode = MarioKartFramework::currentRaceMode;
		if (raceMode.type == 0 || raceMode.type == 1) { // Offline and multiplayer
			switch (raceMode.mode)
			{
			case 0:
			case 2:
				IncreaseStat(Stat::RACES, lastTrack);
				break;
			case 1:
				IncreaseStat(Stat::TT, lastTrack);
				break;
			case 3:
				if (raceMode.submode == 0)
					IncreaseStat(Stat::COIN_BATTLES, lastTrack);
				else if (raceMode.submode == 1)
					IncreaseStat(Stat::BALLOON_BATTLES, lastTrack);
				break;
			default:
				break;
			}
		}
		else if (raceMode.type == 2) {
			// TODO: What about communities?
			switch (raceMode.mode)
			{
			case 0:
			case 2:
				if (g_getCTModeVal == CTMode::ONLINE_NOCTWW)
					IncreaseStat(Stat::ONLINE_RACES, lastTrack);
				else if (g_getCTModeVal == CTMode::ONLINE_CTWW)
					IncreaseStat(Stat::CTWW_RACES, lastTrack);
				else if (g_getCTModeVal == CTMode::ONLINE_CTWW_CD)
					IncreaseStat(Stat::CD_RACES, lastTrack);
				break;
			case 3:
				if (raceMode.submode == 0)
					IncreaseStat(Stat::ONLINE_COIN_BATTLES, lastTrack);
				else if (raceMode.submode == 1)
					IncreaseStat(Stat::ONLINE_BALLOON_BATTLES, lastTrack);
				break;
			default:
				break;
			}
		}
	}

	void StatsHandler::OnMissionFinish(int missionGrade, bool checksumValid, int world) {
		#if CITRA_MODE == 0
		if (!checksumValid)
		#else
		if (world > 8)
		#endif
		{
			IncreaseStat(Stat::CUSTOM_MISSIONS);
		} else
		{
			if (missionGrade == 0) {
				IncreaseStat(Stat::FAILED_MISSIONS);
			} else if (missionGrade == 10) {
				IncreaseStat(Stat::PERFECT_MISSIONS);
			} else {
				IncreaseStat(Stat::COMPLETED_MISSIONS);
			}
			if (missionGrade > 0) UpdateMissionMean(missionGrade);
		}
	}

    void StatsHandler::OnScoreAttackFinish(bool isWeekly, int score, int courseID)
    {
		IncreaseStat(isWeekly ? Stat::WEEKLY_CHALLENGE_ATTEMPTS : Stat::SCORE_ATTACK_RACES);
		if (!isWeekly) {
			IncreaseStat(Stat::SCORE_ATTACK_MEAN, courseID, score);
		}
    }

    static int g_keyboardkey = -1;
	void StatsHandler::StatsMenu(MenuEntry* entry)
	{
		constexpr int NORMALPAGECOUNT = 4;
		std::vector<std::pair<int, int>> raceStats = GetMostPlayedCourses();
		std::vector<std::pair<int, int>> battleStats = GetMostPlayedArenas();
		int launches = GetStat(Stat::LAUNCHES);
		int races = GetStat(Stat::RACES);
		int tt = GetStat(Stat::TT);
		int balloonBattles = GetStat(Stat::BALLOON_BATTLES);
		int coinBattles = GetStat(Stat::COIN_BATTLES);
		int onlineraces = GetStat(Stat::ONLINE_RACES);
		int communityraces = GetStat(Stat::COMMUNITY_RACES);
		int ctwwraces = GetStat(Stat::CTWW_RACES);
		int cdraces = GetStat(Stat::CD_RACES);
		int onlineBallonBattles = GetStat(Stat::ONLINE_BALLOON_BATTLES);
		int onlineCoinBattles = GetStat(Stat::ONLINE_COIN_BATTLES);
		int racePoints = GetStat(Stat::RACE_POINTS);

		int failedMissions = GetStat(Stat::FAILED_MISSIONS);
		int completedMissions = GetStat(Stat::COMPLETED_MISSIONS);
		int maxGradeMissions = GetStat(Stat::PERFECT_MISSIONS);
		int customMissions = GetStat(Stat::CUSTOM_MISSIONS);
		
		int scoreAtTot = GetStat(Stat::SCORE_ATTACK_RACES);
		int weeklyTot = GetStat(Stat::WEEKLY_CHALLENGE_ATTEMPTS);

		std::string enSlid = "\u2282\u25CF";
		std::string disSlid = "\u25CF\u2283";
		int currMenu = 0;
		int raceMenus = ceilf(raceStats.size() / 8.f);
		int battleMenus = ceilf(battleStats.size() / 8.f);
		int totalMenu = NORMALPAGECOUNT + raceMenus + battleMenus;
		int opt = 0;
		Keyboard kbd("dummy");
		kbd.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
			if (event.type == KeyboardEvent::EventType::KeyPressed) {
				if (event.affectedKey == Key::R) {
					g_keyboardkey = 1;
					SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
					k.Close();
				}
				else if (event.affectedKey == Key::L) {
					g_keyboardkey = 2;
					SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
					k.Close();
				}
			}
		});
		do {
			kbd.Populate({ NAME("exit") });
			kbd.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
			std::string topStr = CenterAlign(NAME("statsentry") + "\n");
			std::string fmtStr = std::string(FONT_L " ") + NAME("page") + " (%02d/%02d) " FONT_R;
			topStr += ToggleDrawMode(Render::UNDERLINE) + " " + CenterAlign(Utils::Format(fmtStr.c_str(), currMenu + 1, totalMenu)) + RightAlign(" ", 30, 365) + ToggleDrawMode(Render::UNDERLINE);
			topStr += ToggleDrawMode(Render::LINEDOTTED);
			if (currMenu == 0) {
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NAME("stats_tpl") + ":" + ToggleDrawMode(Render::UNDERLINE) + RightAlign(std::to_string(launches), 30, 320);
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NAME("stats_tot") + ":" + ToggleDrawMode(Render::UNDERLINE) + RightAlign(std::to_string(races + tt + balloonBattles + coinBattles), 30, 320);
				topStr += "\n    " + NAME("race_noun") + ":" + RightAlign(std::to_string(races), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2201) + ":" + RightAlign(std::to_string(tt), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2385) + ":" + RightAlign(std::to_string(balloonBattles), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2386) + ":" + RightAlign(std::to_string(coinBattles), 30, 320);
				if (racePointsPos > 0)
					topStr += "\n    " + NAME("stats_rpoints") + ":" + RightAlign(std::to_string(racePoints) + MissionHandler::GetPtsString(racePoints) + " (" + Language::GenerateOrdinal(racePointsPos) + ")", 30, 320);
				else
					topStr += "\n    " + NAME("stats_rpoints") + ":" + RightAlign(std::to_string(racePoints) + MissionHandler::GetPtsString(racePoints), 30, 320);
			}
			else if (currMenu == 1) {
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NOTE("stats_tot") + ":" + ToggleDrawMode(Render::UNDERLINE) + RightAlign(std::to_string(onlineraces + communityraces + ctwwraces + cdraces + onlineBallonBattles + onlineCoinBattles), 30, 320);
				topStr += "\n    " + NAME("stats_vnll") + ":" + RightAlign(std::to_string(onlineraces), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2096) + ":" + RightAlign(std::to_string(communityraces), 30, 320);
				topStr += "\n    " + NAME("ctww") + ":" + RightAlign(std::to_string(ctwwraces), 30, 320);
				topStr += "\n    " + NAME("cntdwn") + ":" + RightAlign(std::to_string(cdraces), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2385) + ":" + RightAlign(std::to_string(onlineBallonBattles), 30, 320);
				topStr += "\n    " + Language::MsbtHandler::GetString(2386) + ":" + RightAlign(std::to_string(onlineCoinBattles), 30, 320);
			} else if (currMenu == 2) {
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NAME("msstat_msnplay") + ":" + ToggleDrawMode(Render::UNDERLINE) + RightAlign(std::to_string(failedMissions + completedMissions + maxGradeMissions), 30, 320);
				topStr += "\n    " + NAME("msstat_state") + ":" + RightAlign(std::to_string(completedMissions + maxGradeMissions), 30, 320);
				topStr += "\n        " + NAME("msstat_perfgrad") + ":" + RightAlign(std::to_string(maxGradeMissions), 30, 320);
				topStr += "\n    " + NOTE("msstat_state") + ":" + RightAlign(std::to_string(failedMissions), 30, 320);
				topStr += "\n\n    " + NAME("msstat_usermsn") + ":" + RightAlign(std::to_string(customMissions), 30, 320);
			} else if (currMenu == 3) {
				topStr += "\n\n" + NAME("stats_sc") + ":" + RightAlign(std::to_string(scoreAtTot), 30, 320);
				topStr += "\n" + NOTE("stats_sc") + ":" + RightAlign(std::to_string(weeklyTot), 30, 320);
			}
			else if (currMenu >= NORMALPAGECOUNT && currMenu < raceMenus + NORMALPAGECOUNT) {
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NAME("stats_most") + ToggleDrawMode(Render::UNDERLINE);
				for (int i = 0; i < 8; i++) {
					int currPos = (currMenu - NORMALPAGECOUNT) * 8 + i;
					if (currPos >= raceStats.size())
						break;
					topStr += Utils::Format("\n%d.    ", currPos + 1);
					std::pair<int, int>& currCouse = raceStats[currPos];
					CourseManager::getCourseText(topStr, currCouse.first, true);
					topStr += ":" + RightAlign(std::to_string(currCouse.second), 30, 320);
				}
			}
			else if (currMenu >= raceMenus + NORMALPAGECOUNT && currMenu < totalMenu) {
				topStr += ToggleDrawMode(Render::UNDERLINE) + "\n\n" + NOTE("stats_most") + ToggleDrawMode(Render::UNDERLINE);
				for (int i = 0; i < 8; i++) {
					int currPos = (currMenu - raceMenus - NORMALPAGECOUNT) * 8 + i;
					if (currPos >= battleStats.size())
						break;
					topStr += Utils::Format("\n%d.    ", currPos + 1);
					std::pair<int, int>& currCouse = battleStats[currPos];
					CourseManager::getCourseText(topStr, currCouse.first, true);
					topStr += ":" + RightAlign(std::to_string(currCouse.second), 30, 320);
				}
			}
			kbd.GetMessage() = topStr;
			opt = kbd.Open();
			if (g_keyboardkey != -1) opt = g_keyboardkey;
			g_keyboardkey = -1;
			switch (opt)
			{
			case 1:
				currMenu++;
				if (currMenu >= totalMenu)
					currMenu = 0;
				break;
			case 2:
				currMenu--;
				if (currMenu < 0)
					currMenu = totalMenu - 1;
				break;
			default:
				opt = -1;
				break;
			}
		} while (opt != 0 && opt != -1);
	}

	int StatsHandler::GetStat(Stat stat, int courseID)
	{
		Lock lock(statsDocMutex);
		return GetDocStat(GetPendingStats(), stat, courseID) + GetDocStat(GetUploadedStats(), stat, courseID);
	}

    std::pair<s64, s64> StatsHandler::GetStatPair(Stat stat, int courseID)
    {
		Lock lock(statsDocMutex);
		auto pending = GetDocStatPair(GetPendingStats(), stat, courseID);
		auto uploaded = GetDocStatPair(GetUploadedStats(), stat, courseID);
        return std::pair<s64, s64>(pending.first + uploaded.first, pending.second + uploaded.second);
    }

    void StatsHandler::IncreaseStat(Stat stat, int courseID, int amount)
	{
#ifdef IGNORE_STATS
		return;
#endif // IGNORE_STAT

		Lock lock(statsDocMutex);
		IncreaseDocStat(GetPendingStats(), stat, courseID, amount);
		if (stat > Stat::R_START && stat < Stat::R_END && courseID != -1) {
			IncreaseStat(Stat::TRACK_FREQ, courseID);
		}
	}

	double StatsHandler::GetMissionMean() {
		Lock lock(statsDocMutex);
		std::pair<double, int> pending = GetDocMissionMean(GetPendingStats());
		std::pair<double, int> uploaded = GetDocMissionMean(GetUploadedStats());
		int total = (std::get<1>(pending) + std::get<1>(uploaded));
		if (total == 0)
			return 0.0;
		return (std::get<0>(pending) + std::get<0>(uploaded)) / total;
	}

	void StatsHandler::UpdateMissionMean(double newValue, bool increaseCount) {
#ifdef IGNORE_STATS
		return;
#endif // IGNORE_STAT

		Lock lock(statsDocMutex);
		UpdateDocMissionMean(GetPendingStats(), newValue, increaseCount);
	}
}