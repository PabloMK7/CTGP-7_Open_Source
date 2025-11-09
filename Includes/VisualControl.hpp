/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VisualControl.hpp
Open source lines: 393/393 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "Math.hpp"
#include "DataStructures.hpp"

namespace CTRPluginFramework {

    class VisualControl
    {
    public:
        static void nullFunc();
        struct Message {
            const char16_t* data;
            Message() : data(nullptr) {}
            Message(const char16_t* str) : data(str) {}
        };

        struct NwlytControlSight;

        struct NwlytControlSightVtable {
            void(*getRuntimeTypeInfo)(NwlytControlSight* obj);                                                                                          // 0x00
            u32 null;                                                                                                                                   // 0x04
            void(*_NwlytControlSight__deallocating)(NwlytControlSight* obj);                                                                            // 0x08
            void(*build)(NwlytControlSight* obj, u32* createArg);                                                                                       // 0x0C
            void(*calc)(NwlytControlSight* obj);                                                                                                        // 0x10
            void(*draw)(NwlytControlSight* obj);                                                                                                        // 0x14
            u32(*getElementHandle)(NwlytControlSight* obj, const SafeStringBase& element, u32 elementType);                         // 0x18
            u32(*getConstElementHandle)(NwlytControlSight* obj, const SafeStringBase& element, u32 elementType);                    // 0x1C
            void(*setPosImpl)(NwlytControlSight* obj, u32 elementHandle, const Vector3& position);                                       // 0x20
            Vector3&(*getPosImpl)(NwlytControlSight* obj, u32 elementHandle);                                                                           // 0x24
            u32 null0;                                                                                                                                   // 0x28
            Vector3&(*getSizeImpl)(NwlytControlSight* obj, u32 elementHandle);                                                                          // 0x2C
            u32 null1;                                                                                                                                  // 0x30
            u32 null2;                                                                                                                                  // 0x34
            u32 null3;                                                                                                                                  // 0x38
            u32 null4;                                                                                                                                  // 0x3C
            u32 null5;                                                                                                                                  // 0x40
            u32 null6;                                                                                                                                  // 0x44
            void*(*getGlobalMtxImpl)(NwlytControlSight* obj, u32 elementHandle);                                                                        // 0x48
            void(*setAlphaImpl)(NwlytControlSight* obj, u32 elementHandle, u8 amount);                                                                  // 0x4C
            u32 null7;                                                                                                                                  // 0x50
            u8(*getGlobalAlphaImpl)(NwlytControlSight* obj, u32 elementHandle);                                                                         // 0x54
            void(*setVisibleImpl)(NwlytControlSight* obj, u32 elementHandle, bool visible);                                                             // 0x58
            u32 null8;                                                                                                                                  // 0x5C
            bool(*getGlobalVisibleImpl)(NwlytControlSight* obj, u32 elementHandle);                                                                     // 0x60
            u32 null9;                                                                                                                                  // 0x64
            void*(*getPane)(NwlytControlSight* obj, u32 elementHandle);                                                                                 // 0x68
            u32 null10;                                                                                                                                 // 0x6C
            void(*replaceMessageImpl)(NwlytControlSight* obj, u32 elementHandle, const Message& message, void* messageArg, void* letterStepper);  // 0x70
            void(*replaceGraphicImpl)(NwlytControlSight* obj, u32 elementHandle, const SafeStringBase& graphic);                    // 0x74
            u32 null11;                                                                                                                                 // 0x78
            u32 null12;                                                                                                                                 // 0x7C
            void(*setRootPosImpl)(NwlytControlSight* obj, const Vector3& pos);                                                                          // 0x80
            Vector3&(*getRootPosImpl)(NwlytControlSight* obj);                                                                                          // 0x84
            void(*setRootMtxImpl)(NwlytControlSight* obj, const void* matrix);                                                                          // 0x88
            u32 null13;                                                                                                                                 // 0x8C
            void*(*getGlobalRootMtxImpl)(NwlytControlSight* obj);                                                                                       // 0x90
            void(*setRootAlphaImpl)(NwlytControlSight* obj, u8 amount);                                                                                 // 0x94
            u32 null14;                                                                                                                                 // 0x98
            u8(*getGlobalRootAlphaImpl)(NwlytControlSight* obj);                                                                                        // 0x9C
            void(*setRootVisibleImpl)(NwlytControlSight* obj, bool visible);                                                                            // 0xA0
            bool(*getRootVisibleImpl)(NwlytControlSight* obj);                                                                                          // 0xA4
            bool(*getGlobalRootVisibleImpl)(NwlytControlSight* obj);                                                                                    // 0xA8
            u32 null15;                                                                                                                                 // 0xAC
            u32 null16;                                                                                                                                 // 0xB0
            u32 null17;                                                                                                                                 // 0xB4
            void*(*getLayoutInfo)(NwlytControlSight* obj);                                                                                              // 0xB8
            void*(*getRootPane)(NwlytControlSight* obj);                                                                                                // 0xBC
            void*(*getRootPaneConst)(NwlytControlSight* obj);                                                                                           // 0xC0
            void*(*getParentGlobalVisible)(NwlytControlSight* obj);                                                                                     // 0xC4
        };

