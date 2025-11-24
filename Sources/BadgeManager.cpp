/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BadgeManager.cpp
Open source lines: 485/485 (100.00%)
*****************************************************/

#include "BadgeManager.hpp"
#include "set"
#include "Unicode.h"

namespace CTRPluginFramework {
    minibson::document BadgeManager::saved_badges;
	minibson::document BadgeManager::cached_badges;
	Mutex BadgeManager::badges_mutex;
	u64 BadgeManager::cache_badge_time = 0;
	std::vector<u64> BadgeManager::badges_to_cache;
	#if CITRA_MODE == 0
	Task BadgeManager::cacheBadgesTask(BadgeManager::CacheBadgesFunc, nullptr, Task::Affinity::AppCore);
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
		u64 scID = doc.get<u64>("_cID", 0); if (scID == 0) scID = doc.get<u64>("cID", 0);
		if (status == SaveHandler::SaveFile::LoadStatus::SUCCESS && (scID == NetHandler::GetConsoleUniqueHash()
		#ifdef ALLOW_SAVES_FROM_OTHER_CID
		|| true
		#endif
		)) {
			s64 saveID[2];
            saveID[0] = doc.get<s64>("sID0", 0);
            saveID[1] = doc.get<s64>("sID1", 0);
			doc.remove("cID");
            if ((saveID[0] != 0 && saveID[0] != SaveHandler::saveData.saveID[0]) ||
                (saveID[1] != 0 && saveID[1] != SaveHandler::saveData.saveID[1]))
                ;
			else
				saved_badges = std::move(doc);
		}
	}

    std::vector<u64> BadgeManager::GetSavedBadgeList()
    {
        std::vector<std::pair<u64, u64>> temp;
		for (auto it = saved_badges.begin(); it != saved_badges.end(); it++) {
			if (it->first == "_cID" || it->first == "sID0" || it->first == "sID1") continue;
			u64 key = std::strtoll(it->first.c_str(), NULL, 10);
			u64 time = 0;
			if (it->second->get_node_code() == minibson::bson_node_type::document_node) {
				time = static_cast<const minibson::document*>(it->second.get())->get<s64>("time", 0);
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
		if (bin.Size() != 0) {
			ret.icon = BCLIM((void*)bin.Data(), bin.Size());
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
			if (it->first == "_cID" || it->first == "sID0" || it->first == "sID1") continue;
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
		if (!SaveHandler::CheckAndShowServerCommunicationDisabled()) return;

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

    void BadgeManager::ClearAll()
    {
		saved_badges.clear();
    }

    void BadgeManager::CommitToFile()
    {
		saved_badges.set("_cID", NetHandler::GetConsoleUniqueHash());
		saved_badges.set<s64>("sID0", SaveHandler::saveData.saveID[0]);
		saved_badges.set<s64>("sID1", SaveHandler::saveData.saveID[1]);
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
				auto& doc = *static_cast<minibson::document*>(it->second.get());
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
				auto& doc = *static_cast<minibson::document*>(it->second.get());
				if (!doc.empty()) {
					return doc;
				}
			}
		}
		
		// Otherwise return from cache
		if ((mode & GetBadgeMode::CACHED) != 0) {
			auto it = cached_badges.find(key);
			if (it != cached_badges.end() && it->second->get_node_code() == minibson::bson_node_type::document_node) {
				auto& doc = *static_cast<minibson::document*>(it->second.get());
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
			reqDoc.set("badges", MiscUtils::Buffer(badges.data() + start, (end-start) * sizeof(u64)));
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
							out_document.set(key, *static_cast<const minibson::document*>(it->second.get()));
						}
					}					
				}
			}
			reqHandler.Cleanup();
		}
		return ret;
    }
}