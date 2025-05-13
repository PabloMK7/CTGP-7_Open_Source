/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StatsHandler.cpp
Open source lines: 1064/1066 (99.81%)
*****************************************************/

#include "StatsHandler.hpp"
#include "SaveHandler.hpp"
#include "CourseManager.hpp"
#include "Unicode.h"
#include "ExtraResource.hpp"
#include "main.hpp"
#include "malloc.h"
#include "UserCTHandler.hpp"
#include "set"
#include "time.h"

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
		"played_tracks"
	};
	minibson::encdocument StatsHandler::statsDoc;
	minibson::document* StatsHandler::uploadDoc;
	Mutex StatsHandler::statsDocMutex{};
	#if CITRA_MODE == 0
	Task StatsHandler::uploadStatsTask(StatsHandler::UploadStatsFunc, nullptr, Task::Affinity::AppCores);
	#else
	Task StatsHandler::uploadStatsTask(StatsHandler::UploadStatsFunc, nullptr, Task::Affinity::AppCore);
	#endif
	bool StatsHandler::firstReport = true;
	s32 StatsHandler::racePointsPos = -1;

	minibson::document BadgeManager::saved_badges;
	minibson::document BadgeManager::cached_badges;
	Mutex BadgeManager::badges_mutex;
	u64 BadgeManager::cache_badge_time = 0;
	std::vector<u64> BadgeManager::badges_to_cache;
	#if CITRA_MODE == 0
	Task BadgeManager::cacheBadgesTask(BadgeManager::CacheBadgesFunc, nullptr, Task::Affinity::AppCores);
	#else
	Task BadgeManager::cacheBadgesTask(BadgeManager::CacheBadgesFunc, nullptr, Task::Affinity::AppCore);
	#endif
	BCLIM BadgeManager::empty_badge;
	std::array<BCLIM, 8> BadgeManager::badge_slots;

	void BadgeManager::Initialize()
	{
		saved_badges.clear();
		SaveHandler::SaveFile::LoadStatus status;
		minibson::document doc = SaveHandler::SaveFile::Load(SaveHandler::SaveFile::SaveType::BADGES, status);
		u64 scID = doc.get<u64>("cID", 0);
		if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS && (scID == NetHandler::GetConsoleUniqueHash()
		#ifdef ALLOW_SAVES_FROM_OTHER_CID
		|| true
		#endif
		)) {
			saved_badges = doc;
		}
	}

    std::vector<u64> BadgeManager::GetSavedBadgeList()
    {
        std::vector<std::pair<u64, u64>> temp;
		for (auto it = saved_badges.begin(); it != saved_badges.end(); it++) {
			if (it->first == "cID") continue;
			u64 key = std::strtoll(it->first.c_str(), NULL, 10);
			u64 time = 0;
			if (it->second->get_node_code() == minibson::bson_node_type::document_node) {
				time = reinterpret_cast<const minibson::document*>(it->second)->get<s64>("time", 0);
			}
			temp.push_back({key, time});
		}
		std::sort(temp.begin(), temp.end(), [](const std::pair<u64, u64>& a, const std::pair<u64, u64>& b) {
			return a.second > b.second; // Compare second elements in descending order
		});
		std::vector<u64> ret(temp.size());
		for (int i = 0; i < temp.size(); i++) {
			ret[i] = temp[i].first;
		}
		return ret;
    }

    BadgeManager::Badge BadgeManager::GetBadge(u64 badgeID, GetBadgeMode mode)
    {
		Badge ret{};

		auto& doc = GetBadgeDocument(badgeID, mode);
		if (doc.empty()) {
			return ret;
		}

		ret.bID = badgeID;
		ret.epoch = doc.get<s64>("time", 0);
		ret.name = doc.get("name", "");
		ret.desc = doc.get("desc", "");
		auto& bin = doc.get_binary("icon");
		if (bin.length != 0) {
			ret.icon = BCLIM(bin.data, bin.length);
		}

		return ret;
    }
	
    void BadgeManager::ProcessMyBadges(const std::vector<u64> &badges, const std::vector<u64>& times)
    {
		Lock lock(badges_mutex);
		if (badges.size() != times.size()) {
			return;
		}

		std::set<u64> my_badges;
		std::map<u64, u64> my_server_badges_times;
		std::set<u64> my_server_badges;
		for (u32 i = 0; i < badges.size(); i++) {
			u64 bID = badges[i];
			u64 time = times[i];
			my_server_badges.insert(bID);
			my_server_badges_times.insert({bID, time});
		}

		for (auto it = saved_badges.begin(); it != saved_badges.end(); it++) {
			if (it->first == "cID") continue;
			u64 key = std::strtoll(it->first.c_str(), NULL, 10);
			my_badges.insert(key);
		}

		std::set<u64> removed;
		std::set<u64> added;
		std::set_difference(my_badges.begin(), my_badges.end(), my_server_badges.begin(), my_server_badges.end(), std::inserter(removed, removed.begin()));
		std::set_difference(my_server_badges.begin(), my_server_badges.end(), my_badges.begin(), my_badges.end(), std::inserter(added, added.begin()));
		for (auto it = removed.begin(); it != removed.end(); it++) {
			std::string key = Utils::Format("%lld", (s64)*it);
			saved_badges.remove(key);
		}
		if (!added.empty()) {
			std::vector<u64> b(added.begin(), added.end());
			b = RequestBadges(saved_badges, b);
			for (auto it = b.begin(); it != b.end(); it++) {
				auto& doc = GetBadgeDocument(*it, GetBadgeMode::SAVED);
				if (!doc.empty()) {
					doc.set<s64>("time", my_server_badges_times[*it]);
					SaveHandler::saveData.flags1.needsBadgeObtainedMsg = true;
				}
			}
		}
		if (!added.empty() || !removed.empty()) {
			CommitToFile();
		}
    }

	static std::vector<u64> g_badge_list;
	static s32 g_starting_row = 0;
	static s32 g_badge_row = 0;
	static s32 g_badge_col = 0;
	static Clock g_input_clock;
    void BadgeManager::BadgesMenuKbdEvent(Keyboard &kbd, KeyboardEvent &event)
    {
		constexpr s32 row_count = 4;
		constexpr s32 col_count = 8;

		auto row_col_to_index = [col_count](s32 row, s32 col) -> s32 {
			return (g_starting_row + row) * col_count + col;
		};
		if (g_badge_list.size() && row_col_to_index(g_badge_row, g_badge_col) >= g_badge_list.size()) {
			g_starting_row = 0;
			g_badge_row = 0;
			g_badge_col = 0;
		}

		auto side_render_interface = [](void* usrData, bool isRead, Color* c, int posX, int posY) -> void {
			auto* usr = reinterpret_cast<std::pair<IntRect*, Render::Interface*>*>(usrData);
			Render::Interface* interface = usr->second;
			IntRect* pos = usr->first;
			int offsetx = posX - pos->leftTop.x;
			int offsety = posY - pos->leftTop.y;
			if (isRead) {
				interface->ReadPixel(pos->leftTop.x + offsety, pos->leftTop.y + (pos->size.x - offsetx), *c);
			} else {
				interface->DrawPixel(pos->leftTop.x + offsety, pos->leftTop.y + (pos->size.x - offsetx), *c);
			}
		};

		if ((event.type == KeyboardEvent::EventType::KeyPressed || event.type == KeyboardEvent::EventType::KeyDown) && !g_badge_list.empty()) {
			Key key = event.affectedKey;
			if (g_input_clock.HasTimePassed(Milliseconds(200))) {
				u32 prevCol = g_badge_col, prevRow = g_badge_row, prevStart = g_starting_row;	
				if ((key & Key::Right) != 0) {
					g_badge_col++;
					if (g_badge_col >= col_count || row_col_to_index(g_badge_row, g_badge_col) >= g_badge_list.size()) {
						g_badge_col--;
					}
					g_input_clock.Restart();
				} else if ((key & Key::Left) != 0) {
					g_badge_col--;
					if (g_badge_col < 0) {
						g_badge_col = 0;
					}
					g_input_clock.Restart();
				} else if ((key & Key::Up) != 0) {
					if (g_badge_row == 0) {
						if (g_starting_row > 0) {
							g_starting_row--;
						}
					} else {
						g_badge_row--;
					}
					g_input_clock.Restart();
				} else if ((key & Key::Down) != 0) {
					if (g_badge_row == row_count - 1) {
						if (row_col_to_index(g_badge_row + 1, 0) < g_badge_list.size()) {
							g_starting_row++;
							while(row_col_to_index(g_badge_row, g_badge_col) >= g_badge_list.size()) g_badge_col--;
						}
					} else {
						if (row_col_to_index(g_badge_row + 1, 0) < g_badge_list.size()) {
							g_badge_row++;
							while(row_col_to_index(g_badge_row, g_badge_col) >= g_badge_list.size()) g_badge_col--;
						} 
					}
					g_input_clock.Restart();
				}
				if (prevRow != g_badge_row || prevCol != g_badge_col || prevStart != g_starting_row) {
					SoundEngine::PlayMenuSound(SoundEngine::Event::CURSOR);
				}
			}
			if (event.type == KeyboardEvent::EventType::KeyPressed && (key & Key::A) != 0) {
				u64 bID = g_badge_list[row_col_to_index(g_badge_row, g_badge_col)];
				if (SaveHandler::saveData.useBadgeOnline == bID) {
					SoundEngine::PlayMenuSound(SoundEngine::Event::DESELECT);
					SaveHandler::saveData.useBadgeOnline = 0;
				} else {
					SoundEngine::PlayMenuSound(SoundEngine::Event::SELECT);
					SaveHandler::saveData.useBadgeOnline = bID;
				}
			}
		} else if (event.type == KeyboardEvent::FrameTop && !g_badge_list.empty()) {
			constexpr u32 starting_x = 44;
			constexpr u32 starting_y = 51;
			bool draw_bot_arrow = true;
			bool draw_top_arrow = g_starting_row != 0;
			for (int i = 0; i < row_count; i++) {
				for (int j = 0; j < col_count; j++) {
					s32 index = row_col_to_index(i, j);
					if (index >= g_badge_list.size()) {
						draw_bot_arrow = false;
						continue;
					}
					if (index == g_badge_list.size() - 1) {
						draw_bot_arrow = false;
					}
					Badge b = GetBadge(g_badge_list[index], GetBadgeMode::SAVED);
					IntRect pos = IntRect(starting_x + j * (32 + 8), starting_y + i * (32 + 8), 32, 32);
					auto user_data = std::pair<IntRect*, Render::Interface*>(&pos, event.renderInterface);
					b.icon.Render(pos, std::make_pair(&user_data, side_render_interface));
					
					bool is_in_use = SaveHandler::saveData.useBadgeOnline == b.bID;
					bool is_selected = i == g_badge_row && j == g_badge_col;

					pos.leftTop.x--;
					pos.size.x+=2;
					pos.size.y+=2;
					if (is_in_use || is_selected) {
						Color color;
						if (is_in_use && is_selected) {
							color = Color(128, 255, 128);
						} else if (is_in_use) {
							color = Color::Green;
						} else if (is_selected) {
							color = Color::White;
						}
						event.renderInterface->DrawRect(pos, color, false);
					}
				}
			}
			if (draw_bot_arrow) {
				for (int i = 0; i != 8; i++) {
					event.renderInterface->DrawPixel(192 + i, 207 + i, Color::White);
					event.renderInterface->DrawPixel(207 - i, 207 + i, Color::White);
				}
			}
			if (draw_top_arrow) {
				for (int i = 0; i != 8; i++) {
					event.renderInterface->DrawPixel(192 + (7 - i), 41 + i, Color::White);
					event.renderInterface->DrawPixel(207 - (7 - i), 41 + i, Color::White);
				}
			}
		} else if (event.type == KeyboardEvent::FrameBottom) {
			bool is_selected = false;
			if (!g_badge_list.empty()) {
				Badge b = GetBadge(g_badge_list[row_col_to_index(g_badge_row, g_badge_col)], GetBadgeMode::SAVED);
				is_selected = b.bID == SaveHandler::saveData.useBadgeOnline;
				char formatted_date[20] = {0};
				if (b.epoch) {
					time_t time = (time_t)b.epoch;
					struct tm *tm_info = localtime(&time);
					strftime(formatted_date, 19, NAME("badge_date_fmt").c_str(), tm_info);
				} else {
					formatted_date[0] = '-';
				}
				std::string name = b.name;
				if (name.starts_with('_')) name = name.substr(1);
				std::string s = ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + NAME("badge_name_desc") + 
								ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + "\n" + name + "\n\n" +
								ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + NOTE("badge_name_desc") + 
								ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + "\n" + b.desc + "\n\n" +
								ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + NAME("badge_date") + 
								ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + "\n" + formatted_date; 
				event.renderInterface->DrawSysString(s, 25, 25, Color::White, 296, 216, true);
			}
			std::string usage = std::string(FONT_A) + ": " + (is_selected ? NOTE("badges_use_online") : NAME("badges_use_online")) + "\n" FONT_B ": " + NAME("exit");
			event.renderInterface->DrawSysString(usage, 25, 184, Color::White, 296, 216, true);
		}
    }
    void BadgeManager::BadgesMenu(MenuEntry *entry)
    {
		Lock lock(badges_mutex);
		SaveHandler::saveData.flags1.needsBadgeObtainedMsg = false;

		g_badge_list = GetSavedBadgeList();

		Keyboard kbd(NAME("badges_obtained"));
		kbd.Populate({""});
		kbd.OnKeyboardEvent(BadgesMenuKbdEvent);

		g_input_clock.Restart();
		kbd.Open();
		g_badge_list.clear();
    }

	static bool g_processing_badges = false;
    bool BadgeManager::CheckAndShowBadgePending()
    {
		if (g_processing_badges && MarioKartFramework::isDialogOpened()) return true;
    	if (SaveHandler::saveData.flags1.needsBadgeObtainedMsg) {
			SaveHandler::saveData.flags1.needsBadgeObtainedMsg = false;
			MarioKartFramework::openDialog(DialogFlags::Mode::OK, NAME("badge_msg"));
			g_processing_badges = true;
			MarioKartFramework::playTitleFlagOpenTimer = 30;
			return true;
		}
		g_processing_badges = false;
		return false;
    }

    void BadgeManager::CommitToFile()
    {
		saved_badges.set("cID", NetHandler::GetConsoleUniqueHash());
		SaveHandler::SaveFile::Save(SaveHandler::SaveFile::SaveType::BADGES, saved_badges);
    }

    void BadgeManager::UpdateOnlineBadges()
    {
		Lock lock(badges_mutex);
		std::vector<u64> pending_badges;
		for (int i = 0; i < 8; i++) {
			u64 bID = Net::othersBadgeIDs[i];
			if (bID == 0) {
				continue;
			}

			Badge b = GetBadge(bID, GetBadgeMode::BOTH);
			if (b.bID == 0) {
				pending_badges.push_back(bID);
			}
		}
		if (!pending_badges.empty()) {
			CacheBadges(pending_badges);
		}
    }

    s32 BadgeManager::CacheBadgesFunc(void *args)
    {
        if (badges_to_cache.empty()) {
			return 0;
		}
		Lock lock(badges_mutex);
		badges_to_cache = RequestBadges(cached_badges, badges_to_cache);
		for (auto it = badges_to_cache.begin(); it != badges_to_cache.end(); it++) {
			auto& doc = GetBadgeDocument(*it, GetBadgeMode::CACHED);
			if (!doc.empty()) {
				doc.set<s64>("time", (s64)cache_badge_time++);
			}
		}
		for (auto it = cached_badges.begin(); it != cached_badges.end();) {
			if (it->second->get_node_code() == minibson::bson_node_type::document_node) {
				auto& doc = *reinterpret_cast<minibson::document*>(it->second);
				u64 time = doc.get<s64>("time", 0);
				if (cache_badge_time - time > 10) {
					it = cached_badges.erase(it);
					continue;
				}
			}
			it++;
		}
		badges_to_cache.clear();
		return 0;
    }

    void BadgeManager::CacheBadges(const std::vector<u64> &badges)
    {
		badges_to_cache = badges;
		cacheBadgesTask.Start();
    }

    void BadgeManager::ClearBadgeCache()
    {
		cached_badges.clear();
    }

    void BadgeManager::SetupExtraResource()
    {
		ExtraResource::SARC::FileInfo fInfo;
		u8* data;

		data = ExtraResource::mainSarc->GetFile("UI/race.szs/result_grade_empty_r90.bclim", &fInfo);
		empty_badge = BCLIM(data, fInfo.fileSize);

		for (int i = 0; i < 8; i++) {
			data = ExtraResource::mainSarc->GetFile(Utils::Format("UI/race.szs/result_grade_%d_r90.bclim", i), &fInfo);
			badge_slots[i] = BCLIM(data, fInfo.fileSize);
		}
    }

    static minibson::document g_emptyBadgeDocument;
    minibson::document &BadgeManager::GetBadgeDocument(u64 badgeID, GetBadgeMode mode)
    {
		std::string key = Utils::Format("%lld", (s64)badgeID);

		// If we have it saved, return it
		if ((mode & GetBadgeMode::SAVED) != 0) {
			auto it = saved_badges.find(key);
			if (it != saved_badges.end() && it->second->get_node_code() == minibson::bson_node_type::document_node) {
				auto& doc = *reinterpret_cast<minibson::document*>(it->second);
				if (!doc.empty()) {
					return doc;
				}
			}
		}
		
		// Otherwise return from cache
		if ((mode & GetBadgeMode::CACHED) != 0) {
			auto it = cached_badges.find(key);
			if (it != cached_badges.end() && it->second->get_node_code() == minibson::bson_node_type::document_node) {
				auto& doc = *reinterpret_cast<minibson::document*>(it->second);
				if (!doc.empty()) {
					return doc;
				}
			}
		}		

		return g_emptyBadgeDocument;
    }

    std::vector<u64> BadgeManager::RequestBadges(minibson::document &out_document, const std::vector<u64> badges)
    {
		std::vector<u64> ret;
		NetHandler::RequestHandler reqHandler;
		for (size_t i = 0; i < badges.size(); i+=4) {
			size_t start = i;
			size_t end = std::min(badges.size(), start + 4);

			minibson::document reqDoc;
			reqDoc.set("badges", badges.data() + start, (end-start) * sizeof(u64));
			reqHandler.AddRequest(NetHandler::RequestHandler::RequestType::BADGES, reqDoc);

			reqHandler.Start();
			reqHandler.Wait();

			minibson::document reqoutdoc;
			int resultCode = reqHandler.GetResult(NetHandler::RequestHandler::RequestType::BADGES, &reqoutdoc);
			if (resultCode == 0) {
				for (auto it = reqoutdoc.begin(); it != reqoutdoc.end(); it++) {
					if (it->first.starts_with("badge_")) {
						std::string key = it->first.substr(6);
						if (it->second->get_node_code() == minibson::bson_node_type::document_node) {
						 	ret.push_back(std::strtoll(key.c_str(), NULL, 10));
							out_document.set(key, *reinterpret_cast<const minibson::document*>(it->second));
						}
					}					
				}
			}
			reqHandler.Cleanup();
		}
		return ret;
    }

    void StatsHandler::Initialize()
    {
		{
			SaveHandler::SaveFile::LoadStatus status;
			minibson::document doc = SaveHandler::SaveFile::Load(SaveHandler::SaveFile::SaveType::STATS, status);
			u64 scID = doc.get<u64>("cID", 0);
			if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS && (scID == NetHandler::GetConsoleUniqueHash()
			#ifdef ALLOW_SAVES_FROM_OTHER_CID
			|| true
			#endif
			)) {
				statsDoc = doc;
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
			minibson::document& uploadedStats = const_cast<minibson::document&>(GetUploadedStats());

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
    
		BadgeManager::Initialize();
	}

	void StatsHandler::CommitToFile()
	{
		{
			Lock lock(statsDocMutex);
			statsDoc.set("cID", NetHandler::GetConsoleUniqueHash());
			SaveHandler::SaveFile::Save(SaveHandler::SaveFile::SaveType::STATS, statsDoc);
		}
		BadgeManager::CommitToFile();
	}

	void StatsHandler::UploadStats()
	{
		Lock lock(statsDocMutex);
		if (uploadDoc != nullptr || !SaveHandler::saveData.flags1.uploadStats)
			return;
		int seqID = GetSequenceID();
		if (seqID == 0) {
			uploadDoc = new minibson::document();
			uploadDoc->set<int>("seqID", seqID);
			uploadStatsTask.Start();
		}
		else {
			const minibson::document& pending = GetPendingStats();
			if (!pending.empty()) {
				uploadDoc = new minibson::document(pending);
				uploadDoc->set<int>("seqID", seqID);
				uploadDoc->set<bool>("firstReport", firstReport);
				uploadDoc->set<minibson::document>("status", FetchSendStatus());
				uploadStatsTask.Start();
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
		ret.set<bool>("bluecoin", SaveHandler::saveData.IsSpecialAchievementCompleted(SaveHandler::SpecialAchievements::ALL_BLUE_COINS));
		ret.set<bool>("dodgedblue", SaveHandler::saveData.IsSpecialAchievementCompleted(SaveHandler::SpecialAchievements::DODGED_BLUE_SHELL));
		ret.set<bool>("watchedcredits", SaveHandler::saveData.flags1.creditsWatched);

		ret.set<int>("version", achievementReportVersion);
		return ret;
	}
	
	static const minibson::document g_defaultDoc;
	const minibson::document& StatsHandler::GetUploadedStats()
	{
		return statsDoc.get("Uploaded", g_defaultDoc);
	}

	const minibson::document& StatsHandler::GetPendingStats()
	{
		return statsDoc.get("Pending", g_defaultDoc);
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
			minibson::node* value = (*it).second;
			if (value->get_node_code() == minibson::bson_node_type::int32_node)
			{
				int doc1Val = reinterpret_cast<const minibson::int32*>(value)->get_value();
				target.set<int>(key.c_str(), doc1Val);
			} else if (value->get_node_code() == minibson::bson_node_type::double_node)
			{
				double doc1Val = reinterpret_cast<const minibson::Double*>(value)->get_value();
				target.set<double>(key.c_str(), doc1Val);
			}
		}
		for (auto it = doc2.cbegin(); it != doc2.cend(); it++) {
			const std::string& key = (*it).first;
			minibson::node* value = (*it).second;
			if (value->get_node_code() == minibson::bson_node_type::int32_node)
			{
				int doc2Val = reinterpret_cast<const minibson::int32*>(value)->get_value();
				int targetVal = target.get<int>(key.c_str(), 0);
				target.set<int>(key.c_str(), doc2Val + targetVal);
			} else if (value->get_node_code() == minibson::bson_node_type::double_node)
			{
				double doc2Val = reinterpret_cast<const minibson::Double*>(value)->get_value();
				double targetVal = target.get<double>(key.c_str(), 0.0);
				target.set<double>(key.c_str(), doc2Val + targetVal);
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
			firstReport = false;
			RemovePendingUploads();
			s64 racePoints = outdoc.get_numerical("points", -1);
			if (racePoints >= 0) {
				Lock lock(statsDocMutex);
				ForceDocStat(const_cast<minibson::document&>(GetUploadedStats()), Stat::RACE_POINTS, -1, racePoints);
			}
			racePointsPos = outdoc.get<int>("pointsPos", -1);
			SetSequenceID(sequenceID);
			CommitToFile();

			auto& badges_bin = outdoc.get_binary("badges");
			std::vector<u64> badges(badges_bin.length / sizeof(u64));
			memcpy(badges.data(), badges_bin.data, badges_bin.length);
			auto& times_bin = outdoc.get_binary("badge_times");
			std::vector<u64> times(times_bin.length / sizeof(u64));
			memcpy(times.data(), times_bin.data, times_bin.length);
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
			const minibson::document& races = doc.get(statStr[(int)stat], defDoc);
			const_cast<minibson::document&>(races).set<int>(courseName, prevVal + amount);
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
			const minibson::document& races = doc.get(statStr[(int)stat], defDoc);
			const_cast<minibson::document&>(races).set<int>(courseName, amount);
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
			trackFreq.push_back(std::make_pair(i, GetStat(Stat::TRACK_FREQ, i)));
		}
		for (int i = CUSTOMTRACKLOWER; i <= CUSTOMTRACKUPPER; i++) {
			trackFreq.push_back(std::make_pair(i, GetStat(Stat::TRACK_FREQ, i)));
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
		SaveHandler::SaveSettingsAll();
	}

	static int g_keyboardkey = -1;
	void StatsHandler::StatsMenu(MenuEntry* entry)
	{
		constexpr int NORMALPAGECOUNT = 3;
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

	void StatsHandler::IncreaseStat(Stat stat, int courseID, int amount)
	{
#ifdef IGNORE_STATS
		return;
#endif // IGNORE_STAT

		Lock lock(statsDocMutex);
		IncreaseDocStat(const_cast<minibson::document&>(GetPendingStats()), stat, courseID, amount);
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
		UpdateDocMissionMean(const_cast<minibson::document&>(GetPendingStats()), newValue, increaseCount);
	}
}