/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MusicSlotMngr.hpp
Open source lines: 61/61 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include <map>

namespace CTRPluginFramework {
	class MusicSlotMngr
	{	
	public:
		enum BgmMode
		{
			SINGLE,
			MULTI_WATER,
			MULTI_AREA
		};
		struct MusicSlotEntry {
			std::string musicFileName;
			BgmMode mode;
			u32 normalBeatBPM;
			u32 normalBeatOffset;
			u32 fastBeatBPM;
			u32 fastBeatOffset;
			bool isMyStuff;
			std::string musicName;
			std::vector<u16> authors;
			MusicSlotEntry() {
				musicFileName = "";
				mode = BgmMode::SINGLE;
				normalBeatBPM = normalBeatOffset = fastBeatBPM = fastBeatOffset = 0;
				isMyStuff = false;
			}
			MusicSlotEntry(const std::string& musicN, BgmMode musicM, u32 nBPM, u32 nOff, u32 fBPM, u32 fOff, bool userData) {
				musicFileName = musicN;
				mode = musicM;
				normalBeatBPM = nBPM;
				normalBeatOffset = nOff;
				fastBeatBPM = fBPM;
				fastBeatOffset = fOff;
				isMyStuff = userData;
			}
			std::string GetAuthorString();
		};
		static std::map<u32, MusicSlotEntry> entryMap;
		static void initMusicSlotData(const std::string& filename, bool isUserData, std::map<u32, MusicSlotMngr::MusicSlotEntry>& musicMap);
		static void Initialize();

		static std::vector<std::string> GetAllAuthors();
	private:
		static std::vector<std::pair<bool,std::string>> authorNames;
		static u16 InsertAuthor(const std::string& author, bool isUserData);
        static const std::string& GetAuthor(u16 index);
	};
}
