/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CharacterHandler.hpp
Open source lines: 301/301 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "DataStructures.hpp"
#include "ExtraResource.hpp"
#include "GameAlloc.hpp"
#include "rt.hpp"
#include "array"
#include "memory"
#include "unordered_map"
#include "Math.hpp"
#include "BootSceneHandler.hpp"
#include "NetHandler.hpp"
#include "MarioKartFramework.hpp"
#include "SaveHandler.hpp"

namespace CTRPluginFramework {
    class CharacterHandler
    {
    public:
        static std::vector<std::string> pendingDirs;
        static BootSceneHandler::ProgressHandle progressHandle;
        class CharacterEntry {
			public:
                u64 id;
				std::string longName;
				std::string shortName;
				u32 faceRaiderOffset;
                std::string faceRainderBinDataPath;
				std::vector<u16> authors;
				std::shared_ptr<ExtraResource::StreamedSarc> chPack = nullptr;
				int achievementLevel;
                SaveHandler::SpecialAchievements specialAchievement;
				EDriverID origChar;
                int groupID;
                Color4 stdWingColorPole;
                Color4 stdWingColorSheet;
                struct {
                    u32 usesCustomWingColor : 1;
                    u32 creditsAllowed : 1;
                    u32 disableAngry : 1;
                    u32 selectAllowed : 1;
                    u32 rotateChara : 1;
                };
		};
        static void RegisterProgress();
        static void ProcessChPack(const std::string& it);
        static void Initialize();
        static void UpdateMsbtPatches();

        static u32 GetCharCount(EDriverID id) {
            return charEntriesPerDriverID[id].size();
        }

        static CharacterEntry& GetCharEntry(EDriverID id, u32 index) {
            if (index > GetCharCount(id))
                return defaultCharEntry;
            return *charEntriesPerDriverID[id][index];
        }

        static void ApplyCharacter(int playerID, u64 charID) {
            selectedCharaceters[playerID] = charID;
        }
        static u64 GetSelectedCharacter(int playerID);
        static void ApplyMenuCharacter(u64 charID) {
            selectedMenuCharacter = charID;
        }
        static u64 GetSelectedMenuCharacter() {
            return selectedMenuCharacter;
        }
        static std::unordered_map<u64, CharacterEntry>& GetCharEntries() {
            return charEntries;
        }
        static u64 SelectRandomCharacter(EDriverID driverID, bool includeVanilla, bool isCredits = false);
        static void ConfirmCharacters();
        static void ResetCharacters();

        static void EnableOnlineLock();
        static void DisableOnlineLock();
        static void WaitOnlineLock();

        static void SaveCharacterChoices();
        static void LoadCharacterChoices();

        static void LoadDisabledChars();
        static void SaveDisabledChars();

        static u16  InsertAuthor(const std::string& author);
		static std::string GetAuthor(u16 index);
		static std::vector<std::string> GetAllAuthors();
        static void OnGetCharaTextureName(FixedStringBase<char, 64>& outName, ECharaIconType iconType, EDriverID* driverID, int playerID);
        static u8* OnGetCharaTexture(SafeStringBase* file, ExtraResource::SARC::FileInfo* fileInfo);
        static void setupExtraResource();
        static void applySarcPatches();
        static void UpdateMenuCharaText(EDriverID driverID, u64 characterID, bool isBlocked);
        static void UpdateMenuCharaTextures(EDriverID driverID, u8* texture, u8* texture_sh, bool forceReset);
        static void UpdateRaceCharaText();
        static void* OnKartUnitConstructor(void* own, int playerID, CRaceInfo& raceInfo, bool isHiRes);
        static void OnMenu3DLoadResourceTask(void* own);
        static void OnMenuCharaLoadSelectUI(u32* archive, SafeStringBase* file);
        static void OnMenuCharaLoadKartUI(u32* archive, SafeStringBase* file);
        static void OnKartDirectorCreateBeforeStruct(u32* kartDirector, u32* args);
        static void OnPreviewPartsWingReplaceColor(void* own);

        static void* OnLoadResGraphicsFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs);
        static void OnThankYouScreen(bool isBefore);

        static void OnWatchRaceEnabled(bool enabled);

        static inline u32 GetSoundMenuWARCID(EDriverID driverID) {
            if (driverID == EDriverID::DRIVER_SHYGUY)
                return 0x05000000 + 43;
            else
                return 0x05000000 + 82;
        }
        static inline s32 GetSoundMenuFileID(EDriverID driverID, bool isGo) {
            if (soundMenuFileID[(u32)driverID] < 0)
                return -1;
            return soundMenuFileID[(u32)driverID] + (isGo ? 1 : 0);
        }
        static inline u32 GetSoundWARCID(EDriverID driverID, bool isCannon) {
            if ((driverID == EDriverID::DRIVER_KOOPATROOPA && isCannon) || soundBankOffset[(u32)driverID] < 0) {
                return 0;
            }
            const u32 soundIDBase = 0x05000000;
            return soundIDBase + soundBankOffset[(u32)driverID] + (isCannon ? 1 : 0);
        }
        static inline u32 GetSoundGolWARCID(EDriverID driverID) {
            if (soundGolBankOffset[(u32)driverID] < 0) {
                return 0;
            }
            const u32 soundIDBase = 0x05000000;
            return soundIDBase + soundGolBankOffset[(u32)driverID];
        }
        static inline u32 GetSoundWARCCount(EDriverID driverID, bool isCannon) {
            u32 count = soundBankCount[(u32)driverID];
            if (count == 0)
                return 0;
            return isCannon ? 2 : count;
        }
        static inline u32 GetSoundGolWARCCount(EDriverID driverID) {
            if (soundGolBankOffset[(u32)driverID] < 0) {
                return 0;
            }
            return 9;
        }

        enum class HeapVoiceMode {
            NONE,
            RACE,
            MENU,
        };
        static HeapVoiceMode heapVoiceMode;
        static void ClearAllVoices();
        static void SetupRaceVoices(u32* vehicle, CRaceInfo& raceInfo);
        static void SetupMenuVoices(u64 selectedEntry);

        static void StartFetchOnlineSelectedCharacters();
        static void ApplyOnlineSelectedCharacters();

        static void PopulateAvailableCharacters();

        static RT_HOOK getCharaTextureNameHook;
        static RT_HOOK kartUnitConstructorHook;
        static RT_HOOK kartDirectorCreateBeforeStructHook;
        static RT_HOOK menu3DLoadResourceTaskHook;
        static RT_HOOK previewPartsWingReplaceColorHook;

        static Color4** menuStdWingPoleColors;
        static Color4** menuStdWingSheetColors;
        static Color4* raceStdWingPoleColors;
        static Color4* raceStdWingSheetColors;
        
        static std::string potentialCrashReason;

        static bool customKartsEnabled;

        static void CustomCharacterManagerMenu(MenuEntry* entry);
        static void enableCustomKartsSettings(MenuEntry* entry);
    private:

        static Mutex characterHandlerMutex;

        struct FileLoadInfo {
            bool allow{};  
            bool isMenu{};
            bool isCredits{};
            int playerID{};
        };
        
        static const s8 soundMenuFileID[EDriverID::DRIVER_SIZE];
        static const s8 soundGolBankOffset[EDriverID::DRIVER_SIZE];
        static const s8 soundBankOffset[EDriverID::DRIVER_SIZE];
        static const s8 soundBankCount[EDriverID::DRIVER_SIZE];
        static GameAlloc::BasicHeap soundHeap;

