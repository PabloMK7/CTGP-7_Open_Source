/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ItemHandler.cpp
Open source lines: 1278/1280 (99.84%)
*****************************************************/

#include "ItemHandler.hpp"
#include "MarioKartFramework.hpp"
#include "MenuPage.hpp"
#include "ExtraResource.hpp"
#include "DataStructures.hpp"
#include "cheats.hpp"
#include "CwavReplace.hpp"

namespace CTRPluginFramework {

    u32 ItemHandler::extraKouraG = 0;
    u32 ItemHandler::extraKouraR = 0;
    u32 ItemHandler::extraKouraB = 0;

    u32 ItemHandler::fakeBoxDirector = 0;
    u32 ItemHandler::megaMushDirector = 0;

    ItemHandler::ItemDirectorVtable* ItemHandler::FakeBoxHandler::directorVtable = nullptr;
    ItemHandler::FakeBoxHandler::FakeBoxData* ItemHandler::FakeBoxHandler::fakeBoxData[FakeBoxAmount];
    u8* ItemHandler::FakeBoxHandler::fibStandCwav;
    bool ItemHandler::FakeBoxHandler::isHittingFib;
    int ItemHandler::FakeBoxHandler::creatingFakeBox = -1;
    ItemHandler::ItemObjVtable* ItemHandler::FakeBoxHandler::FakeBoxData::fakeBoxVtable = nullptr;
    ObjectItemBoxVtable* ItemHandler::FakeBoxHandler::FakeBoxData::fakeBoxItemBoxVtable = nullptr;

    ItemHandler::ItemObjVtable* ItemHandler::GameAddr::bananaVtable = nullptr; //(ItemHandler::ItemObjVtable*)0x6262E4;
    ObjectItemBoxVtable* ItemHandler::GameAddr::itemBoxVtable = nullptr; //(ObjectItemBoxVtable*)0x0062EFA8;
    ItemHandler::ItemDirectorVtable* ItemHandler::GameAddr::kouraGDirectorVtable = nullptr;//(ItemHandler::ItemDirectorVtable*)0x6274EC;

    u32 ItemHandler::GameAddr::kartItemSetEquipInfoAddr = 0;
    u32 ItemHandler::GameAddr::startDashKinokoAddr = 0;
    u32 ItemHandler::GameAddr::directorBaseEntryUseFuncAddr = 0;
    u32 ItemHandler::GameAddr::sequenceUseKinoko3Addr = 0;
    u32 ItemHandler::GameAddr::sequenceUseKinoko2Addr = 0;
    u32 ItemHandler::GameAddr::sequenceUseAnimAddr = 0;

    u32 ItemHandler::GameAddr::itemDirectorConsAddr = 0;

    u32 ItemHandler::GameAddr::dropEquipAddr = 0;
    u32 ItemHandler::GameAddr::createItemBoxBreakEffectAddr = 0;

    u32 ItemHandler::GameAddr::matrixScaleBasesAddr  = 0;
    u32 ItemHandler::GameAddr::drawMdlSetPoseAddr = 0;

    u32 ItemHandler::GameAddr::getObjectParameterAddr = 0;

    u32 ItemHandler::GameAddr::itemBoxGeneratorConsAddr = 0;
    u32 ItemHandler::GameAddr::itemBoxGeneratorVtableAddr = 0;

    u32 ItemHandler::GameAddr::itemObjConsAddr = 0;
    u32 ItemHandler::GameAddr::itemObjConstInfoAddr = 0;
    u32 ItemHandler::GameAddr::gameParticleConsAddr = 0;

    u32 ItemHandler::GameAddr::sndengine_holdsound = 0;

    u32 ItemHandler::GameAddr::itemObjStateInitUse = 0;

    ItemHandler::ItemObjVtable* ItemHandler::GameAddr::starVtable = (decltype(ItemHandler::GameAddr::starVtable))0x625F14;
    ItemHandler::ItemDirectorVtable* ItemHandler::GameAddr::starDirectorVtable = (decltype(ItemHandler::GameAddr::starDirectorVtable))0x00626E5C;

    

    ItemHandler::ItemDirectorVtable* ItemHandler::MegaMushHandler::directorVtable = nullptr;
    ItemHandler::ItemObjVtable* ItemHandler::MegaMushHandler::itemVtable = nullptr;
    void* ItemHandler::MegaMushHandler::growSound = nullptr;
    void* ItemHandler::MegaMushHandler::shrinkSound = nullptr;
    void* ItemHandler::MegaMushHandler::megaTheme = nullptr;
    u8 ItemHandler::MegaMushHandler::growMapFacePending[8] = {0};

    void ItemHandler::Initialize() {
        FakeBoxHandler::Initialize();
        MegaMushHandler::Initialize();
    }

    u32 ItemHandler::GetDirectorFromSlot(u32 itemDirector, EItemSlot itemSlot) {
        // Do not use R3, for some reason this function in MK7 doesn't follow the arm ABI
        asm ("push {r3}");
        u32* directorArray = (u32*)(itemDirector + 0x28);
        u32 ret = 0;
        switch (itemSlot)
        {
        case EItemSlot::ITEM_BANANA:
            ret = directorArray[EItemDirector::IDIR_BANANA];
            break;
        case EItemSlot::ITEM_KOURAG:
            ret = directorArray[EItemDirector::IDIR_KOURAG];
            break;
        case EItemSlot::ITEM_KOURAR:
            ret = directorArray[EItemDirector::IDIR_KOURAR];
            break;
        case EItemSlot::ITEM_KINOKO:
            ret = directorArray[EItemDirector::IDIR_KINOKO];
            break;
        case EItemSlot::ITEM_BOMBHEI:
            ret = directorArray[EItemDirector::IDIR_BOMB];
            break;
        case EItemSlot::ITEM_GESSO:
            ret = directorArray[EItemDirector::IDIR_GESSO];
            break;
        case EItemSlot::ITEM_KOURAB:
            ret = directorArray[EItemDirector::IDIR_KOURAB];
            break;
        case EItemSlot::ITEM_KINOKO3:
            ret = directorArray[EItemDirector::IDIR_KINOKO];
            break;
        case EItemSlot::ITEM_STAR:
            ret = directorArray[EItemDirector::IDIR_STAR];
            break;
        case EItemSlot::ITEM_KILLER:
            ret = directorArray[EItemDirector::IDIR_KILLER];
            break;
        case EItemSlot::ITEM_THUNDER:
            ret = directorArray[EItemDirector::IDIR_THUNDER];
            break;
        case EItemSlot::ITEM_KINOKOP:
            ret = directorArray[EItemDirector::IDIR_KINOKO];
            break;
        case EItemSlot::ITEM_FLOWER:
            ret = directorArray[EItemDirector::IDIR_FLOWER];
            break;
        case EItemSlot::ITEM_KONOHA:
            ret = directorArray[EItemDirector::IDIR_TAIL];
            break;
        case EItemSlot::ITEM_SEVEN:
            ret = directorArray[EItemDirector::IDIR_SEVEN];
            break;
        case EItemSlot::ITEM_TEST3: // Fake Item Box
            ret = fakeBoxDirector;
            break;
        case EItemSlot::ITEM_TEST4:
            ret = megaMushDirector;
            break;
        case EItemSlot::ITEM_BANANA3:
            ret = directorArray[EItemDirector::IDIR_BANANA];
            break;
        case EItemSlot::ITEM_KOURAG3:
            ret = directorArray[EItemDirector::IDIR_KOURAG];
            break;
        case EItemSlot::ITEM_KOURAR3:
            ret = directorArray[EItemDirector::IDIR_KOURAR];
            break;
        default:
            break;
        }
        // Restore the saved R3
        asm ("pop {r3}");
        return ret;
    }
    u32 ItemHandler::GetDirectorFromType(u32 itemDirector, EItemType itemType) {
        // Do not use R3, for some reason this function in MK7 doesn't follow the arm ABI
        asm ("push {r3}");
        u32* directorArray = (u32*)(itemDirector + 0x28);
        u32 ret = 0;
        switch (itemType)
        {
        case EItemType::ITYPE_KOURAG:
            ret = directorArray[EItemDirector::IDIR_KOURAG];
            break;
        case EItemType::ITYPE_KOURAR:
            ret = directorArray[EItemDirector::IDIR_KOURAR];
            break;
        case EItemType::ITYPE_BANANA:
            ret = directorArray[EItemDirector::IDIR_BANANA];
            break;
        case EItemType::ITYPE_KINOKO:
            ret = directorArray[EItemDirector::IDIR_KINOKO];
            break;
        case EItemType::ITYPE_STAR:
            ret = directorArray[EItemDirector::IDIR_STAR];
            break;
        case EItemType::ITYPE_KOURAB:
            ret = directorArray[EItemDirector::IDIR_KOURAB];
            break;
        case EItemType::ITYPE_THUNDER:
            ret = directorArray[EItemDirector::IDIR_THUNDER];
            break;
        case EItemType::ITYPE_FAKEBOX:
            ret = fakeBoxDirector;
            break;
        case EItemType::ITYPE_KINOKOP:
            ret = 0;
            break;
        case EItemType::ITYPE_BOMB:
            ret = directorArray[EItemDirector::IDIR_BOMB];
            break;
        case EItemType::ITYPE_GESSO:
            ret = directorArray[EItemDirector::IDIR_GESSO];
            break;
        case EItemType::ITYPE_BIGKINOKO:
            ret = megaMushDirector;
            break;
        case EItemType::ITYPE_KILLER:
            ret = directorArray[EItemDirector::IDIR_KILLER];
            break;
        case EItemType::ITYPE_FLOWER:
            ret = directorArray[EItemDirector::IDIR_FLOWER];
            break;
        case EItemType::ITYPE_TAIL:
            ret = directorArray[EItemDirector::IDIR_TAIL];
            break;
        case EItemType::ITYPE_SEVEN:
            ret = directorArray[EItemDirector::IDIR_SEVEN];
            break;
        default:
            break;
        }
        // Restore the saved R3
        asm ("pop {r3}");
        return ret;
    }

