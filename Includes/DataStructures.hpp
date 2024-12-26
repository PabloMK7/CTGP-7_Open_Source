/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: DataStructures.hpp
Open source lines: 911/911 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Math.hpp"

namespace CTRPluginFramework {

    enum EDriverID : s32 {
		DRIVER_BOWSER = 0,
		DRIVER_DAISY,
		DRIVER_DONKEY,
		DRIVER_HONEYQUEEN,
		DRIVER_KOOPATROOPA,
		DRIVER_LAKITU,
		DRIVER_LUIGI,
		DRIVER_MARIO,
		DRIVER_METAL,
		DRIVER_MIIM,
		DRIVER_MIIF,
		DRIVER_PEACH,
		DRIVER_ROSALINA,
		DRIVER_SHYGUY,
		DRIVER_TOAD,
		DRIVER_WARIO,
		DRIVER_WIGGLER,
		DRIVER_YOSHI,
		DRIVER_SIZE,
		DRIVER_SHYGUYCOLORSTART = 100,
		DRIVER_SHYGUYCOLOREND = 107,
		DRIVER_SHYGUYRANDOM = -100,
		DRIVER_RANDOM = -1,
		DRIVER_RECOMMENDED = -2
	};

	enum EBodyID : s32
	{
		BODY_STD = 0,
		BODY_RALLY,
		BODY_RBN,
		BODY_EGG,
		BODY_DSH,
		BODY_CUC,
		BODY_KPC,
		BODY_BOAT,
		BODY_HNY,
		BODY_SABO,
		BODY_GNG,
		BODY_PIPE,
		BODY_TRN,
		BODY_CLD,
		BODY_RACE,
		BODY_JET,
		BODY_GOLD,
        BODY_SIZE,
		BODY_RANDOM = -1,
	};

	enum ETireID : s32
	{
		TIRE_STD = 0,
		TIRE_BIG,
		TIRE_SMALL,
		TIRE_RACE,
		TIRE_CLASSIC,
		TIRE_SPONGE,
		TIRE_GOLD,
		TIRE_WOOD,
		TIRE_BIGRED,
		TIRE_MUSH,
        TIRE_SIZE,
		TIRE_RANDOM = -1,
	};

	enum EWingID : s32 {
		WING_STD = 0,
		WING_PARA,
		WING_UMB,
		WING_FLOWER,
		WING_BASA,
		WING_MET,
		WING_GOLD,
        WING_SIZE,
		WING_RANDOM = -1,
	};

    enum EScrewID : s32 {
        SCREW_STD = 0,
        SCREW_SIZE,
    };

	enum EPlayerType : s32 {
		TYPE_USER = 0,
		TYPE_CPU = 1,
		TYPE_CONTROLLEDUSER = 2,
		TYPE_GHOST = 3,
        TYPE_UNKNOWN = 4,
		TYPE_MAX = 0xFFFFFFF
	};
    enum EItemMode : s32
    {
        ITEMMODE_ALL = 0,
        ITEMMODE_SHELLS = 1,
        ITEMMODE_BANANAS = 2,
        ITEMMODE_MUSHROOMS = 3,
        ITEMMODE_BOBOMBS = 4,
        ITEMMODE_NONE = 5,

        ITEMMODE_CUSTOM = 100,
        ITEMMODE_RANDOM = 101,
    };
    enum ECharaIconType : u32
    {
        CHARAICONTYPE_MAPRACE = 0, // map_(name)_r90.bclim
        CHARAICONTYPE_RANKRACE = 1, // rank_(name)_r90.bclim
        CHARAICONTYPE_RANKMENU = 2, // rank_(name).bclim
        CHARAICONTYPE_SELECT = 3, // select_(name).bclim
        CHARAICONTYPE_SIZE,
    };
    constexpr u32 GetCharaIconTextureSize(ECharaIconType type) {
        switch (type)
        {
        case ECharaIconType::CHARAICONTYPE_MAPRACE:
            return 0x828;
        case ECharaIconType::CHARAICONTYPE_RANKRACE:
        case ECharaIconType::CHARAICONTYPE_RANKMENU:
            return 0x1028;
        case ECharaIconType::CHARAICONTYPE_SELECT:
            return 0x2028;
        default:
            return 0;
        }
    }

