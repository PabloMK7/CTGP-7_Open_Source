/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: OnionFS.hpp
Open source lines: 26/26 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "rt.hpp"
#include "csvc.h"



namespace CTRPluginFramework::OnionFS {
	enum class GameFSFileState {
		NOTEXISTS = 0,
		EXISTS = 1,
		UNKNOWN = 2
	};
	void initGameFsFileMap();
	GameFSFileState CheckGameFSFileExists(const u16* file);
	bool initOnionFSHooks(u32 textSize);
}