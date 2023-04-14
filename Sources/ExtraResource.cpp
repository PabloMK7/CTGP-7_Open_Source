/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraResource.cpp
Open source lines: 408/579 (70.47%)
*****************************************************/

#include "DataStructures.hpp"
#include "CharacterManager.hpp"
#include "CourseManager.hpp"
#include "VersusHandler.hpp"
#include "MenuPage.hpp"
#include "MissionHandler.hpp"
#include "UserCTHandler.hpp"
#include "Sound.hpp"
#include "cheats.hpp"
#include "StaffRoll.hpp"
#include "CrashReport.hpp"

namespace CTRPluginFramework {
	
	ExtraResource::MultiSARC* ExtraResource::mainSarc = nullptr;
	ExtraResource::SARC* ExtraResource::graphicsSarc = nullptr;

	bool ExtraResource::lastTrackFileValid = false;
	u32 ExtraResource::kartPtclFileOffset = 0;
	ExtraResource::SARC::FileInfo ExtraResource::kartPtclFileInfo;


	void ExtraResource::setupExtraResource() {
		if (!GameAlloc::gameHeap) panic("Game heap missing.");
		MissionHandler::setupExtraResource();
#ifdef BETA_GHOST_FILE
		File extraSarc("/CTGP-7/resources/ghostsBeta.bin", File::READ);
#else
		File extraSarc("/CTGP-7/resources/ghosts.bin", File::READ); //Cannot contain GPU data
#endif
		if (!extraSarc.IsOpen()) panic("ghosts.bin missing or corrupted.");
		u32 fileSize = extraSarc.GetSize() - sizeof(ExtraResourceFormat);
		if (fileSize < 0x100 || fileSize > 1000000) panic("ghosts.bin missing or corrupted.");
		u8* extraResource = (u8*)memalign(0x100, fileSize);
		if (!extraResource) panic("ghosts.bin missing or corrupted.");
		ExtraResourceFormat info;
		extraSarc.Read(&info, sizeof(ExtraResourceFormat));
		mainSarc = new MultiSARC(new SARC(extraResource, false));
#ifdef BETA_SARC_FILE
		File graphicdata("/CTGP-7/resources/CTGP-7Beta.sarc");
#else
		File graphicdata("/CTGP-7/resources/CTGP-7.sarc");
#endif
		graphicsSarc = new SARC(graphicdata);
		mainSarc->Append(graphicsSarc, true);
		CharacterManager::applySarcPatches();
		VersusHandler::Initialize();
		Snd::InitializeMenuSounds();
		ItemHandler::Initialize();
		CrashReport::Initialize();
	}

	static inline void concatSZSCustomName(char* dst, const char* archive, const char* file) {
		while (*archive != 0) *dst++ = *archive++;
		*dst++ = '/';
		while (*file != 0) *dst++ = *file++;
		*dst = '\0';
	}