    struct ResourceLoaderLoadArg {
        void* heap;
        u32 alignment;
        u32 unknown;
        bool unknown2;
        u32 archiveID; // 0x9 = ThankYou3D, 0xB = romfs root
    };

    template <class T>
    struct SeadArrayPtr {
        u32 count;
        u32 size;
        T* data;
        
        SeadArrayPtr() : count(0), size(0), data(nullptr) {}

        inline void SetBuffer(int elementCount, T* buffer) {
            if (elementCount >= 0 && buffer) {
                count = 0;
                size = elementCount;
                data = buffer;
            }
        }

        inline void Push(T element) {
            if (count < size) {
                data[count++] = element;
            }
        }

        inline T Pop() {
            return (count > 0) ? data[count--] : T{};
        }

        inline T operator[](int element) {
            return (element < count) ? data[element] : T{};
        }

        inline void Set(int element, T value) {
            if (element < count) data[element] = value;
        }

        inline void Fill(T value) {
            while (data && count < size)
            {
                data[count++] = value;
            }
        }
    };

    template <class T, size_t S>
    struct SeadArray {
        u32 count;
        u32 size;
        T data[S]{};

        SeadArray() : count(0), size(S) {}

        inline void Push(T element) {
            if (count < size) {
                data[count++] = element;
            }
        }

        inline T Pop() {
            return (count > 0) ? data[count--] : T{};
        }

        inline T operator[](int element) {
            return (element < count) ? data[element] : T{};
        }

        inline void Set(int element, T value) {
            if (element < count) data[element] = value;
        }

        inline void Fill(T value) {
            while (data && count < size)
            {
                data[count++] = value;
            }
        }
    };

    template <class T>
    struct FixedArrayPtr {
        u32 size;
        T* data;

        inline T& operator[](int element) {
            return (element < size) ? data[element] : data[0];
        }
    };

    struct ActorArray // Not actual representation, but close enough
    {
        SeadArrayPtr<u32*> array;
        u32 unknown;
        u32 count;

        ActorArray(u32 elCount);

        inline void Push(u32 actor) {
            if (array[count] == 0) {
                array.Set(count++, (u32*)actor);
            }
        }

        inline u32 operator[](int element) {
            return (u32)array[element];
        }
    };

    struct SafeStringBase {
        static u32 vTableSafeStringBase;

        u32 vtable;
        const char* data;
        SafeStringBase(const char* str) {
            vtable = vTableSafeStringBase;
            data = str;
        }

        const char* c_str() {
            return data;
        }
    };

    template <class C, int T>
    struct FixedStringBase {
        SafeStringBase strBase;
        u32 bufferSize;
        C strData[T];
        FixedStringBase() : bufferSize(T), strBase((const char*)strData) {strData[0] = 0;};
    };

    typedef struct _STGIEntry { // Second entry: custom settings
        union {
            u8 NrLaps;
            struct
            {
                u8 forceMinimap : 1;
                u8 forceRenderOptim : 1;
                u8 disableBulletBill : 1;
                u8 speedupMusicOnLap : 1;
            } CustomFlags;
        };
        u8 PolePosition;
        u8 Unknown1;
        u8 Unknown2;
        u32 FlareColor;
        u32 FlareAlpha;
    } STGIEntry;

	struct GOBJEntry
	{
		u16 objID = 0;
		u16 unknownRender = 0;
		Vector3 position = {0.f, 0.f, 0.f};
		Vector3 rotation = {0.f, 0.f, 0.f};
		Vector3 scale = {1.f, 1.f, 1.f};
		s16 routeID = -1;
		u16 settings[8] = {0};
		u16 visibility = 7;
		u16 enemyRoute = -1;
		u16 unknown = 0;
	};
    struct GOBJEntryList {
        u32 signature;
        u32 count;
        GOBJEntry entries[];
    };

	struct GOBJAccessor {
		u32 vtable;
		u16 entryAmount;
		u16 gap1;
		u32 gap2[3];
		GOBJEntry*** entries;
	};