        struct NwlytControlAnimator;
        struct NwlytControlAnimatorVtable {
            u32 null;
            u32 null1;
            u32 null2;
            void(*applyAnimation)(NwlytControlAnimator* obj);
            void* nwlytAnimationFamilyCons;
            u32 null3;
            u32 null4;
        };

        struct NwlytControlSight
        {
            NwlytControlSightVtable* vtable;
            u32 data[];
        };

        struct NwlytControlAnimator
        {
            NwlytControlAnimatorVtable* vtable;
            u32 data[];
            void ApplyAnimation() {
                vtable->applyAnimation(this);
            }
        };

        struct AnimationFamily {
            struct AnimationItem {
                u32 vtable;

                float GetCurrentFrame();
            };
            
            struct AnimationItemArray
            {
                u32 size;
                AnimationItem** data;
            };
            u32 vtable;
            u32 unk;
            u32 unk2;
            AnimationItemArray animArray;
            u32 current_anim;
            float unk3;

            void SetAnimation(u32 subAnimationID, float frame);
            void ChangeAnimation(u32 subAnimationID, float frame);
            void ChangeAnimationByRate(u32 subAnimationID, float frame);

            // Gets progress of current animation
            float GetAnimationRate();

            AnimationItem* GetAnimationItem(u32 subAnimationID) {
                AnimationItem* animationItem = animArray.data[subAnimationID < animArray.size ? subAnimationID : 0];
                return animationItem;
            }

            u32 GetCurrentAnimationItem() {
                return current_anim;
            }
        };

        struct GameVisualControl;

        struct GameVisualControlVtable {
            void*(*getDTIClassInfo)(GameVisualControl* obj);                               // 0x00
            void*(*getDTIClass)(GameVisualControl* obj);                                   // 0x04
            u32 null;                                                       // 0x08
            void(*_deallocating)(GameVisualControl* obj);                    // 0x0C
            void(*create)(GameVisualControl* obj, void* createArg);                        // 0x10
            void(*init)(GameVisualControl* obj);                                           // 0x14
            void(*calc)(GameVisualControl* obj);                                           // 0x18
            void(*render)(GameVisualControl* obj);                                         // 0x1C
            void(*renderMainL)(GameVisualControl* obj);                                    // 0x20
            void(*renderMainR)(GameVisualControl* obj);                                    // 0x24
            void(*renderSub)(GameVisualControl* obj);                                      // 0x28
            u32 null1;                                                      // 0x2C
            void(*accept)(GameVisualControl* obj, void* actorVisitor);                     // 0x30
            void(*callbackInvokeEventID)(GameVisualControl* obj, int eventID);             // 0x34
            u32 null2;                                                      // 0x38
            void(*createOuter)(GameVisualControl* obj, void* data);                        // 0x3C
            void(*initOuter)(GameVisualControl* obj);                                      // 0x40
            u32 null3;                                                      // 0x44
            void(*forceCameraDir)(GameVisualControl* obj);                                 // 0x48
            void(*onCreate)(GameVisualControl* obj, void* createArg);                      // 0x4C
            void(*reset)(GameVisualControl* obj);                                          // 0x50
            void(*readyFadein)(GameVisualControl* obj, int something);                     // 0x54
            void(*readyFadeout)(GameVisualControl* obj, int something);                    // 0x58
            void(*fadeStep)(GameVisualControl* obj);                                       // 0x5C
            bool(*isFadeComplete)(GameVisualControl* obj);                                 // 0x60
            void(*draw)(GameVisualControl* obj, u32 drawScreen);                           // 0x64
            void*(*getRootPane)(GameVisualControl* obj);                                   // 0x68
            void(*generateFader)(GameVisualControl* obj);                                  // 0x6C
            void(*onReset)(GameVisualControl* obj);                                         // 0x70
            void(*onCalc)(GameVisualControl* obj);                                         // 0x74
            void(*onCalcPostAnim)(GameVisualControl* obj);                                 // 0x78
            void(*animMenuIn)(GameVisualControl* obj);                                     // 0x7C
            void(*animMenuOut)(GameVisualControl* obj);                                    // 0x80
            VisualControl* visualControl;                                                  // 0x84
            void* userData;                                                                // 0x88
            u32 null5;                                                                     // 0x8C
        };

