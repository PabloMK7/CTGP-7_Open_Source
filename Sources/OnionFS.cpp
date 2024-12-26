/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: OnionFS.cpp
Open source lines: 413/413 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "OnionFS.hpp"
#include "rt.hpp"
#include "3ds.h"
#include "CourseManager.hpp"
#include "main.hpp"
#include "str16utils.hpp"

#define NUMBER_FILE_OP 9

enum fileSystemBits
{
	OPEN_FILE_OP,
	OPEN_DIRECTORY_OP,
	DELETE_FILE_OP,
	RENAME_FILE_OP,
	DELETE_DIRECTORY_OP,
	DELETE_DIRECTORY_RECURSIVE_OP,
	CREATE_FILE_OP,
	CREATE_DIRECTORY_OP,
	RENAME_DIRECTORY_OP
};

typedef u32(*fsRegArchiveTypeDef)(u8*, u32*, u32, u32);
typedef u32(*userFsTryOpenFileTypeDef)(u32, char16_t*, u32);
typedef u32(*fsMountArchiveTypeDef)(u32*, u32);

typedef u32(*fsu32u16u32)(u32, char16_t*, u32);
typedef u32(*fsu16)(char16_t*);
typedef u32(*fsu16u16)(char16_t*, char16_t*);
typedef u32(*fsu16u64)(char16_t*, u64);
typedef u32(*fsu32u16)(u32, char16_t*);

u8 fsMountArchivePat1[] = { 0x10, 0x00, 0x97, 0xE5, 0xD8, 0x20, 0xCD, 0xE1, 0x00, 0x00, 0x8D };
u8 fsMountArchivePat2[] = { 0x28, 0xD0, 0x4D, 0xE2, 0x00, 0x40, 0xA0, 0xE1, 0xA8, 0x60, 0x9F, 0xE5, 0x01, 0xC0, 0xA0, 0xE3 };
u8 fsRegArchivePat[] = { 0xB4, 0x44, 0x20, 0xC8, 0x59, 0x46, 0x60, 0xD8 };
u8 userFsTryOpenFilePat1[] = { 0x0D, 0x10, 0xA0, 0xE1, 0x00, 0xC0, 0x90, 0xE5, 0x04, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1 };
u8 userFsTryOpenFilePat2[] = { 0x10, 0x10, 0x8D, 0xE2, 0x00, 0xC0, 0x90, 0xE5, 0x05, 0x00, 0xA0, 0xE1, 0x3C, 0xFF, 0x2F, 0xE1 };
u8 formatSavePat[] = { 0xF0, 0x9F, 0xBD, 0xE8, 0x42, 0x02, 0x4C, 0x08 };

namespace CTRPluginFramework::OnionFS
{
	u32* fileOperations[NUMBER_FILE_OP] = { nullptr };
	RT_HOOK fileOpHooks[NUMBER_FILE_OP] = { 0 };
	RT_HOOK	regArchiveHook = { 0 };
	RT_HOOK formatSaveHook = { 0 };
	u32 fsMountArchive = 0;
	bool canSaveRedirect = true;
	const char16_t romfsPath[] = { u"ram:/CTGP-7/gamefs/" };
	const char16_t savePath[] = { u"ram:/CTGP-7/savefs/game/" };
	bool gameFsHashesReady = false;
	std::vector<u32> gameFsFileHashes;

	extern u32 fileOperationFuncs[NUMBER_FILE_OP];

	static inline u32* findNearestSTMFDptr(u32* newaddr) {
		u32 i;
		for (i = 0; i < 1024; i++) {
			newaddr--;
			i++;
			if (*((u16*)newaddr + 1) == 0xE92D) {
				return newaddr;
			}
		}
		return 0;
	}

	static void storeAddrByOffset(u32* addr, u16 offset) {
		if (offset % 4 != 0) return;
		offset >>= 2;
		if (offset < NUMBER_FILE_OP) fileOperations[offset] = addr;
	}

