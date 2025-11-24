/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveBackupHandler.hpp
Open source lines: 62/62 (100.00%)
*****************************************************/

#include "CTRPluginFramework.hpp"
#include "map"
#include "Minibson.hpp"

namespace CTRPluginFramework {
    class SaveBackupHandler {
    public:
        static void AddDoBackupHandler(const std::string& key, minibson::document(*callback)(void)) {
            doBackupCallbacks[key] = callback;
        }
        static void AddRestoreBackupHandler(const std::string& key, bool(*callback)(const minibson::document&)) {
            restoreBackupCallbacks[key] = callback;
        }

        static void BackupHandlerEntry(MenuEntry* entry);

        struct BackupInfo {
            int res;
            s64 saveID[2];
            int daysAgo;
            std::vector<std::string> names;

            void Reset() {
                res = -1;
                saveID[0] = saveID[1] = 0;
                daysAgo = 0;
                names.clear();
            }
        };
        static BackupInfo lastBackupInfo;

        static void Initialize();

        static int DoBackup();
        static BackupInfo GetBackupInfo();

        static int RestoreBackup();
        static int DeleteBackup();

        static bool DetectGameSysFiles();
        static void ClearGameSysFiles() {
            gameSysFiles = {-1, -1};
        }
        static minibson::document BackupGame(bool main);
        static bool RestoreGame(const minibson::document& doc, bool main);

        static bool allowed;
    private:
        static std::pair<s8, s8> gameSysFiles;
        static std::map<std::string, minibson::document(*)(void)> doBackupCallbacks;
        static std::map<std::string, bool(*)(const minibson::document& doc)> restoreBackupCallbacks;
    };
}