        struct GameVisualControl {
            GameVisualControlVtable* vtable;
            u32 data[];

            void* GetMessageDataList() { // Can cast to Language::MsbtHandler::MessageDataList
                return (void*)&data[(0x6C-0x4)/4];
            }

            NwlytControlSight* GetNwlytControl() {
                return (NwlytControlSight*)data[(0x68-0x4)/4];
            }

            NwlytControlAnimator* GetNwlytAnimator() {
                return (NwlytControlAnimator*)data[(0x64-0x4)/4];
            }

            VisualControl* GetVisualControl() {
                return vtable->visualControl;
            }

            AnimationFamily* GetAnimationFamily(u32 animationID) {
                u32* animFamilyList = (u32*)data[(0x64-0x4)/4];
                u32 count = animFamilyList[1];
                u32* animList = (u32*)animFamilyList[2];
                if (animationID < count) {
                    return (AnimationFamily*)animList[animationID];
                } else {
                    return (AnimationFamily*)animList[0];
                }
            }

            void EnableCalc(bool enable) {
                ((bool*)data)[(0x28-4)] = enable;
            }

            void AnimOff() {
                ((void(*)(GameVisualControl*))(GameFuncs::BaseFastControlAnimOff))(this);
            }

            void CalcAnim(u32 &elementHandle) {
                ((void(*)(GameVisualControl*, u32&))GameFuncs::BaseFastControlCalcAnimEl)(this, elementHandle);
            }

            // Must be called in OnCalc
            void UpdatePosRaw(u32 elementHandle, const Vector3& position) {
                float* posMatrix = (float*)(elementHandle + 0x50);
                posMatrix[3] = position.x;
                posMatrix[7] = position.y;
                posMatrix[11] = position.z;
                u8* flags = (u8*)elementHandle + 0xB7;
                *flags = (*flags & 0xBF) | 0x40;
            }

            // Must be called in OnCalc
            void UpdateScaleRaw(u32 elementHandle, const Vector3& scale) {
                float* posMatrix = (float*)(elementHandle + 0x50);
                posMatrix[0] = scale.x;
                posMatrix[5] = scale.y;
                posMatrix[10] = scale.z;
                u8* flags = (u8*)elementHandle + 0xB7;
                *flags = (*flags & 0xBF) | 0x40;
            }

            u32** GetRawTexturePAddr(u32 elementHandle) {
                
                u32 getmat = ((u32**)elementHandle)[0][0x28/4];
                u32 mat = ((u32(*)(u32, u32))getmat)(elementHandle, 0);
                u32** texdata = ((u32***)mat)[0x34/4] + 0x4/4;
                return texdata;
            }

            u32* GetRawTextureVAddr(u32 elementHandle) {
                auto convertPhysToVirt =[](u32 phys) {
                    u32 res = 0;
                    if (phys >= OS_VRAM_PADDR && phys < (OS_VRAM_PADDR + OS_VRAM_SIZE)) {
                        res = phys - (OS_VRAM_PADDR - OS_VRAM_VADDR);
                    }
                    if (phys >= OS_OLD_FCRAM_PADDR && phys < (OS_OLD_FCRAM_PADDR + OS_VRAM_SIZE)) {
                        res = phys - (OS_OLD_FCRAM_PADDR - OS_OLD_FCRAM_VADDR);
                    }
                    return res;
                };
                return (u32*)convertPhysToVirt((u32)*GetRawTexturePAddr(elementHandle));
            }
        };

        struct AnimationDefine;

        struct AnimationDefineVtable {
            void(*defineAnimation)(AnimationDefine* obj);
            u32 null;
            u32 null1;
        };

        struct AnimationDefine {
            AnimationDefineVtable* vtable;
            u32 data[];

