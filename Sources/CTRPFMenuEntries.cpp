/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CTRPFMenuEntries.cpp
Open source lines: 692/700 (98.86%)
*****************************************************/

#include "types.h"
#include "main.hpp"
#include "3ds.h"
#include "rt.hpp"
#include "string.h"
#include "OnionFS.hpp"
#include "Lang.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "LED_Control.hpp"
#include "math.h"
#include "Unicode.h"
#include "entrystructs.hpp"
#include "SaveHandler.hpp"
#include "foolsday.hpp"
#include "Sound.hpp"
#include "ExtraResource.hpp"
#include "VersusHandler.hpp"
#include "MissionHandler.hpp"
#include "NetHandler.hpp"
#include "Net.hpp"
#include "mallocDebug.hpp"
#include "ExtraUIElements.hpp"
#include "BlueCoinChallenge.hpp"

u32 g_currMenuVal = 0;
u8 g_isOnlineMode = (CTRPluginFramework::Utils::Random() | 2) & ~0x1;



namespace CTRPluginFramework
{
	CCSettings ccsettings[2] = {CCSettings(), CCSettings()};
	//const SpeedValues g_SpdValCostants[2] = { {"km/h", 10.f, 130.f, 2.84872641f}, {"mph", 6.21371f, 80.f, 1.75306240615f} };
	extern RT_HOOK socinithook;
	extern RT_HOOK socexithook;

	#ifdef INSTRUMENT_FUNCTIONS
	void save_instrumentation();
	void start_instrumentation();
	#endif
	
	void	menucallback() {
		#ifdef INSTRUMENT_FUNCTIONS
		if (Controller::IsKeyPressed(Key::DPadLeft))
			save_instrumentation();
		if (Controller::IsKeyPressed(Key::DPadRight))
			start_instrumentation();
		#endif
		#ifdef SETUP_BLUE_COINS
		BlueCoinChallenge::SetupBlueCoinsCallback();
		#endif
		if (MarioKartFramework::isGameInRace()) {
			;
		} else {
			;
		}
	}

	void warnItemUse_apply(bool enabled) {
		warnitementry->Name() = NAME("itemled") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.warnItemEnabled = enabled;
	}

