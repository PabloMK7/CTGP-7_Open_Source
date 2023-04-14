/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: OnlineMenuEntry.hpp
Open source lines: 34/34 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
	class OnlineMenuEntry
	{
		using FuncPointer = void(*)(MenuEntry*);
		using NameUpdateFunc = void(*)(MenuEntry*, std::string&);

	public:
		OnlineMenuEntry(MenuEntry* baseEntry, OnlineMenuEntry::FuncPointer gamefunc, OnlineMenuEntry::FuncPointer menufunc, NameUpdateFunc namefunc);

		void setOnlineMode(bool isOnline);
		bool getOnlineMode();
		void updateName();

	private:
		MenuEntry* baseEntry;
		FuncPointer originalGameFunc;
		FuncPointer originalMenuFunc;
		bool isOnline;
		std::string originalName;
		void (*updateNameForSettings)(MenuEntry* entry, std::string& out);
	};
}
