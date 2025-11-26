/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SaveBackupHandler.cpp
Open source lines: 640/640 (100.00%)
*****************************************************/

#include "SaveBackupHandler.hpp"
#include "NetHandler.hpp"
#include "lz.hpp"
#include "Lang.hpp"
#include "tuple"
#include "SaveHandler.hpp"
#include "TextFileParser.hpp"
#include "BadgeManager.hpp"
#include "StatsHandler.hpp"
#include "MiscUtils.hpp"

namespace CTRPluginFramework {
    
    bool SaveBackupHandler::allowed = false;
    SaveBackupHandler::BackupInfo SaveBackupHandler::lastBackupInfo = {0};
    std::map<std::string, minibson::document(*)(void)> SaveBackupHandler::doBackupCallbacks;
    std::map<std::string, bool(*)(const minibson::document& doc)> SaveBackupHandler::restoreBackupCallbacks;
    std::pair<s8, s8> SaveBackupHandler::gameSysFiles;

    static int g_fetchInfoTimer = 0;
    static int g_backupRes = 0;
    static std::pair<int, int> g_progressPair{};
    static std::string g_progressStr() {
        if (g_progressPair.second == 0)
            return "0%";
        return Utils::Format("%d%%", (int)(((float)g_progressPair.first / (float)g_progressPair.second) * 100.f));
    };

