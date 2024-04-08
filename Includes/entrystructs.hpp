/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: entrystructs.hpp
Open source lines: 68/68 (100.00%)
*****************************************************/

#pragma once

#include "CTRPluginFramework.hpp"
#include "OnlineMenuEntry.hpp"
#include "Minibson.hpp"
#include "SaveHandlerInfos.hpp"

namespace CTRPluginFramework {

	#define CC_SINGLE 0.00666666666

    struct  CCSettings
    {
        float             value;
        bool          enabled;
        CCSettings(void)
        {
            enabled = false;
            value = 150;
        }

        CCSettings(minibson::document& doc) {
            value = (float)doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CC_SELECTOR_VALUE), 150.0);
            enabled = doc.get(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CC_SELECTOR_ENABLED), false);
        }

        void serialize(minibson::document& doc) {
            doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CC_SELECTOR_VALUE), (double)value);
            doc.set(CTGP7SaveInfo::getSaveCode(CTGP7SaveInfo::CC_SELECTOR_ENABLED), enabled);
        }
    };

	extern CCSettings ccsettings[2];

    extern PluginMenu* mainPluginMenu;

    extern MenuEntry *ccselectorentry;
    extern MenuEntry *speedometerentry;
    extern MenuEntry *backcamentry;
    extern MenuEntry *warnitementry;
    extern MenuEntry *comcodegenentry;
	extern MenuEntry *courmanentry;
	extern MenuEntry *numbRoundsEntry;
	extern MenuEntry *courseOrderEntry;
	extern MenuEntry *resetGhostsEntry;
    extern MenuEntry *serverEntry;
    extern MenuEntry* improvedTricksEntry;
    extern MenuEntry* renderImproveEntry;
    extern MenuEntry* autoAccelEntry;
    extern MenuEntry* brakeDriftEntry;
    extern MenuEntry* automaticDelayDriftEntry;
    extern MenuEntry* achievementsEntry;

	extern OnlineMenuEntry* ccSelOnlineEntry;
	extern OnlineMenuEntry* comCodeGenOnlineEntry;
	extern OnlineMenuEntry* numbRoundsOnlineEntry;
    extern OnlineMenuEntry* serverOnlineEntry;
    extern OnlineMenuEntry* improvedTricksOnlineEntry;
}