/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CharacterManager.hpp
Open source lines: 160/160 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "MarioKartFramework.hpp"
#include "IPS.h"

namespace CTRPluginFramework {

	using CallbackPointer = void (*)(void);

	class FileCopier
	{
	public:
		FileCopier(TaskFunc fileFunction1, TaskFunc fileFunction2);
		~FileCopier();

		void AddFileTuple(std::tuple<std::string, std::string>& tuple);
		bool GetNextTuple(std::tuple<std::string, std::string>& outTuple);
		int GetTotalFiles();
		int GetRemainingFiles();
		void Run();

	private:

		Task* Tasks[2];
		Mutex GetMutex;
		u32 TotalFiles = 0;

		std::vector<std::tuple<std::string, std::string>> Files;
	};

	class CharacterManager
	{
	public:
		enum CharID : u8 {
			BOWSER = 0,
			DONKEY_KONG,
			DAISY,
			HONEY_QUEEN,
			KOOPA_TROOPA,
			LUIGI,
			LAKITU,
			MII,
			MARIO,
			METAL_MARIO,
			PEACH,
			ROSALINA,
			SHY_GUY,
			TOAD,
			WIGGLER,
			WARIO,
			YOSHI,
			LAST // Always last one, used as size
		};
		static const char* shortNames[CharID::LAST];
		static const char* selectBclimNames[CharID::LAST];
		static const int msbtOrder[CharID::LAST];
		static const CharID driverIDOrder[EDriverID::DRIVER_SIZE];
		struct CharacterSave
		{
			u32 magic;
			u32 version;
			bool needsPatching;
			bool customKartsEnabled;
			char inUseNames[CharID::LAST][0x20];
			CharacterSave() {
				magic = 0x56534843;
				version = 2;
				memset(inUseNames, 0, sizeof(inUseNames));
				needsPatching = true;
				customKartsEnabled = true;
			}
			CharacterSave operator=(CharacterSave other) {
				magic = other.magic;
				version = other.version;
				memcpy(inUseNames, other.inUseNames, sizeof(inUseNames));
				needsPatching = other.needsPatching;
				customKartsEnabled = other.customKartsEnabled;
				return *this;
			}
		};
		static CharacterSave currSave;
		class CharacterEntry {
			public:
				std::string longName;
				std::string shortName;
				std::string folderName;
				u32 faceRaiderOffset;
				std::vector<u16> authors;
				ExtraResource::SARC* sarc;
				int achievementLevel;
				CharID origChar;
				bool creditsAllowed;
		};
		static std::vector<CharacterEntry> charEntries;
		static std::vector<CharacterEntry> enabledEntries;
		static std::vector<std::string> origCharNames;
		static std::vector<std::string> authorNames;
		static void Initialize();
		static u16  InsertAuthor(const std::string& author);
		static std::string GetAuthor(u16 index);
		static std::vector<std::string> GetAllAuthors();
		static void populateCharEntries();
		static void saveSettings();
		static void applyDirectoryPatches();
		static void ipsCallback(IPS::IFile& f);
		static void applySoundPatches();
		static void applySarcPatches();
		static void UpdateMsbtPatches();

		static bool LogPatchProgress(const Screen& screen);
		static std::string patchProgress1;
		static std::string patchProgress2;

		static void copyFile(u8* copyBuffer, u32 copyBufferSize, std::string &dst, std::string &ori, std::string& progStr, int currFile, int totFile);
		static void copyDirectory(std::string dst, std::string ori);
		static void OnFaceRaiderMenuEvent(Keyboard&, KeyboardEvent &event);
		static bool openFaceRaiderMenu(const CharacterEntry& entry);
		static void SetCharacterOption(Keyboard& kbd, u32 option);
		static s32 LoadCharacterImageTaskFunc(void* args);
		static void OnCharacterManagerMenuEvent(Keyboard&, KeyboardEvent &event);
		static void characterManagerSettings(MenuEntry* entry);
		static void	enableCustomKartsSettings(MenuEntry* entry);

		static bool thankYouFilterCharacterFiles;
		static void OnThankYouLoad(bool isBefore);
		static bool filterChararacterFileThankYou(u16* filename);

		static FileCopier* copier;
		static s32 FileCopy1(void* arg);
		static s32 FileCopy2(void* arg);
		static s32 FileCopy(FileCopier* cop, int ID);

		class FaceRaiderImage {
		public:
			FaceRaiderImage(u32 ID);
			~FaceRaiderImage();

			u16* pixelData;
			bool isLoaded;

			static u32 Initialize(const std::string& extraFacesPath);
			static void Finalize();
		private:
			static FS_Archive archive;
			static std::string facesDir;
			static u32 customFacesAmount;
		};
	private:

	};
}