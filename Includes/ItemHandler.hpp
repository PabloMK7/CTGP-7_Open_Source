/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ItemHandler.hpp
Open source lines: 380/380 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "DataStructures.hpp"
#include "Math.hpp"
#include "ExtraResource.hpp"
#include "Sound.hpp"
#include "VisualControl.hpp"
#include "rt.hpp"

namespace CTRPluginFramework {

    enum EItemSlot : u32 {
		ITEM_BANANA = 0,
		ITEM_KOURAG,
		ITEM_KOURAR,
		ITEM_KINOKO,
		ITEM_BOMBHEI,
		ITEM_GESSO,
		ITEM_KOURAB,
		ITEM_KINOKO3,
		ITEM_STAR,
		ITEM_KILLER,
		ITEM_THUNDER,
		ITEM_KINOKOP,
		ITEM_FLOWER,
		ITEM_KONOHA,
		ITEM_SEVEN,
		ITEM_TEST3,
		ITEM_TEST4,
		ITEM_BANANA3,
		ITEM_KOURAG3,
		ITEM_KOURAR3,
		ITEM_SIZE
	};

    enum EItemType {
        ITYPE_KOURAG = 0,
        ITYPE_KOURAR = 1,
		ITYPE_BANANA = 2,
        ITYPE_KINOKO = 3,
        ITYPE_STAR = 4,
        ITYPE_KOURAB = 5,
        ITYPE_THUNDER = 6,
        ITYPE_FAKEBOX = 7,
        ITYPE_KINOKOP = 8,
        ITYPE_BOMB = 9,
        ITYPE_GESSO = 10,
        ITYPE_BIGKINOKO = 11,
        ITYPE_KILLER = 12,
        ITYPE_FLOWER = 13,
        ITYPE_TAIL = 14,
        ITYPE_SEVEN = 15,
	};

    enum EItemDirector {
        IDIR_BANANA = 0, // 0x28
        IDIR_KOURAG = 1, // 0x2C
        IDIR_KOURAR = 2,  // 0x30
        IDIR_BOMB = 3, // 0x34
        IDIR_GESSO = 4, // 0x38
        IDIR_STAR = 5, // 0x3C
        IDIR_THUNDER = 6, // 0x40
        IDIR_KILLER = 7, // 0x44
        IDIR_KOURAB = 8, // 0x48
        IDIR_FLOWER = 9, // 0x4C
        IDIR_TAIL = 10, // 0x50
        IDIR_KINOKO = 11, // 0x54
        IDIR_SEVEN = 12 // 0x58
    };