            static AnimationDefineVtable empty;

            enum class AnimationKind : s32 {
                NOPLAY = -1, // Does not play, frame needs to be set manually
                ONCE = 0, // Plays once, then returns to frame 0
                LOOP = 1, // Plays in a loop until the animation is changed
                NEXT = 2 // Plays once, then goes to the next animation
            };

            void InitAnimationFamilyList(int animationCount);
            void InitAnimationFamily(int animationID, const char* affectedGroup, int unknwownCount);
            void InitAnimation(int subAnimationID, const char* animationName, AnimationKind kind);
            void InitAnimationStopByRate(int subAnimationID, const char* animationName, float rate);
            void InitAnimationReverse(int subAnimationID, const char* animationName, AnimationKind kind);

            VisualControl* GetVisualControl() {return lastLoadVisualControl;}
        };

        struct CreateArg;

        struct CreateArgVtable {
            u32 null;
            void(*userDataDefine)(CreateArg* obj);
            u32 null1;
            AnimationDefine*(*getAnimationDefine)(CreateArg* obj);
            u32 null2;
            u32 null3;
        };

        struct CreateArg
        {
            CreateArgVtable* vtable;
            u32 data[];
        };

        VisualControl(const char* controlName, bool isTopScreen, void* controlvtable = nullptr);
        ~VisualControl();

        enum class ControlType {
            VISUAL_CONTROL,
            BASEMENUVIEW_CONTROL,
            CUP_SELECT_BG_CONTROL,
            CUP_CURSOR_CONTROL,
            CUP_BTN_CONTROL,
            CHARA_NAME,

            ENDING_SCENE_START,
            ENDING_MOVIE,
            ENDING_SCENE_END,
        };

        void Load(u32 page, const char* elementName, ControlType controlType);
        void LoadRace();
        static GameVisualControl* Build(u32* page, const char* controlName, const char* elementName, const AnimationDefineVtable* animationDefineVtable, const GameVisualControlVtable* gameVisualControlVtable,
                ControlType type);
        u32 GetLayoutElementHandle(const char* paneName);
        GameVisualControl* GetGameVisualControl() {return gameVisualControl;};
        void SetUserData(void* userData) {gameVisualControlVtable.userData = userData;}
        void* GetUserData() {return gameVisualControlVtable.userData;}

        void SetAnimationDefineCallback(void(*callback)(AnimationDefine*)) {animationDefineVtable.defineAnimation = callback;}

        void SetOnCreateCallback(void (*callback)(GameVisualControl*, void*)) {gameVisualControlVtable.onCreate = callback;}
        void SetOnResetCallback(void(*callback)(GameVisualControl*)) {gameVisualControlVtable.onReset = callback;}
        void SetOnCalcCallback(void(*callback)(GameVisualControl*)) {gameVisualControlVtable.onCalc = callback;}

        void SetDeallocateCallback(void(*callback)(VisualControl*)) { deallocateCallback = callback; }
        void Deallocate() {originalDeallocate(gameVisualControl); gameVisualControl = nullptr;}

        class GameFuncs {
        public:
            static void* PointControlVtable;
            static u32 CreateArgCons;
            static u32 ControlAnimCons;
            static u32 VisualControlCons;
            static u32 InitCreateArgFunc;
            static u32 EndSetupControlFunc;
            static u32 BaseFastControlAnimOff;
            static u32 BaseFastControlCalcAnimEl;

            static u32 initAnimationFamilyList;
            static u32 initAnimationFamily;
            static u32 initAnimation;
            static u32 initAnimationStopByRate;
            static u32 initAnimationReverse;

            static u32 animFamilySetAnimation;
            static u32 animFamilyChangeAnimation;
            static u32 animFamilyChangeAnimationByRate;
            static u32 animFamilyGetAnimationRate;

            static u32 BaseMenuViewControlCons;
        };
        
    private:
        static void OnReset(GameVisualControl* visualControl);
        static void OnDeallocate(GameVisualControl* control);

        const char* controlName;
        bool topScreen;
        AnimationDefineVtable animationDefineVtable;
        GameVisualControlVtable gameVisualControlVtable;
        
        GameVisualControl* gameVisualControl;

        void(*deallocateCallback)(VisualControl* control);
        void(*originalDeallocate)(GameVisualControl* control);

        static VisualControl* lastLoadVisualControl;        
    };
}