	void warnItemUse(MenuEntry *entry) {
		Keyboard key(NAME("itemled"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		warnItemUse_apply(ret == 0);
	}

   	bool speedometer_value() {
   		Keyboard        keyboard(NAME("spd_sunit"));
        std::vector<std::string>    unitsList = {"km/h", "mph"};
		int ret, ret2;

        // First part
        keyboard.Populate(unitsList);
		keyboard.ChangeSelectedEntry(SaveHandler::saveData.speedometer.unit);
		ret = keyboard.Open();
		if (ret < 0) return false;

		// Second part
		keyboard.GetMessage() = NAME("spd_stype");
		unitsList = {NAME("spd_types1"), NOTE("spd_types1"), NAME("spd_types2")};
		keyboard.Populate(unitsList, true);
		keyboard.ChangeSelectedEntry(SaveHandler::saveData.speedometer.mode);
		ret2 = keyboard.Open();
		if (ret2 < 0) return false;

		SaveHandler::saveData.speedometer.unit = (u8)ret;
		SaveHandler::saveData.speedometer.mode = (u8)ret2;
		return true;
   	}

   	void	speedometer_apply() {
		const char* unitsList[] = {"km/h", "mph"};
		if (SaveHandler::saveData.speedometer.enabled) {
			speedometerentry->Name() = NAME("spdmeter") + " (" + unitsList[SaveHandler::saveData.speedometer.unit] + ")";
		} else {
			speedometerentry->Name() = NAME("spdmeter") + " (" + NOTE("state_mode") + ")";
		}
	}

    void    speedometersettings(MenuEntry *entry) 
    {
        // If no settings, create struct
        using StringVector = std::vector<std::string>;
		const char* unitsList[] = {"km/h", "mph"};
		StringVector modesList = {NAME("spd_types1"), NOTE("spd_types1"), NAME("spd_types2")};

        bool haschanged = false;

		Keyboard        keyboard("dummy");

		bool loop = true;
		while (loop) {

			keyboard.Populate({ ((SaveHandler::saveData.speedometer.enabled) ? NOTE("state_inf") : NAME("state_inf")), NAME("settings"),NAME("exit") });
			keyboard.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
			keyboard.GetMessage() = NAME("spd_set") << "\n\n" << NAME("state") << ": "
				<< ((SaveHandler::saveData.speedometer.enabled) ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() << "\n" << NAME("spd_unit") << ": "
				<< unitsList[SaveHandler::saveData.speedometer.unit] << "\n" << NAME("spd_type") << ": " << Color::White << modesList[SaveHandler::saveData.speedometer.mode];
			int userchoice = keyboard.Open();
			switch (userchoice) {
			case 0:
				SaveHandler::saveData.speedometer.enabled = !SaveHandler::saveData.speedometer.enabled;
				haschanged = true;
				break;
			case 1:
				haschanged = speedometer_value() && SaveHandler::saveData.speedometer.enabled;
				break;
			default:
				loop = false;
				break;
			}
		}
        if (haschanged) {
    		speedometer_apply();
        }
		// Update name & enable
    }

	bool    ccselectorcompare(const void *input, std::string &error)
    {
        // Cast the input into the appropriate type (must match the type provided to Open)
        u32  in = *static_cast<const u32 *>(input);

        // Check the value
        if ((in > 9999) || (in < 1))
        {
            error = Utils::Format(NAME("num_betw").c_str(), 1, 9999);
            // Return that the value isn't valid
			return (false);
        }

        // The value is valid
        return (true);
    }

    bool ccselector_value(float& currvalue) {
    	Keyboard        keyboard(NAME("cc_entv"));
        // Disable hex mode
        keyboard.IsHexadecimal(false);
		// Add compare callback
		keyboard.SetCompareCallback(ccselectorcompare);

		int ret;
		u32 val = currvalue;

        // Open the keyboard and wait for a user input
        if (currvalue)
            ret = keyboard.Open(val, val);
        else
            ret = keyboard.Open(val);

		if (ret < 0) return false;

		currvalue = val;

        return true;
    }

	void    ccselector_apply(MenuEntry* entry) {
		CCSettings   *settings = static_cast<CCSettings *>(entry->GetArg());
		if (settings->enabled && Language::MsbtHandler::common) {
			std::string cc_name = std::to_string((int)settings->value) + "cc";
			for (int i = 1812; i >= 1810; i--) Language::MsbtHandler::SetString(i, cc_name);
		} else if (Language::MsbtHandler::common) {
			for (int i = 1812; i >= 1810; i--) Language::MsbtHandler::SetTextEnabled(i, false);
		}
		ccSelOnlineEntry->updateName();
    }

	void    ccselectorsettings(MenuEntry *entry)
    {
        // If no settings, create struct
        bool haschanged = false;

        using StringVector = std::vector<std::string>;
        CCSettings      *settings = static_cast<CCSettings *>(entry->GetArg());
		float tempVal;

		Keyboard        keyboard("");

		// Force top screen.
		keyboard.DisplayTopScreen = true;
        
		bool loop = true;
		while (loop) {
			keyboard.Populate({ ((settings->enabled) ? NOTE("state_inf") : NAME("state_inf")),NAME("cc_chgv"),NAME("exit") });
			keyboard.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
			keyboard.GetMessage() = NAME("cc_set") << "\n\n" << NAME("state") << ": "
				<< ((settings->enabled) ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() << "\n" << NAME("cc_val") << ": "
				<< Color::White << std::to_string((int)settings->value) << "cc";
			int userchoice = keyboard.Open();
			bool valueSet;
			switch (userchoice) {
			case 0:
				settings->enabled = !settings->enabled;
				haschanged = true;
				break;
			case 1:
				tempVal = settings->value;
				valueSet = ccselector_value(tempVal);
				if (valueSet) settings->value = tempVal;
				haschanged = valueSet && settings->enabled;
				break;
			default:
				loop = false;
				break;
			}
		}
        if (haschanged) {
        	ccselector_apply(entry);
        }
    }

	void backwardscam_apply(bool active) {
		SaveHandler::saveData.flags1.backCamEnabled = active;
		backcamentry->Name() = NAME("backcam") << " (" << (active ? NAME("state_mode") : NOTE("state_mode")) << ")";
	}

	void backwardscam(MenuEntry* entry)
	{
		Keyboard key(NAME("backcam") + "\n\n" + NAME("state") + ": " + (SaveHandler::saveData.flags1.backCamEnabled ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) + ResetColor());
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		backwardscam_apply(ret == 0);
	}

	void changeRoundNumber(MenuEntry *entry) {
		Keyboard kbd(NAME("chgrnd_desc"));
		kbd.IsHexadecimal(false);
		kbd.SetCompareCallback([](const void* val, std::string& error) {
			const u32* nval = (u32*)val;
			if (*nval < 1 || *nval > 32) {
				error = Utils::Format(NAME("num_betw").c_str(), 1, 32);
				return false;
			}
			return true;
		});
		u32 numbRound = SaveHandler::saveData.numberOfRounds;
		int ret = kbd.Open(numbRound, numbRound);
		if (ret >= 0) MarioKartFramework::changeNumberRounds(numbRound);
	}

	void courseOrder_apply(bool isAlphabetical) {
		courseOrderEntry->Name() = NAME("trackorder") << " (" << (isAlphabetical ? NOTE("order_mode") : NAME("order_mode")) << ")";
		SaveHandler::saveData.flags1.isAlphabeticalEnabled = isAlphabetical;
	}

	void courseOrder(MenuEntry* entry)
	{
		Keyboard key(NAME("trackorder"));
		key.Populate({ NAME("order_mode"), NOTE("order_mode") });
		int ret = key.Open();
		if (ret < 0) return;
		courseOrder_apply(ret == 1);
	}
	
	extern s8 g_system3DState;
	void renderImprovements_apply(bool isRenderOptimization) {
		if (renderImproveEntry) renderImproveEntry->Name() = NAME("render_optim") << " (" << (isRenderOptimization ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.renderOptimization = isRenderOptimization;
		MarioKartFramework::is3DEnabled = !isRenderOptimization;
		if (g_system3DState < 0)
			g_system3DState = *(u8*)0x1FF81084;
		*(u8*)0x1FF81084 = MarioKartFramework::is3DEnabled ? g_system3DState : 1;
		if (System::IsNew3DS()) {
			#if CITRA_MODE == 0
			svcKernelSetState(10, isRenderOptimization ? (1 << 2) : (1 << 3)); // Enable or disable speed lock
			svcKernelSetState(10, 0); // Actually change the CPU speed.
			#endif
		}
	}

	void renderImprovements(MenuEntry* entry)
	{
		Keyboard key(NAME("render_optim"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		renderImprovements_apply(ret == 0);
	}

	void improvedTricks_apply(bool enabled) {
		improvedTricksEntry->Name() = NAME("imtrick") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.improvedTricks = enabled;
	}

	void improvedTricks(MenuEntry* entry) {
		Keyboard key(NAME("imtrick"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		improvedTricks_apply(ret == 0);
	}

	void autoAccel_apply(bool enabled) {
		autoAccelEntry->Name() = NAME("autoaccel") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.autoacceleration = enabled;
	}

	bool autoAccel_button() {
   		Keyboard        keyboard(NAME("autoaccel_button"));
        std::vector<std::string>    buttonList = {FONT_Y, FONT_A};
		int ret;

        // First part
        keyboard.Populate(buttonList);
		keyboard.ChangeSelectedEntry(SaveHandler::saveData.flags1.autoaccelerationUsesA);
		ret = keyboard.Open();
		if (ret < 0) return false;

		SaveHandler::saveData.flags1.autoaccelerationUsesA = (u8)ret;
		return true;
   	}

	void autoAccelSetting(MenuEntry* entry) {
		Keyboard        keyboard("dummy");
		bool loop = true;
		while (loop) {

			keyboard.Populate({ ((SaveHandler::saveData.flags1.autoacceleration) ? NOTE("state_inf") : NAME("state_inf")), NAME("settings"),NAME("exit") });
			keyboard.ChangeEntrySound(2, SoundEngine::Event::CANCEL);
			keyboard.GetMessage() = NAME("autoaccel") << "\n\n" << NAME("state") << ": "
				<< ((SaveHandler::saveData.flags1.autoacceleration) ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() << "\n" << NAME("button") << ": "
				<< ((SaveHandler::saveData.flags1.autoaccelerationUsesA) ? FONT_A : FONT_Y);
			int userchoice = keyboard.Open();
			switch (userchoice) {
			case 0:
				autoAccel_apply(!SaveHandler::saveData.flags1.autoacceleration);
				break;
			case 1:
				autoAccel_button();
				break;
			default:
				loop = false;
				break;
			}
		}
	}

	void brakedrift_apply(bool enabled) {
		brakeDriftEntry->Name() = NAME("imbrakedrift") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.brakedrift = enabled;
	}

	void brakeDrift(MenuEntry* entry) {
		Keyboard key(NAME("imbrakedrift"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		brakedrift_apply(ret == 0);
	}

	void automaticdelaydrift_apply(bool enabled) {
		automaticDelayDriftEntry->Name() = NAME("autodelaydrift") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.automaticDelayDrift = enabled;
	}

	void automaticdelaydrift_entryfunc(MenuEntry* entry) {
		Keyboard key(NAME("autodelaydrift"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		automaticdelaydrift_apply(ret == 0);
	}

	void bluecoin_apply(bool enabled) {
		blueCoinsEntry->Name() = NAME("blue_coins") << " (" << (enabled ? NAME("state_mode") : NOTE("state_mode")) << ")";
		SaveHandler::saveData.flags1.blueCoinsEnabled = enabled;
	}

	void bluecoin_entryfunc(MenuEntry* entry) {
		Keyboard key(NAME("blue_coins"));
		key.Populate({ NAME("state_inf"), NOTE("state_inf") });
		key.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
		int ret = key.Open();
		if (ret < 0) return;
		bluecoin_apply(ret == 0);
	}

	static void serverChangeDisplayName(const std::string& miiName) {
		Keyboard kbd(NOTE("serv_displname"));
		kbd.Populate({ NAME("serv_disphid"), NAME("serv_dispmii"), NAME("custom") });
		int opt = kbd.Open();
		if (opt == 0) {
			SaveHandler::saveData.serverDisplayNameMode = (u8)Net::PlayerNameMode::HIDDEN;
		}
		else if (opt == 1) {
			SaveHandler::saveData.serverDisplayNameMode = (u8)Net::PlayerNameMode::SHOW;
		}
		else if (opt == 2) {
			Keyboard nameKbd(NAME("serv_dispcust"));
			nameKbd.SetCompareCallback([](const void* input, std::string& error) {
				const std::string* in = static_cast<const std::string*>(input);
				if (in->size() > 15) {
					error = NOTE("serv_dispcust");
					return false;
				}
				else if (in->empty()) {
					error = NAME("serv_dispcust");
					return false;
				}
				return true;
			});
			std::string name;
			std::string defVal = std::string(SaveHandler::saveData.serverDisplayCustomName);
			if (defVal.empty())
				defVal = miiName;
			int ret = nameKbd.Open(name, defVal);
			if (ret >= 0) {
				strncpy(SaveHandler::saveData.serverDisplayCustomName, name.c_str(), sizeof(SaveHandler::saveData.serverDisplayCustomName) - 1);
				SaveHandler::saveData.serverDisplayNameMode = (u8)Net::PlayerNameMode::CUSTOM;
			}
		}
	} 

	void serverEntryHandler(MenuEntry* entry)
	{
		Keyboard kbd("dummy");
		std::string topStr;
		MarioKartFramework::SavePlayerData sv;
		MarioKartFramework::getMyPlayerData(&sv);
		std::string miiName = sv.miiData.GetName();
		int opt = -1;
		std::string enSlid = Color::LimeGreen << "\u2282\u25CF";
		std::string disSlid = Color::Red << "\u25CF\u2283";
		do {
			topStr = NAME("servsett") + "\n\nVR:\n";
			if (Net::vrPositions[0] > 0)
				topStr += "  " + NAME("ctww") + ": " + std::to_string(SaveHandler::saveData.ctVR) + " " + NAME("vr") +" (" + Language::GenerateOrdinal(Net::vrPositions[0]) + ")\n";
			else
				topStr += "  " + NAME("ctww") + ": " + std::to_string(SaveHandler::saveData.ctVR) + " " + NAME("vr") +"\n";
			if (Net::vrPositions[1] > 0)
				topStr += "  " + NAME("cntdwn") + ": " + std::to_string(SaveHandler::saveData.cdVR) + " " + NAME("vr") + " (" + Language::GenerateOrdinal(Net::vrPositions[1]) + ")\n";
			else
				topStr += "  " + NAME("cntdwn") + ": " + std::to_string(SaveHandler::saveData.cdVR) + " " + NAME("vr") + "\n";
			topStr += NAME("serv_consID") + ":\n  " + Utils::Format("0x%016llX", NetHandler::GetConsoleUniqueHash()) + "\n\n";
			topStr += "1. " + NAME("serv_statsupl") + ":\n  " + ((SaveHandler::saveData.flags1.uploadStats) ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			topStr += "2. " + NAME("serv_displname") + ":\n  ";
			switch (SaveHandler::saveData.serverDisplayNameMode)
			{
			case (u8)Net::PlayerNameMode::HIDDEN:
				topStr += "Player";
				break;
			case (u8)Net::PlayerNameMode::SHOW:
				topStr += miiName;
				break;
			case (u8)Net::PlayerNameMode::CUSTOM:
				topStr += std::string(SaveHandler::saveData.serverDisplayCustomName) + " [" + miiName + "]";
				break;
			default:
				break;
			}
			kbd.GetMessage() = topStr;
			kbd.Populate({ "1. " + ((SaveHandler::saveData.flags1.uploadStats) ? enSlid : disSlid), "2. " + NAME("chng"),
			#if CITRA_MODE == 0
			 	NAME("disc_info"),
			#else
				Color::Gray << NAME("disc_info") << ResetColor(),
			#endif
			NAME("exit") });
			kbd.ChangeEntrySound(0, (SaveHandler::saveData.flags1.uploadStats) ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			kbd.ChangeEntrySound(3, SoundEngine::Event::CANCEL);
			opt = kbd.Open();
			switch (opt)
			{
			case 0:
				SaveHandler::saveData.flags1.uploadStats = !SaveHandler::saveData.flags1.uploadStats;
				break;
			case 1:
				serverChangeDisplayName(miiName);
				break;
			case 2:
				#if CITRA_MODE == 0
				Net::DiscordLinkMenu();
				#endif
				break;
			default:
				opt = -1;
				break;
			}
		} while (opt >= 0);
	}

	void useCTGP7server_apply(bool useCTGP7) {
		Language::MsbtHandler::SetTextEnabled(6001, useCTGP7);
		Language::MsbtHandler::SetTextEnabled(6002, useCTGP7);
		Language::MsbtHandler::SetTextEnabled(6334, useCTGP7);
	}

	static int g_keyboardkey = -1;
	void achievementsEntryHandler(MenuEntry* entry) {
		bool wasMissionMode = MissionHandler::isMissionMode;
		MissionHandler::isMissionMode = false;
		constexpr int normalPages = 3;
		auto generateAchievementsText = [](int page, int totalMenu) -> std::string {
			auto genTextAchv = [](const std::string& text, bool completed) -> std::string {
				std::string ret = "- ";
				if (completed)
					ret += Color::Lime;
				else
					ret += Color::Gray;
				ret += text;
				ret += ResetColor() + "\n";
				return ret;
			};
			auto genProgressString = [](int curr, int total) {
				std::string s;
				if (curr >= total)
					s += Color::Lime;
				s += Utils::Format("%02d/%02d", curr, total);
				s += ResetColor();
				return s;
			};
			std::string topStr = CenterAlign(NAME("achieventry") + "\n");
			std::string fmtStr = std::string(FONT_L " ") + NAME("page") + " (%02d/%02d) " FONT_R;
			topStr += ToggleDrawMode(Render::UNDERLINE) + " " + CenterAlign(Utils::Format(fmtStr.c_str(), page + 1, totalMenu)) + RightAlign(" ", 30, 365) + ToggleDrawMode(Render::UNDERLINE);
			if (page == 0) {
				topStr += "\n" + CenterAlign( ToggleDrawMode(Render::UNDERLINE) + NAME("summary") + ToggleDrawMode(Render::UNDERLINE)) + "\n";
				topStr += Utils::Format((NAME("types_achievs") + ": %d/%d").c_str(), SaveHandler::saveData.GetCompletedAchievementCount(), SaveHandler::TOTAL_ACHIEVEMENTS) + "\n";
				topStr += genTextAchv(NAME("gold_tro_achiev"), SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_GOLD));
				topStr += genTextAchv(NAME("1star_achiev"), SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_ONE_STAR));
				topStr += genTextAchv(NAME("3star_achiev"), SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_THREE_STAR));
				topStr += genTextAchv(NAME("10pts_msn_achiev"), SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::ALL_MISSION_TEN));
				topStr += genTextAchv(NAME("5000vr_achiev"), SaveHandler::saveData.IsAchievementCompleted(SaveHandler::Achievements::VR_5000));
				topStr += std::string("\n") + NOTE("types_achievs") + ":" + "\n";
				topStr += genTextAchv(NAME("blue_coin_achiev"), SaveHandler::saveData.IsSpecialAchievementCompleted(SaveHandler::SpecialAchievements::ALL_BLUE_COINS));
			} else if (page == 1) {
				topStr += "\n" + CenterAlign(ToggleDrawMode(Render::UNDERLINE) + NAME("custom_track_cups") + ToggleDrawMode(Render::UNDERLINE)) + "\n\n";
				auto progressGold = SaveHandler::CupRankSave::CheckModSatisfyProgress(SaveHandler::CupRankSave::SatisfyCondition::GOLD);
				auto progress1Star = SaveHandler::CupRankSave::CheckModSatisfyProgress(SaveHandler::CupRankSave::SatisfyCondition::ONE_STAR);
				auto progress3Star = SaveHandler::CupRankSave::CheckModSatisfyProgress(SaveHandler::CupRankSave::SatisfyCondition::THREE_STAR);
				auto anyProgress = [](std::pair<int, std::array<int, 4>> &prog) -> bool {
					for (int i = 0; i < 4; i++)
						if (prog.second[i] >= prog.first)
							return true;
					return false;
				};
				int tot = progressGold.first;
				std::string cc50 = Language::MsbtHandler::GetString(2220);
				std::string cc100 = Language::MsbtHandler::GetString(2221);
				std::string cc150 = Language::MsbtHandler::GetString(2222);
				std::string ccMirror = Language::MsbtHandler::GetString(2223);
				std::string gold = "- " + (anyProgress(progressGold) ? (std::string() << Color::Lime) : "") + NAME("tro_gold") + ResetColor() + ":";
				std::string star1 = "- " + (anyProgress(progress1Star) ? (std::string() << Color::Lime) : "") + NAME("tro_1star") + ResetColor() + ":";
				std::string star3 = "- " + (anyProgress(progress3Star) ? (std::string() << Color::Lime) : "") + NAME("tro_3star") + ResetColor() + ":";
				topStr += gold + SkipToPixel(120) + cc50 + ": " + genProgressString(progressGold.second[0], tot) + ", " + cc100 + ": " + genProgressString(progressGold.second[1], tot) + "\n" +
							SkipToPixel(120) + cc150 + ": " + genProgressString(progressGold.second[2], tot) + ", " + ccMirror + ": " + genProgressString(progressGold.second[3], tot);
				topStr += HorizontalSeparator();
				topStr += star1 + SkipToPixel(120) + cc50 + ": " + genProgressString(progress1Star.second[0], tot) + ", " + cc100 + ": " + genProgressString(progress1Star.second[1], tot) + "\n" +
							SkipToPixel(120) + cc150 + ": " + genProgressString(progress1Star.second[2], tot) + ", " + ccMirror + ": " + genProgressString(progress1Star.second[3], tot);
				topStr += HorizontalSeparator();
				topStr += star3 + SkipToPixel(120) + cc50 + ": " + genProgressString(progress3Star.second[0], tot) + ", " + cc100 + ": " + genProgressString(progress3Star.second[1], tot) + "\n" +
							SkipToPixel(120) + cc150 + ": " + genProgressString(progress3Star.second[2], tot) + ", " + ccMirror + ": " + genProgressString(progress3Star.second[3], tot);
				
			} else if (page == 2) {
				topStr += "\n" + CenterAlign(ToggleDrawMode(Render::UNDERLINE) + NAME("ms_miss") + ToggleDrawMode(Render::UNDERLINE)) + "\n\n";
				auto prog = MissionHandler::SaveData::GetAllFullGradeFlag();
				topStr += ((prog.second >= prog.first) ? (std::string() << Color::Lime) : "") + NAME("tro_10pts") + ResetColor() + ":";
				topStr += SkipToPixel(120) + genProgressString(prog.second, prog.first);
				topStr += HorizontalSeparator();
				topStr += CenterAlign(ToggleDrawMode(Render::UNDERLINE) + NAME("ctww") + ToggleDrawMode(Render::UNDERLINE)) + "\n\n";
				int ctwwVR = SaveHandler::saveData.ctVR;
				int cdVR = SaveHandler::saveData.cdVR;
				int maxVR = std::max(ctwwVR, cdVR);
				int totalVR = 5000;
				topStr += ((maxVR >= totalVR) ? (std::string() << Color::Lime) : "") + NAME("vr") + ResetColor() + ":";
				topStr += SkipToPixel(120) + NAME("ctww") + ": " + genProgressString(ctwwVR, totalVR) + "\n" +
							SkipToPixel(120) + NAME("cntdwn") + ": " + genProgressString(cdVR, totalVR);
			} else if (page >= normalPages) {
				topStr += "\n" + CenterAlign(ToggleDrawMode(Render::UNDERLINE) + NAME("blue_coins") + ToggleDrawMode(Render::UNDERLINE)) + "\n\n";
				u32 curr = BlueCoinChallenge::GetCollectedCoinCount();
				u32 tot = BlueCoinChallenge::GetTotalCoinCount();
				topStr += ((curr >= tot) ? (std::string() << Color::Lime) : "") + NAME("tro_blue_collected") + ResetColor() + ":";
				topStr += SkipToPixel(120) + genProgressString(curr, tot);
				topStr += HorizontalSeparator();
				u32 currCup = page - normalPages;
				u32 size;
				const u32* cupTransTable = CourseManager::getCupTranslatetable(&size, true);
				if ((currCup & 1) == 0) {
					currCup = cupTransTable[currCup / 2];
				} else {
					currCup = cupTransTable[currCup / 2 + size / 2];
				}
				u32 courseIDs[4];
				bool obtained[4];
				for (int i = 0; i < 4; i++) {
					CourseManager::getGPCourseID(&courseIDs[i], currCup, i, true);
					obtained[i] = BlueCoinChallenge::IsCoinCollected(courseIDs[i]);
				}
				bool allObtainer = obtained[0] && obtained[1] && obtained[2] && obtained[3];
				topStr += (allObtainer ? (std::string() << Color::Blue) : (std::string() << Color::Gray));
				CourseManager::getCupText(topStr, currCup);
				topStr += ResetColor() + "\n";
				for (int i = 0; i < 4; i++) {
					topStr += "    ";
					topStr += (obtained[i] ? (std::string() << Color::Blue) : (std::string() << Color::Gray));
					CourseManager::getCourseText(topStr, courseIDs[i], true);
					topStr += ResetColor() + "\n";
				}
			}
			return topStr;
		};

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

		int opt;
		int currMenu = 0;
		constexpr int totalMenu = normalPages + TOTALALLCUPS;
		do {
			kbd.Populate({  NAME("exit") });
			kbd.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
			kbd.GetMessage() = generateAchievementsText(currMenu, totalMenu);
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

		MissionHandler::isMissionMode = wasMissionMode;
	}
}
