/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Lang.hpp
Open source lines: 341/341 (100.00%)
*****************************************************/

#pragma once

#include "types.h"
#include "CTRPluginFramework/System/System.hpp"
#include "TextFileParser.hpp"
#include "MarioKartFramework.hpp"
#include "VisualControl.hpp"

#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>

#define NAME(key)           (Language::GetName(key))
#define NOTE(key)           (Language::GetNote(key))

namespace CTRPluginFramework
{
    class Language
    {
    public:
        static void     Initialize(void);
		static void     Import(void);
		static void		PopulateLang();

        static const std::string &GetName(const std::string &key);
        static const std::string &GetNote(const std::string &key);
		static const char* GetCurrLangID();

		static std::string GenerateOrdinal(u32 number);
		
		static void GetLanguagePostfix(SafeStringBase* mem);
		static const char* GetLanguagePostfixChar();
		static void UpdateLangSettings();
		static void ShowLangMenu(MenuEntry* entry);

		enum SZSID
		{
			COMMON = 0,
			MENU,
			RACE,
			THANKYOU,
			TROPHY,
			NONE
		};

		static void GetLangSpecificFile(u16* filename, SZSID id, bool isPatch);
		static void FixRegionSpecificFile(u16* filename);

		struct TranslationInfo
		{
			std::string ID;
			std::string Name;
			std::string EURPostfix;
			std::string USAPostfix;
			std::vector<u32> regWhiteList;
			std::vector<u32> regBlackList;
			u32 ordinalMode = 0;

			bool IsAllowed(u32 gameRegion) {
				bool allowed = regWhiteList.empty() || std::find(regWhiteList.begin(), regWhiteList.end(), gameRegion) != regWhiteList.end();
				bool blocked = !regBlackList.empty() && std::find(regBlackList.begin(), regBlackList.end(), gameRegion) != regBlackList.end();
				return allowed && !blocked;
			}
		};

		static std::vector<TranslationInfo> availableLang;
		static int currentTranslation;
		static char currPostfix[3];
		static bool availableSZS[NONE];

		class MsbtHandler
		{
		public:

			struct TXT2Section
			{
				u32 count;
				u32 entries[];

				u16* GetText(u32 entry) {
					if (entry < count)
						return (u16*)((u32)this + entries[entry]);
					else
						return nullptr;
				}
			};

			struct NLI1Section
			{
				u32 count;
				struct {
					u32 id;
					u32 txt2Entry;
				} entries[];

				u32 GetTxt2Entry(u32 id) {
					int lower = 0;
					int upper = count - 1;
					int middle;
					while (lower <= upper) { // Good old binary search
						middle = (lower + upper) / 2;
						if (entries[middle].id == id)
							return entries[middle].txt2Entry;
						else if (entries[middle].id < id) 
							lower = middle + 1;
						else
							upper = middle - 1;
					}
					return 0xFFFFFFFF;
				}
			};

			struct LMSSection
			{
				void* data;
				u32 sectionMagic;
				u32 sectionSize;
				u32 unknown;

				NLI1Section* AsNLI1() {
					return (NLI1Section*)data;
				}

				TXT2Section* AsTXT2() {
					return (TXT2Section*)data;
				}
			};
			
			struct LMSBinaryTag
			{
				void* msbtPtr;
				u32 msbtSize;
				u8 unknown;
				u8 padding;
				u16 numberSections;
				LMSSection* sections;
				int LBL1Count;
				int TXT2Count;
				int ATR1Count;
				int TSY1Count;

				NLI1Section* GetNLI1() {
					if (numberSections > 0)
						return sections[0].AsNLI1();
					else
						return nullptr;
				}

				u32 GetNLI1Size() {
					if (numberSections > 0)
						return sections[0].sectionSize;
					else
						return 0;
				}

				TXT2Section* GetTXT2() {
					if (TXT2Count != -1 && numberSections > 1)
						return sections[1].AsTXT2();
					else
						return nullptr;
				}

				u32 GetTXT2Size() {
					if (TXT2Count != -1 && numberSections > 1)
						return sections[1].sectionSize;
					else
						return 0;
				}
			};

			struct MessageData
			{
				u32 vtable;
				char* name;
				u32 nameBufferSize;
				char nameBuffer[0x20];
				LMSBinaryTag* tag;
				NLI1Section* nliptr;
				u32 startingMsgID;
			};

			struct MessageDataList
			{
				u32 size;
				u32 listSize;
				MessageData** list;