	struct POTIPoint {
		Vector3 position;
		u16 routeSpeed;
		u16 settings;
	};

	struct POTIRoute {
		u16 amount;
		u8 loop;
		u8 smooth;
		POTIPoint points[];
	};

	struct POTIAccessor {
		u32 vtable;
		u16 entryAmount;
		u16 gap1;
		u32 gap2[3];
		POTIRoute*** entries;
	};

	struct MiiData {
		u8 always3;
		struct {
			u8 allowCopy : 1;
			u8 profanity : 1;
			u8 regionLock : 2;
			u8 characterSet : 2;
		} flags;
		struct
		{
			u8 pageIndex : 4;
			u8 slotIndex : 4;
		} pageInfo;
		struct
		{
			u8 unknown : 4;
			u8 version : 3;
		} version;
		u64 systemID;
		struct
		{
			u32 creationDate : 28;
			u32 alwaysSet : 1;
			u32 temporary : 1;
			u32 dsi : 1;
			u32 notspecial : 1;
		} miiID;
		u64 creatorMAC;
		struct
		{
			u16 gender : 1;
			u16 birthdayMonth : 4;
			u16 birthdayDay : 5;
			u16 favColor : 4;
			u16 favMii : 1;
		} generalInfo;
		char16_t name[10];
		u8 anatomyInfo[0x1A];
		char16_t author[10];

		std::string GetName()
		{
			if (!systemID) return "Player";
			char16_t tmpbuf[10 + 2] = { 0 };
			memcpy(tmpbuf, this->name, sizeof(name)); // Need to copy as it is not null terminated
            std::string ret;
            Utils::ConvertUTF16ToUTF8(ret, tmpbuf);
			return ret;
		}
	}PACKED;

    struct ObjFlowEntry {
        u16 objectID;
        u16 padding;
        u32 unknown1;
        u32 unknown2;
        u32 unknown3;
        u32 unknown4;
        u32 unknown5;
        u32 unknown6;
        u32 unknown7;
        char name[0x80];
    };

    struct DrawMdlCreateArgs
    {
        FixedStringBase<char, 128> fileName;
        bool unknown = 0; // 0x8C
        bool unknown2 = false; // 0x8D
        u32 unknown3 = 0; // 0x90
        u32 unknown4 = 0; // 0x94
        int unknown5 = -1; // 0x98
        FixedStringBase<char, 64> modelName;
        int animationCount = -1; // 0xE8
        u32 animationFlags = 0; // 0xEC
        u32 unknown8 = 0; // 0xF0
        u32 sharedModelCount = 0; // 0xF4
        u32 unknown10 = 1; // 0xF8
        bool unknown11 = true; // 0xFC
        bool unknown12 = false; // 0xFD
        bool unknown13 = false; // 0xFE
        bool unknown14 = false; // 0xFF
        int unknown15 = -2; // 0x100
        int unknown16 = -2; // 0x104
        int unknown17 = -2; // 0x108
        int unknown18 = -2; // 0x10C
        u32 unknown19 = 0; // 0x110
        u32 unknown20 = 0; // 0x114
        bool unknown21 = false; // 0x118 
        bool unknown22 = false; // 0x119 
        bool unknown23 = true; // 0x11A
        bool unknown24 = true; // 0x11B
        u32 unknown25 = 0; // 0x11C
        u32 unknown26 = 0xB; // 0x120
    };

    struct FieldObjectParams
    {
        GOBJEntry* objEntry;
        u32 unknown0 = 0x4; // 0x4
        float unk0matrix3x3[9] = {
            1.f, 0.f, 0.f,
            0.f, 1.f, 0.f,
            0.f, 0.f, 1.f
        };
        float unk1matrix3x4[12] = {
            1.f, 0.f, 0.f, 0.f,
            0.f, 1.f, 0.f, 0.f,
            0.f, 0.f, 1.f, 0.f,
        };
        u32 lightSetIndex;
    };
    
    struct FieldObjectCreateArgs
    {
        FieldObjectParams* params;
        SeadArrayPtr<void*>* objFlowEntries; // objectdirector + 0x28
        DrawMdlCreateArgs modelArgs;
        void* cgfxPtr;
        u32 unused[2] = {0};
    };