    class ItemHandler
    {
    private:
        friend class FakeBoxHandler;
        friend void initPatches();
        struct ItemDirectorVtable
        {
            void(*getDTIClassInfo)(u32 self);
            void(*getDTIClass)(u32 self);
            u32 null;
            u32 null1;
            void(*create)(u32 self, void* argumentObj);
            void(*init)(u32 self);
            void(*calc)(u32 self);
            void(*render)(u32 self);
            void(*renderMainL)(u32 self);
            void(*renderMainR)(u32 self);
            void(*renderSub)(u32 self);
            u32 null2;
            void(*accept)(u32 self, void* actorVisitor);
            void(*callbackInvokeEventID)(u32 self, int id);
            u32 null3;
            void(*createOuter)(u32 self, const void* buf);
            void(*initOuter)(u32 self);
            u32 null4;
            void(*createBeforeStructure)(u32 self, const void* argumentObj);
            void(*createAfterStructure)(u32 self, const void* argumentObj);
            void(*initBeforeStructure)(u32 self);
            void(*initAfterStructure)(u32 self);
            void(*calcBeforeStructure)(u32 self);
            void(*calcAfterStructure)(u32 self);
            void(*renderBeforeStructure)(u32 self);
            void(*renderAfterStructure)(u32 self);
            void(*renderMainLBeforeStructure)(u32 self);
            void(*renderMainLAfterStructure)(u32 self);
            void(*renderMainRBeforeStructure)(u32 self);
            void(*renderMainRAfterStructure)(u32 self);
            void(*renderSubBeforeStructure)(u32 self);
            void(*renderSubAfterStructure)(u32 self);
            void(*entry)(u32 self, void* infoProxy, bool isSingle, int multipleAmount);
            u32(*getNum)(u32 self);
            bool(*isEquip_Multi)(u32 self, int id);
            void(*setState_SelfMove)(u32 self, int id, bool unk, Vector2* move);
            void(*dropEquip_Multi)(u32 self, int id);
            u32 null5;
            void(*vanishEquip_Multi)(u32 self, int id, bool unk);
            void(*setDelayFrame)(u32 self, int id, int unk);
            u32 null6;
            u32 null7;
        };
        struct ItemObjVtable {
            void(*getDTIClassInfo)(u32 self);
            void(*getDTIClass)(u32 self);
            u32 null;
            u32 null1;
            void(*create)(u32 self, void* argumentObj);
            void(*init)(u32 self);
            void(*calc)(u32 self);
            void(*render)(u32 self);
            void(*renderMainL)(u32 self);
            void(*renderMainR)(u32 self);
            void(*renderSub)(u32 self);
            u32 null2;
            void(*accept)(u32 self, void* actorVisitor);
            void(*callbackInvokeEventID)(u32 self, int id);
            u32 null3;
            void(*createOuter)(u32 self, const void* buf);
            void(*initOuter)(u32 self);
            u32 null4;
            bool(*isEquip)(u32 self);
            bool(*isEquip_Hang)(u32 self);
            bool(*isEquip_Multi)(u32 self);
            int(*getMultiID)(u32 self);
            float(*getColRadius)();
            float(*getBoxColRadius)();
            float(*getColScale)(u32 self);
            int(*getSoundActorType)();
            void(*calc_HitGnd)(u32 self, const void* hitInfo);
            void(*calc_HitWall)(u32 self, const void* hitInfo);
            void(*exit_Lava)(u32 self);
            void(*setStateStand)(u32 self);
            void(*createInner)(u32 self, const void* createArg);
            void(*initEntryInner_Before)(u32 self);
            void(*initEntryInner)(u32 self);
            void(*calcInner)(u32 self);
            void(*exit_VanishInner)(u32 self);
            void(*exit_BreakInner)(u32 self);
            void(*exitInner)(u32 self);
            void(*react_BreakInner)(u32 self);
            void(*react_VanishInner)(u32 self);
            void(*react_ReflectInner)(u32 self, Vector3* unk1, Vector3* unk2, bool unk3);
            u32 null5;
            void(*stateInitSelfMoveImpl)(u32 self);
            bool(*hasCollision_InUseState)(u32 self);
            float(*getShadowScale)();
            float(*getPressOffsetY_Shadow)(u32 self);
            bool(*hasLight)();
            float(*getOffset_Hang_Y)();
            float(*getOffset_Multi_Y)();
            float(*getOffset_Multi_Z)();
            float(*getRotVelRatio_Multi)();
            float(*getRadius_Multi)();
            void(*getVanishEffectName)(SafeStringBase* out);
            void(*stateInitWait)(u32 self);
            void(*stateWait)(u32 self);
            void(*stateInitEquip_Hang)(u32 self);
            void(*stateEquip_Hang)(u32 self);
            void(*stateInitEquip_Multi)(u32 self);
            void(*stateEquip_Multi)(u32 self);
            void(*stateInitSelfMove)(u32 self);
            void(*stateSelfMove)(u32 self);
            void(*stateInitDrop)(u32 self);
            void(*stateDrop)(u32 self);
            void(*stateInitStand)(u32 self);
            void(*stateStand)(u32 self);
            void(*stateInitUse)(u32 self);
            void(*stateUse)(u32 self);
            void(*stateInitAttacked)(u32 self);
            void(*stateAttacked)(u32 self);
            u32 null6;
            u32 null7;
        };
    public:
        static u32 extraKouraG;
        static u32 extraKouraR;
        static u32 extraKouraB;

