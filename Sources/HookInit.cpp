/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: HookInit.cpp
Open source lines: 42/3525 (1.19%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "rt.hpp"
#include "OnionFS.hpp"
#include "main.hpp"
#include "foolsday.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "HookInit.hpp"
#include "ExtraResource.hpp"
#include "Sound.hpp"
#include "VersusHandler.hpp"
#include "UserCTHandler.hpp"
#include "VisualControl.hpp"
#include "MenuPage.hpp"
#include "ItemHandler.hpp"
#include "DataStructures.hpp"
#include "ExtraUIElements.hpp"
#include "CharacterHandler.hpp"
#include "Net.hpp"
#include "CwavReplace.hpp"
#include "BootSceneHandler.hpp"
#include "BlueCoinChallenge.hpp"
#include "VoiceChatHandler.hpp"

namespace CTRPluginFramework {

	void initPatches() {
		// This function initializes all the 200+ hooks to the game.
		// Each hook defines where to hook as well as which CTGP-7 function to call.
		// For example, when the game reaches address 0x123458, the game will jump to the
		// function CTRPluginFramework::exampleFunction() and run it before returning.
	}
}