    struct FieldObjectVtable {
        void(*calcView)(u32 self);
        void(*updateScaleChange)(u32 self);
        u32 null0;
        void*(*getCollisionResult)(u32 self);
        void*(*getCollisionBase)(u32 self);
        void(*checkCollision)(u32 self, const Vector3&, float, int);
        u32(*getSoundActorType)();
        void(*add_IndividualArgs)(u32 self, FieldObjectCreateArgs&);
        void(*add_IndividualArgs_forLOD)(u32 self, DrawMdlCreateArgs&, int);
        void(*createObjectDefault)(u32 self, FieldObjectCreateArgs&);
        void(*createObjectAnim)(u32 self, FieldObjectCreateArgs&);
        void(*createObjectShadow)(u32 self, FieldObjectCreateArgs&);
        void(*createObjectExtend)(u32 self, FieldObjectCreateArgs&);
        void(*createObjectAnimExtend)(u32 self, FieldObjectCreateArgs&);
        void(*createDefaultMaterialAnim)(u32 self, FieldObjectCreateArgs&);
        void(*createMaterialAnimLeader)(u32 self, FieldObjectCreateArgs&);
        void(*initObjectAnim)(u32 self, FieldObjectCreateArgs&);
        void(*beginRegistAnimation)(u32 self, int);
        void(*endRegistAnimation)(u32 self);
        void(*createPath)(u32 self, FieldObjectCreateArgs&);
        void(*createSoundActor)(u32 self, FieldObjectCreateArgs&);
        void(*initObj)(u32 self);
        void(*calcObj)(u32 self);
        void(*calcPath)(u32 self);
        void(*calcPathPosture)(u32 self);
        void(*calcEnd)(u32 self);
        void(*getGroundCollisionRadius)(u32 self);
        void(*createCollision)(u32 self, void* colset);
        void(*createNoCollision)(u32 self, void* colset);
        void(*createSphereCollision)(u32 self, void* colset);
        void(*createCylinderCollision)(u32 self, void* colset);
        void(*createBoxCollision)(u32 self, void* colset);
        void(*createGroundCollision)(u32 self, void* colset);
        void(*createCapsuleCollision)(u32 self, void* colset);
        void(*createOriginalCollision)(u32 self, void* colset);
        void(*setReaction_fromKart)(u32 self, u32 EGHTReaction, u32 eObjectReactType, u32* vehicle);
        void(*setReaction_fromItem)(u32 self, u32 EGHTReaction, u32 eObjectReactType, u32* itemReactProxy);
        void(*doReaction_againstKart)(u32 self, u32 EGHTReaction,u32* vehicle, void* collisionRes, bool);
        void(*doReaction_againstItem)(u32 self, u32 EGHTReaction, u32* itemReactProxy);
        void(*updateObjectCollisionPos)(u32 self);
        void(*getObjectCollisionPosture)(u32 self);
        void(*onCollisionHit)(u32 self, void* collisionRes);
        void(*getResourceName)(u32 self);
        void(*getModelName)(u32 self);
        void(*getRenderPriorityOpa_Start)(u32 self);
        void(*getRenderPriorityOpa_End)(u32 self);
        void(*getAnimNum)(u32 self);
        void(*getDefaultPlayAnimIndex)(u32 self);
        void(*getDefaultPlayMaterialAnimIndex)(u32 self);
        void(*getObjectEnumIndex)(u32 self);
        void(*setupBoxCol)(u32 self);
        void(*setDefaultNeedFlag)(u32 self);
        void(*isCreateHitEffect)(u32 self);
        u32 null1;
    };

    struct ObjectGeneratorVtable {
        FieldObjectVtable baseVtable;
        u32 null2;
        u32 null3;
        void(*createBeforeStructure)(u32 self, void* argObj);
        void(*createAfterStructure)(u32 self, void* argObj);
        void(*initAfterStructure)(u32 self);
        u32 null4;
        void(*init)(u32 self);
        void(*calc)(u32 self);
        void(*getGenerateNum)(void* mapdataGeoObj);
        void(*getCreateFunc)();
        u32 gap[0x4C/4]; // Undocumented
        void (*createOuter)(u32* actor, void* data);
        u32 gap2[0x48/4]; // Undocumented
    };