        static const char* shortNames[EDriverID::DRIVER_SIZE];
		static const char* selectBclimNames[EDriverID::DRIVER_SIZE];
        static const u8 wingColorOffset [EDriverID::DRIVER_SIZE];
        friend void MarioKartFramework::OnWifiFinishLoadDefaultPlayerSetting(int playerID);
		static const int msbtOrder[EDriverID::DRIVER_SIZE];
        static const char* bodyNames[EBodyID::BODY_SIZE];
        static const char* tireNames[ETireID::TIRE_SIZE];
        static const char* wingNames[EWingID::WING_SIZE];
        static const char* screwNames[EScrewID::SCREW_SIZE];
        static const EDriverID specialPartsDriver[3];
        static const char* bodyUINames[EBodyID::BODY_SIZE];
        static const char* tireUINames[ETireID::TIRE_SIZE];
        static const char* wingUINames[EWingID::WING_SIZE];
        static const u8 bodyUIHasDriver[EBodyID::BODY_SIZE];
        static const u8 wingUIHasDriver[EWingID::WING_SIZE];
        static CharacterEntry defaultCharEntry;
        
        static bool updateRaceCharaNamePending;
        static std::array<u64, MAX_PLAYER_AMOUNT> selectedCharaceters;
        static u64 selectedMenuCharacter;
		static std::vector<std::string> authorNames;
	    static std::array<std::string, EDriverID::DRIVER_SIZE> origCharNames;
        static std::unordered_map<u64, CharacterEntry> charEntries;
        static std::array<std::vector<CharacterEntry*>, EDriverID::DRIVER_SIZE> charEntriesPerDriverID;
        static std::array<std::array<u8*, MAX_PLAYER_AMOUNT>, ECharaIconType::CHARAICONTYPE_SIZE> textureDataPointers;
        static std::array<u8*, EDriverID::DRIVER_SIZE> menuSelectTextureDataPointers;
        static std::array<u8*, EDriverID::DRIVER_SIZE> originalSelectUIfiles;
        static u8* menuKartTextureDataPointers[3][EBodyID::BODY_SIZE][EDriverID::DRIVER_SIZE];
        static u8* originalMenuKartTextureDataPointers[3][EBodyID::BODY_SIZE][EDriverID::DRIVER_SIZE];
        
        static FileLoadInfo lastFileLoadInfo;

        static NetHandler::RequestHandler netRequestHandler;
        static bool onlineLockUsed;
        static LightEvent onlineCharacterFetchedEvent;
        
        class ResourceInfo {
        public:
            enum class Kind {
                UNKNOWN,
                DRIVER_MODEL,
                BODY_MODEL,
                TIRE_MODEL,
                WING_MODEL,
                SCREW_MODEL,
                EMBLEM_MODEL,
                SELECT_UI,
                BODY_UI,
                TIRE_UI,
                WING_UI,
                THANK_YOU_ANIM,
            };
            enum class Attribute {
                UNKNOWN,
                NONE,
                LOD,
                SHADOW,
            };
            Kind kind = Kind::UNKNOWN;
            int id = -1;
            int id2 = -1;
            Attribute attribute = Attribute::UNKNOWN;
        };

        static void* OnLoadKartFile(u32 resourceLoader, SafeStringBase* path, ResourceLoaderLoadArg* loadArgs, ResourceInfo& info);

        static ResourceInfo GetFileInfoFromPath(const char* path, ResourceLoaderLoadArg* loadArgs = nullptr);

        static u8* GetTexturePointer(ECharaIconType iconType, int playerID);
        static u8* GetMenuSelectTexturePointer(EDriverID driverID);
        static const char* GetCharaIconBCLIMName(ECharaIconType iconType);

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
        static FaceRaiderImage* g_faceMenuFaceRaiderImage;
        static void OnFaceRaiderMenuEvent(Keyboard&, KeyboardEvent &event);
		static bool openFaceRaiderMenu(const CharacterEntry& entry);
		static void SetCharacterOption(Keyboard& kbd, u32 option);
		static s32 LoadCharacterImageTaskFunc(void* args);
		static void OnCharacterManagerMenuEvent(Keyboard&, KeyboardEvent &event);

        CharacterHandler() = delete;
    };
}