	static void processFileSystemOperations(u32* funct, u32* endAddr) {

		int i;
		for (i = 0; i < 0x20; i++) { // Search for the closest BL, this BL will branch to getArchObj
			if ((*(funct + i) & 0xFF000000) == 0xEB000000) {
				funct += i;
				break;
			}
		}
		u32 funcAddr;
		u32* addr;
		if (i >= 0x20) { // If there are no branches, the function couldn't be found.
			goto exit;
		}
		funcAddr = decodeARMBranch(funct); // Get the address of getArchObj

		addr = (u32*)0x100000;
		while (addr < endAddr) { // Scan the text section of the code for the fs functions
			if ((*addr & 0xFF000000) == 0xEB000000 && (decodeARMBranch(addr) == funcAddr)) { //If a branch to getArchObj if found analize it.
				u8 regId = 0xFF;
				for (i = 0; i < 1024; i++) { //Scan forwards for the closest BLX, and get the register it is branching to
					int currinst = addr[i];
					if (*((u16*)(addr + i) + 1) == 0xE92D) break; //Stop if STMFD is found (no BLX in this function)
					if ((currinst & ~0xF) == 0xE12FFF30) { //BLX
						regId = currinst & 0xF;
						break;
					}
				}
				if (regId != 0xFF) { // If a BLX is found, scan backwards for the nearest LDR to the BLX register.
					int j = i;
					for (; i > 0; i--) {
						if (((addr[i] & 0xFFF00000) == 0xE5900000) && (((addr[i] & 0xF000) >> 12) == regId)) { //If it is a LDR and to the BLX register
							storeAddrByOffset(findNearestSTMFDptr(addr), addr[i] & 0xFFF); //It is a fs function, store it based on the LDR offset. (This LDR gets the values from the archive object vtable, by checking the vtable offset it is possible to know which function it is)
							break;
						}
					}
					addr += j; // Continue the analysis from the BLX
				}
			}
			addr++;
		}
		for (int i = 0; i < NUMBER_FILE_OP; i++) {
			if (fileOperations[i] == nullptr) continue;
			rtInitHook(&fileOpHooks[i], (u32)fileOperations[i], fileOperationFuncs[i]);
			rtEnableHook(&fileOpHooks[i]);
		}
	exit:
		return;
	}

	static thread_local char16_t *g_buffers[2] = {nullptr, nullptr};

    static inline char16_t     *GetBuffer(bool secondary = false)
    {
        if (g_buffers[secondary] == nullptr)
            g_buffers[secondary] = static_cast<char16_t *>(::operator new(0x200));
        return g_buffers[secondary];
    }

    static inline void concatFileName(char16_t* dest, const char16_t* s1, const char16_t* s2) {
        while (*s1)    *dest++ = *s1++; //Copy the default file path

        while (*s2++ != u'/'); // Skip the archive lowpath

        while (*s2 == '/') ++s2; // Skip any remaining  /

        while (*s2) *dest++ = *s2++; // Copy the rest of the filename

        *dest = '\0';
    }
#ifdef LOG_ONIONFS_FILES
	static File* g_logFile = nullptr;
	static Mutex g_logMutex;
	static void LogFileToSD(char16_t* path) {
		Lock lock(g_logMutex);
		if (!g_logFile) g_logFile = new File("/CTGP-7/filelog.txt", File::RWC);
		std::string out;
		Utils::ConvertUTF16ToUTF8(out, path);
		if (!out.empty())
			g_logFile->WriteLine(out);
	}
#endif
    static char16_t* calculateNewPath(char16_t* initialPath, bool isReadOnly, bool isSecondary = false, bool* shouldReopen = nullptr) {
#ifdef LOG_ONIONFS_FILES
		LogFileToSD(initialPath);
#endif
        const char16_t* basePath;
        if (*((u32*)initialPath) == 0x610064) {
            basePath = savePath;
            if (shouldReopen) *shouldReopen = false;
        }
        else if (!isReadOnly && (*((u32*)initialPath) == 0x6F0072 || *((u32*)initialPath) == 0x610070)) {
            basePath = romfsPath;
        }
        else {
            return initialPath;
        }
        char16_t* dst = GetBuffer(isSecondary);
        concatFileName(dst, basePath, initialPath);
        return dst;
    }

	static inline u32 getArchiveMode(char16_t* initialPath) {
		if (*((u32*)initialPath) == 0x610064) {
			return 1; //SAVE
		}
		else if ((*((u32*)initialPath) == 0x6F0072 || *((u32*)initialPath) == 0x610070)) {
			return 0; //ROMFS
		}
		else {
			return 2; //OTHERS
		}
	}

    static u32  fsOpenFileFunc(u32 a1, char16_t* path, u32 a2) {
        bool reopen = true;
        char16_t* newPath = calculateNewPath(path, 0, false, &reopen);
		MarioKartFramework::changeFilePath(newPath, false);

		if (newPath[0] == '\0' || CheckGameFSFileExists(newPath) == GameFSFileState::NOTEXISTS) // OnionFS operation cancelled, use default path.
			return ((fsu32u16u32)fileOpHooks[OPEN_FILE_OP].callCode)(a1, path, a2);
		
        int ret = ((fsu32u16u32)fileOpHooks[OPEN_FILE_OP].callCode)(a1, newPath, a2);
        if (newPath != path) {
            if (!reopen) return ret;
            if (ret < 0) {
                ret = ((fsu32u16u32)fileOpHooks[OPEN_FILE_OP].callCode)(a1, path, a2);
            }
        }
        return ret;
    }