    struct CustomFieldObjectCreateArgs {
        FieldObjectCreateArgs args;
        FieldObjectParams gObjParams;
        GOBJEntry gObjEntry;
        void* vtablePtr = 0;

        CustomFieldObjectCreateArgs(u32 gobjID);
        ~CustomFieldObjectCreateArgs() {
            if (vtablePtr) delete (u32*)vtablePtr;
        }
    };

    struct ObjectItemBoxVtable {
        FieldObjectVtable baseVtable;
        u32 null1;
        u32 null2;
        void(*calc)(u32 self);
        void(*init)(u32 self);
        u32 gap0[0x20/4];
        void(*registSharedModel)(u32 self, void* model);
        void(*registSharedModelLod)(u32 self, void* model, int unk);
        u32 gap1[0x1C/4];
        u32(*getParamForShare)(u32 self);
        u32 gap2[0x38/4];
        void(*doBreakItemBox)(u32 self);
        void(*init_GrShadow)(u32 self);
        void(*update_GrShadowScale)(u32 self);
        void(*calc_Movement)(u32 self);
        void(*calcAppearing)(u32 self);
        u32 gap4[0x68/4]; // undocumented
    };

    struct ObjectItemBox {
        ObjectItemBoxVtable* vtable;
        u32 gap[0x44/4];
        Vector3 position;
        u32 gap2[0x4C/4];
        u32 actorStart;
        // More undocumented
    };

    struct ObjectGeneratorBase {
        ObjectGeneratorVtable* vtable;
        u32 gap[0x44/4];
        Vector3 position; // 0x48
        u32 gap2[0x4C/4];
        u32 actorStart;
        u32 gap3[0x15C/4];
        SeadArrayPtr<void*> objects;

        inline ObjectItemBox* GetAsItembox() {
            return (ObjectItemBox*)(((u32**)this)[0xBC/4][0] - 0xA0);
        }

        inline bool HasItembox() {
            return ((u32**)this)[0xBC/4] != nullptr;
        }
    };

    struct KartFlags
    {
        /* Researching code
        u32 vehicle = MarioKartFramework::getVehicle(0);
        if (vehicle)
        {
            KartFlags flags;
            flags.raw = *(u32*)(vehicle + 0xC30);
            if (Controller::IsKeyPressed(Key::DPadLeft))
                mask = rol<u32>(mask, 1);
            else if (Controller::IsKeyPressed(Key::DPadRight))
                mask = ror<u32>(mask, 1);;
            if (Controller::IsKeyDown(Key::X))
                flags.raw |= mask;
            else if (Controller::IsKeyDown(Key::DPadDown))
                flags.raw &= ~mask;
            *(u32*)(vehicle + 0xC30) = flags.raw;
            NOXTRACE("sdsdf", "0x%08X 0x%08X", flags.raw, mask);
        }
        */
        union {
            u32 raw;
            struct
            {
            /*0x1*/                u32 isJumping : 1;                                                      
            /*0x2*/                u32 isOlliePitching : 1; // Wing twist related
            /*0x4*/                u32 isLandingFromJump : 1;
            /*0x8*/                u32 isStandStill : 1;
            /*0x10*/                u32 onStickyRoad : 1;
            /*0x20*/                u32 isStartingOOB : 1;
            /*0x40*/                u32 isStartingFallbackOOB : 1; // Out of checkpoints or AI recover
            /*0x80*/                u32 isWingOpened : 1;
            /*0x100*/                u32 isInGliderPad : 1;
            /*0x200*/                u32 isGliding : 1;
            /*0x400*/                u32 wingClosedOutOfPath : 1;
            /*0x800*/                u32 wingTricked : 1;
            /*0x1000*/                u32 isInWater : 1;
            /*0x2000*/                u32 isInMoon : 1;
            /*0x4000*/                u32 isSlidingTurn : 1;
            /*0x8000*/                u32 isInSlope : 1; // Set if kart is in slope or something
            /*0x10000*/                u32 bigAccident : 1;
            /*0x20000*/                u32 unknown_20000 : 1; // Something accident related
            /*0x40000*/                u32 isBouncingMush : 1;
            /*0x80000*/                u32 isPullingBack : 1; // pull back kcl
            /*0x100000*/                u32 isPullingBackExtreme : 1;
            /*0x200000*/                u32 hasTricked : 1;
            /*0x400000*/                u32 isBullet : 1;
            /*0x800000*/                u32 isHangingLakitu : 1;
            /*0x1000000*/                u32 hasTail : 1;
            /*0x2000000*/                u32 isRespawningBattle : 1;
            /*0x4000000*/                u32 userClosedWing : 1;
            /*0x8000000*/                u32 startingVanish : 1; // Mark as respawning
            /*0x10000000*/                u32 isFinishVanishing : 1; // Finish vanishing when respawning
            /*0x20000000*/                u32 unknown_20000000 : 1; // Drift related
            /*0x40000000*/                u32 unknown_40000000 : 1;
            /*0x80000000*/                u32 cameraRollUnlocked : 1;
            };
        };
        static KartFlags& GetFromVehicle(u32 vehicle) {
            return *(KartFlags*)(vehicle + 0xC30);
        }        
    };

