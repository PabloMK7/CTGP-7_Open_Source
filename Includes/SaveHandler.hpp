/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveHandler.hpp
Open source lines: 489/516 (94.77%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "string.h"
#include "main.hpp"
#include "entrystructs.hpp"
#include "VersusHandler.hpp"
#include "Minibson.hpp"
#include "SaveHandlerInfos.hpp"
#include "Net.hpp"
#include "ExtraUIElements.hpp"

namespace CTRPluginFramework {

	class SaveHandler {
	public:
		enum class Achievements {
			ALL_GOLD = (1 << 0),
			ALL_ONE_STAR = (1 << 1),
			ALL_THREE_STAR = (1 << 2),
			ALL_MISSION_TEN = (1 << 3),
			VR_5000 = (1 << 4),
			ALL_SCORE_COMPLETED = (1 << 5),
		};
		enum class SpecialAchievements {
			NONE = -1,
			ALL_BLUE_COINS = 0,
			DODGED_BLUE_SHELL = 1,
		};
		static constexpr u32 TOTAL_ACHIEVEMENTS = 6;
		static constexpr u32 TOTAL_SPECIAL_ACHIEVEMENTS = 2;
		static constexpr u32 BLUE_SHELL_DODGE_COUNT_ACHIEVEMENT = 3;
		class CupRankSave {
			public:
				struct GrandPrixData {
					bool isCompleted{};
					u32 starRank{}; // 4 -> 1 star; 5 -> 2 stars; 6 -> 3 stars
					u32 trophyType{}; // 1 -> bronze; 2 -> silver; 3 -> gold
				};
				static std::map<u32, u8> cupData; 
				static inline void fromPackedToGP(GrandPrixData* out, const u8 inPackedData)
				{
					out->isCompleted = inPackedData & 0x1;
					out->starRank = (inPackedData >> 1) & 7;
					out->trophyType = (inPackedData >> 4) & 3;
				}
				static inline void fromGPToPacked(u8* outPackedData, const GrandPrixData* in)
				{
					*outPackedData = in->isCompleted & 1;
					*outPackedData |= (in->starRank & 7) << 1;
					*outPackedData |= (in->trophyType & 3) << 4;
				}			
				static void getGrandPrixData(u32 saveData, GrandPrixData* out, u32* GPID, u32* engineLevel, bool isMirror);
				static void setGrandPrixData(u32 saveData, GrandPrixData* in, u32* GPID, u32* engineLevel, bool isMirror);

				static void Load();
				static void Save();
				static minibson::document SaveBackup();
				static bool RestoreBackup(const minibson::document& doc);

				enum class SatisfyCondition {
					COMPLETED,
					GOLD,
					ONE_STAR,
					THREE_STAR,
				};
				static bool CheckModAllSatisfy(SatisfyCondition condition);
				static std::pair<int, std::array<int, 4>> CheckModSatisfyProgress(SatisfyCondition condition);
			private:
				static minibson::document ToDocument();
				static void FromDocument(const minibson::document& doc);
		};
		struct CTGP7Save {
			CCSettings cc_settings;
			VersusHandler::CurrentSettings vsSettings;
			struct {
				u32 backCamEnabled : 1;
				u32 warnItemEnabled : 1;
				u32 firstOpening : 1;
				u32 localZoomMapEnabled : 1;
				u32 readyToFool : 1;
				u32 isCTWWActivated : 1;
				u32 isAlphabeticalEnabled : 1;
				u32 improvedRoulette : 1;
				u32 serverCommunication : 1;
				u32 improvedTricks : 1;
				u32 renderOptimization : 1;
				u32 autoacceleration : 1;
				u32 autoaccelerationUsesA : 1;
				u32 brakedrift : 1;
				u32 creditsWatched : 1;
				u32 automaticDelayDrift : 1;
				u32 useCTGP7Server : 1;
				u32 customKartsEnabled : 1;
				u32 blueCoinsEnabled : 1;
				u32 enableVoiceChat : 1;
				u32 needsBadgeObtainedMsg : 1;
				u32 improvedHonk : 1;
			} flags1;
			struct 
			{
				u8 enabled : 1;
				u8 unit : 2;
				u8 mode : 5;
			} speedometer;
			u8 numberOfRounds;
			u8 serverDisplayNameMode;
			char serverDisplayCustomName[0x10];
			u32 ctVR;
			u32 cdVR;
			u64 consoleID;

			bool newlyGeneratedID = false;
			u64 saveID[2] = {0};

			u32 pendingAchievements;
			u32 achievements;
			u32 pendingSpecialAchievements;
			u32 specialAchievements;
			int principalID;

			u64 driverChoices[EDriverID::DRIVER_SIZE];
			std::vector<u64> disabledChars;
			std::vector<u32> collectedBlueCoins;

			char voiceChatServerAddr[0x20];

			u64 useBadgeOnline;

			u32 blueShellDodgeAmount;

			bool IsAchievementPending(Achievements achiev) {
				return pendingAchievements & (u32)achiev;
			}

			bool IsAchievementCompleted(Achievements achiev) {
				#ifdef ALL_ACHIEVEMENTS
				return true;
				#else
				return achievements & (u32)achiev;
				#endif
			}

			void SetAchievementPending(Achievements achiev, bool set) {
				pendingAchievements = (pendingAchievements & ~(u32)achiev) | (set ? (u32)achiev : 0);
			}

			void SetAchievementCompleted(Achievements achiev, bool set) {
				achievements = (achievements & ~(u32)achiev) | (set ? (u32)achiev : 0);
				UpdateAchievementCryptoFiles();
			}

			u32 GetPendingAchievementCount() {
				return __builtin_popcount(pendingAchievements);
			}

			u32 GetCompletedAchievementCount() {
				#ifdef ALL_ACHIEVEMENTS
				return TOTAL_ACHIEVEMENTS;
				#else
				return __builtin_popcount(achievements);
				#endif
			}

			bool IsSpecialAchievementPending(SpecialAchievements achiev) {
				return pendingSpecialAchievements & (1 << (u32)achiev);
			}

			bool IsSpecialAchievementCompleted(SpecialAchievements achiev) {
				#ifdef ALL_ACHIEVEMENTS
				return true;
				#else
				return specialAchievements & (1 << (u32)achiev);
				#endif
			}

			void SetSpecialAchievementPending(SpecialAchievements achiev, bool set) {
				u32 ach = (1 << (u32)achiev);
				pendingSpecialAchievements = (pendingSpecialAchievements & ~(u32)ach) | (set ? (u32)ach : 0);
			}

			void SetSpecialAchievementCompleted(SpecialAchievements achiev, bool set) {
				u32 ach = (1 << (u32)achiev);
				specialAchievements = (specialAchievements & ~(u32)ach) | (set ? (u32)ach : 0);
				UpdateAchievementCryptoFiles();
			}

			u32 GetPendingSpecialAchievementCount() {
				return __builtin_popcount(pendingSpecialAchievements);
			}

			u32 GetCompletedSpecialAchievementCount() {
				#ifdef ALL_ACHIEVEMENTS
				return TOTAL_ACHIEVEMENTS;
				#else
				return __builtin_popcount(specialAchievements);
				#endif
			}

			void GenerateNewSaveID() {
			}

			CTGP7Save& operator=(const CTGP7Save& other) = default;
			CTGP7Save& operator=(CTGP7Save&& other) = default;

			CTGP7Save() {
				cc_settings = CCSettings();
				vsSettings = VersusHandler::CurrentSettings();
				speedometer.enabled = false;
				speedometer.mode = (u8)SpeedometerController::SpeedMode::NUMERIC;
				speedometer.unit = (u8)SpeedometerController::SpeedUnit::KMPH;
				flags1 = { 0 };
				flags1.firstOpening = true;
				flags1.readyToFool = true;
				flags1.isCTWWActivated = true;
				flags1.isAlphabeticalEnabled = true;
				flags1.improvedRoulette = true;
				flags1.serverCommunication = true;
				flags1.improvedTricks = true;
				flags1.renderOptimization = true;
				flags1.autoacceleration = false;
				flags1.autoaccelerationUsesA = false;
				flags1.brakedrift = true;
				flags1.creditsWatched = false;
				flags1.automaticDelayDrift = true;
				flags1.useCTGP7Server = true;
				flags1.customKartsEnabled = true;
				flags1.blueCoinsEnabled = true;
				flags1.enableVoiceChat = false;
				flags1.needsBadgeObtainedMsg = false;
				numberOfRounds = 4;
				serverDisplayNameMode = (u8)Net::PlayerNameMode::SHOW;
				serverDisplayCustomName[0] = '\0';
				consoleID = 0;
				pendingAchievements = 0;
				achievements = 0;
				pendingSpecialAchievements = 0;
				specialAchievements = 0;
				principalID = 0;
				memset(driverChoices, sizeof(driverChoices), 0);
				disabledChars.clear();
				collectedBlueCoins.clear();
				#if CITRA_MODE == 1
				strcpy(voiceChatServerAddr, "127.0.0.1");
				#else
				voiceChatServerAddr[0] = '\0';
				#endif
				useBadgeOnline = 0;
				blueShellDodgeAmount = 0;
			}

			CTGP7Save(const minibson::document& doc, bool generateIDIfMissing) {
				saveID[0] = doc.get<s64>("sID0", (s64)0);
				saveID[1] = doc.get<s64>("sID1", (s64)0);
				if ((saveID[0] == 0 || saveID[1] == 0) && generateIDIfMissing) {
					newlyGeneratedID = true;
					GenerateNewSaveID();
				} else {
					newlyGeneratedID = false;
				}

				cc_settings = CCSettings(doc);
				vsSettings = VersusHandler::CurrentSettings(doc);

				flags1.backCamEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BACKCAM_ENABLED), false);
				flags1.warnItemEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::WARNITEM_ENABLED), false);
				flags1.firstOpening = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::FIRST_OPENING), true);
				flags1.localZoomMapEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::LOCALZOOMMAP_ENABLED), false);
				flags1.readyToFool = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::READY_TO_FOOL), true);
				flags1.isCTWWActivated = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CTWW_ACTIVATED), true);
				flags1.isAlphabeticalEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::ALPHABETICAL_ENABLED), true);
				flags1.improvedRoulette = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVEDROULETTE_ENABLED), true);
				flags1.serverCommunication = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_COMMUNICATION), true);
				flags1.improvedTricks = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVED_TRICKS), true);
				speedometer.enabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_ENABLED), false);
				speedometer.mode = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_MODE), (int)0);
				speedometer.unit = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_UNIT), (int)0);
				numberOfRounds = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::NUMBER_OF_ROUNDS), 4);
				serverDisplayNameMode = (u8)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_NAME_MODE), (int)Net::PlayerNameMode::SHOW);
				std::string n = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_NAME_STR), "");
				strncpy(serverDisplayCustomName, n.c_str(), sizeof(serverDisplayCustomName) - 1);
				serverDisplayCustomName[sizeof(serverDisplayCustomName) - 1] = '\0';
				ctVR = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CUSTOM_TRACKS_VR), 1000);
				cdVR = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::COUNTDOWN_VR), 1000);
				flags1.renderOptimization = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::RENDER_OPTIMIZATION), true);
				flags1.autoacceleration = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTO_ACCELERATION), false);
				flags1.autoaccelerationUsesA = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTO_ACCELERATION_USESA), false);
				flags1.brakedrift = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BRAKE_DRIFT), true);
				flags1.creditsWatched = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CREDITS_WATCHED), false);
				flags1.automaticDelayDrift = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTOMATIC_DELAY_DRIFT), true);
				#if CITRA_MODE == 0
				flags1.useCTGP7Server = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::USE_CTGP7_SERVER), true);
				#else
				flags1.useCTGP7Server = true;
				#endif
				pendingAchievements = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PENDING_ACHIEVEMENTS), (int)0);
				achievements = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::ACHIEVEMENTS), (int)0);
				principalID = (int)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PRINCIPAL_ID), (int)0);
				#if CITRA_MODE == 1
				if (principalID < 100000000)
					principalID = 0;
				#endif
				const auto& char_handler_choices = doc.get_binary(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CHAR_HANDLER_DRIVER_CHOICES));
				if (!char_handler_choices.Empty()) {
					if (char_handler_choices.Size() == sizeof(driverChoices))
						memcpy(driverChoices, char_handler_choices.Data(), char_handler_choices.Size());
				} else memset(driverChoices, 0, sizeof(driverChoices));

				const auto& char_handler_disabled = doc.get_binary(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CHAR_HANDLER_DISABLED));
				if (!char_handler_disabled.Empty()) {
					if (char_handler_disabled.Size() && char_handler_disabled.Size() % sizeof(u64) == 0) {
						disabledChars.resize(char_handler_disabled.Size() / sizeof(u64));
						memcpy(disabledChars.data(), char_handler_disabled.Data(), char_handler_disabled.Size());
					}
				} else disabledChars.clear();

				const auto& collected_blue_coins = doc.get_binary(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::COLLECTED_BLUE_COINS));
				if (!collected_blue_coins.Empty()) {
					if (collected_blue_coins.Size() && collected_blue_coins.Size() % sizeof(u32) == 0) {
						collectedBlueCoins.resize(collected_blue_coins.Size() / sizeof(u32));
						memcpy(collectedBlueCoins.data(), collected_blue_coins.Data(), collected_blue_coins.Size());
					}
				} else collectedBlueCoins.clear();
				pendingSpecialAchievements = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PENDING_SPECIAL_ACHIEVEMENTS), (int)0);
				specialAchievements = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPECIAL_ACHIEVEMENTS), (int)0);
				flags1.customKartsEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CUSTOM_KARTS_ENABLED), true);
				flags1.blueCoinsEnabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BLUE_COINS_ENABLED), true);
				flags1.enableVoiceChat = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VOICE_CHAT_ENABLED), false);
				n = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VOICE_CHAT_SERVER), 
				#if CITRA_MODE == 1
				"127.0.0.1"
				#else
				""
				#endif
				);
				strncpy(voiceChatServerAddr, n.c_str(), sizeof(voiceChatServerAddr) - 1);
				voiceChatServerAddr[sizeof(voiceChatServerAddr) - 1] = '\0';
				useBadgeOnline = (u64)doc.get<s64>(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::USE_BADGE_ONLINE), 0);
				flags1.needsBadgeObtainedMsg = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::NEEDS_BADGE_OBTAINED_MSG), false);
				blueShellDodgeAmount = (u32)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BLUE_SHELL_DODGE_AMOUNT), (int)0);
				flags1.improvedHonk= doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVED_HONK), true);
			}
			
			void serialize(minibson::document& doc) {
				cc_settings.serialize(doc);
				vsSettings.serialize(doc);

				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BACKCAM_ENABLED), (bool)flags1.backCamEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::WARNITEM_ENABLED), (bool)flags1.warnItemEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::FIRST_OPENING), (bool)flags1.firstOpening);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::LOCALZOOMMAP_ENABLED), (bool)flags1.localZoomMapEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::READY_TO_FOOL), (bool)flags1.readyToFool);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CTWW_ACTIVATED), (bool)flags1.isCTWWActivated);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::ALPHABETICAL_ENABLED), (bool)flags1.isAlphabeticalEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVEDROULETTE_ENABLED), (bool)flags1.improvedRoulette);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_COMMUNICATION), (bool)flags1.serverCommunication);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVED_TRICKS), (bool)flags1.improvedTricks);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_ENABLED), (bool)speedometer.enabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_MODE), (int)speedometer.mode);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPEED_UNIT), (int)speedometer.unit);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::NUMBER_OF_ROUNDS), (int)numberOfRounds);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_NAME_MODE), (int)serverDisplayNameMode);
				serverDisplayCustomName[sizeof(serverDisplayCustomName) - 1] = '\0';
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SERVER_NAME_STR), (const char*)serverDisplayCustomName);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CUSTOM_TRACKS_VR), (int)ctVR);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::COUNTDOWN_VR), (int)cdVR);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::RENDER_OPTIMIZATION), (bool)flags1.renderOptimization);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTO_ACCELERATION), (bool)flags1.autoacceleration);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTO_ACCELERATION_USESA), (bool)flags1.autoaccelerationUsesA);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BRAKE_DRIFT), (bool)flags1.brakedrift);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CREDITS_WATCHED), (bool)flags1.creditsWatched);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::AUTOMATIC_DELAY_DRIFT), (bool)flags1.automaticDelayDrift);
				#if CITRA_MODE == 0
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::USE_CTGP7_SERVER), (bool)flags1.useCTGP7Server);
				#endif
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PENDING_ACHIEVEMENTS), (int)pendingAchievements);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::ACHIEVEMENTS), (int)achievements);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PRINCIPAL_ID), (int)principalID);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CHAR_HANDLER_DRIVER_CHOICES), MiscUtils::Buffer(driverChoices, sizeof(driverChoices)));
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CHAR_HANDLER_DISABLED), MiscUtils::Buffer(disabledChars.data(), disabledChars.size() * sizeof(u64)));
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::COLLECTED_BLUE_COINS), MiscUtils::Buffer(collectedBlueCoins.data(), collectedBlueCoins.size() * sizeof(u32)));
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::PENDING_SPECIAL_ACHIEVEMENTS), (int)pendingSpecialAchievements);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::SPECIAL_ACHIEVEMENTS), (int)specialAchievements);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CUSTOM_KARTS_ENABLED), (bool)flags1.customKartsEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BLUE_COINS_ENABLED), (bool)flags1.blueCoinsEnabled);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VOICE_CHAT_ENABLED), (bool)flags1.enableVoiceChat);
				voiceChatServerAddr[sizeof(voiceChatServerAddr) - 1] = '\0';
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::VOICE_CHAT_SERVER), (const char*)voiceChatServerAddr);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::USE_BADGE_ONLINE), (s64)useBadgeOnline);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::NEEDS_BADGE_OBTAINED_MSG), (bool)flags1.needsBadgeObtainedMsg);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::BLUE_SHELL_DODGE_AMOUNT), (int)blueShellDodgeAmount);
				doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::IMPROVED_HONK), (bool)flags1.improvedHonk);
			}
		};
		static bool disableSaving;
		static CTGP7Save saveData;
		static void LoadSettings();
		static void SaveOptions();
		static void ApplySettings();
		static void DefaultSettings();
		static void UpdateAchievementCryptoFiles();
		static void UpdateAchievementsConditions();
		static void UpdateCharacterIterator();
		static bool CheckAndShowAchievementMessages();
		static bool CheckAndShowServerCommunicationDisabled();

		static Mutex saveSettingsMutex;
		
		static void SaveSettingsAll(bool async = true) {
			if (async) {
				saveSettinsTask.Wait();
				saveSettinsTask.Start();
			} else {
				SaveSettingsTaskFunc(nullptr);	
			}			
		}
		static void WaitSaveSettingsAll() {
			saveSettinsTask.Wait();
		}

		class SaveFile
		{
		public:
			enum class LoadStatus {
				SUCCESS = 0,
				FILE_NOT_FOUND = 1,
				MAGIC_MISMATCH = 2,
				TYPE_MISMATCH = 3,
				CORRUPTED_FILE = 4,
				INCORRECT_CID = 5,
				INCORRECT_SID = 6,
			};

			enum class SaveStatus {
				SUCCESS = 0,
				BACKUP_FAILED = 1,
				OPEN_DST_FILE = 2,
				OPEN_DST_FILE_TRUNC = 3,
				SERIALIZE_DOCUMENT = 4,
				WRITE_FILE = 5,
			};

			enum class SaveType : u32 {
				OPTIONS = 0,
				STATS = 1,
				RACES = 2,
				MISSION = 3,
				BADGES = 4,
				POINT = 5,

				MAX_TYPE
			};

			struct CTGP7SaveFile
			{
				u32 magic;
				SaveType type;
				u8 bsondata[];
			};

			static minibson::document Load(SaveType type, LoadStatus& status);
			static SaveStatus Save(SaveType type, const minibson::document& inData);

			static void HandleLoadError(SaveType type, LoadStatus status);
			static void HandleSaveError(SaveType type, SaveStatus status);
		private:
			static constexpr u32 SaveMagic = 0x56533743;
			static const char* SaveNames[(u32)SaveType::MAX_TYPE];
		};
	private:	

		static Task saveSettinsTask;
		static s32 SaveSettingsTaskFunc(void* args);

		static minibson::document SaveSettingsBackup(void);
		static bool RestoreSettingsBackup(const minibson::document& doc);

		static int lastAchievementCount;
		static u32 lastSpecialAchievements;
	};
}