        static u32 fakeBoxDirector;
        static u32 megaMushDirector;

        static void Initialize();

        static u32 GetDirectorFromSlot(u32 itemDirector, EItemSlot itemSlot);
        static u32 GetDirectorFromType(u32 itemDirector, EItemType itemType);
        static u32 GetItemMaxAmount(EItemType item);
        static u32 GetItemMaxAmountNet(EItemType item, bool unused);
        static u32 GetExtraItemNum(EItemType item);
        static u32 GetStripeAmount(u32 mode);
        static void UseItem(u32 itemDirector, EItemSlot item, u32* kartInfo);

        static void OnItemDirectorCreateBeforeStructure(u32 itemDirector);
        static EItemType OnKartHitItem(u32* vehicle, u32 itemReactProxy, u32* reactParams, u32 itemDirector);
        static const char* GetTextureNameForItem(u32 itslottexid);

        static u32 GetDriverTimerForItem(u32 itemDirector, u32 infoProxy);

        static RT_HOOK itemObjTailStateUseHook;
        static bool allowFasterItemDisappear;
        static void ItemObjTailStateUse(u32 itemObjTail);

	    static u32 kartHoldItemFrames[8];
        static RT_HOOK kartItemCalcAfterStructureHook;
        static void OnKartItemCalcAfterStructure(u32 kartItem);
        static void OnVehicleInit(u32 vehicle);

        class GameAddr {
        public:
            static ItemObjVtable* bananaVtable; // 0x6262E4
            static ObjectItemBoxVtable* itemBoxVtable; // 0x0062EFA8
            static ItemDirectorVtable* kouraGDirectorVtable; //0x6274EC

            static u32 kartItemSetEquipInfoAddr; // 0x002D1A50
            static u32 startDashKinokoAddr; //0x002D5850
            static u32 directorBaseEntryUseFuncAddr; //0x002CE8B8
            static u32 sequenceUseKinoko3Addr; //0x004882A0
            static u32 sequenceUseKinoko2Addr; //0x00488258
            static u32 sequenceUseAnimAddr; //0x0047FEBC

            static u32 itemDirectorConsAddr; //0x002CE91C

            static u32 dropEquipAddr; //0x002BA4DC
            static u32 createItemBoxBreakEffectAddr; // 0x003F3B10

            static u32 matrixScaleBasesAddr; // 0x0030EB14
            static u32 drawMdlSetPoseAddr; // 0x00434398

            static u32 getObjectParameterAddr; // 0x00396A04
            static u32 itemBoxGeneratorConsAddr; //0x003A4838
            static u32 itemBoxGeneratorVtableAddr; // 0x63EDD4

            static u32 itemObjConsAddr; // 0x002B8310
            static u32 itemObjConstInfoAddr; // 0x665CA8
            static u32 gameParticleConsAddr; // 0x003ED1E0

            static u32 sndengine_holdsound; // 0x003D9244

            static ItemObjVtable* starVtable;
            static ItemDirectorVtable* starDirectorVtable;
            static u32 itemObjStateInitUse;

        };

        class FakeBoxHandler
        {
        public:
            enum class FakeBoxState {
                NON_INIT = -1,
                WAIT = 0,
                EQUIP = 1,
                SELFMOVE = 2,
                DROP = 3,
                STAND = 4,
                USE = 5,
                ATTACKED = 6
            };
            static constexpr int FakeBoxAmount = 6;
            class FakeBoxData
            {
            public:
                
                CustomFieldObjectCreateArgs* itemBoxCustomCreateArgs = nullptr;
                ObjectGeneratorBase* fakeBoxItemBoxGens = nullptr;
                