    struct BoostFlags {
        union {
            u32 raw;
            struct
            {
                u32 mushroomBoost : 1;
                u32 boostPadBoost : 1;
                u32 lakituRecoveryBoost : 1;
                u32 miniturboBoost : 1;
                u32 superminiturboBoost : 1;
                u32 startLv1Boost : 1;
                u32 startLv2Boost : 1;
                u32 startLv3Boost : 1;
                u32 startLv4Boost : 1;
                u32 startLv5Boost : 1;
                u32 trickBoost : 1;
                u32 trickWaterDiveBoost : 1;
                u32 trickWaterBoost : 1;
                u32 starRingBoost : 1;
                u32 coinBoost : 1;
                u32 waterDiveBoost : 1; 
                u32 unknown_10000 : 1; // Accident related
                u32 unknown_20000 : 1; // Accident related
                u32 unknown_40000 : 1; // Maybe related to solid OOB
                u32 doTailJump : 1;
                u32 isRidingWall : 1; // Used for failsafe respawn
                u32 hasWingClosed : 1;
                u32 unknown_400000 : 1;
                u32 unknown_800000 : 1;
                u32 unknown_1000000 : 1;
                u32 unknown_2000000 : 1;
                u32 unknown_4000000 : 1;
                u32 unknown_8000000 : 1;
                u32 unknown_10000000 : 1;
                u32 unknown_20000000 : 1;
                u32 unknown_40000000 : 1;
                u32 unknown_80000000 : 1;
            };
        };
        static BoostFlags& GetFromVehicle(u32 vehicle) {
            return *(BoostFlags*)(vehicle + 0xC28);
        }        
    };

    struct DepthBufferReaderVTable {
        void(*getDTIClassInfo)(u32 self);
        void(*getDTIClass)(u32 self);
        u32 null0;
        u32 null1;
        void*(*create)(u32 self, void* argumentobj);
        void*(*init)(u32 self);
        void(*calc)(u32 self);
        void(*render)(u32 self);
        void(*renderMainL)(u32 self);
        void(*renderMainR)(u32 self);
        void(*renderSub)(u32 self);
        u32 null2;
        void(*accept)(u32 self, void* visitor);
        void(*callbackInvokeEventID)(u32 self, int eventID);
        u32 null3;        
        void(*createOuter)(u32 self);
        void(*initOuter)(u32 self);
        u32 null4;  
        u32 null5;  
        u32 null6;
    };

    struct SelectMenuSendFormat {
        u8 unk;
        u8 unk1;
        u8 unk2;
        u8 unk3;
        u8 driverID;
        u8 bodyID;
        u8 wingID;
        u8 tireID;
        // Rest of fields to fill until 0xA0
    };

