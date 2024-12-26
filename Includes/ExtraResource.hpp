/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraResource.hpp
Open source lines: 210/229 (91.70%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "DataStructures.hpp"
#include "GameAlloc.hpp"
#include "malloc.h"

namespace CTRPluginFramework {
	class ExtraResource {
		public:
			class SARC
			{
			public:
				struct Header {
					u32 magic;
					u16 headerLength;
					u16 byteOrder;
					u32 fileLength;
					u32 dataOffset;
					u32 unknown;
				};
				struct SFAT {
					u32 magic;
					u16 headerLength;
					u16 nodeCount;
					u32 hashMultiplier;
				};
				struct SFATEntry  {
					u32 nameHash;
					u32 nameOffset;
					u32 startOffset;
					u32 endOffset;
				};
				struct FileInfo {
					u32 dataStart{};
					u32 fileSize{};
				};
				Header header;
				u32 hashMultiplier;
				std::vector<SFATEntry> entries;
				u8* fileData;
				bool allocatedInHeap;
				bool isInMemory;
				bool processed;
				bool enabled = true;
				
				SARC(File& sarcFile, bool loadToMemory = true);
				SARC(u8* data, bool allocateHeap = true);

				u32 GetHash(const char* name);
				u32 GetFileCount() const {
					return entries.size();
				}

				template <typename T>
				static inline u32 CalculateHash(const T* name, u32 hashMultiplier)
				{
					u32 result = 0;
					while (*name) {
						result = *name++ + (result * hashMultiplier);
					}
					return result;
				}

				inline u8* GetFile(SafeStringBase& name, FileInfo* info)
				{
					return GetFile(name.data, info);
				}
				inline u8* GetFile(const std::string& name, FileInfo* info)
				{
					return GetFile(name.c_str(), info);
				}
				u8* GetFile(const char* name, FileInfo* info);
				u8* GetFile(u32 fileHash, FileInfo* info);

				void Reload(File& sarcFile);

				bool IsEnabled() {return enabled;}
				void SetEnabled(bool enabled) {this->enabled = enabled;}

			};
			class StreamedSarc {
			public:
				
				inline const bool ReadFile(void* dest, SARC::FileInfo& destSize, SafeStringBase& name, bool allowCropSize = false) {
					return ReadFile(dest, destSize, name.data, allowCropSize);
				}

				inline const bool ReadFile(void* dest, SARC::FileInfo& destSize, const std::string& name, bool allowCropSize = false) {
					return ReadFile(dest, destSize, name.c_str(), allowCropSize);
				}

				const bool ReadFile(void* dest, SARC::FileInfo& destSize, const char* name, bool allowCropSize = false);

				const bool ReadFileDirectly(void* dest, SARC::FileInfo& destSize, SARC::FileInfo& inputFileInfo, bool allowCropSize = false);

				template <typename T>
				bool ReadFullFile(T& outData, const std::string& name) {
					SARC::FileInfo info;
					bool found = GetFileInfo(info, name);
					if (found) {
						SARC::FileInfo dstFileInfo = info;
						outData.resize(dstFileInfo.fileSize);
						return ReadFileDirectly(outData.data(), dstFileInfo, info);
					}
					return found;
				}

				inline const bool GetFileInfo(SARC::FileInfo& outFileInfo, SafeStringBase& name) {
					return GetFileInfo(outFileInfo, name.data);
				}

				inline const bool GetFileInfo(SARC::FileInfo& outFileInfo, const std::string& name) {
					return GetFileInfo(outFileInfo, name.c_str());
				}

				const bool GetFileInfo(SARC::FileInfo& outFileInfo, const char* name);

				u64 GetNonSecureDataChecksum(u32 bufferSize = 0x4000);
				u32 GetFileCount() const {
					if (!processed) return 0;
					return _sarc->GetFileCount();
				}
				const std::string& GetFileName() {return _fileName;}
				const u32 GetOffsetInSARCFile(const SARC::FileInfo& fInfo) {
					return _sarc->header.dataOffset + fInfo.dataStart;
				}

				StreamedSarc() {}
				StreamedSarc(const std::string& fileName, u32 bufferSize = 0x1000);
				~StreamedSarc();
				bool IsEnabled() {return _sarc->IsEnabled();}
				void SetEnabled(bool enabled);

				bool processed = false;
			private:
				SARC* _sarc = nullptr;
				File* _file = nullptr;
				std::string _fileName = "";
				u32 _fileSize = 0;

				inline bool SetFile(SafeStringBase& name, SARC::FileInfo* info)
				{
					return SetFile(name.data, info);
				}
				inline bool SetFile(std::string& name, SARC::FileInfo* info)
				{
					return SetFile(name.c_str(), info);
				}
				bool SetFile(const char* name, SARC::FileInfo* info);

				bool SetFileDirectly(SARC::FileInfo* info);

				const void* GetData(void* dest, u32 size);
			};
			class MultiSARC
			{
			public:
				struct MultiSFATEntry
				{
					u32 sarcID;
					SARC::SFATEntry entry;
					MultiSFATEntry(SARC::SFATEntry sfatEntry, u32 ID) {
						entry = sfatEntry;
						sarcID = ID;
					}
				};
				std::vector<SARC*> sarcFiles;
				std::vector<MultiSFATEntry> entries;
				bool enabled = true;

				MultiSARC(SARC* start);
				void Append(SARC* newSarc, bool destroyOri);
				u8* GetFile(SafeStringBase& name, SARC::FileInfo* info);
				u8* GetFile(const char* name, SARC::FileInfo* info);
				u8* GetFile(const std::string& name, SARC::FileInfo* info);
				void UpdateFileSize(void* data, u32 newSize);

				bool IsEnabled() {return enabled;}
				void SetEnabled(bool enabled) {this->enabled = enabled;}
			private:
				void NaturalMerge(std::vector<SARC::SFATEntry>& mergeVec, u32 id);
			};
			struct ExtraResourceFormat {
			};
			static MultiSARC* mainSarc;
			static SARC* graphicsSarc;
			
			static void setupExtraResource();
			static u8* loadExtraFile(u32* archive, SafeStringBase* file, SARC::FileInfo* fileInfo);
			static void reloadGraphicsSarc();

			static bool lastTrackFileValid;
			static u32 kartPtclFileOffset;
			static SARC::FileInfo kartPtclFileInfo;

			static u32* latestMenuSzsArchive;
			static bool isValidMenuSzsArchive();
		private:
			static constexpr u32 SARCVER = 0xFABA0001;
	};
}