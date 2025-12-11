/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: main.cpp
Open source lines: 431/444 (97.07%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "types.h"
#include "OnionFS.hpp"
#include "main.hpp"
#include <cstdio>

#include <string>
#include <vector>

#include "3ds.h"
#include "Net.hpp"
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
#include "CrashReport.hpp"
#include "StatsHandler.hpp"
#include "UserCTHandler.hpp"
#include "ItemHandler.hpp"
#include "CourseCredits.hpp"
#include "plgldr.h"
#include "mallocDebug.hpp"
#include "HokakuCTR.hpp"
#include "CharacterHandler.hpp"
#include "BootSceneHandler.hpp"
#include "Stresser.hpp"
#include "TLSAccessPatcher.hpp"
#include "AsyncRunner.hpp"
#include "BadgeManager.hpp"
#include "SaveBackupHandler.hpp"

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
	MenuEntry* improvedHornEntry;
	MenuEntry* achievementsEntry;
	MenuEntry* badgesEntry;
	MenuEntry* blueCoinsEntry;
	MenuEntry* saveBackupEntry;

	OnlineMenuEntry* ccSelOnlineEntry;
	OnlineMenuEntry* numbRoundsOnlineEntry;
	OnlineMenuEntry* serverOnlineEntry;
	OnlineMenuEntry* improvedTricksOnlineEntry;

	u32 AsyncRunner::counter = 0;

	void 	HandleProcessEvent(Process::Event event);

    // Global
    LightEvent mainEvent0;
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

	class CustomRandomBackend : public Utils::RandomBackend {
	public:
		void Seed(u64 seed) override {
			random.Init((u32)(seed >> 32));
			random.Init(random.Get() ^ ((u32)(seed)));
		}
        u32 Random(void) override {
			return random.Get();
		}
	private:
		SeadRandom random{};
	};
	static CustomRandomBackend g_rnd{};

	// Define this so that the game stack is used to init arrays
	void  UseGameStackToInit() {}

	// Ran before anything else
    void  PatchProcess(FwkSettings &settings)
    {
		TLSAccessPatcher::PatchPlugin();
		g_StresserInit();

		Utils::UseRandomBackend(&g_rnd);
		Utils::AutoSeedRandom();

		CrashReport::stateID = CrashReport::StateID::STATE_PATCHPROCESS;

		BootSceneHandler::Initialize();
		Process::SetProcessEventCallback(HandleProcessEvent);
		Directory::ChangeWorkingDirectory("/CTGP-7/resources/");
		if (!checkCompTID(Process::GetTitleID())) panic();
		System::OnAbort = CrashReport::OnAbort;
#ifdef RELEASE_BUILD
		Process::exceptionCallback = CrashReport::CTGPExceptCallback;
		Process::ThrowOldExceptionOnCallbackException = true;
		OSD::LightweightMode = true;
#endif
		Process::OnPauseResume = [](bool isGoingToPause) {
			MarioKartFramework::playMusicAlongCTRPF(isGoingToPause);
		};
		initPatches();
		OnionFS::InitFSFileMapThread();
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
		settings.CloseMenuWithB = true;
		MarioKartFramework::getLaunchInfo();
		LightEvent_Init(&mainEvent1, RESET_ONESHOT);
		LightEvent_Init(&mainEvent0, RESET_ONESHOT);
		
		// Check we are in 80MB mode (real value is 0x05000000, but some is taken by the plugin)
		if (OS_KernelConfig->memregion_sz[0] < 0x04800000) {
			panic(Utils::Format("Invalid MemMode: 0x%08X", OS_KernelConfig->memregion_sz[0]).c_str());
		}

		g_setCTModeVal = CTMode::OFFLINE;
		g_updateCTMode();

		Language::RegisterProgress();
		CharacterHandler::RegisterProgress();
		UserCTHandler::RegisterProgress();
		Net::Initialize(); 
		return;
    }

	// Runs on the game's main thread, do not place anything here that creates threads!
	void InitMainClasses(void) {
		CrashReport::stateID = CrashReport::StateID::STATE_INITIALIZE;
		Sleep(Seconds(2));
		BootSceneHandler::ProgressHandle mainProg = BootSceneHandler::RegisterProgress(8);
		BootSceneHandler::Progress(mainProg);

		if (!Directory::IsExists("/CTGP-7/savefs"))
			Directory::Create("/CTGP-7/savefs");
		if (!Directory::IsExists("/CTGP-7/savefs/game"))
			Directory::Create("/CTGP-7/savefs/game");
		if (!Directory::IsExists("/CTGP-7/savefs/mod"))
			Directory::Create("/CTGP-7/savefs/mod");
		
		BootSceneHandler::Progress(mainProg);

		
		SaveBackupHandler::Initialize();
		SaveHandler::LoadSettings();
		renderImprovements_apply(SaveHandler::saveData.flags1.renderOptimization);
		CharacterHandler::customKartsEnabled = SaveHandler::saveData.flags1.customKartsEnabled;

		BootSceneHandler::Progress(mainProg);

		if (!OnionFS::initOnionFSHooks(Process::GetTextSize())) {
			panic();
		}

		BootSceneHandler::Progress(mainProg);

		Language::Initialize();
		CharacterHandler::Initialize();
		Language::Import(); BootSceneHandler::Progress(mainProg);
		MusicSlotMngr::Initialize(); BootSceneHandler::Progress(mainProg);
		MarioKartFramework::InitializeLedPatterns(); BootSceneHandler::Progress(mainProg);
		UserCTHandler::Initialize();
		CourseCredits::Initialize(); BootSceneHandler::Progress(mainProg);
		if (checkFoolsDay()) {
			loadJokeResources();
			setFoolsSeed();
		}

		LightEvent_Signal(&mainEvent0);
	}

    void InitMenu(void)
    {
		u32 menuType = 1;
		#ifndef RELEASE_BUILD
		menuType = 0;
		#endif
		mainPluginMenu = new PluginMenu(Color::Gray << std::string("CTGP") << Color::White << std::string("-") << Color::Red << std::string("7") << ResetColor() + 
		#ifdef BETA_BUILD
		" (BETA)" +
		#endif
		#ifndef RELEASE_BUILD
		" (DEBUG)" +
		#endif
		"", "\nCTGP-7 modpack for Mario Kart 7\n\nWebsite: https://ctgp-7.github.io/ \nDiscord: https://invite.gg/ctgp7 \n\nHave fun!\n- CTGP-7 Team", menuType);
        PluginMenu      &menu = *mainPluginMenu;

        //-----
        #ifndef RELEASE_BUILD
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
		menu.Append(new MenuEntry("Quick Vector3 Watch", [](MenuEntry* entry) {
			u32* addrPtr = (u32*)entry->GetArg();
			if (addrPtr) {
				Vector3* addr = (Vector3*)*addrPtr;
				NOXTRACEPOS("quickwatchf", 10, 60, "%0.2f %0.2f %0.2f", addr->x, addr->y, addr->z);
			}
		}, [](MenuEntry* entry) {
			static u32 addr = 0x100000;
			Keyboard kbd("Set watch address.");
			kbd.IsHexadecimal(true);
			if (kbd.Open(addr, addr) == 0) {
				entry->SetArg(&addr);
			}
		}, ""));
		menu.Append(new MenuEntry("Heap Usage", nullptr, [](MenuEntry* entry) {
			struct mallinfo mi = mallinfo();
			std::string t =  Utils::Format("Heap used:   %d bytes\n", mi.uordblks) +
    				  		Utils::Format("Heap free:   %d bytes\n", mi.fordblks) +
    						Utils::Format("Heap total:  %d bytes\n", mi.arena);
			Keyboard kbd(t);
			kbd.Populate({"Exit"});
			kbd.Open();
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
			automaticDelayDriftEntry = new MenuEntry(NAME("autodelaydrift"), nullptr, automaticdelaydrift_entryfunc, NOTE("autodelaydrift")),
			improvedHornEntry = new MenuEntry(NAME("improvedhorn"), nullptr, improvedhorn_entryfunc, NOTE("improvedhorn")),
			blueCoinsEntry = new MenuEntry(NAME("blue_coins"), nullptr, bluecoin_entryfunc, NAME("blue_coins_desc"))
		});

		MenuFolder* settings = new MenuFolder(NAME("settings_folder"), NOTE("settings_folder"), {
			numbRoundsEntry = new MenuEntry(NAME("chgrnd"), nullptr, changeRoundNumber, NOTE("chgrnd")),
			customCharactersEntry = new MenuEntry(NAME("charman") , nullptr, CharacterHandler::CustomCharacterManagerMenu, NOTE("charman")),
			customKartEntry = new MenuEntry(NAME("cuskart"), nullptr, CharacterHandler::enableCustomKartsSettings, NOTE("cuskart")),
			courseOrderEntry = new MenuEntry(NAME("trackorder"), nullptr, courseOrder, NOTE("trackorder")),
			renderImproveEntry = new MenuEntry(NAME("render_optim"), nullptr, renderImprovements, NOTE("render_optim")),
			serverEntry = new MenuEntry(NAME("servsett"), nullptr, serverEntryHandler, NOTE("servsett")),
		});

		MenuFolder* other = new MenuFolder(NAME("other_folder"), NOTE("other_folder"), {
			saveBackupEntry = new MenuEntry(NAME("cloudsaveentry"), nullptr, SaveBackupHandler::BackupHandlerEntry, NOTE("cloudsaveentry")),
			statsEntry = new MenuEntry(NAME("statsentry"), nullptr, StatsHandler::StatsMenu, NOTE("statsentry")),
			resetGhostsEntry = new MenuEntry(NAME("resghost"), nullptr, CourseManager::resetGhost, NOTE("resghost"))
		});

		menu.Append(features);
		menu.Append(settings);
		menu.Append(other);

		menu.Append(achievementsEntry = new MenuEntry(NAME("achieventry"), nullptr, achievementsEntryHandler, NOTE("achieventry")));
		menu.Append(badgesEntry = new MenuEntry(NAME("badgesentry"), nullptr, BadgeManager::BadgesMenu, NOTE("badgesentry")));
		menu.Append(langSettingEntry = new MenuEntry("Language", nullptr, Language::ShowLangMenu, " "));

		customKartEntry->Name() = NAME("cuskart") + " (" + (SaveHandler::saveData.flags1.customKartsEnabled ? NAME("state_mode") : NOTE("state_mode")) + ")";
		langSettingEntry->Name() = "Language (" + Language::availableLang[Language::currentTranslation].Name + ")";
		
		ccSelOnlineEntry = new OnlineMenuEntry(ccselectorentry, nullptr, ccselectorsettings,
			[](MenuEntry* entry, std::string& out) {
				CCSettings* settings = static_cast<CCSettings*>(entry->GetArg());
				if (settings->enabled) out += "(" + std::to_string((int)settings->value) + "cc)";
				else out += "(" + NOTE("state_mode") + ")";
			}
		);
		
		serverOnlineEntry = new OnlineMenuEntry(serverEntry, nullptr, serverEntryHandler, [](MenuEntry* e, std::string& s) {});
		improvedTricksOnlineEntry = new OnlineMenuEntry(improvedTricksEntry, nullptr, improvedTricks, [](MenuEntry* e, std::string& s) {});
		numbRoundsOnlineEntry = new OnlineMenuEntry(numbRoundsEntry, nullptr, changeRoundNumber,
			[](MenuEntry* entry, std::string& out) {
				out	+= "(" + std::to_string(SaveHandler::saveData.numberOfRounds) + ")";
			}
		);

		#if CITRA_MODE == 1
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
		SaveHandler::ApplySettings();
		
		improvedRouletteEntry->Name() = NAME("itmprb") + " (" + (SaveHandler::saveData.flags1.improvedRoulette ? NAME("state_mode") : NOTE("state_mode")) + ")";

        // Main callback loop
        menu.Callback(menucallback);
		menu.SynchronizeWithFrame(true);
		menu.UpdateEveryOtherFrame(true);

#ifndef RELEASE_BUILD
		menu.ShowWelcomeMessage(false);
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
		LightEvent_Wait(&mainEvent0);

		// Initialize menu
		InitMenu();

        // Run menu
		LightEvent_Wait(&mainEvent1);
		mainPluginMenu->Run();
        // delete mainPluginMenu; <</ Crashes...

        // Exit plugin
        return (0);
    }

	void HandleProcessEvent(Process::Event event) {
		if (event == Process::Event::EXIT) {
			SaveHandler::SaveSettingsAll(true);
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
			#ifdef USE_HOKAKU
			delete mainLogger;
			#endif
			Process::exceptionCallback = nullptr;
		}
	}
}