    struct CKartInfo {
        EBodyID bodyID;
        ETireID tireID;
        EWingID wingID;
        EScrewID screwID;
        EDriverID driverID;
        EPlayerType playerType;
        u32 teamType;
        u32 unknown2;
        u16 racePoints;
        u16 raceRank;
        u16 totalRaceRank;
        u16 totalUniqueRaceRank;
        u32 vr;
    };

    struct CRaceMode { // Demo race 3 6 2, Demo coin 3 7 0, Demo balloon 3 7 1, Winning run 3 4 2, live 2 2 0
        u32 type; // 0 -> SP, 1 -> MP, 2 -> Online 3 -> Demo
        u32 mode; // 0 -> GP, 1 -> TT, 2 -> VS, 3 -> Bt, 4 -> award, 5 -> course prev, 6 -> demo race, 7 -> demo battle, 8 -> menu video preview (unused)
        u32 submode; // 3 0 -> coin, 3 1 -> balloon, 0 2 -> Race
    };

    struct CRaceInfo {
        CKartInfo kartInfos[8];
        u32 courseID;
        CRaceMode raceMode;
        u32 engineLevel; 
        bool isMirror;
        bool teamsEnabled;
        u8 unknown4;
        u32 raceModeFlag;
        EItemMode itemMode;
        u32 playerAmount;
        u16 detailID;
        u16 masterID;
        u32 randomSeeds[2];
    };

    struct ModeManagerData
	{
		CRaceInfo modeRaceInfo;
        u8 _gap2;
		u8 driverPositionsZeroIndex[8];
		u8 driverPositions[8];
		u8 _gap3[3];
		u32 _gap4[0x7C4 / 4];
		u32 isMultiCPUEnabled;
		u32 multiCPUCount;
	};

    enum class StarGrade : u8 {
        NONE = 0,
        // Unused
        C = 1,
        B = 2,
        A = 3,
        // Used
        STAR_1 = 4,
        STAR_2 = 5,
        STAR_3 = 6,
        // Badges
        BADGE_EMPTY = 7,
        BADGE_SLOT_0 = 8,
        BADGE_SLOT_1 = 9,
        BADGE_SLOT_2 = 10,
        BADGE_SLOT_3 = 11,
        BADGE_SLOT_4 = 12,
        BADGE_SLOT_5 = 13,
        BADGE_SLOT_6 = 14,
        BADGE_SLOT_7 = 15,
        // Invalid
        INVALID = 0xFF
    };

    struct PACKED CustomCTGP7KartData 
    {
        u8 version;
        u8 megaMushTimer;
        struct {
            u8 reserved : 8;
        } info;
        u8 padding[1];

        static const u8 dataVersion = 2;

        CustomCTGP7KartData() {
            Init();
        }

        void Init() {
            version = 0;
            megaMushTimer = 0;
            info.reserved = 0;
            padding[0] = 0;
        }

        void MakeValid() {
            version = (0b101 << 5) | dataVersion;
        }

        bool IsValid() {
            return version == ((0b101 << 5) | dataVersion);
        }
    };

    struct KCLTerrainInfo
    {
        float speedLoss;
        float accelLoss;
        float slippery;
    };

    struct KartButtonData {
        union
        {
            u32 raw;
            struct
            {
                u32 accel : 1;
                u32 brake : 1;
                u32 drift : 1;
                u32 item : 1;
            };
            
        };
        static KartButtonData GetFromVehicle(u32 vehicle);
    };

    class SndLfoSin
    {
    public:

        SndLfoSin() : output(0.f), speed(0.f), progress(0.f), enabled(false) {}
        
        inline void SetSpeed(float speed) 
        {
            this->speed = speed * (360.f/60.f);
        }

        inline void Start(bool resume)
        {
            enabled = true;
            if (!resume)
                progress = 0.f;
        }

        inline void Stop()
        {
            enabled = false;
        }

        inline void Calc()
        {
            CalcImpl(this);
        }

        inline float Get()
        {
            return output;
        }

    private:
        float output;
        float speed;
        float progress;
        bool enabled;

        friend void initPatches();
        static void (*CalcImpl)(SndLfoSin* myself);
     };
}
