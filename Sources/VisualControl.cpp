/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: VisualControl.cpp
Open source lines: 261/261 (100.00%)
*****************************************************/

#include "VisualControl.hpp"
#include "MarioKartFramework.hpp"
#include "str16utils.hpp"
#include "ExtraResource.hpp"
#include "MenuPage.hpp"

namespace CTRPluginFramework {

    VisualControl::AnimationDefineVtable VisualControl::AnimationDefine::empty = {
        .defineAnimation = (decltype(VisualControl::AnimationDefineVtable::defineAnimation))VisualControl::nullFunc
    };

    float VisualControl::AnimationFamily::AnimationItem::GetCurrentFrame() {
        float(*AnimationItemGetCurrentFrame)(void*) = (decltype(AnimationItemGetCurrentFrame))MenuPageHandler::GameFuncs::AnimationItemGetCurrentFrame;
        return AnimationItemGetCurrentFrame(this);
    }

    // Sets the animation and disables automatic updating
    void VisualControl::AnimationFamily::SetAnimation(u32 subAnimationID, float frame) {
        ((void(*)(AnimationFamily*, int, float))VisualControl::GameFuncs::animFamilySetAnimation)(this, subAnimationID, frame);
    }

    // Sets the animation and enables automatic updating
    void VisualControl::AnimationFamily::ChangeAnimation(u32 subAnimationID, float frame) {
        ((void(*)(AnimationFamily*, int, float))VisualControl::GameFuncs::animFamilyChangeAnimation)(this, subAnimationID, frame);
    }

    // Sames as ChangeAnimation, but takes the animation progress (rate) between [0, 1]
    void VisualControl::AnimationFamily::ChangeAnimationByRate(u32 subAnimationID, float rate) {
        ((void(*)(AnimationFamily*, int, float))VisualControl::GameFuncs::animFamilyChangeAnimationByRate)(this, subAnimationID, rate);
    }

    // Gets the current animation progress (rate)
    float VisualControl::AnimationFamily::GetAnimationRate() {
        return ((float(*)(AnimationFamily*))VisualControl::GameFuncs::animFamilyGetAnimationRate)(this);
    }

    void VisualControl::AnimationDefine::InitAnimationFamilyList(int animationCount) {
        ((void(*)(AnimationDefine*, int))VisualControl::GameFuncs::initAnimationFamilyList)(this, animationCount);
    }
    void VisualControl::AnimationDefine::InitAnimationFamily(int animationID, const char* affectedGroup, int unknwownCount) {
        ((void(*)(AnimationDefine*, int, const SafeStringBase&, int))VisualControl::GameFuncs::initAnimationFamily)(this, animationID, SafeStringBase(affectedGroup), unknwownCount);
    }

    // Inits the animation normally (see AnimationKind)
    void VisualControl::AnimationDefine::InitAnimation(int subAnimationID, const char* animationName, AnimationKind kind) {
        ((void(*)(AnimationDefine*, int, const SafeStringBase&, AnimationKind))VisualControl::GameFuncs::initAnimation)(this, subAnimationID, SafeStringBase(animationName), kind);
    }
    // Adds an stop point. When the animation is selected, it's set to the progress (rate) specified and stopped.
    void VisualControl::AnimationDefine::InitAnimationStopByRate(int subAnimationID, const char* animationName, float rate) {
        ((void(*)(AnimationDefine*, int, const SafeStringBase&, float))VisualControl::GameFuncs::initAnimationStopByRate)(this, subAnimationID, SafeStringBase(animationName), rate);
    }
    // Inits the subanimation in reverse (see AnimationKind)
    void VisualControl::AnimationDefine::InitAnimationReverse(int subAnimationID, const char* animationName, AnimationKind kind) {
        ((void(*)(AnimationDefine*, int, const SafeStringBase&, AnimationKind))VisualControl::GameFuncs::initAnimationReverse)(this, subAnimationID, SafeStringBase(animationName), kind);
    }

    void* VisualControl::GameFuncs::PointControlVtable = nullptr; // (void*)0x61A0F0;
    u32 VisualControl::GameFuncs::CreateArgCons = 0; // 0x00157998;
    u32 VisualControl::GameFuncs::ControlAnimCons = 0; // 0x0015B42C;
    u32 VisualControl::GameFuncs::VisualControlCons = 0; // 0x001579F0;
    u32 VisualControl::GameFuncs::InitCreateArgFunc = 0; // 0x00167BD8;
    u32 VisualControl::GameFuncs::EndSetupControlFunc = 0; // 0x00167EF8;

    u32 VisualControl::GameFuncs::BaseFastControlAnimOff = 0; // 0x0015ABA0;
    u32 VisualControl::GameFuncs::BaseFastControlCalcAnimEl = 0; // 0x0015AC8C;