				MessageData* GetDataForID(u32 id) {
					int entry = listSize - 1;
					
					while (entry >= 0) {
						if ((u32)list[entry] < 0x14000000 || (u32)list[entry] > 0x17800000)
							entry--;
						else if (id >= list[entry]->startingMsgID)
							break;
						else
							entry--;
					}
					if (entry < 0) return nullptr;
					return list[entry];
				}
			};

			struct PluginMessageDataList
			{
				MessageDataList msg;
				struct PrevMsbtInfos
				{
					void* txt2StartPtr;
					void* txt2EndPtr;
				};
				PrevMsbtInfos* infos;

				PluginMessageDataList(u32 capacity) {
					msg.size = capacity;
					msg.listSize = capacity;
					msg.list = (MessageData**)operator new(capacity * sizeof(MessageData*));
					infos = (PrevMsbtInfos*)operator new(capacity * sizeof(PrevMsbtInfos));
				}

				~PluginMessageDataList() {
					operator delete(msg.list);
					operator delete(infos);
				}
			};

			struct ControlString {
				enum class Group : u16 {
					RENDERING = 0,
					STRFORMAT = 1,
				};
				enum class RenderingType : u16 {
					RUBY_TEXT = 0,
					FONT_SIZE = 2,
					FONT_COLOR = 3,
				};
				enum class DashColor {
					RED = 0,
					BLUE = 1,
					GREEN = 2,
					CUSTOM = 0x8000,
					RESET = 0xFFFF
				};
				struct Header {
					u16 magic; // Always 0xE
					Group group;
					u16 type;
					u16 argAmount;
				};
				Header header;
				u8 args[];

				u32 GetSizeBytes() const {
					return sizeof(Header) + header.argAmount;
				}

				bool GetChoice(u16* out, int choice) const;

				static string16 GenSizeControlString(s16 size);
				static string16 GenColorControlString(DashColor color, const Color& customColor = Color::White);
				static void GetColor(u32* out, u32 messageWritter, u16 color);
			};

			static MessageData* common;
			static MessageData* menu;
			static MessageData* race;
			static PluginMessageDataList* allList;

			static void GetMessage(VisualControl::Message& out, MessageData* data, u32 id);
			static void GetMessageFromList(VisualControl::Message& out, MessageDataList* list, u32 id);
			static void RecalculateCrossMsbtPtr(u32 msbtIndex, void* newStart);
			static void OnMessageDataConstruct(MessageData* data);
			static void ApplyCustomText();
			static void ApplyQueuedReferenceStr();

			static u32 GetTextLenNoFormatting(const u16* text);
			static void SkipTextControlString(u16* dst, const u16* src);
			static string16 DashTextWithTagsToString(const u16* src);
			static const ControlString* FindControlString(const u16* text, u16 group, u16 type, const u16** nextChar = nullptr);
			static const u16* GetText(u32 id);
			static std::string GetString(const u16* text);
			static std::string GetString(u32 id);
			static void SetText(u32 id, const u16* text, bool insertFront = false, u32 textSize = 0xFFFFFFFF);
			static void SetString(u32 id, const string16& text, bool insertFront = false) {SetText(id, text.data(), insertFront, text.size());}
			static void SetString(u32 id, const char* text, bool insertFront = false);
			static void SetString(u32 id, const std::string& text, bool insertFront = false) {SetString(id, text.c_str(), insertFront);}
			static void RemoveAllString(u32 id);
			static void SetTextEnabled(u32 id, bool enabled);
		};

		struct MessageString
        {
            VisualControl::Message message;
			MessageString* next;
			bool didCopy;
			bool isEnabled;

            MessageString(u16* src, bool makeCopy = false, u32 size = 0xFFFFFFFF);
            MessageString(const char* str);
            MessageString(const std::string& str) : MessageString(str.c_str()) {}
            ~MessageString() { if (didCopy) delete (u16*)message.data;}

			const VisualControl::Message* GetMessage() const {
				if (isEnabled) return &message;
				else return (next) ? next->GetMessage() : nullptr;
			}
        };

		static bool			msbtReady;		
		static void			doTextPatchesAfterMsbtReady();

    private:

		friend MsbtHandler;
		static std::unordered_map<int, MessageString*> customText;
		struct LangFileSettings {
			u32 magic;
			char langID[4];
			LangFileSettings() {
				magic = 0x474C4553;
				*(u32*)langID = 0;
			}
		};
		static LangFileSettings currSettings;
		static int currOrdinalMode;
		static void		loadSettings();
        static TextFileParser  currLanguage;
		static std::vector<std::pair<int,int>> queuedReferenceStr;
		static void LoadGameReplacement();        
    };
}