    u32 ItemHandler::GetItemMaxAmount(EItemType item) {
        u32 raceDirector = MarioKartFramework::getRaceDirector();
        u32 itemMode = (((u32**)raceDirector)[0x1BC/4] + 0x64/4)[0x17C/4];
        bool isSpecialMode = (itemMode == 1 || itemMode == 2 || itemMode == 3 || itemMode == 4);
        bool fibAllowed = g_getCTModeVal == CTMode::OFFLINE || g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD;
        bool megaAllowed = g_getCTModeVal == CTMode::OFFLINE || g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD;

        switch (item)
        {
        case EItemType::ITYPE_KOURAG:
            return (itemMode == 2) ? 16 : ((fibAllowed ? 7 : 8) + extraKouraG);
        case EItemType::ITYPE_KOURAR:
            return (itemMode == 2) ? 16 : ((fibAllowed ? 7 : 8) + extraKouraR);
        case EItemType::ITYPE_BANANA:
            return (itemMode == 3) ? 24 : (fibAllowed ? 6 : 8);
        case EItemType::ITYPE_KINOKO:
            return (itemMode == 1) ? 24 : 8;
        case EItemType::ITYPE_STAR:
            return 8;
        case EItemType::ITYPE_KOURAB:
            return 1 + extraKouraB;
        case EItemType::ITYPE_THUNDER:
            return (!isSpecialMode) ? 8 : 1;
        case EItemType::ITYPE_FAKEBOX:
            return (!isSpecialMode) ? (fibAllowed ? FakeBoxHandler::FakeBoxAmount : 1) : 1;
        case EItemType::ITYPE_KINOKOP:
            return 5;
        case EItemType::ITYPE_BOMB:
            return (itemMode == 4) ? 24 : 8;
        case EItemType::ITYPE_GESSO:
            return (!isSpecialMode) ? 9 : 1;
        case EItemType::ITYPE_BIGKINOKO:
            return (!isSpecialMode) ? (megaAllowed ? MegaMushHandler::MegaMushAmount : 1) : 1;
        case EItemType::ITYPE_KILLER:
            return (!isSpecialMode) ? 8 : 1;
        case EItemType::ITYPE_FLOWER:
            return (!isSpecialMode) ? 3 : 1;
        case EItemType::ITYPE_TAIL:
            return (!isSpecialMode) ? 8 : 1;
        case EItemType::ITYPE_SEVEN:
            return 0;
            break;
        default:
            return -1;
            break;
        }
    }

    u32 ItemHandler::GetItemMaxAmountNet(EItemType item, bool unused) { // Used for net and local, for some reason
        u32 raceDirector = MarioKartFramework::getRaceDirector();
        u32 itemMode = (((u32**)raceDirector)[0x1BC/4] + 0x64/4)[0x17C/4];
        bool isSpecialMode = (itemMode == 1 || itemMode == 2 || itemMode == 3 || itemMode == 4);
        bool fibAllowed = g_getCTModeVal == CTMode::OFFLINE || g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD;
        bool megaAllowed = g_getCTModeVal == CTMode::OFFLINE || g_getCTModeVal == CTMode::ONLINE_CTWW || g_getCTModeVal == CTMode::ONLINE_CTWW_CD;

        switch (item)
        {
        case EItemType::ITYPE_KOURAG:
            return (itemMode == 2) ? 16 : ((fibAllowed ? 7 : 8) + extraKouraG) ;
        case EItemType::ITYPE_KOURAR:
            return (itemMode == 2) ? 16 : ((fibAllowed ? 7 : 8) + extraKouraR);
        case EItemType::ITYPE_BANANA:
            return (itemMode == 3) ? 24 : (fibAllowed ? 6 : 8);
        case EItemType::ITYPE_KINOKO:
            return (itemMode == 1) ? 24 : 8;
        case EItemType::ITYPE_STAR:
            return 2;
        case EItemType::ITYPE_KOURAB:
            return 1 + extraKouraB;
        case EItemType::ITYPE_THUNDER:
            return (!isSpecialMode) ? 3 : 1;
        case EItemType::ITYPE_FAKEBOX:
            return (!isSpecialMode) ? (fibAllowed ? FakeBoxHandler::FakeBoxAmount : 1) : 1;
        case EItemType::ITYPE_KINOKOP:
            return 5;
        case EItemType::ITYPE_BOMB:
            return (itemMode == 4) ? 24 : 8;
        case EItemType::ITYPE_GESSO:
            return 1;
        case EItemType::ITYPE_BIGKINOKO:
            return (!isSpecialMode) ? (megaAllowed ? MegaMushHandler::MegaMushAmount : 1) : 1;
        case EItemType::ITYPE_KILLER:
            return 1;
        case EItemType::ITYPE_FLOWER:
            return (!isSpecialMode) ? 3 : 1;
        case EItemType::ITYPE_TAIL:
            return (!isSpecialMode) ? 3 : 1;
        case EItemType::ITYPE_SEVEN:
            return 7;
            break;
        default:
            return -1;
            break;
        }
    }

    u32 ItemHandler::GetExtraItemNum(EItemType item) {
        u32 raceDirector = MarioKartFramework::getRaceDirector();
        u32 itemMode = (((u32**)raceDirector)[0x1BC/4] + 0x64/4)[0x17C/4];
        switch (item)
        {
        case EItemType::ITYPE_KOURAG:
            return (itemMode == 2) ? 7 : 3;
        case EItemType::ITYPE_KOURAR:
            return (itemMode == 2) ? 7 : 3;
        case EItemType::ITYPE_BANANA:
            return (itemMode == 3) ? 7 : 3;
        case EItemType::ITYPE_KINOKO:
            return (itemMode == 1) ? 7 : 3;
        case EItemType::ITYPE_BOMB:
            return (itemMode == 4) ? 7 : 3;
        default:
            return 0;
            break;
        }
    }