    u32 VisualControl::GameFuncs::initAnimationFamilyList = 0; // 0x0015B2B0;
    u32 VisualControl::GameFuncs::initAnimationFamily = 0; // 0x0015B1D8;
    u32 VisualControl::GameFuncs::initAnimation = 0; // 0x0015B150;
    u32 VisualControl::GameFuncs::initAnimationStopByRate = 0; // 0x00142358;
    u32 VisualControl::GameFuncs::initAnimationReverse = 0; // 0x0015B34C;

    u32 VisualControl::GameFuncs::animFamilySetAnimation = 0; // 0x0015B480;
    u32 VisualControl::GameFuncs::animFamilyChangeAnimation = 0; // 0x0015B51C;
    u32 VisualControl::GameFuncs::animFamilyChangeAnimationByRate = 0; // 0x0015B5B8;
    u32 VisualControl::GameFuncs::animFamilyGetAnimationRate = 0; // 0x004E529C

    u32 VisualControl::GameFuncs::BaseMenuViewControlCons = 0; // 0x0016A5BC;

    VisualControl* VisualControl::lastLoadVisualControl = nullptr;

    void VisualControl::nullFunc() { return; }
    static u32 return0xC() {return 0xC;}
    static VisualControl::AnimationDefine* getAnimationDefine(VisualControl::CreateArg* createArgs) {
        return (VisualControl::AnimationDefine*)((u32)createArgs + 0x78);
    }
    static VisualControl::AnimationDefine* getAnimationDefineBaseMenuButtonControl(VisualControl::CreateArg* createArgs) {
        return *(VisualControl::AnimationDefine**)((u32)createArgs + 0x78);
    }

    void VisualControl::OnReset(GameVisualControl* visualControl) {
        visualControl->AnimOff();
    }

    void VisualControl::OnDeallocate(GameVisualControl* visualControl) {
        VisualControl* v = visualControl->vtable->visualControl;
        if (v->deallocateCallback)
            v->deallocateCallback(v);
        else
            v->Deallocate();
    }

    VisualControl::VisualControl(const char* cName, bool isTopScreen, void* controlVtable) {
        this->controlName = cName;
        this->topScreen = isTopScreen;

        // Setup animationDefineVtable
        animationDefineVtable.defineAnimation = (void(*)(AnimationDefine*))nullFunc;

        controlVtable = controlVtable ? controlVtable : GameFuncs::PointControlVtable;

        // Setup VisualControlVtable
        memcpy(&gameVisualControlVtable, GameFuncs::PointControlVtable, sizeof(gameVisualControlVtable));
        originalDeallocate = gameVisualControlVtable._deallocating;
        gameVisualControlVtable._deallocating = OnDeallocate;
        gameVisualControlVtable.onCreate = (void(*)(GameVisualControl*, void*))nullFunc; // OnCreate
        gameVisualControlVtable.onReset = OnReset; // OnReset
        gameVisualControlVtable.onCalc = (void(*)(GameVisualControl*))nullFunc; // OnCalc
        gameVisualControlVtable.animMenuIn = (void(*)(GameVisualControl*))nullFunc; // OnMenuIn
        gameVisualControlVtable.animMenuOut = (void(*)(GameVisualControl*))nullFunc; // OnMenuOut

        gameVisualControlVtable.visualControl = this;
        gameVisualControlVtable.userData = nullptr;

        deallocateCallback = nullptr;
    }

    VisualControl::~VisualControl() {}

    
    static u32 CreateArgDefaultVtable[] = {0, (u32)VisualControl::nullFunc, 0, (u32)getAnimationDefine, 0, 0};
    static u32 CreateArgBaseMenuButtonControl[] = {0, (u32)VisualControl::nullFunc, 0, (u32)getAnimationDefineBaseMenuButtonControl, 0, 0};
    static u32 BaseMenuViewCreateArgVtable[] = {0, (u32)VisualControl::nullFunc, 0, (u32)return0xC, 0, 0};