                FakeBoxState state = FakeBoxState::NON_INIT;
                FakeBoxState prevState = FakeBoxState::NON_INIT;
                bool changedState = false;
                bool needScaleBases = true;
                u8 reflectAmount = 0;

                FakeBoxData() = default;
                ~FakeBoxData() {
                    if (itemBoxCustomCreateArgs) delete itemBoxCustomCreateArgs;
                    // ItemBoxGens is part of the game heap
                };

                static ItemObjVtable* fakeBoxVtable;
                static ObjectItemBoxVtable* fakeBoxItemBoxVtable;
            };
        private:
            friend class ItemHandler;
            static ItemDirectorVtable* directorVtable;
            static FakeBoxData* fakeBoxData[FakeBoxAmount];
            static void nullfunc(u32 fakebox);
            static u8* fibStandCwav;
            static void PatchObjFlowForFIB();
            
            static void CalcAppearing(u32 itemBox, float progress);
            static void OnCalcAppearing(u32 itemBox);
            static void OnDoBreakItemBox(u32 itemBox);
            static u32 GetParameterForShare(u32 itemBox);
            static float GetColRadius();
            static float GetBoxColRadius();

            static void ChangeState(u32 fakeBox, FakeBoxState state);
            static void OnInitWait(u32 fakeBox);
            static void OnInitEquip(u32 fakeBox);
            static void OnInitSelfMove(u32 fakeBox);
            static void OnInitDrop(u32 fakeBox);
            static void OnInitStand(u32 fakeBox);
            static void OnInitUse(u32 fakeBox);
            static void OnInitAttacked(u32 fakeBox);
            
        public:

            static void Initialize();

            static bool isHittingFib;
            static int creatingFakeBox;
            static bool PlaySoundEffect(u32 fakebox, Snd::SoundID soundID);
            static u32 GetParamReplacementID(u32 objID);
            static u8* GetFakeItemBox(ExtraResource::SARC::FileInfo* fileInfo);
            static void OnItemBoxCreate(ObjectItemBox* itemBox);
            static u32* FakeBoxConstructor(u32* fakebox, int id);
            static void CreateBeforeStructure(u32 fakeBoxDirector, const void* createArg);
            static void ObjCreateInner(u32 fakeBoxObj, const void* createArg);
            static u32 GetFakeItemBoxFieldObjParam(u32 self);
            static void OnBananaPlayStandSE(u32 fakeBox);
            static float OnBananaShouldReflect(u32 fakeBox, float surfaceY, float reflectThreshold);
            static void PatchKartPctl(u8* kartPctl);

            static void FakeBoxCalc(u32 fakeboxObj);
        };
        class MegaMushHandler {
        public:
            static constexpr int MegaMushAmount = 2;

            static void Initialize();
            static void CreateBeforeStructure(u32 megaMushDirector, const void* createArg);
            static void ObjCreateInner(u32 megaMushObj, const void* createArg);
            static void StateInitUse(u32 megaMushObj);
            static void PlayChangeSizeSound(u32* vehicleMove, bool isGrow);
            static void CalcMegaTheme(u32* sndEngine, u32* sndHandle);
            static u32* CalcEnemyMegaTheme(u32* sndActorKart);
            static void DefineBgCharaMapControlAnimation(VisualControl::AnimationDefine* animDefine);
            static void SetMapFaceState(int playerID, bool isMega);
            static void Start(int playerID);
            static void End(int playerID, bool resetState);
            static void CalcNetRecv(int playerID, int frames);
            
            static u8 growMapFacePending[8];
        private:
            friend class ItemHandler;
            static ItemDirectorVtable* directorVtable;
            static ItemObjVtable* itemVtable;
            static void* growSound;
            static void* shrinkSound;
            static void* megaTheme;
            static float getMegaSizeFactor(int playerID);
        };
    };
}