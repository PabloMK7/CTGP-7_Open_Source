/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraResource.cpp
Open source lines: 552/727 (75.93%)
*****************************************************/

#include "DataStructures.hpp"
#include "CourseManager.hpp"
#include "VersusHandler.hpp"
#include "MenuPage.hpp"
#include "MissionHandler.hpp"
#include "UserCTHandler.hpp"
#include "Sound.hpp"
#include "main.hpp"
#include "StaffRoll.hpp"
#include "CrashReport.hpp"
#include "CharacterHandler.hpp"
#include "BlueCoinChallenge.hpp"
#include "StatsHandler.hpp"

namespace CTRPluginFramework {
	
	ExtraResource::MultiSARC* ExtraResource::mainSarc = nullptr;
	ExtraResource::SARC* ExtraResource::graphicsSarc = nullptr;

	bool ExtraResource::lastTrackFileValid = false;
	u32 ExtraResource::kartPtclFileOffset = 0;
	ExtraResource::SARC::FileInfo ExtraResource::kartPtclFileInfo;

	u32* ExtraResource::latestMenuSzsArchive = 0;


	void ExtraResource::setupExtraResource() {
		if (!GameAlloc::gameHeap) panic("Game heap missing.");
		MissionHandler::setupExtraResource();
		CharacterHandler::setupExtraResource();
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
		
		SARC::FileInfo sarc_finfo;
		u32* sarc_version = (u32*)mainSarc->GetFile("sarc_version.bin", &sarc_finfo);
		if (sarc_version == nullptr || sarc_finfo.fileSize != 4 || *sarc_version != SARCVER) panic("CTGP-7.sarc outdated or corrupted.");

		VersusHandler::Initialize();
		Snd::InitializeMenuSounds();
		ItemHandler::Initialize();
		CrashReport::Initialize();
		BlueCoinChallenge::Initialize();
		CharacterHandler::applySarcPatches();
		BadgeManager::SetupExtraResource();
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
		u32 len = strlen(file->data);
		if (archiveN[2] == '/' && archiveN[3] == 'm' && archiveN[7] == '.') {
			latestMenuSzsArchive = archive;
		}
		// Handle custom character texture
		if (file->data[len - 1] == 0xFF) {
			return CharacterHandler::OnGetCharaTexture(file, fileInfo);
		}
		if (archiveN[2] == '/' && archiveN[3] == 'm' && archiveN[7] == '.') {
			if (!strncmp(file->data, "select_", 7)) {
				CharacterHandler::OnMenuCharaLoadSelectUI(archive, file);
			} else if (!strncmp(file->data, "b_", 2) || !strncmp(file->data, "t_", 2) || !strncmp(file->data, "g_", 2)) {
				CharacterHandler::OnMenuCharaLoadKartUI(archive, file);
			}
		}
		if (archiveN[2] != '/' && archiveN[3] == 'r') {
			if (MissionHandler::isMissionMode) return MissionHandler::GetReplacementFile(file, fileInfo, (u8*)(archive[0xE] - 0x14));
			if (BlueCoinChallenge::coinSpawned && !strcmp(file->data, "IceBoundStar/IceBoundStar.bcmdl")) {
				return mainSarc->GetFile((BlueCoinChallenge::IsCoinCollected(CourseManager::lastLoadedCourseID) || BlueCoinChallenge::coinDisabledCCSelector) ?
					"RaceCommon.szs/blueCoin/blueCoinCollected.bcmdl" :
					"RaceCommon.szs/blueCoin/blueCoin.bcmdl"
					 , fileInfo);
			}
			if (BlueCoinChallenge::coinSpawned && !strcmp(file->data, "IceBoundStar/IceBoundStar.kcl")) {
				return mainSarc->GetFile("RaceCommon.szs/blueCoin/blueCoin.kcl", fileInfo);
			}
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

	bool ExtraResource::isValidMenuSzsArchive() {
		if (!latestMenuSzsArchive)
			return false;
		u32 value32;
		u8 value8;
		if (!Process::Read32((u32)&latestMenuSzsArchive[-5], value32))
			return false;
		char* archiveN = (char*)value32;
		if (
			!Process::Read8((u32)&archiveN[2], value8) || value8 != '/' ||
			!Process::Read8((u32)&archiveN[3], value8) || value8 != 'm' ||
			!Process::Read8((u32)&archiveN[7], value8) || value8 != '.'
		)
			return false;
		if (!Process::Read32((u32)&latestMenuSzsArchive[0xE], value32))
			return false;
		if (!Process::Read32(value32 - 0x14, value32) && value32 != 0x43524153)
			return false;
		return true;
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
			return (u8*)CryptoResource::ProcessCryptoFile(ret);
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
		_fileName = fileName;
		_file = new File(fileName, File::READ);
		_sarc = new SARC(*_file, false);
		processed = _sarc->processed;
		if (!processed) {
			delete _sarc;
			delete _file;
		} else {
			_fileSize = _file->GetSize();
		}
	}
	
	ExtraResource::StreamedSarc::~StreamedSarc() {
		if (processed) {
			delete _sarc;
			delete _file;
		}
	}

	void ExtraResource::StreamedSarc::SetEnabled(bool enabled) {
		if (!processed)
			return;
		_sarc->SetEnabled(enabled);
		if (enabled && !_file->IsOpen()) {
			File::Open(*_file, _fileName, File::READ);
		}
		if (!enabled && _file->IsOpen()) {
			_file->Close();
		}
	}

	bool ExtraResource::StreamedSarc::SetFile(const char* name, SARC::FileInfo* info) {
		if (!processed)
			return false;
		info->fileSize = -1;
		_sarc->GetFile(name, info);
		if (info->fileSize == -1) return false;
		u32 _currFileOffset = _sarc->header.dataOffset + info->dataStart;
		if (_file->Seek(_currFileOffset, File::SeekPos::SET) != File::OPResult::SUCCESS)
			return false;
		return true;
	}

	bool ExtraResource::StreamedSarc::SetFileDirectly(SARC::FileInfo* info) {
		if (!processed)
			return false;
		u32 _currFileOffset = _sarc->header.dataOffset + info->dataStart;
		if (_file->Seek(_currFileOffset, File::SeekPos::SET) != File::OPResult::SUCCESS)
			return false;
		return true;
	}

	const void* ExtraResource::StreamedSarc::GetData(void* dest, u32 size) {
		if (!processed) return nullptr;
		u32 remainingSize = _file->Tell();
		u32 readSize = ((_fileSize - remainingSize < size) ? (_fileSize - remainingSize) : (size));
		
		// This addresses will crash if passed to FS directly.
		if (((u32)dest & (1 << 31)) || (((u32)dest & 0xFF000000) == 0x1F000000)) {
			bool worked = true;
			void* tmpData = operator new(0x4000);
			u32 offset = 0;
			while (readSize) {
				u32 toRead = readSize >= 0x4000 ? 0x4000 : readSize;
				if (_file->Read(tmpData, toRead) != File::OPResult::SUCCESS) {
					worked = false;
					break;
				}
				memcpy((u8*)dest + offset, tmpData, toRead);
				readSize -= toRead;
				offset += toRead;
			}
			operator delete(tmpData);
			return worked ? dest : nullptr;
		} else {
			if (_file->Read(dest, readSize) == File::OPResult::SUCCESS)
				return dest;
			else
				return nullptr;
		}
	}

	const bool ExtraResource::StreamedSarc::ReadFile(void* dest, SARC::FileInfo& destFileInfo, const char* name, bool allowCropSize) {
		if (!processed || !dest || !destFileInfo.fileSize || !_sarc->processed || !_sarc->IsEnabled()) {
			return false;
		}
		SARC::FileInfo info;
		bool fileFound = SetFile(name, &info);
		if (!fileFound || (info.fileSize > destFileInfo.fileSize && !allowCropSize)) {
			return false;
		}
		u32 destSize = std::min(info.fileSize, destFileInfo.fileSize);
		if (!GetData(dest, destSize))
			return false;
		svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)dest & ~0xFFF), (info.fileSize & ~0xFFF) + 0x2000);
		destFileInfo.fileSize = info.fileSize;
		CryptoResource::ProcessCryptoFile(dest, &destFileInfo);
		return true;
	}

	const bool ExtraResource::StreamedSarc::ReadFileDirectly(void* dest, SARC::FileInfo& destFileInfo, SARC::FileInfo& inputFileInfo, bool allowCropSize) {
		if (!processed || !dest || !destFileInfo.fileSize || !_sarc->processed || !_sarc->IsEnabled())
			return false;
		SARC::FileInfo& info = inputFileInfo;
		bool fileFound = SetFileDirectly(&info);
		if (!fileFound || (info.fileSize > destFileInfo.fileSize && !allowCropSize))
			return false;
		u32 destSize = std::min(info.fileSize, destFileInfo.fileSize);
		if (!GetData(dest, destSize))
			return false;
		svcFlushProcessDataCache(CUR_PROCESS_HANDLE, ((u32)dest & ~0xFFF), (info.fileSize & ~0xFFF) + 0x2000);
		destFileInfo.fileSize = info.fileSize;
		CryptoResource::ProcessCryptoFile(dest, &destFileInfo);
		return true;
	}

	const bool ExtraResource::StreamedSarc::GetFileInfo(SARC::FileInfo& outFileInfo, const char* name) {
		if (!processed || !_sarc->processed || !_sarc->IsEnabled())
			return false;
		outFileInfo.fileSize = -1;
		_sarc->GetFile(name, &outFileInfo);
		if (outFileInfo.fileSize == -1) return false;
		return true;
	}


	u64 ExtraResource::StreamedSarc::GetNonSecureDataChecksum(u32 bufferSize) {
		if (!processed || !_file->IsOpen()) return 0;
		u32 remaining = _sarc->header.fileLength - _sarc->header.dataOffset;
		_file->Seek(_sarc->header.dataOffset, File::SeekPos::SET);
		u32 ret1 = 0;
		u32 ret2 = 0;
		void* _buffer = memalign(0x1000, bufferSize);
		u32 _bufferSize = bufferSize;
		while (remaining)
		{
			u32 currblockSize = (remaining > _bufferSize) ? _bufferSize : remaining;
			_file->Read(_buffer, currblockSize);
			for (int i = 0; i < currblockSize / sizeof(u32); i+=2) {
				ret1 += ((u32*)_buffer)[i] * 47;
				ret2 += ((u32*)_buffer)[i+1] * 47;
			}
			remaining -= currblockSize;
		}
		free(_buffer);
		return ret1 | ((u64)ret2 << 32);
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

