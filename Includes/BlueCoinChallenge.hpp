/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: BlueCoinChallenge.hpp
Open source lines: 51/51 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "DataStructures.hpp"
#include "map"

namespace CTRPluginFramework {
    class BlueCoinChallenge {
    private:
        static u32 rotateProgress;
        static u32 coinObjBase;
        static u8* blueCoinSound;
        static u32 coinCollectedDialogFrames;
        static std::map<u32, std::pair<Vector3, u16>> coinLocations;
        static string16 blueCoinCollectedStr;
        static void InitCoinLocations();
        static void SetCoinCollected(u32 courseID);
        static bool closeCoinCollectedDialog(const Screen &s);
    public:
        class GameAddr {
        public:
            static u32 coinCreateTrigEffect;
        };

        static bool coinSpawned;
        static bool coinWasSpawned;
        static bool coinDisabledCCSelector;
        
        static bool IsCoinCollected(u32 courseID);
        static u32 GetCollectedCoinCount();
        static u32 GetTotalCoinCount();

        static void Initialize();
        static void InitializeLanguage();
        static bool HasChallenge();
        static GOBJEntry GetBlueCoinGOBJ();
        static void CalcObj(u32 objBase);
        static void DespawnCoin();
        static void OnKartHitCoin(u32 vehicleMove);
        static void SaveCoinLoc();
        static void DumpCoinLocsToFile();
    };
}