    VisualControl::GameVisualControl* VisualControl::Build(u32* page, const char* controlName, const char* elementName,
    const AnimationDefineVtable* animationDefineVtable, const GameVisualControlVtable* gameVisualControlVtable, ControlType type) {
        u8 visualControlCreateArg[0x90];
		u8* controlAnimator = visualControlCreateArg + 0x78;
		u32* controlInitializer = (u32*)(page[0x6C/4]);
        SafeStringBase cName(controlName);
        SafeStringBase elName(elementName);

        //lastLoadVisualControl = this;

		// Construct visualcontrolcreatearg
		((u8*(*)(u8*))(GameFuncs::CreateArgCons))(visualControlCreateArg);
        if (type == VisualControl::ControlType::CUP_BTN_CONTROL)
            *((u32*)visualControlCreateArg) = (u32)CreateArgBaseMenuButtonControl;
        else
		    *((u32*)visualControlCreateArg) = (u32)CreateArgDefaultVtable;

        if (animationDefineVtable) {
            // Construct controlanimator
            ((u8*(*)(u8*))(GameFuncs::ControlAnimCons))(controlAnimator);
            *((u32*)controlAnimator) = (u32)animationDefineVtable;
        }
		
		// Construct visualcontrol
        u32* visualControl;
        switch (type)
        {
        case ControlType::VISUAL_CONTROL :
            visualControl = (u32*)GameAlloc::game_operator_new_autoheap(0x94); // 0x001097C8
            memset(visualControl, 0, 0x94);
            ((u32*(*)(u32*))(GameFuncs::VisualControlCons))(visualControl);
            *visualControl = (u32)gameVisualControlVtable;
            visualControl[0x7C/4] = 0;
            visualControl[0x80/4] = 0;
            visualControl[0x84/4] = 0;
            break;

        case ControlType::BASEMENUVIEW_CONTROL :
            visualControl = (u32*)GameAlloc::game_operator_new_autoheap(0xB0); // 0x001097C8
            memset(visualControl, 0, 0xB0);
            ((u32*(*)(u32*))(GameFuncs::BaseMenuViewControlCons))(visualControl);
            *visualControl = (u32)gameVisualControlVtable;
            break;
        
        case ControlType::CUP_CURSOR_CONTROL:
        case ControlType::CUP_SELECT_BG_CONTROL:
            visualControl = (u32*)GameAlloc::game_operator_new_autoheap(0xA8); // 0x001097C8
            memset(visualControl, 0, 0xA8);
            ((u32*(*)(u32*))(GameFuncs::BaseMenuViewControlCons))(visualControl);
            *visualControl = (u32)gameVisualControlVtable;
            break;

        case ControlType::CUP_BTN_CONTROL:
            visualControl = (u32*)GameAlloc::game_operator_new_autoheap(0x23C); // 0x001097C8
            ((u32*(*)(u32*))(MenuPageHandler::GameFuncs::BaseMenuButtonControlCons))(visualControl);
            *visualControl = (u32)gameVisualControlVtable;
            visualControl[0x7C/4] = 0x30;
            visualControl[0x80/4] = 0x800;
            ((u8*)visualControl)[0x221] = 1;
            ((u8*)visualControl)[0x222] = 2;
            ((u8*)visualControl)[0x223] = 3;
            break;
        case ControlType::CHARA_NAME:
            visualControl = (u32*)GameAlloc::game_operator_new_autoheap(0xB4); // 0x001097C8
            memset(visualControl, 0, 0xB4);
            ((u32*(*)(u32*))(GameFuncs::BaseMenuViewControlCons))(visualControl);
            *visualControl = (u32)gameVisualControlVtable;
            ((float*)visualControl)[0xAC/4] = 1.f;
            ((float*)visualControl)[0xB0/4] = 0.f;
            break;

        default:
            break;
        }
		

		// Setup visualcontrol
		if (!visualControl[0x10/4]) {
			visualControl[0x10/4] = (u32)visualControl; // ??? Okay game
		}
		u32* something = (u32*)(visualControl[0x10/4]);
		something[0x4/4] = controlInitializer[0];
		((void(*)(u32, u32*))((u32***)controlInitializer)[0][0x14/4][0])(controlInitializer[0] + 0x14, visualControl + 0x8/4); // Linked list append

		// Initialize visualcontrol
		controlInitializer[0x4/4] = (u32)visualControl;
		((void(*)(u32*, u8*, SafeStringBase*, SafeStringBase*))GameFuncs::InitCreateArgFunc)(controlInitializer, visualControlCreateArg, &cName, &elName);

		// Finish initializing visualcontrol
		controlInitializer[0x8/4] = (u32)visualControlCreateArg;
		((u32(*)(u32*, u8*))GameFuncs::EndSetupControlFunc)(controlInitializer, visualControlCreateArg);
        if (type == ControlType::BASEMENUVIEW_CONTROL || type == ControlType::CUP_SELECT_BG_CONTROL || type == ControlType::CUP_BTN_CONTROL || type == ControlType::CHARA_NAME) *((u32*)visualControlCreateArg) = (u32)BaseMenuViewCreateArgVtable;

		// Append visualcontrol to the race page
        SeadArrayPtr<GameVisualControl*>* elementArray = (SeadArrayPtr<GameVisualControl*>*)(page + 0xAC/4);
        elementArray->Push((GameVisualControl*)visualControl);

		// Return created visual control
		return (GameVisualControl*)visualControl;
    }

    u32 VisualControl::GetLayoutElementHandle(const char* paneName) {
        NwlytControlSight* layout = gameVisualControl->GetNwlytControl();
        SafeStringBase pName(paneName);
        return layout->vtable->getConstElementHandle(layout, pName, 0);
    }

    void VisualControl::LoadRace() {
        gameVisualControl = Build((u32*)MarioKartFramework::getBaseRacePage(), controlName, topScreen ? "Point" : "Coin", &animationDefineVtable, &gameVisualControlVtable, ControlType::VISUAL_CONTROL);
        lastLoadVisualControl = this;
    }

    void VisualControl::Load(u32 page, const char* elementName, ControlType controlType) {
        gameVisualControl = Build((u32*)page, controlName, elementName, &animationDefineVtable, &gameVisualControlVtable, controlType);
        lastLoadVisualControl = this;
    }
}