    void SaveBackupHandler::BackupHandlerEntry(MenuEntry *entry)
    {
        if (!allowed) {
            Keyboard kbd(NAME("sbak_avail"));
            kbd.Populate({NAME("exit")});
            kbd.Open();
            return;
        }

        if (!SaveHandler::CheckAndShowServerCommunicationDisabled()) return;

        Keyboard kbd(NAME("sbak_fetch"));
        g_fetchInfoTimer = 0;
        lastBackupInfo.res = 100;
        kbd.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
            if (event.type == KeyboardEvent::EventType::FrameTop) {
                if (g_fetchInfoTimer == 5) {
                    lastBackupInfo = GetBackupInfo();
                    k.Close(); 
                } else {
                    g_fetchInfoTimer++;
                }
            }
        });
        kbd.Populate({std::string("")});
        kbd.Open();
        kbd.OnKeyboardEvent(nullptr);

        if (lastBackupInfo.res != 0) {
            kbd.GetMessage() = Utils::Format(NOTE("sbak_fetch").c_str(), lastBackupInfo.res); 
            kbd.Populate({NAME("exit")});
            kbd.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
            kbd.ChangeSelectedEntry(0);
            kbd.Open();
            lastBackupInfo.Reset();
            return;
        }

        kbd.GetMessage() = ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + NAME("sbak_info") + ToggleDrawMode(Render::FontDrawMode::UNDERLINE);
        kbd.GetMessage() += "\n\n";
        if (lastBackupInfo.daysAgo >= 0) {
            kbd.GetMessage() += Color::Green << NAME("sbak_found") << ResetColor() + "\n";
            kbd.GetMessage() += NAME("sbak_created") + " ";
            if (lastBackupInfo.daysAgo == 0) {
                kbd.GetMessage() += NAME("sbak_cretime");
            } else {
                kbd.GetMessage() += Utils::Format(NOTE("sbak_cretime").c_str(), lastBackupInfo.daysAgo);
            }
        } else {
            kbd.GetMessage() += Color::Red << NOTE("sbak_found") << ResetColor() + "\n";
        }
        kbd.Populate({
            NAME("sbak_create"),
            ((lastBackupInfo.daysAgo >= 0) ? std::string("") : "" << Color::Gray) + NAME("sbak_restore"),
            ((lastBackupInfo.daysAgo >= 0) ? std::string("") : "" << Color::Gray) + NAME("sbak_delete"),
            NAME("exit"),
        });
        kbd.ChangeEntrySound(3, SoundEngine::Event::CANCEL);
        kbd.ChangeSelectedEntry(3);
        int res;
        while (true) {
            res = kbd.Open();
            if (res == 0) {
                Keyboard kbd2(NAME("sbak_new"));
                if (lastBackupInfo.daysAgo >= 0) {
                    kbd2.GetMessage() += "\n\n" << Color::Yellow << NOTE("sbak_new");
                }
                kbd2.Populate({NAME("continue"), NAME("exit")});
                kbd2.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
                kbd2.ChangeSelectedEntry(1);
                int res2 = kbd2.Open();
                if (res2 == 0) {
                    Task doBackupTask([](void* arg) -> s32 {
                        g_backupRes = SaveBackupHandler::DoBackup();
                        SaveBackupHandler::ClearGameSysFiles();
                        return 0;
                    }, nullptr, Task::Affinity::AppCore);
                    g_progressPair = {0, 0};
                    g_backupRes = INT32_MAX;
                    kbd2.GetMessage() = Utils::Format(NAME("sbak_newprog").c_str(), g_progressStr().c_str());
                    kbd2.Populate({""});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                        if (event.type == KeyboardEvent::EventType::FrameTop) {
                            if (g_backupRes == INT32_MAX) {
                                k.GetMessage() = Utils::Format(NAME("sbak_newprog").c_str(), g_progressStr().c_str());
                            } else {
                                k.Close();
                            }
                        }
                    });
                    doBackupTask.Start();
                    kbd2.Open();
                    doBackupTask.Wait();
                    if (g_backupRes == 0) {
                        kbd2.GetMessage() = Color::Green << NAME("sbak_newres");
                    } else {
                        kbd2.GetMessage() = Utils::Format(NOTE("sbak_newres").c_str(), g_backupRes);
                    }
                    kbd2.Populate({NAME("exit")});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    kbd2.OnKeyboardEvent(nullptr);
                    kbd2.Open();
                    break;
                }
            } else if (res == 1 && lastBackupInfo.daysAgo >= 0) {
                Keyboard kbd2(NAME("sbak_rest"));
                std::string folder1 = "/CTGP-7/savefs/gameBackup";
                std::string folder2 = "/CTGP-7/savefs/modBackup";
                kbd2.GetMessage() += std::string("\n\n") + NOTE("sbak_rest") + "\n" + ToggleDrawMode(Render::FontDrawMode::UNDERLINE) + folder1 + "\n" + folder2 + ToggleDrawMode(Render::FontDrawMode::UNDERLINE);
                if (Directory::IsExists(folder1) || Directory::IsExists(folder2)) {
                    kbd2.GetMessage() += std::string("\n\n") << Color::Orange << NAME("sbak_restex");
                }
                kbd2.Populate({NAME("continue"), NAME("exit")});
                kbd2.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
                kbd2.ChangeSelectedEntry(1);
                int res2 = kbd2.Open();
                if (res2 == 0) {
                    SaveHandler::disableSaving = true;
                    Task restoreBackupTask([](void* arg) -> s32 {
                        g_backupRes = SaveBackupHandler::RestoreBackup();
                        return 0;
                    }, nullptr, Task::Affinity::AppCore);
                    g_progressPair = {0, 0};
                    g_backupRes = INT32_MAX;
                    kbd2.GetMessage() = Utils::Format(NAME("sbak_restprog").c_str(), g_progressStr().c_str());
                    kbd2.Populate({""});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                        if (event.type == KeyboardEvent::EventType::FrameTop) {
                            if (g_backupRes == INT32_MAX) {
                                k.GetMessage() = Utils::Format(NAME("sbak_restprog").c_str(), g_progressStr().c_str());
                            } else {
                                k.Close();
                            }
                        }
                    });
                    restoreBackupTask.Start();
                    kbd2.Open();
                    restoreBackupTask.Wait();
                    if (g_backupRes == 0) {
                        kbd2.GetMessage() = Color::Green << NAME("sbak_restres") + ResetColor();
                    } else {
                        std::string info = (g_backupRes != 300) ? (std::string("\n\n") << Color::Orange << NAME("sbak_restres2") << "\n" + folder1 << "\n" << folder2 << ResetColor()) : "";
                        kbd2.GetMessage() = Utils::Format(NOTE("sbak_restres").c_str(), g_backupRes) + info;
                    }
                    kbd2.GetMessage() += std::string("\n\n") + NOTE("sbak_restres2");
                    kbd2.Populate({NAME("exit")});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    kbd2.OnKeyboardEvent(nullptr);
                    kbd2.Open();
                    Sleep(Seconds(1));
                    Process::ReturnToHomeMenu();
                }
            } else if (res == 2 && lastBackupInfo.daysAgo >= 0) {
                Keyboard kbd2(NAME("sbak_dele"));
                kbd2.GetMessage() += "\n\n" << Color::Orange << NOTE("sbak_dele") << ResetColor();
                kbd2.Populate({NAME("continue"), NAME("exit")});
                kbd2.ChangeEntrySound(1, SoundEngine::Event::CANCEL);
                kbd2.ChangeSelectedEntry(1);
                int res2 = kbd2.Open();
                if (res2 == 0) {
                    Task deleteBackupTask([](void* arg) -> s32 {
                        g_backupRes = SaveBackupHandler::DeleteBackup();
                        return 0;
                    }, nullptr, Task::Affinity::AppCore);
                    g_backupRes = INT32_MAX;
                    kbd2.GetMessage() = NAME("sbak_deleprog");
                    kbd2.Populate({""});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.OnKeyboardEvent([](Keyboard& k, KeyboardEvent& event) {
                        if (event.type == KeyboardEvent::EventType::FrameTop) {
                            if (g_backupRes != INT32_MAX) {
                                k.Close();
                            }
                        }
                    });
                    deleteBackupTask.Start();
                    kbd2.Open();
                    deleteBackupTask.Wait();
                    if (g_backupRes == 0) {
                        kbd2.GetMessage() = Color::Green << NAME("sbak_deleres");
                    } else {
                        kbd2.GetMessage() = Utils::Format(NOTE("sbak_deleres").c_str(), g_backupRes);
                    }
                    kbd2.Populate({NAME("exit")});
                    kbd2.ChangeSelectedEntry(0);
                    kbd2.ChangeEntrySound(0, SoundEngine::Event::CANCEL);
                    kbd2.OnKeyboardEvent(nullptr);
                    kbd2.Open();
                    break;
                }
            } else if (res == 3 || res < 0) {
                break;
            }
        }       
        lastBackupInfo.Reset();
    }

    void SaveBackupHandler::Initialize()
    {
        AddDoBackupHandler("system", []() -> minibson::document {
            return BackupGame(true);
        });
        AddRestoreBackupHandler("system", [](const minibson::document& doc) -> bool {
            return RestoreGame(doc, true);
        });
    }

    int SaveBackupHandler::DoBackup()
    {
        Lock lock(SaveHandler::saveSettingsMutex);

        int totOps = doBackupCallbacks.size() * 2 + 2;
        int currOps = 0;
        int resultCode = 0;
        auto increaseOp = [&currOps, &totOps]() {
            currOps++;
            g_progressPair = {currOps, totOps};
        };

        NetHandler::RequestHandler req;
        minibson::document doc;

        resultCode = DetectGameSysFiles();
        if (resultCode != 0)
            return resultCode;

        doc.set("op", "start");
        doc.set<s64>("sID0", SaveHandler::saveData.saveID[0]);
        doc.set<s64>("sID1", SaveHandler::saveData.saveID[1]);
        req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, doc);
        req.Start();
        req.Wait();
        minibson::document outdoc;
        resultCode = req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, &outdoc);
        if (resultCode != 0) {
            return resultCode;
        }
        std::string tok = outdoc.get("tok", "");
        if (tok.empty()) {
            return 200;
        }
        req.Cleanup();
        doc.clear();
        outdoc.clear();
        increaseOp();
        u32 totSize = 0;

        for (auto it = doBackupCallbacks.begin(); it != doBackupCallbacks.end(); it++) {
            auto savedoc = it->second();

            int backupres = savedoc.get<int>("backupres", 0);
            if (backupres != 0) {
                return backupres;
            }
            savedoc.remove("backupres");

            MiscUtils::Buffer buffer = savedoc.serialize_to_buffer();
            savedoc.clear();

            LZArg lzArg = {0};
            lzArg.isCompress = true;
            lzArg.inputAddr = buffer.Data();
            lzArg.inputSize = buffer.Size();
            LZ77Lock();
            LZ77Perform(lzArg);
            LZ77Wait();
            const auto& lzOut = LZ77Result();
            if (!lzOut.good) {
                LZ77Cleanup();
                LZ77Unlock();
                return 201;
            }
            buffer.Clear();

            doc.set("op", "item");
            doc.set("tok", tok);
            doc.set("name", it->first);
            doc.set("data", MiscUtils::Buffer(lzOut.outputBuffer.Data(), lzOut.outputSize));
            totSize += lzOut.outputSize;
            LZ77Cleanup();
            LZ77Unlock();
            increaseOp();

            req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, std::move(doc));
            doc.clear();
            req.Start();
            req.Wait();
            resultCode = req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, &outdoc);
            if (resultCode != 0)  {
                return resultCode;
            }
            req.Cleanup();
            outdoc.clear();
            increaseOp();
        }

        doc.set("op", "commit");
        doc.set("tok", tok);
        doc.set<int>("totSize", totSize);
        req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, doc);
        doc.clear();
        req.Start();
        req.Wait();
        resultCode = req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, &outdoc);
        if (resultCode != 0)  {
            return resultCode;
        }

        increaseOp();
        Sleep(Seconds(0.5f));

        return 0;
    }

    SaveBackupHandler::BackupInfo SaveBackupHandler::GetBackupInfo()
    {
        BackupInfo res = {0};
        NetHandler::RequestHandler req;
        minibson::document doc;

        doc.set("op", "info");
        req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_GET, doc);
        req.Start();
        req.Wait();
        minibson::document outdoc;
        res.res = req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_GET, &outdoc);
        if (res.res != 0) {
            return res;
        }

        res.daysAgo = outdoc.get_numerical("daysAgo");
        if (res.daysAgo < 0)
            return res;

        s64 saveID[2];
        saveID[0] = outdoc.get<s64>("sID0", 0);
        saveID[1] = outdoc.get<s64>("sID1", 0);
        if (saveID[0] == 0 || saveID[1] == 0) {
            res.res = 101;
            return res;
        }
        res.saveID[0] = saveID[0];
        res.saveID[1] = saveID[1];

        res.names = TextFileParser::Split(outdoc.get("names", ""), ":");
        if (res.names.empty() || res.names[0].empty()) {
            res.res = 102;
            return res;
        }

        return res;
    }

    int SaveBackupHandler::RestoreBackup()
    {
        Lock lock(SaveHandler::saveSettingsMutex);

        int totOps = restoreBackupCallbacks.size() * 2 + 7;
        int currOps = 0;
        int resultCode = 0;
        auto increaseOp = [&currOps, &totOps]() {
            currOps++;
            g_progressPair = {currOps, totOps};
        };

        Directory::Remove("/CTGP-7/savefs/modBackup");
        Directory::Remove("/CTGP-7/savefs/gameBackup");
        if (Directory::Rename("/CTGP-7/savefs/mod", "/CTGP-7/savefs/modBackup") != 0 || Directory::Create("/CTGP-7/savefs/mod") != 0 ||
            Directory::Rename("/CTGP-7/savefs/game", "/CTGP-7/savefs/gameBackup") != 0 || Directory::Create("/CTGP-7/savefs/game") != 0) {
            return 300;
        }
        BadgeManager::ClearAll();

        increaseOp();

        lastBackupInfo = GetBackupInfo();
        if (lastBackupInfo.res != 0 || lastBackupInfo.daysAgo < 0)
            return 301;

        NetHandler::RequestHandler req;
        minibson::document doc;
        minibson::document outdoc;

        increaseOp();

        for (auto it = restoreBackupCallbacks.begin(); it != restoreBackupCallbacks.end(); it++) {

            if (std::find(lastBackupInfo.names.begin(), lastBackupInfo.names.end(), it->first) == lastBackupInfo.names.end()) {
                increaseOp(); increaseOp();
                // Server does not have backup for given entry, assume empty
                if (!it->second(minibson::document()))
                    return 302;
            }

            doc.set("op", "item");
            doc.set("name", it->first);
            doc.set<s64>("sID0", lastBackupInfo.saveID[0]);
            doc.set<s64>("sID1", lastBackupInfo.saveID[1]);
            req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_GET, doc);
            doc.clear();
            req.Start();
            req.Wait();
            resultCode = req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_GET, &outdoc);
            if (resultCode != 0)  {
                return resultCode;
            }

            increaseOp();

            auto& buf = outdoc.get_binary("data");
            if (buf.Size() < 4) {
                return 303;
            }

            LZArg lzArg = {0};
            lzArg.isCompress = false;
            lzArg.inputAddr = buf.Data();
            lzArg.inputSize = buf.Size();
            LZ77Lock();
            LZ77Perform(lzArg);
            LZ77Wait();
            outdoc.clear(); // Do not use buf as it's no longer valid
            const auto& lzOut = LZ77Result();
            if (!lzOut.good) {
                LZ77Cleanup();
                LZ77Unlock();
                return 304;
            }

            minibson::document tmpDoc(lzOut.outputBuffer.Data(), lzOut.outputSize);
            LZ77Cleanup();
            LZ77Unlock();

            if (!it->second(tmpDoc))
                return 305;

            increaseOp();
        }

        SaveHandler::saveData.saveID[0] = lastBackupInfo.saveID[0];
        SaveHandler::saveData.saveID[1] = lastBackupInfo.saveID[1];

        increaseOp();

        if (Directory::IsExists("/CTGP-7/savefs/gameBackup/replay")) {
            if (!MiscUtils::CopyDirectory("/CTGP-7/savefs/game/replay", "/CTGP-7/savefs/gameBackup/replay", false)) {
                return 306;
            }
        } else {
            Directory::Create("/CTGP-7/savefs/game/replay");
        }
        if (Directory::IsExists("/CTGP-7/savefs/gameBackup/replay_")) {
            if (!MiscUtils::CopyDirectory("/CTGP-7/savefs/game/replay_", "/CTGP-7/savefs/gameBackup/replay_", false)) {
                return 307;
            }
        } else {
            Directory::Create("/CTGP-7/savefs/game/replay_");
        }

        increaseOp();

        int prevSeqID = StatsHandler::GetSequenceID();
        StatsHandler::UploadStats(false, true); // Get badges or fix sequence ID
        increaseOp();
        if (prevSeqID == 0 || (prevSeqID + 1) != StatsHandler::GetSequenceID()) {
            // Call again if first call was to get proper sequence ID.
            StatsHandler::UploadStats(false, true);
        }
        increaseOp();
        SaveHandler::saveData.flags1.needsBadgeObtainedMsg = false;

        SaveHandler::disableSaving = false;
        SaveHandler::SaveSettingsAll(false);
        SaveHandler::disableSaving = true;

        increaseOp();
        Sleep(Seconds(0.5f));

        return 0;
    }

    int SaveBackupHandler::DeleteBackup()
    {
        int res = 0;
        NetHandler::RequestHandler req;
        minibson::document doc;
        minibson::document outdoc;

        doc.set("op", "delete");
        req.AddRequest(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, doc);
        req.Start();
        req.Wait();
        return req.GetResult(NetHandler::RequestHandler::RequestType::SAVE_BACKUP_PUT, &outdoc);
    }

    bool SaveBackupHandler::DetectGameSysFiles()
    {
        int listRes = 0;
        s8 sysIDs[2] = {-1, -1};
        std::vector<std::string> files;
        Directory dir("/CTGP-7/savefs/game");
        if (!dir.IsOpen()) {
            return 400;
        }
        listRes = dir.ListFiles(files, "system");
        dir.Close();
        if (listRes != 2 && listRes != 1) {
            return 401;
        }

        for (auto it = files.begin(); it != files.end(); it++) {
            if (it->size() == 11 && it->starts_with("system") && it->ends_with(".dat") && (*it)[6] >= '0' && (*it)[6] <= '9') {
                s8 sysID = (s8)((*it)[6] - '0');
                if (sysIDs[0] != -1) {
                    if (std::abs(sysIDs[0] - sysID) != 1 && std::abs(sysIDs[0] - sysID) != 9) {
                        return 402;
                    }
                    sysIDs[1] = sysID;
                } else {
                    sysIDs[0] = sysID;
                }
            } else {
                return 403;
            }
        }

        if (sysIDs[1] == -1 || sysIDs[0] < sysIDs[1]) {
            gameSysFiles = {sysIDs[0], sysIDs[1]};
        } else {
            gameSysFiles = {sysIDs[1], sysIDs[0]};
        }
        if (gameSysFiles.first == 0 && gameSysFiles.second == 9) gameSysFiles = {9, 0};
        return 0;
    }

    minibson::document SaveBackupHandler::BackupGame(bool main)
    {
        int res = 0;
        minibson::document doc;
        constexpr size_t GAME_SAVE_SIZE = 0x50D4;
        MiscUtils::Buffer buffer(GAME_SAVE_SIZE);
        File sysFile;

        s8 sysID = main ? gameSysFiles.first : gameSysFiles.second;
        std::string sysIDStr = std::string(1, (char)(sysID + '0'));
        if (sysID == -1) {
            if (main) {
                res = 501;
            }
            goto exit;
        }
       
        if (File::Open(sysFile, "/CTGP-7/savefs/game/system" + sysIDStr + ".dat", File::READ) < 0 || !sysFile.IsOpen()) {
            res = 502;
            goto exit;
        }
        if (sysFile.GetSize() != GAME_SAVE_SIZE) {
            res = 503;
            goto exit;
        }
        if (sysFile.Read(buffer.Data(), GAME_SAVE_SIZE) < 0) {
            res = 504;
            goto exit;
        }
        doc.set(sysIDStr, std::move(buffer));
        
    exit:
        if (res != 0) {
            doc.clear();
            doc.set<int>("backupres", res);
        }
        return doc;
    }

    bool SaveBackupHandler::RestoreGame(const minibson::document &doc, bool main)
    {
        constexpr size_t GAME_SAVE_SIZE = 0x50D4;
        if (!main && doc.empty()) {
            return true;
        }
        if (doc.size() != 1) {
            return false;
        }

        auto it = doc.cbegin();
        if (it->first.length() == 1 && it->first[0] >= '0' && it->first[0] <= '9' && it->second->get_node_code() == minibson::bson_node_type::binary_node) {
            std::string name = std::string("system") + it->first + ".dat";
            auto& val = static_cast<const minibson::binary*>(it->second.get())->get_value();
            if (val.Size() != GAME_SAVE_SIZE)
                return false;
            File sysFile("/CTGP-7/savefs/game/" + name, File::RWC);
            if (sysFile.Write(val.Data(), val.Size()) < 0)
                return false;
        } else {
            return false;
        }
        return true;
    }
}