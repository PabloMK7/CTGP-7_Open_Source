/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: OnlineMenuEntry.cpp
Open source lines: 47/47 (100.00%)
*****************************************************/

#include "OnlineMenuEntry.hpp"
#include "Lang.hpp"

namespace CTRPluginFramework{
	OnlineMenuEntry::OnlineMenuEntry(MenuEntry* baseEntry, OnlineMenuEntry::FuncPointer gamefunc, OnlineMenuEntry::FuncPointer menufunc, OnlineMenuEntry::NameUpdateFunc namefunc)
	{
		originalName = baseEntry->Name();
		this->baseEntry = baseEntry;
		this->originalGameFunc = gamefunc;
		this->originalMenuFunc = menufunc;
		this->updateNameForSettings = namefunc;
		this->isOnline = false;
	}
	void OnlineMenuEntry::setOnlineMode(bool isOnline)
	{
		this->isOnline = isOnline;
		updateName();
	}
	bool OnlineMenuEntry::getOnlineMode()
	{
		return this->isOnline;
	}
	void OnlineMenuEntry::updateName()
	{
		if (isOnline) {
			baseEntry->Name() = Color::Gray << originalName << " ("+ NOTE("state_mode") + ")";
			baseEntry->SetGameFunc(nullptr);
			baseEntry->SetMenuFunc([](MenuEntry* e){});
		}
		else {
			std::string config = "";
			updateNameForSettings(baseEntry, config);
			baseEntry->Name() = originalName << " " << config;
			baseEntry->SetGameFunc(originalGameFunc);
			baseEntry->SetMenuFunc(originalMenuFunc);
		}
	}
} 