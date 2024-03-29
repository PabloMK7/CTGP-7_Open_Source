/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: cheats.cpp
Open source lines: 632/640 (98.75%)
*****************************************************/

#include "types.h"
#include "cheats.hpp"
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

u32 g_currMenuVal = 0;
u8 g_isOnlineMode = (CTRPluginFramework::Utils::Random() | 2) & ~0x1;



namespace CTRPluginFramework
{
	CCSettings ccsettings[2] = {CCSettings(), CCSettings()};
	//const SpeedValues g_SpdValCostants[2] = { {"km/h", 10.f, 130.f, 2.84872641f}, {"mph", 6.21371f, 80.f, 1.75306240615f} };
	bool g_ComForcePtrRestore = false;
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
		if (MarioKartFramework::isGameInRace()) {
			;
		} else {
			if (g_ComForcePtrRestore) { 
				Sleep(Seconds(0.2f));
				g_ComForcePtrRestore = false;
				MarioKartFramework::restoreComTextPtr();
			}
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
        StringVector    unitsList = {"km/h", "mph"};
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

	void 	createcommcode(MenuEntry *entry) {
		std::string enSlid = "\u2282\u25CF";
		std::string disSlid = "\u25CF\u2283";
		OnlineSettingsv2 onlineset;
		onlineset.ver = COMMUSETVER;
		CCSettings   *cc_settings = static_cast<CCSettings *>(ccselectorentry->GetArg());
		if (cc_settings->enabled) {
			float val;
			if (cc_settings->value < 1) val = 1;
			else if (cc_settings->value > 9999) val = 9999;
			else val = cc_settings->value;
			onlineset.speed = (u16)val;
		} else {
			onlineset.speed = 0;
		}
		std::string comanager = NAME("comanager");
		Keyboard kbd(NAME("rnd_sel"));
		Keyboard optsKbd("dummy");
		kbd.Populate({ "2", "4", "8", "16" });
		kbd.ChangeSelectedEntry(1);
		int opt = kbd.Open();
		if (opt < 0) return;
		int optsOpt = 2;
		bool cten = true, orien = true, forcerandom = false, camallow = true, ledallow = true, cpuRacers = false, imprTricks = true, customItem = true, automaticdelaydrift = true;
		do {
			optsKbd.GetMessage() = NAME("settings") + "\n\n" << (cten ? Color::LimeGreen : Color::Red) << NAME("track_ct") + " (" + (cten ? NAME("state_mode") : NOTE("state_mode")) + ")\n";
			optsKbd.GetMessage() += (cten ? (orien ? Color::LimeGreen : Color::Red) : Color::Gray) << NAME("track_ori") + " (" + (orien ? NAME("state_mode") : NOTE("state_mode")) + ")\n";
			optsKbd.Populate({ (cten ? Color::LimeGreen << enSlid : Color::Red << disSlid), (cten ? (orien ? Color::LimeGreen << enSlid : Color::Red << disSlid) : ""), NAME("next") });
			optsKbd.ChangeEntrySound(0, cten ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(1, orien ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeSelectedEntry(optsOpt);
			optsOpt = optsKbd.Open();
			if (optsOpt < 0) return;
			if (optsOpt == 0) {
				cten = !cten;
				if (!cten) orien = true;
			}
			if (optsOpt == 1) {
				orien = !orien || !cten;
			}
		} while (optsOpt != 2);
		optsOpt = 7;
		do {
			optsKbd.GetMessage() = NAME("settings") + "\n\n";
			optsKbd.GetMessage() += NAME("rand_tr") + ": " << (forcerandom ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			optsKbd.GetMessage() += NAME("backcam") + ": " << (camallow ? (Color::LimeGreen << NAME("allow_inf")) : (Color::Red << NOTE("allow_inf"))) << ResetColor() + "\n";
			optsKbd.GetMessage() += NAME("itemled") + ": " << (ledallow ? (Color::LimeGreen << NAME("allow_inf")) : (Color::Red << NOTE("allow_inf"))) << ResetColor() + "\n";
			optsKbd.GetMessage() += NAME("CpuAm") + ": " << (cpuRacers ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			optsKbd.GetMessage() += NAME("imtrick") + ": " << (imprTricks ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			optsKbd.GetMessage() +=	NAME("cusitem") + ": " << (customItem ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			optsKbd.GetMessage() += NAME("autodelaydrift") + ": " << (automaticdelaydrift ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
			optsKbd.Populate({ (forcerandom ? Color::LimeGreen << enSlid : Color::Red << disSlid), (camallow ? Color::LimeGreen << enSlid : Color::Red << disSlid), (ledallow ? Color::LimeGreen << enSlid : Color::Red << disSlid), (cpuRacers ? Color::LimeGreen << enSlid : Color::Red << disSlid), (imprTricks ? Color::LimeGreen << enSlid : Color::Red << disSlid), (customItem ? Color::LimeGreen << enSlid : Color::Red << disSlid), (automaticdelaydrift ? Color::LimeGreen << enSlid : Color::Red << disSlid), NAME("next") });
			optsKbd.ChangeEntrySound(0, forcerandom ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(1, camallow ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(2, ledallow ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(3, cpuRacers ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(4, imprTricks ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(5, customItem ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeEntrySound(6, automaticdelaydrift ? SoundEngine::Event::DESELECT : SoundEngine::Event::SELECT);
			optsKbd.ChangeSelectedEntry(optsOpt);
			optsOpt = optsKbd.Open();
			if (optsOpt < 0) return;
			if (optsOpt == 0) {
				forcerandom = !forcerandom;
			}
			if (optsOpt == 1) {
				camallow = !camallow;
			}
			if (optsOpt == 2) {
				ledallow = !ledallow;
			}
			if (optsOpt == 3) {
				cpuRacers = !cpuRacers;
			}
			if (optsOpt == 4) {
				imprTricks = !imprTricks;
			}
			if (optsOpt == 5) {
				customItem = !customItem;
			}
			if (optsOpt == 6) {
				automaticdelaydrift = !automaticdelaydrift;
			}
		} while (optsOpt != 7);
		onlineset.rounds = opt;
		onlineset.areOrigTracksAllowed = orien;
		onlineset.areCustomTracksAllowed = cten;
		onlineset.areRandomTracksForced = forcerandom;
		onlineset.isBackcamAllowed = camallow;
		onlineset.isLEDItemsAllowed = ledallow;
		onlineset.cpuRacers = cpuRacers;
		onlineset.improvedTricksAllowed = imprTricks;
		onlineset.customItemsAllowed = customItem;
		onlineset.automaticDelayDriftAllowed = automaticdelaydrift;
		onlineset.unused = Utils::Random();
		onlineset.checksum = MarioKartFramework::getOnlinechecksum(&onlineset);
		char comCode[14];
		MarioKartFramework::encodeFromVal(comCode, *((u64*)&onlineset));
		int speed = onlineset.speed;
		int roundCount = 1 << (onlineset.rounds + 1);
		std::string msgstr = NAME("code") + std::string(": ") + std::string(comCode) + "\n\n";
		msgstr += (cc_settings->enabled ? std::to_string(speed) + "cc" : NAME("def_spd")) + "\n";
		msgstr += ((cten) ? (Color::LimeGreen << NAME("ct_endis")) : (Color::Red << NOTE("ct_endis"))) + "\n" + ((orien) ? (Color::LimeGreen << NAME("ori_endis")) : (Color::Red << NOTE("ori_endis"))) + ResetColor() + std::string("\n");
		msgstr += NAME("rounds") + std::string(": ") + std::to_string(roundCount) + std::string("\n");
		msgstr += NAME("rand_tr") + ": " << (forcerandom ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
		msgstr += NAME("backcam") + ": " << (camallow ? (Color::LimeGreen << NAME("allow_inf")) : (Color::Red << NOTE("allow_inf"))) << ResetColor() + "\n";
		msgstr += NAME("itemled") + ": " << (ledallow ? (Color::LimeGreen << NAME("allow_inf")) : (Color::Red << NOTE("allow_inf"))) << ResetColor() + "\n";
		msgstr += NAME("CpuAm") + ": " << (cpuRacers ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
		msgstr += NAME("imtrick") + ": " << (imprTricks ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
		msgstr += NAME("cusitem") + ": " << (customItem ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
		msgstr += NAME("autodelaydrift") + ": " << (automaticdelaydrift ? (Color::LimeGreen << NAME("state_mode")) : (Color::Red << NOTE("state_mode"))) << ResetColor() + "\n";
		(MessageBox(NAME("commugen"), msgstr, DialogType::DialogOk ,ClearScreen::Both))();
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
        StringVector    buttonList = {FONT_Y, FONT_A};
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
				topStr += "  " + NAME("ctww") + ": " + std::to_string(SaveHandler::saveData.ctVR) + " VR (" + Language::GenerateOrdinal(Net::vrPositions[0]) + ")\n";
			else
				topStr += "  " + NAME("ctww") + ": " + std::to_string(SaveHandler::saveData.ctVR) + " VR\n";
			if (Net::vrPositions[1] > 0)
				topStr += "  " + NAME("cntdwn") + ": " + std::to_string(SaveHandler::saveData.cdVR) + " VR (" + Language::GenerateOrdinal(Net::vrPositions[1]) + ")\n";
			else
				topStr += "  " + NAME("cntdwn") + ": " + std::to_string(SaveHandler::saveData.cdVR) + " VR\n";
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
}
