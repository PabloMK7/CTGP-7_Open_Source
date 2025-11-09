/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BadgeManager.hpp
Open source lines: 72/72 (100.00%)
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
    class BadgeManager {
        public:
            
            class Badge {
            public:
                u64 bID;
                u64 epoch;
                std::string name;
                std::string desc;
                BCLIM icon;
            };

            enum GetBadgeMode {
                SAVED = (1 << 0),
                CACHED = (1 << 1),
                BOTH = SAVED | CACHED,
            };

            static void Initialize();

            static std::vector<u64> GetSavedBadgeList();
            static Badge GetBadge(u64 badgeID, GetBadgeMode mode);

            static void ProcessMyBadges(const std::vector<u64>& badges, const std::vector<u64>& times);

            static void BadgesMenuKbdEvent(Keyboard& kbd, KeyboardEvent &event);
            static void BadgesMenu(MenuEntry* entry);

            static bool CheckAndShowBadgePending();

            static void CommitToFile();
            
            static void UpdateOnlineBadges();
            static s32 CacheBadgesFunc(void* args);
            static void CacheBadges(const std::vector<u64>& badges);
            static void ClearBadgeCache();

            static void SetupExtraResource();

            static minibson::document saved_badges;
            static minibson::document cached_badges;
            static Mutex badges_mutex;

            static u64 cache_badge_time;
            static std::vector<u64> badges_to_cache;
            static Task cacheBadgesTask;
            static BCLIM empty_badge;
            static std::array<BCLIM, 8> badge_slots;
        private:
            static minibson::document& GetBadgeDocument(u64 badgeID, GetBadgeMode mode);
            static std::vector<u64> RequestBadges(minibson::document& out_document, const std::vector<u64> badges);
        };
}