/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Sound.cpp
Open source lines: 76/76 (100.00%)
*****************************************************/

#include "Sound.hpp"
#include "CTRPluginFramework.hpp"
#include "3ds.h"
#include "csvc.h"
#include "cheats.hpp"
#include "OSDManager.hpp"
#include "ExtraResource.hpp"

extern "C" Result csndPlaySoundExtended(int chn, u32 flags, u32 sampleRate, float vol, float pan, void* data0, void* data1, u32 size);

namespace CTRPluginFramework {

	u32 Snd::soundObject = 0;
	u32(*Snd::playSysSe)(u32, Snd::SoundID) = nullptr;
	/*
	void (*Snd::setSoundFileAddress)(u32 soundLoader, u32 soundID, void* address) = (void(*)(u32, u32, void*))0x0026D768;
	void* (*Snd::getSoundFileAddress)(u32 soundLoader, u32 soundID) = (void* (*)(u32, u32))0x004FB824;
	u32(*Snd::soundIDToFileID)(u32 archiveLoader, u32 soundID) = (u32(*)(u32, u32))0x004FD5B0;*/

	void Snd::setupGamePlaySeFunc(u32 func)
	{
		playSysSe = (u32(*)(u32, SoundID))func;
	}
	void Snd::setupGameSoundObject(u32 object)
	{
		soundObject = object;
	}
	u32 Snd::PlayMenu(SoundID id)
	{
		if (playSysSe == nullptr || soundObject == 0) return 0;
		return playSysSe(soundObject, id);
	}

	static const std::pair<SoundEngine::Event, std::string> g_menuSoundInfo[] = {
		std::make_pair(SoundEngine::Event::CURSOR, "sound_cursor.bcwav"),
		std::make_pair(SoundEngine::Event::ACCEPT, "sound_accept.bcwav"),
		std::make_pair(SoundEngine::Event::CANCEL, "sound_cancel.bcwav"),
		std::make_pair(SoundEngine::Event::SELECT, "sound_select.bcwav"),
		std::make_pair(SoundEngine::Event::DESELECT, "sound_deselect.bcwav")
	};
	void Snd::InitializeMenuSounds()
	{
		for (int i = 0; i < sizeof(g_menuSoundInfo) / sizeof(std::pair<SoundEngine::Event, std::string>); i++) {
			ExtraResource::SARC::FileInfo fInfo;
			std::string fileName = "Plugin/snd/" + g_menuSoundInfo[i].second;
			u8* fileBuffer = ExtraResource::mainSarc->GetFile(fileName, &fInfo);
			if (fileBuffer == nullptr || fInfo.fileSize < 4) continue;
			Sound menuSnd(fileBuffer, 3);
			if (menuSnd.GetLoadStatus() != Sound::CWAVStatus::SUCCESS) continue;
			SoundEngine::RegisterMenuSoundEvent(g_menuSoundInfo[i].first, menuSnd);
		}
	}

	/*
	inline u32 Snd::getArchiveLoader()
	{
		u32 step1 = *(u32*)(MarioKartFramework::baseAllPointer + 0x10) + 0x1E0;
		u8 isPtrValid = *(u8*)(step1 + 0x50);
		if (!isPtrValid) return 0;
		u32 step2 = *(u32*)(step1 + 0x4C) ^ MarioKartFramework::baseAllPointerXor;
		return *(u32*)(*(u32*)(step2 + 0x28) + 8);
	}
	inline u32 Snd::getSoundLoader()
	{
		return ((u32**)soundObject)[0xB][0x9];
	}*/
}