    u32 ItemHandler::GetStripeAmount(u32 mode) {
        if (mode == 2)
            return 47 + extraKouraG + extraKouraR + extraKouraB;
        else
            return 23 + extraKouraG + extraKouraR + extraKouraB;
    }

    void ItemHandler::UseItem(u32 itemDirector, EItemSlot item, u32* kartItem) {
        u32 infoProxy = kartItem[0x2C/4];
        u32 vehicle = ((u32*)infoProxy)[0];
        int playerID = ((u32*)vehicle)[0x84/4];
        u32* directorArray = (u32*)(MarioKartFramework::getItemDirector() + 0x28);
        void (*kartItemSetEquipInfo)(u32* kartItem, EItemSlot item, u32 equipType, int amount, bool unknown2) = (void(*)(u32*, EItemSlot, u32, int, bool))GameAddr::kartItemSetEquipInfoAddr;
        void (*startDashKinoko)(u32 vehicle, bool unknown) = (void(*)(u32, bool))GameAddr::startDashKinokoAddr;
        u32* (*directorBaseEntryUseFunc)(u32 director, u32 kartInfoProxy, bool something) = (u32*(*)(u32, u32, bool))GameAddr::directorBaseEntryUseFuncAddr;
        void (*sequenceUseKinoko3)(int playerID) = (void(*)(int))GameAddr::sequenceUseKinoko3Addr;
        void (*sequenceUseKinoko2)(int playerID) = (void(*)(int))GameAddr::sequenceUseKinoko2Addr;
        void (*sequenceUseAnim)() = (void(*)())GameAddr::sequenceUseAnimAddr;

        // Custom
        MarioKartFramework::warnLedItem(item);
        MarioKartFramework::handleItemCD(vehicle, item);
        //

        switch (item)
        {
        case ITEM_BANANA:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_BANANA];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_BANANA, 1, 1, true);
            }
        }
        break;
        case ITEM_KOURAG:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KOURAG];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_KOURAG, 1, 1, true);
            }
        }
        break;
        case ITEM_KOURAR:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KOURAR];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_KOURAR, 1, 1, true);
            }
        }
        break;
        case ITEM_KINOKO:
        {
            startDashKinoko(vehicle, false);
            FixedArrayPtr<u32>* boostTimers = (FixedArrayPtr<u32>*)(itemDirector + 0xE8);
            (*boostTimers)[playerID] = ((u32**)itemDirector)[0xF0/4][0x4D8/4];
        }
        break;
        case ITEM_BOMBHEI:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_BOMB];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_BOMBHEI, 1, 1, true);
            }
        }
        break;
        case ITEM_GESSO:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_GESSO];
            directorBaseEntryUseFunc(typeDirector, infoProxy, false);
        }
        break;
        case ITEM_KOURAB:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KOURAB];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_KOURAB, 1, 1, true);
            }
        }
        break;
        case ITEM_KINOKO3:
        {
            startDashKinoko(vehicle, false);
            FixedArrayPtr<u32>* boostTimers = (FixedArrayPtr<u32>*)(itemDirector + 0xE8);
            (*boostTimers)[playerID] = ((u32**)itemDirector)[0xF0/4][0x4D8/4];
            u32 itemAmount = kartItem[0x3C/4];
            if (itemAmount == 3)
                sequenceUseKinoko3(playerID);
            else if (itemAmount == 2)
                sequenceUseKinoko2(playerID);
        }
        break;
        case ITEM_STAR:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_STAR];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, -1);
        }
        break;
        case ITEM_KILLER:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KILLER];
            directorBaseEntryUseFunc(typeDirector, infoProxy, false);
        }
        break;
        case ITEM_THUNDER:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_THUNDER];
            directorBaseEntryUseFunc(typeDirector, infoProxy, false);
        }
        break;
        case ITEM_KINOKOP:
        {
            startDashKinoko(vehicle, false);
            FixedArrayPtr<u32>* boostTimers = (FixedArrayPtr<u32>*)(itemDirector + 0xE8);
            (*boostTimers)[playerID] = ((u32**)itemDirector)[0xF0/4][0x4D8/4];
        }
        break;
        case ITEM_FLOWER:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_FLOWER];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_FLOWER, 1, 1, kartItem[0x50/4] == -1);
            }
        }
        break;
        case ITEM_KONOHA:
        {
            if (!((u8*)vehicle)[0x9E]) {
                u32 flags = ((u32*)vehicle)[0xC30/4];
                bool isMaster = ((bool*)vehicle)[0x98];
                if (flags & 0x1000000) {
                    if (isMaster) sequenceUseAnim();
                    break;
                }
            }
            u32 typeDirector = directorArray[EItemDirector::IDIR_TAIL];
            directorBaseEntryUseFunc(typeDirector, infoProxy, false);
        }
        break;
        case ITEM_SEVEN:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_SEVEN];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, 7);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_SEVEN, 2, 7, true);
            }
        }
        break;
        case ITEM_TEST3: // Fake Box
        {
            u32 typeDirector = fakeBoxDirector;
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, true, -1);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_TEST3, 1, 1, true);
            }
        }
        break;
        case ITEM_TEST4:
        {
            u32 typeDirector = megaMushDirector;
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, -1);
        }
        break;
        case ITEM_BANANA3:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_BANANA];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, 3);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_BANANA3, 2, 3, true);
            }
        }
        break;
        case ITEM_KOURAG3:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KOURAG];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, 3);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_KOURAG3, 2, 3, true);
            }
        }
        break;
        case ITEM_KOURAR3:
        {
            u32 typeDirector = directorArray[EItemDirector::IDIR_KOURAR];
            u32*(*directorBaseEntryFunc)(u32 director, u32 kartInfoProxy, bool isSingle, u32 multipleAmount) = (u32*(*)(u32, u32, bool, u32))(((u32**)typeDirector)[0][0x80/4]);
            u32* ret = directorBaseEntryFunc(typeDirector, infoProxy, false, 3);
            if (ret) {
                kartItemSetEquipInfo(kartItem, ITEM_KOURAR3, 2, 3, true);
            }
        }
        break;
        default:
            break;
        }
    }

    void ItemHandler::OnItemDirectorCreateBeforeStructure(u32 itemDirector) {
        // FIB
        fakeBoxDirector = (u32)GameAlloc::game_operator_new_autoheap(0xBC);
        if (fakeBoxDirector) {
            u32(*itemDirectorCons)(u32) = (u32(*)(u32))GameAddr::itemDirectorConsAddr;
            fakeBoxDirector = itemDirectorCons(fakeBoxDirector);
            ((u32*)fakeBoxDirector)[0] = (u32)FakeBoxHandler::directorVtable;
            ((u32*)fakeBoxDirector)[4/4] = itemDirector;
        }
        ActorArray* actorArray = (ActorArray*)(itemDirector + 0x14);
        actorArray->Push(fakeBoxDirector);
        // Mega Mush
        megaMushDirector = (u32)GameAlloc::game_operator_new_autoheap(0xBC);
        if (megaMushDirector) {
            u32(*itemDirectorCons)(u32) = (u32(*)(u32))GameAddr::itemDirectorConsAddr;
            megaMushDirector = itemDirectorCons(megaMushDirector);
            ((u32*)megaMushDirector)[0] = (u32)MegaMushHandler::directorVtable;
            ((u32*)megaMushDirector)[4/4] = itemDirector;
        }
        actorArray->Push(megaMushDirector);
    }

    EItemType ItemHandler::OnKartHitItem(u32* vehicle, u32 itemReactProxy, u32* reactParams, u32 itemDirector) { 
        // reactParams[0] -> itemTypeEquip, reactParams[1] -> isInvencible
        u8* objBase = ((u8**)itemReactProxy)[0];
        EItemType iType = (EItemType)objBase[0x156];
        u32 playerID = vehicle[0x84/4];

        if (!reactParams[1] && MarioKartFramework::megaMushTimers[playerID]) {
            MarioKartFramework::ignoreKartAccident = true;
        }

        if (iType == EItemType::ITYPE_FAKEBOX) {
            if (reactParams[0] > 0x10)
                reactParams[0] = EItemType::ITYPE_KOURAG + 0x11;
            else
                reactParams[0] = EItemType::ITYPE_KOURAG;

            if (!reactParams[1]) { // If not stunt
                

                if (MarioKartFramework::kartCanAccident(vehicle) && !MarioKartFramework::megaMushTimers[playerID])
                {
                    FakeBoxHandler::isHittingFib = true;
                    u32 itemDirector = MarioKartFramework::getItemDirector();
                    void(*dropEquip)(u32 self, u32 playerID) = (void(*)(u32, u32))GameAddr::dropEquipAddr;
                    dropEquip(itemDirector, playerID);
                }
                
                Vector3* itemPos = (Vector3*)(objBase + 0xA8);
                Vector3 effectPos = *itemPos;
                float itemBoxYoffset = 18.f; //((float*)0x65FF60)[0xC/4];
                effectPos.y += (itemBoxYoffset - GameAddr::bananaVtable->getOffset_Hang_Y() - 3.f);
                void(*createItemBoxBreakEffect)(u32 effectDirector, u32 playerID, Vector3* position) = (void(*)(u32, u32, Vector3*))GameAddr::createItemBoxBreakEffectAddr;
                createItemBoxBreakEffect(MarioKartFramework::getEffectDirector(), playerID, &effectPos);

                FakeBoxHandler::PlaySoundEffect((u32)objBase, Snd::SoundID::BREAK_ITEMBOX);
            }
            
            return EItemType::ITYPE_KOURAG;      
        } /*else if (iType == EItemType::ITYPE_BIGKINOKO) {
            SeadArrayPtr<u32*>& kartItems = *(SeadArrayPtr<u32*>*)(itemDirector + 0xC0);
            UseItem(itemDirector, EItemSlot::ITEM_TEST4, kartItems[playerID]);
        }*/
        return iType;
    }

    const char* ItemHandler::GetTextureNameForItem(u32 itslottexid) {
        if (itslottexid == 19)
            return "fib";
        else if (itslottexid == 20)
            return "mega";
        else
            return nullptr;
    }

    u32 ItemHandler::GetDriverTimerForItem(u32 itemDirector, u32 infoProxy) {
        EItemType type = (EItemType)(((u8*)(((u32**)itemDirector)[0x10/4][0x0]))[0x156]);
        u32* vehicle = ((u32**)infoProxy)[0];
        if (type == EItemType::ITYPE_STAR) {
            return vehicle[0xFF4/4];
        } else if (type == EItemType::ITYPE_BIGKINOKO) {
            return MarioKartFramework::megaMushTimers[vehicle[0x84/4]];
        }
        return 0;
    }

    void ItemHandler::FakeBoxHandler::Initialize() {
        ExtraResource::SARC::FileInfo fInfo;
        fibStandCwav = ExtraResource::mainSarc->GetFile("RaceCommon.szs/fakeBox/fakeBoxStand.bcwav", &fInfo);

        directorVtable = (ItemDirectorVtable*)operator new(sizeof(ItemDirectorVtable));
        memcpy(FakeBoxHandler::directorVtable, (void*)GameAddr::kouraGDirectorVtable, sizeof(ItemDirectorVtable));
        
        directorVtable->createBeforeStructure = CreateBeforeStructure;

        FakeBoxData::fakeBoxVtable = (ItemObjVtable*)operator new(sizeof(ItemObjVtable));
        memcpy(FakeBoxData::fakeBoxVtable, GameAddr::bananaVtable, sizeof(ItemObjVtable));

        FakeBoxData::fakeBoxVtable->createInner = ObjCreateInner;
        FakeBoxData::fakeBoxVtable->calc = FakeBoxCalc;
        FakeBoxData::fakeBoxVtable->getColRadius = GetColRadius;
        FakeBoxData::fakeBoxVtable->getBoxColRadius = GetBoxColRadius;

        FakeBoxData::fakeBoxVtable->stateInitWait = OnInitWait;
        FakeBoxData::fakeBoxVtable->stateInitEquip_Hang = OnInitEquip;
        FakeBoxData::fakeBoxVtable->stateInitSelfMove = OnInitSelfMove;
        FakeBoxData::fakeBoxVtable->stateInitDrop = OnInitDrop;
        FakeBoxData::fakeBoxVtable->stateInitStand = OnInitStand;
        FakeBoxData::fakeBoxVtable->stateInitUse = OnInitUse;
        FakeBoxData::fakeBoxVtable->stateInitAttacked = OnInitAttacked;

        FakeBoxData::fakeBoxItemBoxVtable = (ObjectItemBoxVtable*)operator new(sizeof(ObjectItemBoxVtable));
        memcpy(FakeBoxData::fakeBoxItemBoxVtable, GameAddr::itemBoxVtable, sizeof(ObjectItemBoxVtable));

        FakeBoxData::fakeBoxItemBoxVtable->getParamForShare = GetParameterForShare;
        FakeBoxData::fakeBoxItemBoxVtable->calcAppearing = OnCalcAppearing;
        FakeBoxData::fakeBoxItemBoxVtable->doBreakItemBox = OnDoBreakItemBox;
    }

    void ItemHandler::FakeBoxHandler::nullfunc(u32 fakebox) {return;}

    void ItemHandler::FakeBoxHandler::PatchObjFlowForFIB() {
        SeadArrayPtr<ObjFlowEntry**>* flowEntries = (SeadArrayPtr<ObjFlowEntry**>*)(MarioKartFramework::getObjectDirector() + 0x28);
        for(int i = 0; i < flowEntries->count; i++) {
            ObjFlowEntry* cur = (*flowEntries)[i][0];
            if (cur->objectID == 0x143) {
                strcpy(cur->name, "fakeBox");
                break;
            }
        }
    }

    void ItemHandler::FakeBoxHandler::CalcAppearing(u32 itemBox, float progress) {
        u32 itemBoxFlags = itemBox + 0x200;
        u32* itemBoxPtr = (u32*)itemBox;
        Vector3* itemBoxPos = (Vector3*)(itemBox + 0x48);
        Vector3* itemBoxScale = (Vector3*)(itemBox + 0x60);
        Vector3* questionPos = (Vector3*)(itemBox + 0x1F8);
        FakeBoxData* currBoxData;
        for (int i = 0; i < FakeBoxAmount; i++) {
            if (fakeBoxData[i]->fakeBoxItemBoxGens->GetAsItembox() == (ObjectItemBox*)itemBox) {
                currBoxData = fakeBoxData[i];
                break;
            }
        }

        *questionPos = *itemBoxPos;
        questionPos->y -= 15.f;

        
        ((u8*)itemBoxFlags)[0x7] = 0;
        float multFactor = ((float*)itemBox)[0x210/4];

        progress = progress * multFactor;
        //float finalScale = (progress * 3.6667f) - (progress * progress * 2.6667f);
        float finalScale = progress;
        if (((u8*)itemBoxFlags)[0x14] != 0) {
            *itemBoxPos = *questionPos;
            itemBoxPos->y += finalScale * 15.f;
            itemBoxPtr[0x6C/4] |= 1;
        }

        itemBoxScale->x = finalScale;
        itemBoxScale->y = finalScale;
        itemBoxScale->z = finalScale;
        itemBoxPtr[0x6C/4] |= 4;

        float questionPose[12];
        memcpy(questionPose, (float*)(((u32**)itemBoxPtr)[0x1F4/4][0x40/4]), 12 * 4);
        void(*matrixScaleBases)(float matrix[12], float scalex, float scaley, float scalez) = (void(*)(float*, float, float, float))GameAddr::matrixScaleBasesAddr;
        matrixScaleBases(questionPose, finalScale, finalScale, finalScale);
        questionPose[3] = itemBoxPos->x;
        questionPose[7] = itemBoxPos->y;
        questionPose[11] = itemBoxPos->z;
        void(*drawMdlSetPose)(u32 drawModel, float pose[12]) = (void(*)(u32, float*))GameAddr::drawMdlSetPoseAddr;
        drawMdlSetPose(itemBoxPtr[0x1F4/4], questionPose);

        if (!(itemBoxPtr[0x4/4] & 0x10)) {
            currBoxData->needScaleBases = false;
            ((ObjectItemBox*)itemBox)->vtable->update_GrShadowScale(itemBox);
        }
    }

    void ItemHandler::FakeBoxHandler::OnCalcAppearing(u32 itemBox) {
        /*Vector3* itemBoxPos = (Vector3*)(itemBox + 0x48);
        Vector3* questionPos = (Vector3*)(itemBox + 0x1F8);
        Vector3 itBoxBak = *itemBoxPos;

        CalcAppearing(itemBox);
        *itemBoxPos = itBoxBak;*/
        u32* itemBoxPtr = (u32*)itemBox;
        u32 itemBoxFlags = itemBox + 0x200;
        u16* timer = ((u16*)itemBoxFlags) + 0x4/2;
        
        if (*timer == 20) {
            ((u8*)itemBoxPtr)[0x7C] = 1;
            if (!(itemBoxPtr[0x4/4] & 0x10)) ((u8*)(itemBoxPtr[0x260/4]))[0x6C] = 0;
        }
        float progress = (20 - *timer) + 1;
        if (*timer == 5) *timer = 6;
        float multiplier;
        if (*timer >= 5)
            multiplier = 0.7f;
        else
            multiplier = ((5 - *timer) / 5.f) * 0.3f + 0.7f;
        CalcAppearing(itemBox, progress * multiplier);
    }

    void ItemHandler::FakeBoxHandler::OnDoBreakItemBox(u32 itemBox) {
        GameAddr::itemBoxVtable->doBreakItemBox(itemBox);
        ((u16*)itemBox)[0x20E/2] = 1;
    }

    u32 ItemHandler::FakeBoxHandler::GetParameterForShare(u32 itemBox) {
        return ((u32(*)(u32))GameAddr::getObjectParameterAddr)(0x143);
    }

    float ItemHandler::FakeBoxHandler::GetColRadius() {
        return 8.f;
    }

    float ItemHandler::FakeBoxHandler::GetBoxColRadius() {
        return 12.f;
    }

    void ItemHandler::FakeBoxHandler::ChangeState(u32 fakeBox, FakeBoxState state) {
        u32 id = ((u32*)fakeBox)[0x160/4];
        fakeBoxData[id]->prevState = fakeBoxData[id]->state;
        fakeBoxData[id]->state = state;
        fakeBoxData[id]->changedState = true;
    }

    void ItemHandler::FakeBoxHandler::OnInitWait(u32 fakeBox) {
        ChangeState(fakeBox, FakeBoxState::WAIT);
        GameAddr::bananaVtable->stateInitWait(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitEquip(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::EQUIP);
        GameAddr::bananaVtable->stateInitEquip_Hang(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitSelfMove(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::SELFMOVE);
        GameAddr::bananaVtable->stateInitSelfMove(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitDrop(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::DROP);
        GameAddr::bananaVtable->stateInitDrop(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitStand(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::STAND);
        GameAddr::bananaVtable->stateInitStand(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitUse(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::USE);
        GameAddr::bananaVtable->stateInitUse(fakeBox);
    }

    void ItemHandler::FakeBoxHandler::OnInitAttacked(u32 fakeBox) {
        ChangeState(fakeBox,  FakeBoxState::ATTACKED);
        GameAddr::bananaVtable->stateInitAttacked(fakeBox);
    }

    bool ItemHandler::FakeBoxHandler::PlaySoundEffect(u32 fakebox, Snd::SoundID soundID) {
        u32 soundPlayer = ((u32*)fakebox)[0x1A8/4];
        void*(*playSound)(u32 soundPlayer, u32 ID, void* playerInfo) = (void*(*)(u32,u32,void*))(((u32**)soundPlayer)[0][0x70/4]);
        return playSound(soundPlayer, (u32)soundID, nullptr) != nullptr;
    }

    u32 ItemHandler::FakeBoxHandler::GetParamReplacementID(u32 objID) {
        if (creatingFakeBox >= 0 && objID == 4)
            return 0x143;
        else
            return 4;
    }

    u8* ItemHandler::FakeBoxHandler::GetFakeItemBox(ExtraResource::SARC::FileInfo* fileInfo) {
        if (creatingFakeBox >= 0)
            return ExtraResource::mainSarc->GetFile("RaceCommon.szs/fakeItemBox/fakeItemBox.bcmdl", fileInfo);
        else
            return nullptr;
    }
    
    void ItemHandler::FakeBoxHandler::OnItemBoxCreate(ObjectItemBox* itemBox) {
        if (creatingFakeBox < 0) return;

        itemBox->vtable = FakeBoxData::fakeBoxItemBoxVtable;
        itemBox->actorStart = (u32)itemBox->vtable + 0x184;
    }

    u32* ItemHandler::FakeBoxHandler::FakeBoxConstructor(u32* fakebox, int id) {
        u32*(*ItemObjCons)(u32* obj, EItemType type, int id) = (u32*(*)(u32*,EItemType,int))GameAddr::itemObjConsAddr;

        fakebox = ItemObjCons(fakebox, EItemType::ITYPE_FAKEBOX, id);
        if (fakeBoxData[id]) delete fakeBoxData[id];
        fakeBoxData[id] = new FakeBoxData();
        fakebox[0] = (u32)FakeBoxData::fakeBoxVtable;
        fakebox[0x1F8/4] = 0;
        ((float*)fakebox)[0x1FC] = 1.f;
        ((float*)fakebox)[0x200] = 1.f;
        ((float*)fakebox)[0x204] = 1.f;
        fakebox[0x208/4] = -1;
        //((u8*)fakebox)[0x156] = EItemType::ITYPE_BANANA;
        u32* miscInfo = (u32*)GameAddr::itemObjConstInfoAddr;
        fakebox[0xD0/4] = miscInfo[0x30/4];

        fakeBoxData[id]->itemBoxCustomCreateArgs = new CustomFieldObjectCreateArgs(0x143);
        fakeBoxData[id]->itemBoxCustomCreateArgs->args.params->lightSetIndex = 1;
        fakeBoxData[id]->itemBoxCustomCreateArgs->args.modelArgs.animationCount = 1;
        fakeBoxData[id]->itemBoxCustomCreateArgs->args.modelArgs.animationFlags = 0xA;
        fakeBoxData[id]->itemBoxCustomCreateArgs->args.modelArgs.sharedModelCount = FakeBoxAmount;
        fakeBoxData[id]->fakeBoxItemBoxGens = MarioKartFramework::GenerateItemBox(*fakeBoxData[id]->itemBoxCustomCreateArgs);

        creatingFakeBox = id;
        fakeBoxData[id]->fakeBoxItemBoxGens->vtable->createOuter(&fakeBoxData[id]->fakeBoxItemBoxGens->actorStart, nullptr);

        fakeBoxData[id]->fakeBoxItemBoxGens->vtable->init((u32)fakeBoxData[id]->fakeBoxItemBoxGens);
        if (fakeBoxData[id]->fakeBoxItemBoxGens->HasItembox()) {
            fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox()->vtable->init((u32)fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox());
            //fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox()->vtable->doBreakItemBox((u32)fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox());
            ((u32*)(fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox()))[0x120/4] = 0;
            ((bool*)(fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox()))[0x7D] = true;
        }
        creatingFakeBox = -1;
        return fakebox;
    }

    void ItemHandler::FakeBoxHandler::CreateBeforeStructure(u32 fakeBoxDirector, const void* createArg) {
        u32 itemAmount = GetItemMaxAmount(EItemType::ITYPE_FAKEBOX);
        u32 extraItemAmount = GetExtraItemNum(EItemType::ITYPE_FAKEBOX);
        u32* fakeBoxDirectorPtr = (u32*)fakeBoxDirector;
        u32* itemCount = (u32*)(fakeBoxDirector + 0x1C);
        *itemCount = itemAmount + extraItemAmount;
        ActorArray* actorArray = new ((ActorArray*)(fakeBoxDirector + 0x8)) ActorArray(itemAmount + extraItemAmount);

        PatchObjFlowForFIB();
        float* rankItemTexIDConverter = *(float**)(MarioKartFramework::getSequenceEngine() + 0xD4);
        rankItemTexIDConverter[EItemSlot::ITEM_TEST3] = 21.f;

        for (int i = 0; i < *itemCount; i++) {
            u32* fakeBox = (u32*)GameAlloc::game_operator_new_autoheap(0x23C);
            fakeBox = FakeBoxConstructor(fakeBox, i);
            actorArray->Push((u32)fakeBox);
        }
    }

    void ItemHandler::FakeBoxHandler::ObjCreateInner(u32 fakeBoxObj, const void* createArg) {
        // DrawMdlCreateArgs modelArgs;
        // strncpy((char*)modelArgs.fileName.strBase.data, "itemBox/itemBox.bcmdl", modelArgs.fileName.bufferSize);
        // modelArgs.unknown2 = true;
        // modelArgs.unknown5 = 0;
        // modelArgs.unknown6 = 2;
        // modelArgs.unknown7 = 4;
        // modelArgs.unknown14 = true;
        // modelArgs.unknown26 = 1;
        // void(*Actor3DMdlCreateModel)(u32 actor3dmodel, DrawMdlCreateArgs& args, void* drawmodel) = (void(*)(u32, DrawMdlCreateArgs&,void*))0x00410868;
        // void(*ItemObjBaseSetVisible)(u32 itemobj, bool visible, bool unk) = (void(*)(u32,bool,bool))0x002B4E90;
        u32*(*GameParticleCons)(u32* particle, const SafeStringBase& name, u32 unk) = (u32*(*)(u32*,const SafeStringBase&,u32))GameAddr::gameParticleConsAddr;
        
        //Actor3DMdlCreateModel(fakeBoxObj, modelArgs, nullptr);

        // ItemObjBaseSetVisible(fakeBoxObj, false, true);

        u32* particleEffect = (u32*)GameAlloc::game_operator_new_autoheap(0x70);
        particleEffect = GameParticleCons(particleEffect, SafeStringBase("cdn_item_bleak"), 0);
        ((u32**)fakeBoxObj)[0x1F8/4] = particleEffect;
    }
   
    void ItemHandler::FakeBoxHandler::FakeBoxCalc(u32 fakebox) {
        u32 id = ((u32*)fakebox)[0x160/4];
        u32 itemBox = fakeBoxData[id]->fakeBoxItemBoxGens->HasItembox() ? (u32)fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox() : 0;
        if (fakeBoxData[id]->state == FakeBoxState::NON_INIT)
            ChangeState(fakebox, FakeBoxState::WAIT);
        FakeBoxState currState = fakeBoxData[id]->state;
        FakeBoxState prevState = fakeBoxData[id]->prevState;
        bool stateChanged = fakeBoxData[id]->changedState;
        
        if (itemBox) {        
            Vector3* bananaPos = (Vector3*)(fakebox + 0xA8);
            Vector3* itemBoxPos = (Vector3*)(itemBox + 0x48);
            *itemBoxPos = *bananaPos;
            float itemBoxYoffset = 18.f;//((float*)0x65FF60)[0xC/4];
            itemBoxPos->y += (itemBoxYoffset - GameAddr::bananaVtable->getOffset_Hang_Y() - 3.f);

            if (currState != FakeBoxState::WAIT) {
                constexpr float startRed = 130000.f;
                constexpr float endRed = 50000.f;
                float distance = DistanceNoSqrt(*itemBoxPos, *MarioKartFramework::GetGameCameraPosition());
                float val = (distance - startRed) / (endRed - startRed);
                u32 itemBoxFlags = itemBox + 0x200;
                u16* timer = ((u16*)itemBoxFlags) + 0x4/2;

                MarioKartFramework::ObjModelBaseChangeAnimation(itemBox, 0, val * 60.f);             

                if (*timer) {
                    fakeBoxData[id]->needScaleBases = true;
                }
                
                fakeBoxData[id]->fakeBoxItemBoxGens->vtable->calc((u32)fakeBoxData[id]->fakeBoxItemBoxGens);
                fakeBoxData[id]->fakeBoxItemBoxGens->vtable->baseVtable.calcView((u32)fakeBoxData[id]->fakeBoxItemBoxGens);

                if (currState != FakeBoxState::EQUIP && *timer > 4 && stateChanged) {
                    *timer = 4;
                }
                if (currState == FakeBoxState::SELFMOVE && stateChanged) {
                    bool isBackDrop = ((bool*)fakebox)[0xF4];
                    if (!isBackDrop) fakeBoxData[id]->reflectAmount = 2;
                }

                if ((!(((u32*)(itemBox))[0x4/4] & 0x10)) && fakeBoxData[id]->needScaleBases) {
                    fakeBoxData[id]->needScaleBases = false;
                    ((ObjectItemBox*)itemBox)->vtable->update_GrShadowScale(itemBox);
                }
                
                u32* colFlags = ((u32**)fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox())[0x84/4] + 0x38/4;
                if (!(*colFlags & (1 << 20)))
                    *colFlags |= (1 << 20);
            } else if (currState == FakeBoxState::WAIT && stateChanged) {
                fakeBoxData[id]->fakeBoxItemBoxGens->GetAsItembox()->vtable->doBreakItemBox(itemBox);
                fakeBoxData[id]->reflectAmount = 0;

                fakeBoxData[id]->fakeBoxItemBoxGens->vtable->calc((u32)fakeBoxData[id]->fakeBoxItemBoxGens);
                fakeBoxData[id]->fakeBoxItemBoxGens->vtable->baseVtable.calcView((u32)fakeBoxData[id]->fakeBoxItemBoxGens);
            }
        }
        fakeBoxData[id]->changedState = false;
        GameAddr::bananaVtable->calc(fakebox);
    }

    void ItemHandler::FakeBoxHandler::OnBananaPlayStandSE(u32 fakeBox) {
        u32 id = ((u32*)fakeBox)[0x160/4];
        if (((u32*)fakeBox)[0] == (u32)(FakeBoxData::fakeBoxVtable)) {
            CwavReplace::LockState();
            if (PlaySoundEffect(fakeBox, Snd::SoundID::BANANA_STAND))
                CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::bananaDropSE, fibStandCwav, 1);
            CwavReplace::UnlockState();
            return;
        }
        PlaySoundEffect(fakeBox, Snd::SoundID::BANANA_STAND);
    }

    float ItemHandler::FakeBoxHandler::OnBananaShouldReflect(u32 fakeBox, float surfaceY, float reflectThreshold) {
        u32 id = ((u32*)fakeBox)[0x160/4];
        if (((u32*)fakeBox)[0] == (u32)(FakeBoxData::fakeBoxVtable)) {
            if (fakeBoxData[id]->reflectAmount != 0) {
                fakeBoxData[id]->reflectAmount--;
                if (surfaceY > reflectThreshold) {
                    Vector3& bananaSpeed = *(Vector3*)(fakeBox + 0xB4);
                    bananaSpeed *= 0.55f;
                    PlaySoundEffect(fakeBox, Snd::SoundID::BOMB_GND);
                    return surfaceY + 1.f;
                }
            }
        }
        return reflectThreshold;
    }

    void ItemHandler::FakeBoxHandler::PatchKartPctl(u8* kartPctl) {
        // 0x85A0
        constexpr u32 dsFlipperSparkColorOffset = 0x1C3C4;
        Color4* colors0 = (Color4*)(kartPctl + dsFlipperSparkColorOffset);
        colors0[3] = Color4(1.f, 0.f, 0.f, 1.f);
        colors0[4] = Color4(0.5f, 0.f, 0.f, 1.f);
        colors0[5] = Color4(0.25f, 0.f, 0.f, 1.f);
        
    }

    void ItemHandler::MegaMushHandler::Initialize() {
        ExtraResource::SARC::FileInfo fInfo;
        growSound = ExtraResource::mainSarc->GetFile("RaceCommon.szs/megaMush/megaMushGrow.bcwav", &fInfo);
        shrinkSound = ExtraResource::mainSarc->GetFile("RaceCommon.szs/megaMush/megaMushShrink.bcwav", &fInfo);
        megaTheme = ExtraResource::mainSarc->GetFile("RaceCommon.szs/megaMush/megaMushTheme.bcwav", &fInfo);

        directorVtable = (ItemDirectorVtable*)operator new(sizeof(ItemDirectorVtable));
        memcpy(MegaMushHandler::directorVtable, (void*)GameAddr::starDirectorVtable, sizeof(ItemDirectorVtable));
        
        directorVtable->createBeforeStructure = CreateBeforeStructure;

        itemVtable = (ItemObjVtable*)operator new(sizeof(ItemObjVtable));
        memcpy(itemVtable, GameAddr::starVtable, sizeof(ItemObjVtable));
        itemVtable->createInner = ObjCreateInner;
        itemVtable->stateInitUse = StateInitUse;
        itemVtable->stateUse = FakeBoxHandler::nullfunc;
    }

    void ItemHandler::MegaMushHandler::CreateBeforeStructure(u32 megaMushDirector, const void* createArg) {
        u32 itemAmount = GetItemMaxAmount(EItemType::ITYPE_BIGKINOKO);
        u32* megaMushDirectorrPtr = (u32*)megaMushDirector;
        u32* itemCount = (u32*)(megaMushDirector + 0x1C);
        *itemCount = itemAmount;
        ActorArray* actorArray = new ((ActorArray*)(megaMushDirector + 0x8)) ActorArray(itemAmount);
        u32*(*ItemObjCons)(u32* obj, EItemType type, int id) = (u32*(*)(u32*,EItemType,int))GameAddr::itemObjConsAddr;

        for (int i = 0; i < *itemCount; i++) {
            u32* megaMush = (u32*)GameAlloc::game_operator_new_autoheap(0x1F8);
            megaMush = ItemObjCons(megaMush, EItemType::ITYPE_BIGKINOKO, i);
            megaMush[0] = (u32)itemVtable;
            actorArray->Push((u32)megaMush);
        }

        float* rankItemTexIDConverter = *(float**)(MarioKartFramework::getSequenceEngine() + 0xD4);
        rankItemTexIDConverter[EItemSlot::ITEM_TEST4] = 22.f;
    }

    void ItemHandler::MegaMushHandler::ObjCreateInner(u32 megaMushObj, const void* createArg) {
        /*DrawMdlCreateArgs modelArgs;
        strncpy((char*)modelArgs.fileName.strBase.data, "Item/itemBigKinoko/itemBigKinoko.bcmdl", modelArgs.fileName.bufferSize);
        modelArgs.unknown2 = true;
        modelArgs.unknown5 = 0;
        modelArgs.animationCount = 2;
        modelArgs.animationFlags = 4;
        modelArgs.unknown14 = true;
        modelArgs.unknown26 = 1;
        void(*Actor3DMdlCreateModel)(u32 actor3dmodel, DrawMdlCreateArgs& args, void* drawmodel) = (void(*)(u32, DrawMdlCreateArgs&,void*))0x00410868;
        void(*ItemObjBaseSetVisible)(u32 itemobj, bool visible, bool unk) = (void(*)(u32,bool,bool))0x002B4E90;

        Actor3DMdlCreateModel(megaMushObj, modelArgs, nullptr);

        ItemObjBaseSetVisible(megaMushObj, false, true);*/

        return;
    }

    void ItemHandler::MegaMushHandler::StateInitUse(u32 megaMushObj) {
        void(*objBaseStateInitUse)(u32 obj) = (decltype(objBaseStateInitUse))GameAddr::itemObjStateInitUse;
        objBaseStateInitUse(megaMushObj);
        u32 infoProxy = ((u32*)megaMushObj)[0x158/4];
        u32 vehicle = ((u32*)infoProxy)[0];
        int playerID = ((u32*)vehicle)[0x84/4];
        Start(playerID);
    }

    void ItemHandler::MegaMushHandler::PlayChangeSizeSound(u32* vehicleMove, bool isGrow) {
        u32 soundPlayer = vehicleMove[0xDC/4];
        u32*(*sndactorkart_startsound)(u32 soundPlayer, u32 soundID, u32* soundHandle) = (decltype(sndactorkart_startsound))(((u32**)soundPlayer)[0][0x70/4]);
        CwavReplace::LockState();
        u32* retHandle = sndactorkart_startsound(soundPlayer, isGrow ? Snd::SoundID::TAIL_APPEAR : Snd::SoundID::TAIL_DISAPPEAR, nullptr);
        if (retHandle)
            CwavReplace::SetReplacementCwav(isGrow ? CwavReplace::KnownIDs::konohaStartSE : CwavReplace::KnownIDs::konohaEndSE, isGrow ? growSound : shrinkSound, 1, 1.75f);
        CwavReplace::UnlockState();
    }

    static u32 g_starthemeunholdframes = 0;
    static bool g_wasStarTheme = false;
    void ItemHandler::MegaMushHandler::CalcMegaTheme(u32* sndEngine, u32* sndHandle) {
        bool(*sndengine_holdsound)(u32* sndEngine, u32* sndHandle, u32 soundID, u32* params) = (decltype(sndengine_holdsound))GameAddr::sndengine_holdsound;
        u32* vehicle = (u32*)MarioKartFramework::getVehicle(MarioKartFramework::masterPlayerID);
        u32 starTimer = vehicle[0xFF4/4];
        u32 megaTimer = MarioKartFramework::megaMushTimers[MarioKartFramework::masterPlayerID];
        bool isStar = starTimer;
        bool isMega = megaTimer;
        bool soundHasStarted = sndHandle[0] != 0 && ((u32**)sndHandle)[0][0x9C/4] == Snd::SoundID::STAR_THEME_PLAYER;
        if (isMega && !isStar && !soundHasStarted && !g_starthemeunholdframes) {
            CwavReplace::LockState();
            bool wasPlayed = sndengine_holdsound(sndEngine, sndHandle, Snd::SoundID::STAR_THEME_PLAYER, nullptr);
            if (wasPlayed)
                CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::starThemeSE, megaTheme, 2, 1.75f);
            CwavReplace::UnlockState();
        } else if (!g_starthemeunholdframes) {
            sndengine_holdsound(sndEngine, sndHandle, Snd::SoundID::STAR_THEME_PLAYER, nullptr);
        }
        if (g_starthemeunholdframes) g_starthemeunholdframes--;
        if (g_wasStarTheme && !isStar && isMega && megaTimer > 4) { // Star just finished and is mega
            g_starthemeunholdframes = 4;
        } else if (!g_wasStarTheme && isStar && isMega && starTimer > 4) { // Star just started and was mega
            g_starthemeunholdframes = 4;
        }
        g_wasStarTheme = isStar;
    }

    static bool enemyStarThemeHasStarted(u32* sndActorKart) {
        if (((u8*)sndActorKart)[0x1F1] || ((u8*)sndActorKart)[0x1F2])
            return true;
        u8* something = (u8*)sndActorKart[0xA0/4];
        if (something[0x3C] != 0 || something[0xD3] == 2)
            return true;
        u32* handleList = sndActorKart + 0xDC/4;
        for (int i = 0; i < 4; i++) {
            u32 handle = handleList[i];
            if (!handle)
                continue;
            if (((u32*)handle)[0x9C/4] == Snd::SoundID::STAR_THEME_ENEMY)
                return true;
        }
        return false;
    }

    static u32 g_starthemeunholdframesEnemy[8];
    static bool g_wasStarThemeEnemy[8];
    u32* ItemHandler::MegaMushHandler::CalcEnemyMegaTheme(u32* sndActorKart) {
        u32* vehicle = (u32*)sndActorKart[0x1E0/4];
        int playerID = vehicle[0x21];
        u32 starTimer = vehicle[0xFF4/4];
        u32 megaTimer = MarioKartFramework::megaMushTimers[playerID];
        bool isStar = starTimer;
        bool isMega = megaTimer;
        u32*(*sndactorkart_holdsound)(u32 sndActorKart, u32 soundID, u32* soundHandle) = (decltype(sndactorkart_holdsound))(((u32**)sndActorKart)[0][0x88/4]);
        u32* rethandle = nullptr;
        if (isMega && !isStar && !enemyStarThemeHasStarted(sndActorKart) && !g_starthemeunholdframesEnemy[playerID]) {
            CwavReplace::LockState();
            rethandle = sndactorkart_holdsound((u32)sndActorKart, Snd::SoundID::STAR_THEME_ENEMY, nullptr);
            if (rethandle)
                CwavReplace::SetReplacementCwav(CwavReplace::KnownIDs::starThemeSE, megaTheme, 1, 1.75f);
            CwavReplace::UnlockState();
        } else if (!g_starthemeunholdframesEnemy[playerID]) {
            rethandle = sndactorkart_holdsound((u32)sndActorKart, Snd::SoundID::STAR_THEME_ENEMY, nullptr);
        }
        if (g_starthemeunholdframesEnemy[playerID]) g_starthemeunholdframesEnemy[playerID]--;
        if (g_wasStarThemeEnemy[playerID] && !isStar && isMega && megaTimer > 4) { // Star just finished and is mega
            g_starthemeunholdframesEnemy[playerID] = 4;
        } else if (!g_wasStarThemeEnemy[playerID] && isStar && isMega && starTimer > 4) { // Star just started and was mega
            g_starthemeunholdframesEnemy[playerID] = 4;
        }
        g_wasStarThemeEnemy[playerID] = isStar;        
        return rethandle;
    }

    void ItemHandler::MegaMushHandler::DefineBgCharaMapControlAnimation(VisualControl::AnimationDefine* animDefine) {
        animDefine->InitAnimationFamilyList(1);

        animDefine->InitAnimationFamily(0, "G_action", 8);
        animDefine->InitAnimationStopByRate(0, "in_thunder", 0.f);
        animDefine->InitAnimation(1, "in_thunder", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        animDefine->InitAnimationStopByRate(2, "out_thunder", 0.f);
        animDefine->InitAnimation(3, "out_thunder", VisualControl::AnimationDefine::AnimationKind::ONCE);

        animDefine->InitAnimationStopByRate(4, "in_mega", 0.f);
        animDefine->InitAnimation(5, "in_mega", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        animDefine->InitAnimationStopByRate(6, "out_mega", 0.f);
        animDefine->InitAnimation(7, "out_mega", VisualControl::AnimationDefine::AnimationKind::ONCE);
    }

    void ItemHandler::MegaMushHandler::SetMapFaceState(int playerID, bool isMega) {
        u32 baseRacePage = MarioKartFramework::getBaseRacePage();
        if (!baseRacePage)
            return;
        VisualControl::GameVisualControl* raceMapCharaControl = ((VisualControl::GameVisualControl**)(baseRacePage + 0x2A8))[playerID];
        raceMapCharaControl->GetAnimationFamily(0)->ChangeAnimation(isMega ? 5 : 7, 0.f);
    }

    void ItemHandler::MegaMushHandler::Start(int playerID) {
        u32* vehicle = (u32*)MarioKartFramework::getVehicle(playerID);
        if (((u8*)vehicle)[0x9E])
            return;
        MarioKartFramework::startSizeChangeAnimation(playerID, getMegaSizeFactor(playerID), true);
        MarioKartFramework::megaMushTimers[playerID] = 7 * 60 + 1 * 30;
        u32 soundActorKart = vehicle[0xDC/4];
		MarioKartFramework::SndActorKartSetStarState((u32*)soundActorKart, (1 << 1));
        MegaMushHandler::PlayChangeSizeSound((u32*)vehicle, true);
        struct {u32 shrunkTimer; u32 pressTimer;} *timers1 = (decltype(timers1))(vehicle + 0x1000/4);
        if (timers1->shrunkTimer) timers1->shrunkTimer = 1;
        if (timers1->pressTimer) timers1->pressTimer = 1;
        if (timers1->shrunkTimer)
            growMapFacePending[playerID] = 5;
        else
            MegaMushHandler::SetMapFaceState(playerID, true);
    }

    void ItemHandler::MegaMushHandler::End(int playerID, bool resetState) {
        u32* vehicle = (u32*)MarioKartFramework::getVehicle(playerID);
        u32 soundActorKart = vehicle[0xDC/4];
        MarioKartFramework::megaMushTimers[playerID] = 0;
        MarioKartFramework::SndActorKartSetStarState((u32*)soundActorKart, (1 << 2));
        if (resetState) {
            MarioKartFramework::resizeInfos[playerID].Reset();
            if (playerID == MarioKartFramework::masterPlayerID) MarioKartFramework::playerMegaCustomFov = 52.f;
        } else
            MarioKartFramework::startSizeChangeAnimation(playerID, 1.f, false);
        SetMapFaceState(playerID, false);
    }

    void ItemHandler::MegaMushHandler::CalcNetRecv(int playerID, int frames) {
        if (frames) {
            if (!MarioKartFramework::megaMushTimers[playerID]) {
                MarioKartFramework::megaMushTimers[playerID] = frames;
                u32* vehicle = (u32*)MarioKartFramework::getVehicle(playerID);
                MarioKartFramework::startSizeChangeAnimation(playerID, getMegaSizeFactor(playerID), true);
                u32 soundActorKart = vehicle[0xDC/4];
                MarioKartFramework::SndActorKartSetStarState((u32*)soundActorKart, (1 << 1));
                MegaMushHandler::PlayChangeSizeSound((u32*)vehicle, true);
                struct {u32 shrunkTimer; u32 pressTimer;} *timers1 = (decltype(timers1))(vehicle + 0x1000/4);
                if (timers1->shrunkTimer)
                    growMapFacePending[playerID] = 5;
                else
                    MegaMushHandler::SetMapFaceState(playerID, true);
            }
        } else {
            if (MarioKartFramework::megaMushTimers[playerID]) {
                MarioKartFramework::megaMushTimers[playerID] = frames;
                u32* vehicle = (u32*)MarioKartFramework::getVehicle(playerID);
                u32 soundActorKart = vehicle[0xDC/4];
                MarioKartFramework::SndActorKartSetStarState((u32*)soundActorKart, (1 << 2));
                MarioKartFramework::startSizeChangeAnimation(playerID, 1.f, false);
                SetMapFaceState(playerID, false);
            }
        }
        MarioKartFramework::megaMushTimers[playerID] = frames;
    }

    float ItemHandler::MegaMushHandler::getMegaSizeFactor(int playerID) {
        const u8 playerSizeType[DRIVER_SIZE] = {2, 1, 2, 2, 0, 0, 1, 1, 2, 1, 1, 1, 2, 0, 0, 2, 2, 1};
        const u8 tireSizeType[TIRE_SIZE] = {0, 2, 1, 0, 1, 0, 0, 1, 2, 1};
        const float sizeFactor[3][3] {
            {
                2.1f,
                2.1f,
                2.f,
            },
            {
                1.85f,
                1.85f,
                1.7f
            },
            {
                1.7f,
                1.7f,
                1.65f
            }
        };
        CRaceInfo* raceInfo = MarioKartFramework::getRaceInfo(true);
        return sizeFactor[playerSizeType[raceInfo->kartInfos[playerID].driverID]][tireSizeType[raceInfo->kartInfos[playerID].tireID]];
    }
}