	static void getFileFromArchive(char* out, char* in) {
		char* startFrom = in;
		while (*in) {
			if (*in == '/') startFrom = in + 1;
			in++;
		}
		while (*startFrom && *startFrom != '.') {
			*out++ = *startFrom++;
		}
		*out = '\0';
	}
	u8* ExtraResource::loadExtraFile(u32* archive, SafeStringBase* file, SARC::FileInfo* fileInfo) {
		char* archiveN = (char*)archive[-5];
		if (archiveN[2] != '/' && archiveN[3] == 'r') {
			u32 len = strlen(file->data);
			if (MissionHandler::isMissionMode) return MissionHandler::GetReplacementFile(file, fileInfo, (u8*)(archive[0xE] - 0x14));
			u8* replFile = UserCTHandler::LoadTextureFile(archive, file, fileInfo);
			if (replFile) return replFile;
		}
		if (!strcmp(file->data, "itemBox/itemBox.bcmdl")) {
			u8* ret = ItemHandler::FakeBoxHandler::GetFakeItemBox(fileInfo);
			if (!ret && MissionHandler::isMissionMode) ret = MissionHandler::GetReplacementItemBox(fileInfo);
			if (ret) return ret;
		}
		if (!strcmp(file->data, "Effect/Kart/Kart.ptcl")) {
			if (!kartPtclFileOffset) {
				SARC sarc((u8*)(archive[0xE] - 0x14), false);
				u8* ret = sarc.GetFile("Effect/Kart/Kart.ptcl", &kartPtclFileInfo);
				kartPtclFileOffset = (u32)(ret - ((u8*)(archive[0xE] - 0x14)));
			}
			ItemHandler::FakeBoxHandler::PatchKartPctl(((u8*)(archive[0xE] - 0x14)) + kartPtclFileOffset);
			if (fileInfo)
				*fileInfo = kartPtclFileInfo;
			return ((u8*)(archive[0xE] - 0x14)) + kartPtclFileOffset;
		}
		if (VersusHandler::IsVersusMode && SaveHandler::saveData.vsSettings.cpuOption != VersusHandler::VSCPUDifficulty::STANDARD) {
			if (strstr(file->data, "Enemy/EnemyProbabilityTable")) {
				SafeStringBase newFile(SaveHandler::saveData.vsSettings.cpuOption == VersusHandler::VSCPUDifficulty::VERY_HARD ?
				 "RaceCommon.szs/Enemy/EnemyProbabilityTableVeryHard.bin" :
				 "RaceCommon.szs/Enemy/EnemyProbabilityTableInsane.bin");
				return mainSarc->GetFile(newFile, fileInfo);
			}
		}
		if (MissionHandler::isMissionMode) {
			if (!strcmp(file->data, "mode_cd_r90.bclim")) {
				u8* ret = MissionHandler::GetReplacementScoreIcon(fileInfo);
				if (ret) return ret;
			}
			if (!strcmp(file->data, "Coin_lod/Coin_lod.bcmdl")) {
				u8* ret = MissionHandler::GetReplacementCoin(fileInfo, true);
				if (ret) return ret;
			}
			if (!strcmp(file->data, "Coin_detail/Coin_detail.bcmdl")) {
				u8* ret = MissionHandler::GetReplacementCoin(fileInfo, false);
				if (ret) return ret;
			}
		}

		if (MenuPageHandler::MenuEndingPage::loadCTGPCredits && !strcmp(file->data, "top_replace_target.bclim")) {
			MenuPageHandler::MenuEndingPage* page = MenuPageHandler::MenuEndingPage::GetInstance();
			if (page && page->staffroll && page->staffroll->GetStaffRollImage()) {
				if (fileInfo) {
					fileInfo->dataStart = 0;
					fileInfo->fileSize = 0x4028;
				}
				return page->staffroll->GetStaffRollImage()->GetBclimData();
			}
		}
		
		char newFileName[0x100];
		if (strlen(archiveN) + strlen(file->data) + 2 > 0x100) panic("Invalid SARC file name.");
		concatSZSCustomName(newFileName, archiveN, file->data);
		SafeStringBase newFile(newFileName);
		return mainSarc->GetFile(newFile, fileInfo);
	}

	void ExtraResource::reloadGraphicsSarc() {
		if (!graphicsSarc) return;
		#ifdef BETA_SARC_FILE
				File graphicdata("/CTGP-7/resources/CTGP-7Beta.sarc");
		#else
				File graphicdata("/CTGP-7/resources/CTGP-7.sarc");
		#endif
		graphicsSarc->Reload(graphicdata);
	}

