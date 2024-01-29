/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: main.cpp
Open source lines: 351/364 (96.43%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "types.h"
#include "OnionFS.hpp"
#include "cheats.hpp"
#include <cstdio>

#include <string>
#include <vector>

#include "3ds.h"
#include "Net.hpp"
#include "IPS.h"
#include "csvc.h"
#include "Lang.hpp"
#include "rt.hpp"
#include "MarioKartFramework.hpp"
#include "Unicode.h"
#include "SaveHandler.hpp"
#include "entrystructs.hpp"
#include "CourseManager.hpp"
#include "foolsday.hpp"
#include "Sound.hpp"
#include "HookInit.hpp"
#include "MusicSlotMngr.hpp"
#include "CharacterManager.hpp"
#include "CrashReport.hpp"
#include "StatsHandler.hpp"
#include "UserCTHandler.hpp"
#include "ItemHandler.hpp"
#include "CourseCredits.hpp"
#include "plgldr.h"
#include "mallocDebug.hpp"
#include "HokakuCTR.hpp"

extern bool g_checkMenu;
extern u32* g_gameSrvHandle;

namespace CTRPluginFramework
{
	PluginMenu* mainPluginMenu;

    MenuEntry *ccselectorentry;
    MenuEntry *speedometerentry;
    MenuEntry *allowOnlineCTentry;
    MenuEntry *backcamentry;
    MenuEntry *warnitementry;
    MenuEntry *courmanentry;
    MenuEntry *comcodegenentry;
    MenuEntry *numbRoundsEntry;
	MenuEntry *customCharactersEntry;
	MenuEntry *customKartEntry;
	MenuEntry *courseOrderEntry;
	MenuEntry *resetGhostsEntry;
	MenuEntry *statsEntry;
	MenuEntry *serverEntry;
	MenuEntry *langSettingEntry;
	MenuEntry *improvedRouletteEntry;
	MenuEntry* improvedTricksEntry;
	MenuEntry* renderImproveEntry;
	MenuEntry* autoAccelEntry;
	MenuEntry* brakeDriftEntry;
	MenuEntry* automaticDelayDriftEntry;

	OnlineMenuEntry* ccSelOnlineEntry;
	OnlineMenuEntry* comCodeGenOnlineEntry;
	OnlineMenuEntry* numbRoundsOnlineEntry;
	OnlineMenuEntry* serverOnlineEntry;
	OnlineMenuEntry* improvedTricksOnlineEntry;

	void 	HandleProcessEvent(Process::Event event);

    // Global
    LightEvent mainEvent1;
    static Handle g_event = 0;
    bool    gi_isUpdated = false;
    bool    ge_isUpdated = false;

	void    InitMenuSettings(void)
	{
		FwkSettings& settings = FwkSettings::Get();

		settings.BackgroundSecondaryColor = Color(0x24, 0xf, 0xf);
		settings.BackgroundBorderColor = Color(0xff, 0, 0);
		settings.MainTextColor = Color(0xC0, 0xC0, 0xC0);
		settings.MenuSelectedItemColor = Color(0xC8, 0xC8, 0xC8);
		settings.MenuUnselectedItemColor = settings.MainTextColor;
		settings.WindowTitleColor = Color::White;
		settings.CustomKeyboard.BackgroundBorder = settings.BackgroundBorderColor;
		settings.CustomKeyboard.BackgroundMain = settings.BackgroundMainColor;
		settings.CustomKeyboard.BackgroundSecondary = settings.BackgroundSecondaryColor;
		settings.CustomKeyboard.ScrollBarBackground = Color(0xCC, 0x30, 0x30);
		settings.CustomKeyboard.ScrollBarThumb = Color(0x87, 0x1F, 0x1F);
	}

	#ifdef INSTRUMENT_FUNCTIONS
	void init_instrumentation();
	#endif

    void  PatchProcess(FwkSettings &settings)
    {
		#ifdef INSTRUMENT_FUNCTIONS
		init_instrumentation();
		#endif
		Process::SetProcessEventCallback(HandleProcessEvent);
		Directory::ChangeWorkingDirectory("/CTGP-7/resources/");
		if (!checkCompTID(Process::GetTitleID())) panic();

		CrashReport::stateID = CrashReport::StateID::STATE_PATCHPROCESS;
#ifdef RELEASE_BUILD
		System::OnAbort = CrashReport::OnAbort;
		Process::exceptionCallback = CrashReport::CTGPExceptCallback;
		Process::ThrowOldExceptionOnCallbackException = true;
#endif
		Process::OnPauseResume = [](bool isGoingToPause) {
			MarioKartFramework::playMusicAlongCTRPF(isGoingToPause);
		};
		SaveHandler::LoadSettings();
		renderImprovements_apply(SaveHandler::saveData.flags1.renderOptimization);
		if (!OnionFS::initOnionFSHooks(Process::GetTextSize())) {
			panic();
		}
		initPatches();
#ifdef USE_HOKAKU
		InitHokaku();
#endif
		settings.AllowActionReplay = false;
#ifdef RELEASE_BUILD
		settings.AllowSearchEngine = false;
#endif // RELEASE_BUILD
		settings.ThreadPriority = 0x3E;
		settings.WaitTimeToBoot = Seconds(0);
		settings.TryLoadSDSounds = false;
		settings.UseGameHidMemory = true;
		settings.AreN3DSButtonsAvailable = false;
		MarioKartFramework::getLaunchInfo();
		LightEvent_Init(&mainEvent1, RESET_ONESHOT);

		if (!Directory::IsExists("/CTGP-7/Screenshots"))
			Directory::Create("/CTGP-7/Screenshots");
		if (Directory::IsExists("/CTGP-7/savefs/replay") && !Directory::IsExists("/CTGP-7/savefs/game")) {
			Directory::Rename("/CTGP-7/savefs", "/CTGP-7/savefstmp");
			Directory::Create("/CTGP-7/savefs");
			Directory::Rename("/CTGP-7/savefstmp", "/CTGP-7/savefs/game");
		}
		if (!Directory::IsExists("/CTGP-7/savefs"))
			Directory::Create("/CTGP-7/savefs");
		if (!Directory::IsExists("/CTGP-7/savefs/game"))
			Directory::Create("/CTGP-7/savefs/game");
		if (!Directory::IsExists("/CTGP-7/savefs/mod"))
			Directory::Create("/CTGP-7/savefs/mod");
		if (File::Exists("/CTGP-7/resources/moreSav.bin")) {
			File::Rename("/CTGP-7/resources/moreSav.bin", "/CTGP-7/savefs/mod/extCupSave.sav");
		}
		
		Language::Initialize();
		CharacterManager::Initialize();
		Language::Import();
		MusicSlotMngr::Initialize();
		MarioKartFramework::InitializeLedPatterns();
		StatsHandler::Initialize();
		UserCTHandler::Initialize();
		CourseCredits::Initialize();
		Net::Initialize();
		if (checkFoolsDay()) {
			loadJokeResources();
			setFoolsSeed();
		}
		g_setCTModeVal = CTMode::OFFLINE;
		g_updateCTMode();
		svcFlushEntireDataCache();
		return;
    }

    void InitMenu(void)
    {
		u32 menuType = 1;
		#ifndef RELEASE_BUILD
		menuType = 0;
		#endif
		mainPluginMenu = new PluginMenu(Color::Gray << std::string("CTGP") << Color::White << std::string("-") << Color::Red << std::string("7"), "\nCTGP-7 modpack for Mario Kart 7\n\nWebsite: https://ctgp-7.github.io/ \nDiscord: https://invite.gg/ctgp7 \n\nHave fun!\n- CTGP-7 Team", menuType);
        PluginMenu      &menu = *mainPluginMenu;

        //-----
		#ifdef BETA_BUILD
		menu.Append(new MenuEntry("BETA BUILD", nullptr, [](MenuEntry* entry) {}, "BETA BUILD"));
		#endif
        #ifndef RELEASE_BUILD
		menu.Append(new MenuEntry("DEBUG BUILD", nullptr, [](MenuEntry* entry) {Snd::PlayMenu(Snd::RACE_OK); }, "DEBUG BUILD"));
		menu.Append(new MenuEntry("Quick u32 Watch", [](MenuEntry* entry) {
			u32* addrPtr = (u32*)entry->GetArg();
			if (addrPtr) {
				u32* addr = (u32*)*addrPtr;
				NOXTRACEPOS("quickwatchu", 10, 20, "0x%08X", *addr);
			}
		}, [](MenuEntry* entry) {
			static u32 addr = 0x100000;
			Keyboard kbd("Set watch address.");
			kbd.IsHexadecimal(true);
			if (kbd.Open(addr, addr) == 0) {
				entry->SetArg(&addr);
			}
		}, ""));
		menu.Append(new MenuEntry("Quick float Watch", [](MenuEntry* entry) {
			u32* addrPtr = (u32*)entry->GetArg();
			if (addrPtr) {
				float* addr = (float*)*addrPtr;
				NOXTRACEPOS("quickwatchf", 10, 40, "%0.4f", *addr);
			}
		}, [](MenuEntry* entry) {
			static u32 addr = 0x100000;
			Keyboard kbd("Set watch address.");
			kbd.IsHexadecimal(true);
			if (kbd.Open(addr, addr) == 0) {
				entry->SetArg(&addr);
			}
		}, ""));
        #endif
		MenuFolder* features = new MenuFolder(NAME("gameplay_folder"), NOTE("gameplay_folder"), {
			ccselectorentry = new MenuEntry(NAME("ccsel"), nullptr, ccselectorsettings, NOTE("ccsel")),
			speedometerentry = new MenuEntry(NAME("spdmeter"), nullptr, speedometersettings, NOTE("spdmeter")),
			backcamentry = new MenuEntry(NAME("backcam"), nullptr, backwardscam, NOTE("backcam")),
			warnitementry = new MenuEntry(NAME("itemled"), nullptr, warnItemUse, NOTE("itemled")),
			improvedRouletteEntry = new MenuEntry(NAME("itmprb"), nullptr, MarioKartFramework::improvedRouletteSettings, NOTE("itmprb")),
			improvedTricksEntry = new MenuEntry(NAME("imtrick"), nullptr, improvedTricks, NOTE("imtrick")),
			autoAccelEntry = new MenuEntry(NAME("autoaccel"), nullptr, autoAccelSetting, NOTE("autoaccel")),
			brakeDriftEntry = new MenuEntry(NAME("imbrakedrift"), nullptr, brakeDrift, NOTE("imbrakedrift")),
			automaticDelayDriftEntry = new MenuEntry(NAME("autodelaydrift"), nullptr, automaticdelaydrift_entryfunc, NOTE("autodelaydrift"))
		});

		MenuFolder* settings = new MenuFolder(NAME("settings_folder"), NOTE("settings_folder"), {
			numbRoundsEntry = new MenuEntry(NAME("chgrnd"), nullptr, changeRoundNumber, NOTE("chgrnd")),
			customCharactersEntry = new MenuEntry(NAME("charman") , nullptr, CharacterManager::characterManagerSettings, NOTE("charman")),
			customKartEntry = new MenuEntry(NAME("cuskart"), nullptr, CharacterManager::enableCustomKartsSettings, NOTE("cuskart")),
			courseOrderEntry = new MenuEntry(NAME("trackorder"), nullptr, courseOrder, NOTE("trackorder")),
			renderImproveEntry = new MenuEntry(NAME("render_optim"), nullptr, renderImprovements, NOTE("render_optim")),
			serverEntry = new MenuEntry(NAME("servsett"), nullptr, serverEntryHandler, NOTE("servsett")),
		});

		MenuFolder* other = new MenuFolder(NAME("other_folder"), NOTE("other_folder"), {
			statsEntry = new MenuEntry(NAME("statsentry"), nullptr, StatsHandler::StatsMenu, NOTE("statsentry")),
			comcodegenentry = new MenuEntry(NAME("commugen"), nullptr, createcommcode, NOTE("commugen")),
			resetGhostsEntry = new MenuEntry(NAME("resghost"), nullptr, CourseManager::resetGhost, NOTE("resghost"))
		});

		menu.Append(features);
		menu.Append(settings);
		menu.Append(other);

		menu.Append(langSettingEntry = new MenuEntry("Language", nullptr, Language::ShowLangMenu, " "));

		customKartEntry->Name() = NAME("cuskart") + " (" + (CharacterManager::currSave.customKartsEnabled ? NAME("state_mode") : NOTE("state_mode")) + ")";
		langSettingEntry->Name() = "Language (" + Language::availableLang[Language::currentTranslation].Name + ")";
		
		ccSelOnlineEntry = new OnlineMenuEntry(ccselectorentry, nullptr, ccselectorsettings,
			[](MenuEntry* entry, std::string& out) {
				CCSettings* settings = static_cast<CCSettings*>(entry->GetArg());
				if (settings->enabled) out += "(" + std::to_string((int)settings->value) + "cc)";
				else out += "(" + NOTE("state_mode") + ")";
			}
		);
		
		comCodeGenOnlineEntry = new OnlineMenuEntry(comcodegenentry, nullptr, createcommcode, [](MenuEntry* e, std::string& s) {});
		serverOnlineEntry = new OnlineMenuEntry(serverEntry, nullptr, serverEntryHandler, [](MenuEntry* e, std::string& s) {});
		improvedTricksOnlineEntry = new OnlineMenuEntry(improvedTricksEntry, nullptr, improvedTricks, [](MenuEntry* e, std::string& s) {});
		numbRoundsOnlineEntry = new OnlineMenuEntry(numbRoundsEntry, nullptr, changeRoundNumber,
			[](MenuEntry* entry, std::string& out) {
				out	+= "(" + std::to_string(SaveHandler::saveData.numberOfRounds) + ")";
			}
		);

		#if CITRA_MODE == 1
		comcodegenentry->Hide();
		warnitementry->Hide();
		#endif

		#ifdef RELEASE_BUILD
			menu.OnOpening = MarioKartFramework::allowOpenCTRPFMenu;
		#endif
		menu.OnClosing = []() {
			SaveHandler::SaveSettingsAll();
		};

		// Load settings
		Controller::Update();
		InitMenuSettings();
		if (Controller::IsKeysDown(Key::R | Key::L | Key::A | Key::X)) SaveHandler::DefaultSettings();
		SaveHandler::ApplySettings();
		
		improvedRouletteEntry->Name() = NAME("itmprb") + " (" + (SaveHandler::saveData.flags1.improvedRoulette ? NAME("state_mode") : NOTE("state_mode")) + ")";

		PluginMenu::ScreenshotPath() = "/CTGP-7/Screenshots/";
		PluginMenu::ScreenshotFilePrefix() = "CTGP-7";
		PluginMenu::ScreenshotSetcallback(MarioKartFramework::allowTakeScreenshot);
		PluginMenu::ScreenshotUpdatePaths();

        // Main callback loop
        menu.Callback(menucallback);
		menu.SynchronizeWithFrame(true);

#ifndef RELEASE_BUILD
		menu.ShowWelcomeMessage(true);
#else
		menu.ShowWelcomeMessage(false);
		menu.SetHexEditorState(false);
#endif // !RELEASE_BUILD 

#ifdef DEBUG_MALLOC
		OSD::Notify("MALLOC DEBUG");
#endif
    }

    int     main(void)
    {
		// Initialize menu
		InitMenu();

        // Run menu
		LightEvent_Wait(&mainEvent1);
		mainPluginMenu->Run();
        delete mainPluginMenu;

        // Exit plugin
        return (0);
    }

	void HandleProcessEvent(Process::Event event) {
		if (event == Process::Event::EXIT) {
			SaveHandler::SaveSettingsAll();
			SaveHandler::WaitSaveSettingsAll();
			Net::WaitOnlineStateMachine();
			Net::UpdateOnlineStateMahine(Net::OnlineStateMachine::OFFLINE);
			Net::WaitOnlineStateMachine();
			NetHandler::Session::Terminate();
			#if CITRA_MODE == 0
			if (System::IsNew3DS() && SaveHandler::saveData.flags1.renderOptimization)
				svcKernelSetState(10, (1 << 3));
			svcKernelSetState(0x10080, 0);
			#endif
			Process::exceptionCallback = nullptr;
		}
	}
}