	static inline char16_t* skipArchive(char16_t* src) {
		while (*src++ != u'/'); //Skip the archive lowpath

		while (*src == u'/') ++src; // Skip any remaining  /

		return src - 1; //Return the position of the last /
	}

	static int checkFileExistsWithDir(char16_t* path) { // Sometimes game devs choose to check if a file exists by doing open directory on it.
											// The problem is that SD archive doesn't behave the same way as other archives.
											// Doing openDir on a file in the SD returns "doesn't exist" while on the save file it returns "operation not supported".
		Handle file;
		Result fileExists = MAKERESULT(RL_USAGE, RS_NOTSUPPORTED, RM_FS, 770); // If the file exists return "not supported" error.

		GameFSFileState fileState = CheckGameFSFileExists(path);

		if (fileState == GameFSFileState::EXISTS)
			return fileExists;
		else if (fileState == GameFSFileState::NOTEXISTS)
		{
			// Let the game decide
			return 0;
		}

		// Otherwise operation is not in romfs, need to check with OpenFileDirectly
		int ret = FSUSER_OpenFileDirectly(&file, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""), fsMakePath(PATH_UTF16, skipArchive(path)), FS_OPEN_READ, 0);
		if (R_SUCCEEDED(ret)) { // If the file exists...
			FSFILE_Close(file);
			return fileExists; //.. return "not supported" error.
		}
		return 0;
	}

	static u32  fsOpenDirectoryFunc(u32 a1, char16_t* path) {
		char16_t* newPath = calculateNewPath(path, 0);
		MarioKartFramework::changeFilePath(newPath, true);
		if (newPath[0] == '\0') // OnionFS operation cancelled, use default path.
			return ((fsu32u16)fileOpHooks[OPEN_DIRECTORY_OP].callCode)(a1, path);
		if (newPath != path && *((u32*)newPath) == 0x00610072) {
			int res = checkFileExistsWithDir(newPath);
			if (res) return res;
			if (getArchiveMode(path) == 0) newPath = path;
		}
		int ret = ((fsu32u16)fileOpHooks[OPEN_DIRECTORY_OP].callCode)(a1, newPath);
		return ret;
	}

    static u32  fsDeleteFileFunc(char16_t* path) {
        char16_t* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_FILE_OP].callCode)(newPath);
        return ret;
    }

    static u32  fsRenameFileFunc(char16_t* path1, char16_t* path2) {
        char16_t* newPath1 = calculateNewPath(path1, 1);
        char16_t* newPath2 = calculateNewPath(path2, 1, true);
        int ret = ((fsu16u16)fileOpHooks[RENAME_FILE_OP].callCode)(newPath1, newPath2);
        return ret;
    }

    static u32  fsDeleteDirectoryFunc(char16_t* path) {
        char16_t* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_DIRECTORY_OP].callCode)(newPath);
        return ret;
    }
    static u32  fsDeleteDirectoryRecFunc(char16_t* path) {
        char16_t* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[DELETE_DIRECTORY_RECURSIVE_OP].callCode)(newPath);
        return ret;
    }
    static u32  fsCreateFileFunc(char16_t* path, u64 a2) {
        char16_t* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16u64)fileOpHooks[CREATE_FILE_OP].callCode)(newPath, a2);
        return ret;
    }
    static u32  fsCreateDirectoryFunc(char16_t* path) {
        char16_t* newPath = calculateNewPath(path, 1);
        int ret = ((fsu16)fileOpHooks[CREATE_DIRECTORY_OP].callCode)(newPath);
        return ret;
    }
    static u32  fsRenameDirectoryFunc(char16_t* path1, char16_t* path2) {
        char16_t* newPath1 = calculateNewPath(path1, 1);
        char16_t* newPath2 = calculateNewPath(path2, 1, true);
        int ret = ((fsu16u16)fileOpHooks[RENAME_DIRECTORY_OP].callCode)(newPath1, newPath2);
        return ret;
    }

    static u32 fsRegArchiveCallback(u8* path, u32* arch, u32 isAddOnContent, u32 isAlias) {
        u32 ret;
        static u32 isFisrt = 1;
        ret = ((fsRegArchiveTypeDef)regArchiveHook.callCode)(path, arch, isAddOnContent, isAlias);
        if (isFisrt) {
            u32 ret2;
            u32 sdmcArchive = 0;
            isFisrt = 0;
            ((fsMountArchiveTypeDef)fsMountArchive)(&sdmcArchive, 9);
            if (sdmcArchive) {
                ret2 = ((fsRegArchiveTypeDef)regArchiveHook.callCode)((u8*)"ram:", (u32*)sdmcArchive, 0, 0);
            }
        }
        return ret;
    }

    static int fsFormatSaveData(int *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10, char a11) {
        Directory::Remove("/CTGP-7/savefs/game");
        Directory::Create("/CTGP-7/savefs/game");
        return 0;
    }

	static void initFileMapRec(const std::string dir, const std::string prefix) {
		Directory startDir(dir);
		if (!startDir.IsOpen()) return;
		std::vector<std::string> dirs;
		std::vector<std::string> files;
		startDir.ListDirectories(dirs);
		startDir.ListFiles(files);

		for (int i = 0; i < files.size(); i++) {
			std::string file = prefix << dir << "/" << files[i];
			gameFsFileHashes.push_back(ExtraResource::SARC::CalculateHash<char>(file.c_str(), 0x65));
		}
		files.clear();
		for (int i = 0; i < dirs.size(); i++) {
			initFileMapRec(dir << "/" << dirs[i], prefix);
		}
	}

	Thread gameFsFileHashesThread;
	LightEvent gameFsFileHashesEvent;

	static void initGameFsFileMapFunc(void* arg) {
		LightEvent_Wait(&gameFsFileHashesEvent);
		initFileMapRec("/CTGP-7/gamefs", "ram:");
		initFileMapRec("/CTGP-7/MyStuff/stream", "ram:");
		std::sort(gameFsFileHashes.begin(), gameFsFileHashes.end());
		gameFsHashesReady = true;
	}

	void initGameFsFileMap() {
		LightEvent_Signal(&gameFsFileHashesEvent);
	}

	GameFSFileState CheckGameFSFileExists(const char16_t* file) {
		if (!gameFsHashesReady || (!strcmp16(file, romfsPath) && !strcmp16(file, u"ram:/CTGP-7/MyStuff/stream/")))
			return GameFSFileState::UNKNOWN;

		u32 hash = ExtraResource::SARC::CalculateHash<char16_t>(file, 0x65);
		auto it = std::lower_bound(gameFsFileHashes.begin(), gameFsFileHashes.end(), hash);
		if (it != gameFsFileHashes.end() && *it == hash)
			return GameFSFileState::EXISTS;
		else
			return GameFSFileState::NOTEXISTS;
	}

	void InitFSFileMapThread() {
		LightEvent_Init(&gameFsFileHashesEvent, RESET_ONESHOT);
		gameFsFileHashesThread = threadCreate(initGameFsFileMapFunc, nullptr, 0x1000, 0x30, -2, true);
	}

	bool initOnionFSHooks(u32 textSize) {

		u32* addr = (u32*)0x100000;
		u32* endAddr = (u32*)(0x100000 + textSize);
		bool contOpen = true, contMount = true, contReg = true, contDelete = true;
		while (addr < endAddr && (contOpen || contMount || contReg || contDelete)) {
			if (contOpen && memcmp(addr, userFsTryOpenFilePat1, sizeof(userFsTryOpenFilePat1)) == 0 || memcmp(addr, userFsTryOpenFilePat2, sizeof(userFsTryOpenFilePat2)) == 0) {
				u32* fndaddr = findNearestSTMFDptr(addr);
				contOpen = false;
				processFileSystemOperations(fndaddr, endAddr);
			}
			if (contMount && memcmp(addr, fsMountArchivePat1, sizeof(fsMountArchivePat1)) == 0 || memcmp(addr, fsMountArchivePat2, sizeof(fsMountArchivePat2)) == 0) {
				u32* fndaddr = findNearestSTMFDptr(addr);
				contMount = false;
				fsMountArchive = (u32)fndaddr;
			}
			if (contReg && memcmp(addr, fsRegArchivePat, sizeof(fsRegArchivePat)) == 0) {
				contReg = false;
				u32* fndaddr = findNearestSTMFDptr(addr);

				rtInitHook(&regArchiveHook, (u32)fndaddr, (u32)fsRegArchiveCallback);
				rtEnableHook(&regArchiveHook);
			}
			if (contDelete && memcmp(addr, formatSavePat, sizeof(formatSavePat)) == 0) {
				contDelete = false;
				u32* fndaddr = findNearestSTMFDptr(addr);
				rtInitHook(&formatSaveHook, (u32)fndaddr, (u32)fsFormatSaveData);
				rtEnableHook(&formatSaveHook);
			}
			addr++;
		}
		if (!(formatSaveHook.isEnabled && regArchiveHook.isEnabled)) {
			return false;
		}
		
		return true;
	}

	u32 fileOperationFuncs[NUMBER_FILE_OP] = { (u32)fsOpenFileFunc, (u32)fsOpenDirectoryFunc, (u32)fsDeleteFileFunc, (u32)fsRenameFileFunc, (u32)fsDeleteDirectoryFunc, (u32)fsDeleteDirectoryRecFunc, (u32)fsCreateFileFunc, (u32)fsCreateDirectoryFunc, (u32)fsRenameDirectoryFunc };
}