	ExtraResource::SARC::SARC(File& sarcFile, bool loadToMemory)
	{
		processed = false;
		isInMemory = loadToMemory;
		if (!sarcFile.IsOpen()) return;
		sarcFile.Seek(0, File::SET);
		u32 offset = 0;
		sarcFile.Read(&header, sizeof(Header));
		if (sarcFile.Tell() - offset != sizeof(Header)) return;
		if (header.magic != 0x43524153) return;
		u32 toRead = header.dataOffset - header.headerLength;
		u8* buff = (u8*)::operator new(toRead);
		if (!buff) return;
		sarcFile.Seek(header.headerLength, File::SET);
		offset = sarcFile.Tell();
		sarcFile.Read(buff, toRead);
		if (sarcFile.Tell() - offset != toRead) {
			::operator delete(buff);
			return;
		}
		SFAT* sfat = (SFAT*)buff;
		if (sfat->magic != 0x54414653) {
			::operator delete(buff);
			return;
		}
		hashMultiplier = sfat->hashMultiplier;
		SFATEntry* nodes = (SFATEntry*)(buff + sfat->headerLength);
		for (u32 i = 0; i < sfat->nodeCount; i++) {
			entries.push_back(nodes[i]);
		}
		
		if (isInMemory) {
			sarcFile.Seek(header.dataOffset, File::SET);
			offset = sarcFile.Tell();
			toRead = header.fileLength - header.dataOffset;
			fileData = (u8*)GameAlloc::MemAlloc(toRead, 0x80);
			if (!fileData) {
				::operator delete(buff);
				return;
			}
			sarcFile.Read(fileData, toRead);
			if (sarcFile.Tell() - offset != toRead) {
				::operator delete(buff);
				return;
			}
			svcFlushProcessDataCache(CUR_PROCESS_HANDLE,((u32)fileData & ~0xFFF), (toRead & ~0xFFF) + 0x1000);
		} else fileData = nullptr;
		::operator delete(buff);
		processed = true;
 	}
	ExtraResource::SARC::SARC(u8* data, bool allocateHeap)
	{
		isInMemory = true;
		allocatedInHeap = allocateHeap;
		processed = false;
		memcpy(&header, data, sizeof(Header));
		if (header.magic != 0x43524153) return;
		SFAT* sfat = (SFAT*)(data + header.headerLength);
		if (sfat->magic != 0x54414653) return;
		hashMultiplier = sfat->hashMultiplier;
		SFATEntry* nodes = (SFATEntry*)(data + header.headerLength + sfat->headerLength);
		for (u32 i = 0; i < sfat->nodeCount; i++) {
			entries.push_back(nodes[i]);
		}
		u32 dataSize = header.fileLength - header.dataOffset;
		if (allocatedInHeap) {
			fileData = (u8*)GameAlloc::MemAlloc(dataSize, 0x80);
			if (!fileData) return;
			memcpy(fileData, data + header.dataOffset, dataSize);
			svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)fileData & ~0xFFF), (dataSize & ~0xFFF) + 0x1000);
		} else fileData = data + header.dataOffset;
		processed = true;
	}

	u32 ExtraResource::SARC::GetHash(const char* name)
	{
		return processed ? CalculateHash<char>(name, hashMultiplier) : 0;
	}


	u8* ExtraResource::SARC::GetFile(const char* name, FileInfo* info)
	{
		if (!processed) return nullptr;
		u32 hash = GetHash(name);
		return GetFile(hash, info);
	}
	
	u8* ExtraResource::SARC::GetFile(u32 fileHash, FileInfo* info) {
		if (!enabled)
			return nullptr;
		SFATEntry entry;
		entry.nameHash = fileHash;
		auto it = std::lower_bound(entries.begin(), entries.end(), entry, [](SFATEntry a, SFATEntry b) { return a.nameHash < b.nameHash; });
		if (it == entries.end() || (*it).nameHash != fileHash) return nullptr;
		if (info) {
			info->dataStart = (*it).startOffset;
			info->fileSize = (*it).endOffset - info->dataStart;
		}
		if (!isInMemory) return nullptr;
		else {
			u8* ret = fileData + (*it).startOffset;
		}
	}

	void ExtraResource::SARC::Reload(File& sarcFile) {
		if (!processed || !sarcFile.IsOpen() || !isInMemory) return;
		sarcFile.Seek(header.dataOffset, File::SET);
		u32 toRead = header.fileLength - header.dataOffset;
		sarcFile.Read(fileData, toRead);
		svcFlushProcessDataCache(CUR_PROCESS_HANDLE,((u32)fileData & ~0xFFF), (toRead & ~0xFFF) + 0x1000);
	}

	ExtraResource::StreamedSarc::StreamedSarc(const std::string& fileName, u32 bufferSize) {
		_file = new File(fileName, File::READ);
		_sarc = new SARC(*_file, false);
		processed = _sarc->processed;
		_buffer = (u8*)memalign(0x1000, bufferSize);
		_bufferSize = bufferSize;
		_fileSize = _file->GetSize();
	}
	
	ExtraResource::StreamedSarc::~StreamedSarc() {
		free(_buffer);
		delete _sarc;
		delete _file;
	}

	bool ExtraResource::StreamedSarc::SetFile(const char* name, SARC::FileInfo* info) {
		currFileOffset = 0;
		info->fileSize = -1;
		_sarc->GetFile(name, info);
		if (info->fileSize == -1) return false;
		currFileOffset = _sarc->header.dataOffset + info->dataStart;
		if (_file->Seek(currFileOffset, File::SeekPos::SET) != File::OPResult::SUCCESS)
			return false;
		return true;
	}

	const u8* ExtraResource::StreamedSarc::GetData(s32 offset) {
		if (!currFileOffset) return nullptr;
		if (offset >= 0)
			if (_file->Seek(currFileOffset + offset, File::SeekPos::SET) != File::OPResult::SUCCESS)
				return nullptr;
		u32 readSize = ((_fileSize - _file->Tell() < _bufferSize) ? (_fileSize - _file->Tell()) : (_bufferSize));
		if (_file->Read(_buffer, readSize) == File::OPResult::SUCCESS)
			return _buffer;
		else
			return nullptr;
	}

	const bool ExtraResource::StreamedSarc::ReadFile(void* dest, SARC::FileInfo& destFileInfo, const char* name, bool allowCropSize) {
		if (!dest || !destFileInfo.fileSize || !_sarc->processed || !_sarc->IsEnabled())
			return false;
		SARC::FileInfo info;
		bool fileFound = SetFile(name, &info);
		if (!fileFound || (info.fileSize > destFileInfo.fileSize && !allowCropSize))
			return false;
		u32 destSize = std::min(info.fileSize, destFileInfo.fileSize);
		u32 offset = 0;
		while (destSize) {
			const u8* data = GetData();
			if (!data)
				return false;
			u32 toCopy = (destSize > _bufferSize) ? _bufferSize : destSize;
			destSize -= toCopy;
			memcpy((u8*)dest + offset, data, toCopy);
			offset += toCopy;
		}
		svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)dest & ~0xFFF), (info.fileSize & ~0xFFF) + 0x2000);
		destFileInfo.fileSize = info.fileSize;
		return true;
	}


	ExtraResource::MultiSARC::MultiSARC(SARC* start)
	{
		if (start->processed == false) panic("MultiSARC not initialised.");
		this->Append(start, true);
	}
	void ExtraResource::MultiSARC::Append(SARC* newSarc, bool destroyOri)
	{
		if (!newSarc->processed) return;
		u32 currID = sarcFiles.size();
		NaturalMerge(newSarc->entries, currID);
		sarcFiles.push_back(newSarc);
		if (destroyOri) {
			newSarc->entries.clear(); newSarc->entries.shrink_to_fit();
		}
	}
	inline u8* ExtraResource::MultiSARC::GetFile(SafeStringBase& name, SARC::FileInfo* info)
	{
		return GetFile(name.data, info);
	}
	u8* ExtraResource::MultiSARC::GetFile(const std::string& name, SARC::FileInfo* info)
	{
		return GetFile(name.c_str(), info);
	}
	u8* ExtraResource::MultiSARC::GetFile(const char* name, SARC::FileInfo* info)
	{
		if (!enabled)
			return nullptr;
		u32 hash = sarcFiles[0]->GetHash(name);
		MultiSFATEntry entry(SARC::SFATEntry(), 0);
		entry.entry.nameHash = hash;
		auto it = std::lower_bound(entries.begin(), entries.end(), entry, [](MultiSFATEntry a, MultiSFATEntry b) { return a.entry.nameHash < b.entry.nameHash; });
		if (it == entries.end() || (*it).entry.nameHash != hash || !sarcFiles[(*it).sarcID]->IsEnabled()) return nullptr;
		if (info) {
			info->dataStart = (*it).entry.startOffset;
			info->fileSize = (*it).entry.endOffset - info->dataStart;
		}
		return sarcFiles[(*it).sarcID]->fileData + (*it).entry.startOffset;
	}
	void ExtraResource::MultiSARC::UpdateFileSize(void* data, u32 newSize)
	{
		u8* dataU8 = (u8*)data;
		for (auto it = entries.begin(); it != entries.end(); it++) {
			if (dataU8 == (sarcFiles[(*it).sarcID]->fileData + (*it).entry.startOffset)) {
				(*it).entry.endOffset = (*it).entry.startOffset + newSize;
				break;
			}
		}
	}
	void ExtraResource::MultiSARC::NaturalMerge(std::vector<SARC::SFATEntry>& mergeVec, u32 id)
	{
		auto oriIt = entries.begin();
		auto otherIt = mergeVec.begin();
		while (oriIt != entries.end() && otherIt != mergeVec.end())
		{
			if ((*oriIt).entry.nameHash < (*otherIt).nameHash) oriIt++;
			else if ((*oriIt).entry.nameHash == (*otherIt).nameHash) {
				if ((*oriIt).sarcID != 0) {
					u32 continueAt = (oriIt - entries.begin()) + 1;
					MultiSFATEntry newEntry(*otherIt, id);
					entries.insert(oriIt, newEntry);
					oriIt = entries.begin() + continueAt;
					entries.erase(oriIt);
					oriIt = entries.begin() + continueAt;
				}
				otherIt++;
			}
			else {
				u32 continueAt = (oriIt - entries.begin()) + 1;
				MultiSFATEntry newEntry(*otherIt++, id);
				entries.insert(oriIt, newEntry);
				oriIt = entries.begin() + continueAt;
			}
		}
		while (otherIt != mergeVec.end()) {
			MultiSFATEntry newEntry(*otherIt++, id);
			entries.push_back(newEntry);
		}
	}
}

