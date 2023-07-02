/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MenuPage.cpp
Open source lines: 2215/2549 (86.90%)
*****************************************************/

#include "MenuPage.hpp"
#include "StaffRoll.hpp"
#include "ExtraResource.hpp"
#include "MarioKartFramework.hpp"
#include "Lang.hpp"
#include "VersusHandler.hpp"
#include "MissionHandler.hpp"
#include "foolsday.hpp"
#include "CustomTextEntries.hpp"
#include "CourseManager.hpp"
#include "UserCTHandler.hpp"
#include "MarioKartTimer.hpp"
#include "HookInit.hpp"
#include "Unicode.h"
#include "CharacterManager.hpp"

extern "C" {
    void coursePageInitOmakaseTfunc();
	u32 g_coursePageInitOmakaseTret;
	void coursePageInitOmakaseBfunc();
	u32 g_coursePageInitOmakaseBret;
	void trophyPageSelectNextScenefunc();
	u32 g_trophyPageSelectNextSceneret;
	void thankyouPageInitControlfunc();
	u32 g_thankyouPageInitControlret;
}

namespace CTRPluginFramework {
    MenuPageHandler::ExecutableSectionClassInfoVtableList* MenuPageHandler::classInfoVtableList;

    MenuPageHandler::MenuSingleModePage* MenuPageHandler::menuSingleMode;
    MenuPageHandler::MenuSingleCupGPPage* MenuPageHandler::menuSingleCupGP;
    MenuPageHandler::MenuMultiCupGPPage* MenuPageHandler::menuMultiCupGP;
    MenuPageHandler::MenuSingleCupPage* MenuPageHandler::menuSingleCup;
    MenuPageHandler::MenuMultiCupPage* MenuPageHandler::menuMultiCup;
    MenuPageHandler::MenuWifiCupPage* MenuPageHandler::menuWifiCup;
    MenuPageHandler::MenuSingleCoursePage* MenuPageHandler::menuSingleCourse;
    MenuPageHandler::MenuMultiCoursePage* MenuPageHandler::menuMultiCourse;
    MenuPageHandler::MenuWifiCoursePage* MenuPageHandler::menuWifiCourse;
    MenuPageHandler::MenuMultiCourseVote* MenuPageHandler::menuMultiCourseVote;
    MenuPageHandler::MenuWifiCourseVote* MenuPageHandler::menuWifiCourseVote;
    MenuPageHandler::MenuEndingPage* MenuPageHandler::menuEndingPage;
    int MenuPageHandler::MenuSingleCupBasePage::startingButtonID = 0;
    int MenuPageHandler::MenuSingleCupBasePage::selectedCupIcon = 0;
    const VisualControl::AnimationDefineVtable MenuPageHandler::MenuSingleCupBasePage::cupSelectBGAnimationDefineVtable = {MenuPageHandler::MenuSingleCupBasePage::CupSelectBGControlAnimationDefine, 0, 0};
    bool MenuPageHandler::MenuSingleModePage::IsButtonConstructing = false;

    RT_HOOK MenuPageHandler::MenuSingleCupBasePage::onPageEnterHook;
    RT_HOOK MenuPageHandler::MenuSingleCupBasePage::onPagePreStepHook;
    RT_HOOK MenuPageHandler::MenuSingleCupBasePage::initControlHook;
    RT_HOOK MenuPageHandler::MenuSingleCupBasePage::buttomHandlerOKHook;

    RT_HOOK MenuPageHandler::MenuSingleCupGPPage::onPagePreStepHook;
    RT_HOOK MenuPageHandler::MenuSingleCupGPPage::buttonHandlerOKHook;

    RT_HOOK MenuPageHandler::MenuSingleCourseBasePage::onPageEnterHook;
    RT_HOOK MenuPageHandler::MenuSingleCourseBasePage::coursePageInitOmakaseTHook;
    RT_HOOK MenuPageHandler::MenuSingleCourseBasePage::coursePageInitOmakaseBHook;

    std::vector<u32> MenuPageHandler::MenuSingleCourseBasePage::blockedCourses;

    RT_HOOK MenuPageHandler::trophyPageSelectNextSceneHook;
    RT_HOOK MenuPageHandler::thankyouPageInitControlHook;

    bool MenuPageHandler::MenuEndingPage::loadCTGPCredits = false;

    VisualControl::GameVisualControlVtable* MenuPageHandler::MenuSingleCharaPage::controlVtable;
    bool MenuPageHandler::MenuSingleCharaPage::isInSingleCharaPage = false;
    void (*MenuPageHandler::MenuSingleCharaPage::initControlBackup)(GameSequenceSection* own) = nullptr;
    void (*MenuPageHandler::MenuSingleCharaPage::pageEnterBackup)(GameSequenceSection* own) = nullptr;
    void (*MenuPageHandler::MenuSingleCharaPage::pageExitBackup)(GameSequenceSection* own) = nullptr;
    void (*MenuPageHandler::MenuSingleCharaPage::pagePreStepBackup)(GameSequenceSection* own) = nullptr;

    u32 MenuPageHandler::GameFuncs::SectionClassInfoBaseSubObject = 0; // 0x004C0E14;
    u32 MenuPageHandler::GameFuncs::SectionClassInfoListAddSectionClassInfo = 0; // 0x004B9A80;
    u32 MenuPageHandler::GameFuncs::movieBasePageCons = 0; //0x004A44CC;
    u32 MenuPageHandler::GameFuncs::SingleModeVtable = 0; // 0x647534;
    u32 MenuPageHandler::GameFuncs::BaseMenuPageInitSlider = 0; // 0x00472394;
    u32 MenuPageHandler::GameFuncs::SetupControlMovieView = 0; // 0x005DBC10;
    u32 MenuPageHandler::GameFuncs::SetupControlMenuButton = 0; // 0x005DAE94;
    u32 MenuPageHandler::GameFuncs::SetupControlBackButton = 0; // 0x005DAC78;
    u32 MenuPageHandler::GameFuncs::MovieViewSetMoviePane = 0; // 0x0018420C;
    u32 MenuPageHandler::GameFuncs::BaseMenuButtonControlSetTex = 0; // 0x001730DC;
    u32 MenuPageHandler::GameFuncs::CaptionAnimationDefineVtable = 0; // 0x61980C;
    u32 MenuPageHandler::GameFuncs::BaseMenuViewVtable = 0; // 0x619830;
    u32 MenuPageHandler::GameFuncs::ControlSliderSetSlideH = 0; // 0x00481AE4;
    u32 MenuPageHandler::GameFuncs::initAnimationTouchSelect = 0; // 0x001712C4;
    u32 MenuPageHandler::GameFuncs::CursorMoveGetDir = 0; // 0x001431AC;
    u32 MenuPageHandler::GameFuncs::MenuCaptionAnimIn = 0; // 0x0014CA48;
    u32 MenuPageHandler::GameFuncs::StartFadeIn = 0; // 0x00471CD0;
    u32 MenuPageHandler::GameFuncs::SetPlayerMasterID = 0; // 0x004D3BD4;
    u32 MenuPageHandler::GameFuncs::SetCameraTargetPlayer = 0; // 0x004D3EE4;
    u32 MenuPageHandler::GameFuncs::menuSingleCupGPVtable = 0; //0x00647F5C;
    u32 MenuPageHandler::GameFuncs::SequenceBasePageInitMenu = 0; // 0x004D4428;
    u32 MenuPageHandler::GameFuncs::menuMultiCupGPVtable = 0; // 0x647140;
    u32 MenuPageHandler::GameFuncs::menuSingleCupVtable = 0; //0x646708;
    u32 MenuPageHandler::GameFuncs::menuMultiCupVtable = 0; //0x645E70;
    u32 MenuPageHandler::GameFuncs::MenuWifiCupVtable = 0; //0x645A0C;
    u32 MenuPageHandler::GameFuncs::MenuSingleCourseCons = 0; //0x004A7110;
    u32 MenuPageHandler::GameFuncs::MenuMultiCourseCons = 0; //0x00497720;
    u32 MenuPageHandler::GameFuncs::MenuWifiCourseCons = 0; //0x004921DC;
    u32 MenuPageHandler::GameFuncs::MenuMultiCourseVoteCons = 0; //0x004BCDB4;
    u32 MenuPageHandler::GameFuncs::MenuWifiCourseVoteCons = 0; //0x004B8C68;
    u32 MenuPageHandler::GameFuncs::setupOmakaseView = 0; //0x005DB094;
    u32 MenuPageHandler::GameFuncs::setupCourseName = 0; //0x005DB3C0;
    u32 MenuPageHandler::GameFuncs::setupCourseButtonDummy = 0; //0x005DB518;
    u32 MenuPageHandler::GameFuncs::setupRaceDialogButton = 0; //0x005DC9B8;
    u32 MenuPageHandler::GameFuncs::setupBackButtonBothControl = 0; //0x005DC080;
    u32 MenuPageHandler::GameFuncs::baseMenuButtonControlSetCursor = 0; //0x00173254;
    u32 MenuPageHandler::GameFuncs::cursorMoveSetType = 0; //0x00143360;
    u32 MenuPageHandler::GameFuncs::cupSelectBGVtable = 0; //0x61955C;
    u32 MenuPageHandler::GameFuncs::setDelayH = 0; //0x00481A6C;
    u32 MenuPageHandler::GameFuncs::cupButtonVtable = 0; //0x61EAB4;
    u32 MenuPageHandler::GameFuncs::cupCursorAnimVtable = 0; //0x61EB40;
    u32 MenuPageHandler::GameFuncs::cupCursorVtable = 0; //0x61EB64;
    u32 MenuPageHandler::GameFuncs::setupOKButton2 = 0; //0x005DBD24;
    u32 MenuPageHandler::GameFuncs::moflexUpdateCup = 0; //0x004A3E60;
    u32 MenuPageHandler::GameFuncs::MoflexEnablePane = 0; //0x001843C0;
    u32 MenuPageHandler::GameFuncs::MoflexReset = 0; //0x004A3D74;
    u32 MenuPageHandler::GameFuncs::MenuCourseNameSet = 0; //0x00158274;
    u32 MenuPageHandler::GameFuncs::UIManipulatorSetCursor = 0; //0x0014C314;
    u32 MenuPageHandler::GameFuncs::AnimationItemGetCurrentFrame = 0; //0x0015B004;
    u32 MenuPageHandler::GameFuncs::moflexGetSelectedCupCourse = 0; //0x004EA260;
    u32 MenuPageHandler::GameFuncs::buttonKeyHandlerCommon = 0; // 0x00170F5C;
    u32 MenuPageHandler::GameFuncs::controlSliderStepH = 0; //0x00481040;
    u32 MenuPageHandler::GameFuncs::controlSliderStepV = 0; //0x00481474;
    u32 MenuPageHandler::GameFuncs::basePageCalcNormalControl = 0; //0x004D3B88;
    u32 MenuPageHandler::GameFuncs::MoflexInit = 0; //0x004A43F0;
    u32 MenuPageHandler::GameFuncs::MoflexUpdateFrame = 0; //0x0017DC74;
    u32 MenuPageHandler::GameFuncs::controlSliderStartH = 0; //0x004815FC;
    u32 MenuPageHandler::GameFuncs::controlSliderStartV = 0; //0x00481868;
    u32 MenuPageHandler::GameFuncs::garageDirectorFade = 0; //0x003F9898;
    u32 MenuPageHandler::GameFuncs::SequenceStartFadeout = 0; //0x00480430;
    u32 MenuPageHandler::GameFuncs::SequenceReserveFadeIn = 0; //0x004841FC;
    u32 MenuPageHandler::GameFuncs::moflexUpdateCourse = 0; // 0x4A41C8;
    u32 MenuPageHandler::GameFuncs::uiMovieViewAnimOut = 0; //0x001845B0;
    u32 MenuPageHandler::GameFuncs::omakaseViewVtable = 0; //0x619A48;
    u32 MenuPageHandler::GameFuncs::BaseMenuButtonControlCons = 0; //0x00173284;
    u32 MenuPageHandler::GameFuncs::creditsRollStaffTextVtable = 0; // 0x0061F098
    u32 MenuPageHandler::GameFuncs::BasicSoundStartPrepared = 0; // 0x002700C8
    u32 MenuPageHandler::GameFuncs::UItstDemoButton = 0; // 0x00157A68
    u32 MenuPageHandler::GameFuncs::BasePageCons = 0; // 0x004D4F70
    u32 MenuPageHandler::GameFuncs::EndingPageVtable = 0; // 0x645428
    
    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleModePage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x2E8);
        section = MenuSingleModePageCons(section);
        u32* sectionU32 = (u32*)section;
        sectionU32[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleModePage::MenuSingleModePageCons(MenuPageHandler::GameSequenceSection* own) {
        ((GameSequenceSection*(*)(GameSequenceSection*))GameFuncs::movieBasePageCons)(own); // Base Page constructor
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::SingleModeVtable, sizeof(vtable));
        ((u32*)own)[0] = (u32)&vtable;
        own->GetButtonArray(0x2C0).SetBuffer(sizeof(buttonList) / sizeof(VisualControl::GameVisualControl*), buttonList);
        ((u32*)own)[0x290/4] = 2;
        ((u32*)own)[0x2BC/4] = 0;
        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuSingleModeDeallocate;
        vtable.initControl = InitControl;
        vtable.onPageEnter = OnPageEnter;
        vtable.buttonHandler_SelectOn = OnSelect;
        buttonOKBackup = vtable.buttonHandler_OK;
        vtable.buttonHandler_OK = OnOK;
        vtable.userData = this;
        gameSection = own;
        return own;
    }

    int MenuPageHandler::MenuSingleModePage::HandleCursor(CursorMove* move, CursorItem* item) {
        CursorMove::KeyType k = move->GetDir(item);
        const u8 moveCursorTable[6][4] = { // Down, Right, Up, Left
            {4, 1, 4, 3},
            {4, 2, 4, 0},
            {5, 3, 5, 1},
            {5, 0, 5, 2},
            {1, 5, 1, 5},
            {2, 4, 2, 4}
        };
        k = (CursorMove::KeyType)(k & 0xF);
        if (!k) return -1;
        int ret = item->elementID;
        while (k) {
            if (k & CursorMove::KeyType::KEY_UP) {
                ret = moveCursorTable[ret][0];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_UP);
            } else if (k & CursorMove::KeyType::KEY_RIGHT) {
                ret = moveCursorTable[ret][1];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_RIGHT);
            } else if (k & CursorMove::KeyType::KEY_DOWN) {
                ret = moveCursorTable[ret][2];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_DOWN);
            } else if (k & CursorMove::KeyType::KEY_LEFT) {
                ret = moveCursorTable[ret][3];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_LEFT);
            }
        }
        return ret;
    }

    static void SetCaption(VisualControl::GameVisualControl* button, VisualControl::GameVisualControl* caption, u32 messageID) {
        ((u32*)button)[0x218/4] = (u32)caption;
        ((u32*)button)[0x21C/4] = messageID;
    }

    static void GarageRequestChangeState(u32 garageDirector, u32 state, u32 state2) {
        ((u8*)garageDirector)[0x66] = state;
        ((u8*)garageDirector)[0x67] = state2;
    }

    static void courseButtonDmySelectOn(VisualControl::GameVisualControl* control) {
        control->GetAnimationFamily(2)->SetAnimation(0, 1.f);
    }

    static void courseButtonDmySelectOff(VisualControl::GameVisualControl* control) {
        control->GetAnimationFamily(2)->SetAnimation(0, 0.f);
    }
    
    void MenuPageHandler::MenuSingleModePage::OnPageEnter(GameSequenceSection* own) {
        u32* ownU32 = (u32*)own;
        u32 selectedEntry = ((u32***)own)[0x88/4][0][0x100/4];
        
        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2E4/4];
        u32 movieViewTextElementID = ((u32*)movieView)[0x7C/4];
        const u32 messageIDs[] = {2200, 2201, 2204, CustomTextEntries::mission, 2202, 2203};
        const u16* textStr = Language::MsbtHandler::GetText(messageIDs[selectedEntry]);
        movieView->GetNwlytControl()->vtable->replaceMessageImpl(movieView->GetNwlytControl(), movieViewTextElementID, VisualControl::Message(textStr), nullptr, nullptr);
        
        ((void(*)(u32))GameFuncs::MenuCaptionAnimIn)(ownU32[0x2DC/4]);

        if (ownU32[0x48/4] != 1)
            ((void(*)(u32, u32, u32))GameFuncs::StartFadeIn)(0, 0x1E, 2);

        ((void(*)(u32))GameFuncs::SetPlayerMasterID)(0);
        ((void(*)(u32))GameFuncs::SetCameraTargetPlayer)(0);

        if (MissionHandler::isMissionMode)
            GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 6, 1);
        else
            GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 3, ownU32[0x48/4] != 1 ? 1 : 0);

        VersusHandler::OnMenuSingleEnterCallback();
    }

    void MenuPageHandler::MenuSingleModePage::OnSelect(GameSequenceSection* own, int selected) {
        u32* ownU32 = (u32*)own;

        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2E4/4];
        const u32 messageIDs[] = {2200, 2201, 2202, 2203, 2204, CustomTextEntries::mission};

        movieView->GetAnimationFamily(1)->ChangeAnimation(1, 0.f);
        ((u32*)movieView)[0xA8/4] = messageIDs[selected];
    }

    void  MenuPageHandler::MenuSingleModePage::OnOK(GameSequenceSection* own, int selected) {
        MenuSingleModePage* page = (MenuSingleModePage*)own->vtable->userData;
        u32 selectedEntry = ((u32***)own)[0x88/4][0][0x100/4];

        if (!g_isFoolActive)
            VersusHandler::OnMenuSingleOKCallback(selectedEntry);

        page->buttonOKBackup(own, selected);
    }

    void MenuPageHandler::MenuSingleModePage::InitControl(GameSequenceSection* own) {

        u32* ownU32 = (u32*)own;
        VisualControl::GameVisualControl*(*setupMenuButton)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::SetupControlMenuButton;
        VisualControl::GameVisualControl*(*setupBackButton)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::SetupControlBackButton;
        void (*setButtonTex)(VisualControl::GameVisualControl*, int, int) = (void(*)(VisualControl::GameVisualControl*, int, int))GameFuncs::BaseMenuButtonControlSetTex;

        ((void(*)(GameSequenceSection*, int))GameFuncs::BaseMenuPageInitSlider)(own, 1);

        VisualControl::GameVisualControl* movieView = ((VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&, u32))GameFuncs::SetupControlMovieView)
                    (own, SafeStringBase("mode_movie"), SafeStringBase("mode_movie"), 8);
        ownU32[0x2E4/4] = (u32)movieView;
        ((void(*)(VisualControl::GameVisualControl*, const SafeStringBase&))GameFuncs::MovieViewSetMoviePane)(movieView, SafeStringBase("P_movie_dammy"));
        
        {
            u32 frameElHandle = movieView->GetNwlytControl()->vtable->getElementHandle(movieView->GetNwlytControl(), SafeStringBase("W_frame"), 0);
            u32 dammyElHandle = movieView->GetNwlytControl()->vtable->getElementHandle(movieView->GetNwlytControl(), SafeStringBase("P_movie_dammy"), 0);
            movieView->GetNwlytControl()->vtable->setVisibleImpl(movieView->GetNwlytControl(), frameElHandle, false);
            movieView->GetNwlytControl()->vtable->setVisibleImpl(movieView->GetNwlytControl(), dammyElHandle, false);
        }

        IsButtonConstructing = true;
        VisualControl::GameVisualControl* gpButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_00"));
        own->GetButtonArray(0x2C0).Push(gpButton);

        VisualControl::GameVisualControl* ttButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_01"));
        own->GetButtonArray(0x2C0).Push(ttButton);

        VisualControl::GameVisualControl* vsButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_02"));
        own->GetButtonArray(0x2C0).Push(vsButton);

        VisualControl::GameVisualControl* msButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_03"));
        own->GetButtonArray(0x2C0).Push(msButton);

        VisualControl::GameVisualControl* bbButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_11"));
        own->GetButtonArray(0x2C0).Push(bbButton);

        VisualControl::GameVisualControl* cbButton = setupMenuButton(own, SafeStringBase("chara_select_btn"), SafeStringBase("chara_select_btn_8_12"));
        own->GetButtonArray(0x2C0).Push(cbButton);
        IsButtonConstructing = false;

        VisualControl::GameVisualControl* caption = VisualControl::Build(ownU32, "caption", "caption", (VisualControl::AnimationDefineVtable*)GameFuncs::CaptionAnimationDefineVtable, (VisualControl::GameVisualControlVtable*)GameFuncs::BaseMenuViewVtable, VisualControl::ControlType::BASEMENUVIEW_CONTROL);
        ownU32[0x2DC/4] = (u32)caption;

        ((u32*)gpButton)[0x230/4] = 0;
        ((u32*)ttButton)[0x230/4] = !g_isFoolActive ? 1 : 0;
        ((u32*)vsButton)[0x230/4] = 0;
        ((u32*)msButton)[0x230/4] = 0;
        ((u32*)bbButton)[0x230/4] = !g_isFoolActive ? 2 : 0;
        ((u32*)cbButton)[0x230/4] = !g_isFoolActive ? 3 : 0;

        ((u32*)gpButton)[0x210/4] = 0;
        ((u32*)ttButton)[0x210/4] = !g_isFoolActive ? 1 : 0;
        ((u32*)vsButton)[0x210/4] = 0;
        ((u32*)msButton)[0x210/4] = 0;
        ((u32*)bbButton)[0x210/4] = !g_isFoolActive ? 2 : 0;
        ((u32*)cbButton)[0x210/4] = !g_isFoolActive ? 3 : 0;

        ((u32*)gpButton)[0x214/4] = 0;
        ((u32*)ttButton)[0x214/4] = 1;
        ((u32*)vsButton)[0x214/4] = 4;
        ((u32*)msButton)[0x214/4] = 5;
        ((u32*)bbButton)[0x214/4] = 2;
        ((u32*)cbButton)[0x214/4] = 3;

        setButtonTex(gpButton, 0, 2);
        setButtonTex(ttButton, 1, 2);
        setButtonTex(vsButton, 2, 2);
        setButtonTex(msButton, 3, 2);
        setButtonTex(bbButton, 4, 2);
        setButtonTex(cbButton, 5, 2);

        ((u8*)gpButton)[0x224] = 65;
        ((u8*)ttButton)[0x224] = 65;
        ((u8*)vsButton)[0x224] = 65;
        ((u8*)msButton)[0x224] = 65;
        ((u8*)bbButton)[0x224] = 65;
        ((u8*)cbButton)[0x224] = 65;
        
        SetCaption(gpButton, caption, 2210);
        SetCaption(ttButton, caption, 2211);
        SetCaption(bbButton, caption, 2212);
        SetCaption(cbButton, caption, 2213);
        SetCaption(vsButton, caption, 2214);
        SetCaption(msButton, caption, CustomTextEntries::missionDesc);

        SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
        for (int i = 0; i < 6; i++)
            ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[0], own->GetButtonArray(0x2C0)[i]);

        u8* defaultSlider = (u8*)controlSliderArray[0];
        defaultSlider[0x1864] = 61;
        defaultSlider[0x1865] = 62;

        if (ownU32[0x284/4] & 1) {
            VisualControl::GameVisualControl* backButton = setupBackButton(own, SafeStringBase("cmn_back_btn"), SafeStringBase("cmn_back_btn"));
            ownU32[0x2E0/4] = (u32)backButton;
            ((u32*)backButton)[0x230/4] = ownU32[0x5C/4];
            ((u8*)backButton)[0x224] = 9;
        }

        ((u8*)(((u32**)ownU32)[0x88/4][0]))[0x118] = 1;

        own->GetControlMove()->keyHandler = HandleCursor;
    }

    bool MenuPageHandler::MenuSingleModePage::DefineAnimation(VisualControl::AnimationDefine* animDefine) {
        if (!IsButtonConstructing) return false;
        animDefine->InitAnimationFamilyList(5);

        ((void(*)(VisualControl::AnimationDefine*))GameFuncs::initAnimationTouchSelect)(animDefine);

        animDefine->InitAnimationFamily(2, "G_chara", 1);
        animDefine->InitAnimation(0, "mode_texptn", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        /*animDefine->InitAnimationFamily(3, "G_chara", 2);
        animDefine->InitAnimation(0, "icon_loop-00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        animDefine->InitAnimationStopByRate(1, "icon_loop-00", 0.f);*/
        animDefine->InitAnimationFamily(3, "G_loop", 2);
        animDefine->InitAnimation(0, "loop", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        animDefine->InitAnimationStopByRate(1, "loop", 0.f);

        animDefine->InitAnimationFamily(4, "G_loop", 2);
        animDefine->InitAnimation(0, "loop", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        animDefine->InitAnimationStopByRate(1, "loop", 0.f);

        return true;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCupBasePage::MenuSingleCupBasePageCons(GameSequenceSection* own) {
        void*(*MenuMoviePageBaseCons)(GameSequenceSection* own) = (decltype(MenuMoviePageBaseCons))GameFuncs::movieBasePageCons;
        MenuMoviePageBaseCons(own);
        own->GetButtonArray(0x2E0).SetBuffer(sizeof(buttonList) / sizeof(VisualControl::GameVisualControl*), buttonList);
        ((u32*)own)[0x290/4] = 6;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCupGPPage::MenuSingleCupGPPageCons(GameSequenceSection* own) {
        own = MenuSingleCupBasePage::MenuSingleCupBasePageCons(own);
        gameSection = own;
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::menuSingleCupGPVtable, sizeof(vtable));
        own->vtable = &vtable;
        ((u32*)own)[0x314/4] = -1;
        ((u8*)own)[0x318/1] = 0;
        void(*SequenceBasePageInitMenu)(GameSequenceSection*, int) = (decltype(SequenceBasePageInitMenu))GameFuncs::SequenceBasePageInitMenu;
        SequenceBasePageInitMenu(own, 1);
        ((u8*)own)[0x2BC/1] = 0;

        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuSingleCupGPDeallocate;
        vtable.initControl = MenuSingleCupBasePage::InitControl;
        vtable.onPageEnter = MenuSingleCupBasePage::OnPageEnter;
        vtable.enterCursor = MenuSingleCupBasePage::EnterCursor;
        vtable.onPagePreStep = MenuSingleCupGPPage::OnPagePreStep;
        vtable.buttonHandler_SelectOn = (decltype(vtable.buttonHandler_SelectOn))VisualControl::nullFunc;
        vtable.buttonHandler_OK = MenuSingleCupGPPage::ButtonHandler_OK;
        vtable.onPageComplete = MenuSingleCupGPPage::OnPageComplete;
        vtable.onPageExit = [](GameSequenceSection* own) { MenuSingleCupBasePage::OnPageExit(own, false);};
        vtable.userData = this;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCupGPPage::MenuMultiCupGPPageCons(GameSequenceSection* own) {
        own = MenuSingleCupBasePage::MenuSingleCupBasePageCons(own);
        gameSection = own;
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::menuMultiCupGPVtable, sizeof(vtable));
        own->vtable = &vtable;
        ((u32*)own)[0x314/4] = -1;
        ((u8*)own)[0x318/1] = 0;
        void(*SequenceBasePageInitMenu)(GameSequenceSection*, int) = (decltype(SequenceBasePageInitMenu))GameFuncs::SequenceBasePageInitMenu;
        SequenceBasePageInitMenu(own, 1);

        ((u8*)own)[0x2BC/1] = 0;
        ((u8*)own)[0x319/1] = 0;
        ((u32*)own)[0x31C/4] = 0;
        ((u32*)own)[0x32C/4] = 0;
        ((u32*)own)[0x324/4] = -1;
        ((u8*)own)[0x320/1] = 1;
        ((u8*)own)[0x284/1] &= ~1;
        ((u32*)own)[0x290/4] = 0x14;

        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuMultiCupGPDeallocate;
        vtable.enterCursor = MenuSingleCupBasePage::EnterCursor;
        vtable.buttonHandler_SelectOn = (decltype(vtable.buttonHandler_SelectOn))VisualControl::nullFunc;
        OnPageCompleteBackup = vtable.onPageComplete;
        vtable.onPageComplete = OnPageComplete;
        vtable.onPageExit = [](GameSequenceSection* own) { MenuSingleCupBasePage::OnPageExit(own, false);};
        vtable.userData = this;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCupPage::MenuSingleCupPageCons(GameSequenceSection* own) {
        own = MenuSingleCupBasePage::MenuSingleCupBasePageCons(own);
        gameSection = own;
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::menuSingleCupVtable, sizeof(vtable));
        own->vtable = &vtable;
        ((u8*)own)[0x2BC/1] = 1;

        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuSingleCupDeallocate;
        vtable.initControl = MenuSingleCupBasePage::InitControl;
        vtable.enterCursor = MenuSingleCupBasePage::EnterCursor;
        vtable.onPagePreStep = MenuSingleCupBasePage::OnPagePreStep;
        vtable.buttonHandler_SelectOn = (decltype(vtable.buttonHandler_SelectOn))VisualControl::nullFunc;
        vtable.buttonHandler_OK = MenuSingleCupBasePage::ButtonHandler_OK;
        vtable.onPageExit = [](GameSequenceSection* own) { MenuSingleCupBasePage::OnPageExit(own, true);};
        vtable.userData = this;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCupPage::MenuMultiCupPageCons(GameSequenceSection* own) {
        own = MenuSingleCupBasePage::MenuSingleCupBasePageCons(own);
        gameSection = own;
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::menuMultiCupVtable, sizeof(vtable));
        own->vtable = &vtable;
        ((u8*)own)[0x2BC/1] = 2;
        ((u8*)own)[0x284] = (((u8*)own)[0x284] & ~1) | 0x20;
        ((u32*)own)[0x290/4] = 0x14;

        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuMultiCupDeallocate;
        vtable.initControl = MenuMultiCupPage::InitControl;
        vtable.enterCursor = MenuSingleCupBasePage::EnterCursor;
        onPageEnterBackup = vtable.onPageEnter;
        vtable.onPageEnter = MenuMultiCupPage::OnPageEnter;
        vtable.buttonHandler_SelectOn = (decltype(vtable.buttonHandler_SelectOn))VisualControl::nullFunc;
        vtable.onPageExit = [](GameSequenceSection* own) { MenuSingleCupBasePage::OnPageExit(own, true);};
        vtable.userData = this;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuWifiCupPage::MenuWifiCupPageCons(GameSequenceSection* own) {
        own = MenuSingleCupBasePage::MenuSingleCupBasePageCons(own);
        gameSection = own;
        memcpy(&vtable, (GameSequenceSectionVtable*)GameFuncs::MenuWifiCupVtable, sizeof(vtable));
        own->vtable = &vtable;
        ((u8*)own)[0x2BC/1] = 2;
        ((u8*)own)[0x284] = (((u8*)own)[0x284] & ~0x21) | 0x40;
        ((u32*)own)[0x290/4] = 0x21;

        deallocatingBackup = vtable._deallocating;
        vtable._deallocating = OnMenuWifiCupDeallocate;
        vtable.initControl = MenuMultiCupPage::InitControl;
        vtable.enterCursor = MenuSingleCupBasePage::EnterCursor;
        vtable.onPageEnter = MenuWifiCupPage::OnPageEnter;
        vtable.buttonHandler_SelectOn = (decltype(vtable.buttonHandler_SelectOn))VisualControl::nullFunc;
        vtable.onPageExit = [](GameSequenceSection* own) { MenuSingleCupBasePage::OnPageExit(own, true);};
        vtable.userData = this;
        return own;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCupGPPage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x31C);
        section = MenuSingleCupGPPageCons(section);
        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCupGPPage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x334);
        section = MenuMultiCupGPPageCons(section);
        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCupPage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x314);
        section = MenuSingleCupPageCons(section);
        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCupPage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x31C);
        section = MenuMultiCupPageCons(section);
        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuWifiCupPage::Load(void* sectionDirector) {
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x31C);
        section = MenuWifiCupPageCons(section);
        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuSingleCoursePage::Load(void* sectionDirector) {
        GameSequenceSection*(*MenuSingleCourseCons)(GameSequenceSection* own) = (decltype(MenuSingleCourseCons))GameFuncs::MenuSingleCourseCons;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x360);
        section = MenuSingleCourseCons(section);

        gameSection = section;
        memcpy(&vtable, section->vtable, sizeof(vtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuSingleCourseDeallocate;
        buttonHandler_SelectOnbackup = section->vtable->buttonHandler_SelectOn;
        section->vtable->buttonHandler_SelectOn = buttonHandler_SelectOn;
        section->vtable->onPageExit = OnPageExit;

        parent = (MenuSingleCupBasePage**)&menuSingleCup;

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCoursePage::Load(void* sectionDirector) {
        GameSequenceSection*(*MenuMultiCourseCons)(GameSequenceSection* own) = (decltype(MenuMultiCourseCons))GameFuncs::MenuMultiCourseCons;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x344);
        section = MenuMultiCourseCons(section);

        gameSection = section;
        memcpy(&vtable, section->vtable, sizeof(vtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuMultiCourseDeallocate;
        buttonHandler_SelectOnbackup = section->vtable->buttonHandler_SelectOn;
        section->vtable->buttonHandler_SelectOn = MenuSingleCoursePage::buttonHandler_SelectOn;
        section->vtable->onPageExit = MenuSingleCoursePage::OnPageExit;

        parent = (MenuSingleCupBasePage**)&menuMultiCup;

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuWifiCoursePage::Load(void* sectionDirector) {
        GameSequenceSection*(*MenuWifiCourseCons)(GameSequenceSection* own) = (decltype(MenuWifiCourseCons))GameFuncs::MenuWifiCourseCons;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x344);
        section = MenuWifiCourseCons(section);

        gameSection = section;
        memcpy(&vtable, section->vtable, sizeof(vtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuWifiCourseDeallocate;
        buttonHandler_SelectOnbackup = section->vtable->buttonHandler_SelectOn;
        section->vtable->buttonHandler_SelectOn = MenuSingleCoursePage::buttonHandler_SelectOn;
        section->vtable->onPageExit = MenuSingleCoursePage::OnPageExit;

        parent = (MenuSingleCupBasePage**)&menuWifiCup;

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuMultiCourseVote::Load(void* sectionDirector) {
        GameSequenceSection*(*MenuMultiCourseVoteCons)(GameSequenceSection* own) = (decltype(MenuMultiCourseVoteCons))GameFuncs::MenuMultiCourseVoteCons;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x308);
        section = MenuMultiCourseVoteCons(section);

        gameSection = section;
        memcpy(&vtable, section->vtable, sizeof(vtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuMultiCourseVoteDeallocate;

        initControlBackup = section->vtable->initControl;
        section->vtable->initControl = InitControl;
        onPageExitBackup = section->vtable->onPageExit;
        section->vtable->onPageExit = OnPageExit;

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuWifiCourseVote::Load(void* sectionDirector) {
        GameSequenceSection*(*MenuWifiCourseVoteCons)(GameSequenceSection* own) = (decltype(MenuWifiCourseVoteCons))GameFuncs::MenuWifiCourseVoteCons;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x308);
        section = MenuWifiCourseVoteCons(section);

        gameSection = section;
        memcpy(&vtable, section->vtable, sizeof(vtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuWifiCourseVoteDeallocate;

        initControlBackup = section->vtable->initControl;
        section->vtable->initControl = InitControl;
        onPageExitBackup = section->vtable->onPageExit;
        section->vtable->onPageExit = OnPageExit;

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::MenuEndingPage::Load(void* sectionDirector) {
        GameSequenceSection*(*BasePageCons)(GameSequenceSection* own) = (decltype(BasePageCons))MenuPageHandler::GameFuncs::BasePageCons;
        GameSequenceSectionVtable* EndingPagevtable = (decltype(EndingPagevtable))MenuPageHandler::GameFuncs::EndingPageVtable;
        GameSequenceSection* section = (GameSequenceSection*)GameAlloc::game_operator_new_autoheap(0x2B4);
        memset(section, 0, 0x2B4);
        section = BasePageCons(section);

        gameSection = section;
        memcpy(&vtable, EndingPagevtable, sizeof(GameSequenceSectionVtable));
        section->vtable = &vtable;

        section->vtable->userData = this;
        deallocatingBackup = section->vtable->_deallocating;
        section->vtable->_deallocating = OnMenuEndingPageDeallocate;

        if (loadCTGPCredits) {
            section->vtable->initControl = InitControl;
            section->vtable->onPageEnter = onPageEnter;
            section->vtable->onPagePreStep = onPagePreStep;
            section->vtable->onPageComplete = onPageComplete;
            section->vtable->onPageExit = onPageExit;
        }

        ((u32*)section)[0x4/4] = (u32)sectionDirector;
        u32* someArrCount = ((u32*)sectionDirector) + 0x24/4;
        u32* someArrData = *(((u32**)sectionDirector) + 0x1C/4);
        if (someArrData[*someArrCount] == 0) {
            someArrData[*someArrCount] = (u32)section;
            (*someArrCount)++;
        }
        return section;
    }

    void MenuPageHandler::MenuSingleCupBasePage::CupSelectBGControlAnimationDefine(VisualControl::AnimationDefine* animDefine) {
        animDefine->InitAnimationFamilyList(1);
        
        animDefine->InitAnimationFamily(0, "G_conv", 1);
        animDefine->InitAnimation(0, "conv_move", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    void MenuPageHandler::MenuSingleCupBasePage::SetCupButtonVisible(int buttonID, bool isVisible) {
        if (buttonID >= 0 && buttonID < 10) {
            VisualControl::NwlytControlSight* buttonNwlyt = buttonList[buttonID]->GetNwlytControl(); 
            buttonNwlyt->vtable->setVisibleImpl(buttonNwlyt, (u32)buttonNwlyt->vtable->getRootPane(buttonNwlyt), isVisible);
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::SetCupButtonBG(int buttonID, u32 paneID) {
        if (buttonID < 0 || buttonID > 9)
            return;
        std::string paneName;
        if  (paneID < 8) {
            paneName = Utils::Format("N_btn-0%d", paneID).c_str();
        } else if (paneID == 9) {
            paneName = "N_btn-TL";
        } else if (paneID == 10) {
            paneName = "N_btn-TR";
        } else if (paneID == 11) {
            paneName = "N_btn-BL";
        } else if (paneID == 12) {
            paneName = "N_btn-BR";
        } else {
            return;
        }
        
        SafeStringBase paneNameStr(paneName.c_str());
        VisualControl::NwlytControlSight* cupBGControlSight = cupBgControl->GetNwlytControl();
        VisualControl::GameVisualControl* button = buttonList[buttonID];
        ((u32*)button)[0x84/4] = (u32)cupBGControlSight;
        u32 paneHandle = cupBGControlSight->vtable->getConstElementHandle(cupBGControlSight, paneNameStr, 0);
        ((u32*)button)[0x8C/4] = paneHandle;
    }

    void MenuPageHandler::MenuSingleCupBasePage::SetButtonEnabledState(int button, bool state) {
		if (button >= 0 && button < 10) {
            buttonEnabledState[button] = state;
            if (conveyorState == ConveyorState::STOPPED)
                UpdateButtonEnabledState();
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::UpdateButtonEnabledState() {
        for (int button = 0; button < 10; button++) {
            bool state = buttonEnabledState[button] && conveyorState == ConveyorState::STOPPED;
            u32* buttons = (u32*)(((MenuPageHandler::GameSequenceSection*)gameSection)->GetButtonArray(0x2E0).data);
            u32 enabledMode = ((u8*)gameSection)[0x2BC] ? 0 : -2;
            ((u32*)buttons[button])[0x230 / 4] = state ? enabledMode : -1;
            ((u8*)buttons[button])[0x224] = state ? 90 : 0;
            ((u8*)buttons[button])[0x225] = !state ? 91 : 0;
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::UpdateCupButtonState(int mode) {
        u32 size;
		const u32* cupTransTable = CourseManager::getCupTranslatetable(&size);
        if (startingButtonID >= (size / 2)) {
            startingButtonID = 0;
        }
        bool isAllRandom = (VersusHandler::IsVersusMode && VersusHandler::IsRandomCourseMode()) || (g_getCTModeVal == CTMode::ONLINE_COM && CourseManager::isRandomTracksForcedComm);
        if (mode == 0) {
            for (int i = 0; i < 8; i++) {
                VisualControl::GameVisualControl* currCupButton = buttonList[i];
                u32 finalCup;
                if (i < 4) {
                    finalCup = cupTransTable[(startingButtonID + i) % (size / 2)];
                } else {
                    finalCup = cupTransTable[(startingButtonID + i - 4) % (size / 2) + (size / 2)];
                }
                ((u32*)currCupButton)[0x214/4] = isAllRandom ? VERSUSCUPID : finalCup;
                CourseManager::BaseMenuButtonControl_setTex((u32)currCupButton, (finalCup == USERCUPID || isAllRandom) ? 8 : finalCup, 2);
                ((u32*)currCupButton)[0x210/4] = isAllRandom ? VERSUSCUPID : finalCup;
            }
        }
        if (mode == 1 || mode == 2) { // Right scroll and left scroll
            int start = startingButtonID;
            if (mode == 1 && startingButtonID == 0) // Edge case to prevent negative module
                start = (size / 2);
            u32 top = cupTransTable[(start + ((mode == 1) ? -1 : 4)) % (size / 2)];
            u32 bot = cupTransTable[(start + ((mode == 1) ? -1 : 4)) % (size / 2) + (size / 2)];
            CourseManager::BaseMenuButtonControl_setTex((u32)buttonList[8], (top == USERCUPID || isAllRandom) ? 8 : top, 2);
            CourseManager::BaseMenuButtonControl_setTex((u32)buttonList[9], (bot == USERCUPID || isAllRandom) ? 8 : bot, 2);
            ((u32*)buttonList[8])[0x214/4] = isAllRandom ? VERSUSCUPID : top;
            ((u32*)buttonList[9])[0x214/4] = isAllRandom ? VERSUSCUPID : bot;
            ((u32*)buttonList[8])[0x210/4] = isAllRandom ? VERSUSCUPID : top;
            ((u32*)buttonList[9])[0x210/4] = isAllRandom ? VERSUSCUPID : bot;
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::InitControl(GameSequenceSection* own) {
        u32* ownU32 = (u32*)own;
        MenuSingleCupBasePage* obj = (MenuSingleCupBasePage*)own->vtable->userData;
        VisualControl::GameVisualControl*(*setupOmakaseView)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::setupOmakaseView;
        VisualControl::GameVisualControl*(*setupCourseName)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::setupCourseName;
        VisualControl::GameVisualControl*(*setupCourseButtonDummy)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::setupCourseButtonDummy;
        VisualControl::GameVisualControl*(*setupRaceDialogButton)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&, int) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&, int))GameFuncs::setupRaceDialogButton;
        VisualControl::GameVisualControl*(*setupBackButton)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::SetupControlBackButton;
        VisualControl::GameVisualControl*(*setupBackButtonBothControl)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::setupBackButtonBothControl;
        
        void(*baseMenuButtonControlSetCursor)(VisualControl::GameVisualControl*, VisualControl::GameVisualControl*, const SafeStringBase&) =
            (void(*)(VisualControl::GameVisualControl*, VisualControl::GameVisualControl*, const SafeStringBase&))GameFuncs::baseMenuButtonControlSetCursor;
        void(*cursorMoveSetType)(CursorMove*, int) =
            (void(*)(CursorMove*, int))GameFuncs::cursorMoveSetType;
        

        ((void(*)(GameSequenceSection*, int))GameFuncs::BaseMenuPageInitSlider)(own, 3);
        SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
        u8* defaultSlider = (u8*)controlSliderArray[0];
        defaultSlider[0x1864] = 61;
        defaultSlider[0x1865] = 62;
        
        VisualControl::GameVisualControl* movieView = ((VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&, u32))GameFuncs::SetupControlMovieView)
                    (own, SafeStringBase("course_movie"), SafeStringBase("course_movie_T"), 8);
        ownU32[0x2C0/4] = (u32)movieView;
        ((void(*)(VisualControl::GameVisualControl*, const SafeStringBase&))GameFuncs::MovieViewSetMoviePane)(movieView, SafeStringBase("P_movie_dammy"));

        VisualControl::GameVisualControl* omakaseView = obj->ctPreview.GetControl(own);
        ownU32[0x2DC/4] = (u32)omakaseView;
        void* movieRootPane = movieView->vtable->getRootPane(movieView);
        ((u32*)omakaseView)[0xA8/4] = (u32)movieRootPane;
        void* omakaseViewRootPane = omakaseView->vtable->getRootPane(omakaseView);
        Vector3* movieRootPanePos = (Vector3*)((u32)movieRootPane + 0x28);
        Vector3* omakaseViewRootPanePos = (Vector3*)((u32)omakaseViewRootPane + 0x28);
        ((float*)omakaseView)[0xAC/4] = omakaseViewRootPanePos->y - movieRootPanePos->y;

        VisualControl::GameVisualControl* courseName = setupCourseName(own, SafeStringBase("course_name"), SafeStringBase("course_name"));
        ownU32[0x2C4/4] = (u32)courseName;

        ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[2], courseName);
        ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[0], movieView);

        VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
        courseButtonDummies[0] = setupCourseButtonDummy(own, SafeStringBase("course_btn_dmy"), SafeStringBase("course_btn_dmy_T_0"));
        courseButtonDummies[1] = setupCourseButtonDummy(own, SafeStringBase("course_btn_dmy"), SafeStringBase("course_btn_dmy_T_1"));
        courseButtonDummies[2] = setupCourseButtonDummy(own, SafeStringBase("course_btn_dmy"), SafeStringBase("course_btn_dmy_T_2"));
        courseButtonDummies[3] = setupCourseButtonDummy(own, SafeStringBase("course_btn_dmy"), SafeStringBase("course_btn_dmy_T_3"));
        for (int i = 0; i < 4; i++)
            ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[0], courseButtonDummies[i]);

        bool* isCupLockedArray = (bool*)(ownU32 + 0x30C/4);
        for (int i = 0; i < 8; i++) {
            /* Check if cup is open
            bool isOpen = ((bool(*)(const int*))0x4D158C)(&i);
            */
            bool isOpen = true;
            isCupLockedArray[i] = !isOpen;
        }

        VisualControl::GameVisualControl* cupSelectBg = VisualControl::Build(ownU32, "cup_select_bg", "cup_select_bg", &cupSelectBGAnimationDefineVtable, (VisualControl::GameVisualControlVtable*)GameFuncs::cupSelectBGVtable, VisualControl::ControlType::CUP_SELECT_BG_CONTROL);
        ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[1], cupSelectBg);
        ((void(*)(void*, int))GameFuncs::setDelayH)(controlSliderArray[1], 0); //SetDelayH
        obj->cupBgControl = cupSelectBg;

        for (int i = 0; i < 10; i++) {
            VisualControl::GameVisualControl* currCupButton = VisualControl::Build(ownU32, "cup_btn", "cup_btn", 0, (VisualControl::GameVisualControlVtable*)GameFuncs::cupButtonVtable, VisualControl::ControlType::CUP_BTN_CONTROL);
            
            ((u8*)currCupButton)[0x224/1] = 90;
            ((u32*)currCupButton)[0x230/4] = ((u8*)own)[0x2BC/1] ? 0 : -2;
            
            own->GetButtonArray(0x2E0).Push(currCupButton);

            if (i < 8) {
                obj->SetCupButtonBG(i, i);
            } else {
                obj->SetCupButtonVisible(i, false);
            }
        }

        VisualControl::GameVisualControl* cupCursor = VisualControl::Build(ownU32, "cup_cursor", "cup_cursor", (VisualControl::AnimationDefineVtable*)GameFuncs::cupCursorAnimVtable, (VisualControl::GameVisualControlVtable*)GameFuncs::cupCursorVtable, VisualControl::ControlType::CUP_CURSOR_CONTROL);
        ownU32[0x2D8/4] = (u32)cupCursor;

        for (int i = 0; i < 10; i++) {
            baseMenuButtonControlSetCursor(own->GetButtonArray(0x2E0)[i], cupCursor, SafeStringBase("N_btn"));
        }

        if (!((u8*)own)[0x2BC/1]) {
            VisualControl::GameVisualControl* racestart = setupRaceDialogButton(own, SafeStringBase("race_startbtn"), SafeStringBase("race_startbtn"), 0);
            ((u32*)racestart)[0x210/4] = 9;
            ((u32*)racestart)[0x230/4] = 0;
            VisualControl::Message message;
            Language::MsbtHandler::GetMessageFromList(message, (Language::MsbtHandler::MessageDataList*)racestart->GetMessageDataList(), 2400);
            racestart->GetNwlytControl()->vtable->replaceMessageImpl(racestart->GetNwlytControl(), ((u32*)racestart)[0x94/4], message, nullptr, nullptr);
        }

        if (ownU32[0x284/4] & 1) {
            VisualControl::GameVisualControl* backButton;
            if (((u8*)own)[0x2BC/1]) {
                backButton = setupBackButton(own, SafeStringBase("cmn_back_btn"), SafeStringBase("cmn_back_btn"));
                ((u32*)backButton)[0x230/4] = ownU32[0x5C/4];
            } else {
                backButton = setupBackButtonBothControl(own, SafeStringBase("cmn_back_btn"), SafeStringBase("cmn_back_btn"));
                ((u32*)backButton)[0x210/4] = 8;
                ((u32*)backButton)[0x230/4] = ownU32[0x5C/4];
                ((u32*)backButton)[0x234/4] = -3;
            }
            obj->backButtonControl = backButton;
        }
        
        own->GetControlMove()->keyHandler = [](CursorMove* move, CursorItem* item) {return HandleCursor(move, item, false);};
        ((u32*)own->GetControlMove())[0x14/4] = (u32)obj; // Normally is number of cups per row, but used as argument passing
    }

    void MenuPageHandler::MenuMultiCupPage::InitControl(GameSequenceSection* own) {
        VisualControl::GameVisualControl*(*setupOKButton2)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&) = 
            (VisualControl::GameVisualControl*(*)(GameSequenceSection*, const SafeStringBase&, const SafeStringBase&))GameFuncs::setupOKButton2;
        
        MenuSingleCupBasePage::InitControl(own);
        u32* ownU32 = (u32*)own;
        MenuSingleCupBasePage* obj = (MenuSingleCupBasePage*)own->vtable->userData;

        VisualControl::GameVisualControl* randomButton = setupOKButton2(own, SafeStringBase("menu_ok_btn"), SafeStringBase("course_random_btn"));
        ownU32[0x314/4] = (u32)randomButton;
        obj->hasRandomButton = true;
        
        VisualControl::Message mOut;
        Language::MsbtHandler::GetMessageFromList(mOut, (Language::MsbtHandler::MessageDataList*)randomButton->GetMessageDataList(), 5014);
        randomButton->GetNwlytControl()->vtable->replaceMessageImpl(randomButton->GetNwlytControl(), ((u32*)randomButton)[0x23C/4], mOut, 0, 0);
    
        ((u32*)randomButton)[0x210/4] = 9;
        ((u32*)randomButton)[0x214/4] = 9;
        ((u32*)randomButton)[0x230/4] = 1;
        
        SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
        ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[1], randomButton);
        
        own->GetControlMove()->keyHandler = [](CursorMove* move, CursorItem* item) {return HandleCursor(move, item, true);};
        ((u32*)own->GetControlMove())[0x14/4] = (u32)obj; // Normally is number of cups per row, but used as argument passing
    }

    void MenuPageHandler::MenuSingleCupBasePage::OnPageEnter(GameSequenceSection* own) {
        u32* ownU32 = (u32*)own;
        void(*moflexUpdateCup)(GameSequenceSection*) = (decltype(moflexUpdateCup))GameFuncs::moflexUpdateCup;
        void(*MoflexEnablePane)(VisualControl::GameVisualControl*, bool enable) = (decltype(MoflexEnablePane))GameFuncs::MoflexEnablePane;
        void(*MoflexReset)(bool enable) = (decltype(MoflexReset))GameFuncs::MoflexReset;
        
        GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 0xB, ownU32[0x48/4] == 1 ? 0 : 1);
        MenuSingleCupBasePage* obj = (MenuSingleCupBasePage*)own->vtable->userData;
        obj->isInPage = true;
        obj->comesFromOK = false;
        obj->autoButtonPressed = false;
        obj->UpdateCupButtonState(0);
        obj->ctPreview.Load();
        if (!obj->ctPreviewChildTrack) obj->ctPreview.SetPreview(-1, 0, true);
        else obj->ctPreview.SetTarget(obj->ctPreviewChildAnim);
        if ((obj->selectedCupIcon == 10 && !obj->hasRandomButton) || obj->selectedCupIcon < 0) obj->selectedCupIcon = 0;
        u32 lastSelectedCup = obj->selectedCupIcon;  // Check it's not 0xA random button
        own->SetLastSelectedButton(obj->selectedCupIcon);
        moflexUpdateCup(own);
        
        if (obj->selectedCupIcon == 10) {
            bool isCupLocked = true;
            VisualControl::GameVisualControl* omakaseView = (VisualControl::GameVisualControl*)ownU32[0x2DC/4];
            u32 omakaseRootHandle = (u32)(omakaseView->vtable->getRootPane(omakaseView));
            omakaseView->GetNwlytControl()->vtable->setVisibleImpl(omakaseView->GetNwlytControl(), omakaseRootHandle, isCupLocked);
            VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2C0/4]; 
            MoflexEnablePane(movieView, !isCupLocked);
            MoflexReset(!isCupLocked);
            if (((u16*)own)[0x1E/2] != 1) {
                VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
                for (int i = 0; i < 4; i++) {
                    courseButtonDmySelectOff(courseButtonDummies[i]);
                }
            }
        } else {
            int cupID = ((u32*)(own->GetButtonArray(0x2E0)[lastSelectedCup]))[0x214/4];
            bool isCupLocked = cupID < 0 || cupID > 7 || MissionHandler::isMissionMode;
            VisualControl::GameVisualControl* omakaseView = (VisualControl::GameVisualControl*)ownU32[0x2DC/4];
            u32 omakaseRootHandle = (u32)(omakaseView->vtable->getRootPane(omakaseView));
            omakaseView->GetNwlytControl()->vtable->setVisibleImpl(omakaseView->GetNwlytControl(), omakaseRootHandle, isCupLocked);
            VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2C0/4]; 
            MoflexEnablePane(movieView, !isCupLocked);
            MoflexReset(!isCupLocked);
            changeCup(own, cupID & 0xFF);

            if (((u16*)own)[0x1E/2] != 1) {
                VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
                for (int i = 0; i < 4; i++) {
                    if (i == 0)
                        courseButtonDmySelectOn(courseButtonDummies[i]);
                    else
                        courseButtonDmySelectOff(courseButtonDummies[i]);
                }
            }
        }
        
        obj->prevSelectedButton = -1;
        obj->ctPreviewCurrTrack = 0;
        obj->ctPreviewFrameCounter = 0;

        // Other needed stuf
        ownU32[0x98/4] = 0;
        if (g_getCTModeVal == CTMode::ONLINE_COM && MarioKartFramework::allowCPURacersComm && !MarioKartFramework::israndomdriverchoicesshuffled)
			MarioKartFramework::getMyNetworkSelectMenuBuffer()[0x12] = Utils::Random();

        for (int i = 0; i < 8; i++) {
            obj->SetButtonEnabledState(i, true);
        }
    }

    void MenuPageHandler::MenuMultiCupPage::OnPageEnter(GameSequenceSection* own) {
        MenuMultiCupPage* obj = (MenuMultiCupPage*)own->vtable->userData;
        obj->onPageEnterBackup(own);
        MenuSingleCupBasePage::OnPageEnter(own);
    }

    void MenuPageHandler::MenuWifiCupPage::OnPageEnter(GameSequenceSection* own) {
        menuMultiCup->onPageEnterBackup(own);
        MenuSingleCupBasePage::OnPageEnter(own);
        ((u8*)own)[0x284/1] |= 0x40;
    }

    void MenuPageHandler::MenuSingleCupBasePage::EnterCursor(GameSequenceSection* own, int cursor) {
        if (cursor >= 0)
            return;
        u32** sequeceEngine = (u32**)MarioKartFramework::getSequenceEngine();
        u32 cupID = sequeceEngine[0xE8/4][0x10/4];
        if (cupID < 8) {
            /* Check if cup is open
            bool isOpen = ((bool(*)(const int*))0x4D158C)(&i);
            */
            bool isOpen = true;
            if (!isOpen) sequeceEngine[0xE8/4][0x10/4] = 0;
        }
    }

    
    void MenuPageHandler::MenuSingleCupBasePage::changeCup(GameSequenceSection* own, int cupID) {
        void(*MenuCourseNameSet)(VisualControl::GameVisualControl*, u32 cupID, u32& engineLevel, bool isMirror, bool unk0, bool unk1) = (decltype(MenuCourseNameSet))GameFuncs::MenuCourseNameSet;
        u32* ownU32 = (u32*)own;
        u8* ownU8 = (u8*)own;
        CRaceInfo* raceInfo = MarioKartFramework::getRaceInfo(true);
        VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
        bool isCupLocked = cupID < 0 || cupID > 7 && cupID < 0xA;
        for (int i = 0; i < 4; i++) {
            u32 nameID = CourseManager::getCourseGlobalIDName(cupID, i);
            u32 courseID; CourseManager::getGPCourseID(&courseID, cupID, i);
            bool courseLocked = std::find(MenuSingleCourseBasePage::blockedCourses.begin(), MenuSingleCourseBasePage::blockedCourses.end(), courseID) != MenuSingleCourseBasePage::blockedCourses.end();
            
            u32 courseButtonHandle = (u32)(courseButtonDummies[i]->vtable->getRootPane(courseButtonDummies[i]));
            courseButtonDummies[i]->GetNwlytControl()->vtable->setVisibleImpl(courseButtonDummies[i]->GetNwlytControl(), courseButtonHandle, !isCupLocked);

            VisualControl::Message message;
            string16 nameReplacement;
            Language::MsbtHandler::GetMessageFromList(message, (Language::MsbtHandler::MessageDataList*)courseButtonDummies[i]->GetMessageDataList(), nameID);
            if (courseLocked) {
                nameReplacement = message.data;
                nameReplacement = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 0, 0)) + nameReplacement;
                message.data = nameReplacement.c_str();
            }
            courseButtonDummies[i]->GetNwlytControl()->vtable->replaceMessageImpl(courseButtonDummies[i]->GetNwlytControl(), ((u32*)courseButtonDummies[i])[0x7C/4], message, nullptr, nullptr);
        }
        bool unk0, unk1;
        if (ownU8[0x2BC/1] == 1) {
            unk0 = 0;
            unk1 = 1;
        } else if (ownU8[0x2BC/1] == 0) {
            unk0 = 1;
            unk1 = 0;
        } else {
            unk0 = 0;
            unk1 = 0;
        }
        VisualControl::GameVisualControl* menuCourseName = (VisualControl::GameVisualControl*)ownU32[0x2C4/4];
        MenuCourseNameSet(menuCourseName, cupID, raceInfo->engineLevel, raceInfo->isMirror, unk0, unk1);
    
        u32 menuCourseNameHandle = (u32)(menuCourseName->vtable->getRootPane(menuCourseName));
        menuCourseName->GetNwlytControl()->vtable->setVisibleImpl(menuCourseName->GetNwlytControl(), menuCourseNameHandle, !isCupLocked);
    }
    
    static const s8 g_moveCursorTableSingle[10][4] = { // Down, Right, Up, Left
        {4, 1, 4, 8},
        {5, 2, 5, 0},
        {6, 3, 6, 1},
        {7, 8, 7, 2},
        {0, 5, 0, 9},
        {1, 6, 1, 4},
        {2, 7, 2, 5},
        {3, 9, 3, 6},
        {9, 8, 9, 8},
        {8, 9, 8, 9}
    };

    static const s8 g_moveCursorTableMulti[11][4] = { // Down, Right, Up, Left
        {4, 1, 10, 8},
        {5, 2, 10, 0},
        {6, 3, 10, 1},
        {7, 8, 10, 2},
        {10, 5, 0, 9},
        {10, 6, 1, 4},
        {10, 7, 2, 5},
        {10, 9, 3, 6},
        {9, 8, 9, 8},
        {8, 9, 8, 9},
        {-3, -1, -2, -1}
    };

    int MenuPageHandler::MenuSingleCupBasePage::HandleCursor(CursorMove* move, CursorItem* item, bool isMulti) {
        
        CursorMove::KeyType k = move->GetDir(item);

        MenuSingleCupBasePage* obj = (MenuSingleCupBasePage*)((u32*)move)[0x14/4];
        if (obj->conveyorState != ConveyorState::STOPPED || (VersusHandler::IsVersusMode && VersusHandler::IsRandomCourseMode())) {
            return -1;
        }
        
        if (k & CursorMove::KeyType::KEY_NONE)
            return -1;
        
        k = (CursorMove::KeyType)(k & 0xF);
        if (!k) return -1;

        int ret = item->elementID;
        const s8 (*moveCursorTable)[4] = isMulti ? g_moveCursorTableMulti : g_moveCursorTableSingle; 

        while (k) {
            int prevRet = ret;

            if (ret == 0xA) // Disable left and right input if current button is the random button
                k = (CursorMove::KeyType)(k & ~(CursorMove::KeyType::KEY_LEFT | CursorMove::KeyType::KEY_RIGHT));
            
            if (k & CursorMove::KeyType::KEY_UP) {
                ret = moveCursorTable[ret][0];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_UP);
            } else if (k & CursorMove::KeyType::KEY_RIGHT) {
                ret = moveCursorTable[ret][1];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_RIGHT);
            } else if (k & CursorMove::KeyType::KEY_DOWN) {
                ret = moveCursorTable[ret][2];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_DOWN);
            } else if (k & CursorMove::KeyType::KEY_LEFT) {
                ret = moveCursorTable[ret][3];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_LEFT);
            }
            if (ret == -1) {
                break;
            } else if (ret == -2) {
                ret = move->prevButton + 4;
            } else if (ret == -3) {
                ret = move->prevButton;
            } else if (ret == 10 && prevRet != 10) {
                move->prevButton = prevRet % 4;
            }
        }
        if ((ret == 8 || ret == 9) && obj->conveyorBlockedFrames)
            return -1;
        return ret;
    }

    bool MenuPageHandler::MenuSingleCupBasePage::StartConveyor(bool directionRight) {
        if (conveyorState != ConveyorState::STOPPED || conveyorBlockedFrames)
            return false;
        conveyorIncrement = (100.f / valueConveyorFrames) * (directionRight ? 1.f : -1.f);
        conveyorBlockedFrames = valueConveyorFrames + valueConveyorBlockedFrames;
        if (directionRight) {
            SetCupButtonBG(8, 9);
            SetCupButtonBG(9, 11);
            UpdateCupButtonState(1);
            
        } else {
            SetCupButtonBG(8, 10);
            SetCupButtonBG(9, 12);
            UpdateCupButtonState(2);
        }
        SetCupButtonVisible(8, true);
        SetCupButtonVisible(9, true);
        Snd::PlayMenu(Snd::SCROLL_LIST_STOP);
        conveyorState = ConveyorState::GOING;
        UpdateButtonEnabledState();
        return true;
    }

    void MenuPageHandler::MenuSingleCupBasePage::CalcConveyor() {
        int lastSelectedButtton = gameSection->GetLastSelectedButton();
        if (lastSelectedButtton == 8 || lastSelectedButtton == 9) {
            if (prevSelectedButton == 0 || prevSelectedButton == 4) {
                StartConveyor(true);
            } else if (prevSelectedButton == 3 || prevSelectedButton == 7) {
                StartConveyor(false);
            }
        }
        if (Controller::IsKeyDown(Key::R) && ((u8*)gameSection)[0x8F/1] == 0) {
            StartConveyor(false);
        } else if (Controller::IsKeyDown(Key::L) && ((u8*)gameSection)[0x8F/1] == 0) {
            StartConveyor(true);
        }       

        if (conveyorIncrement) {
            conveyorTimer += conveyorIncrement;
            if (conveyorTimer <= 0.f || conveyorTimer >= 200.f) {
                bool dirRight = conveyorIncrement > 0;
                conveyorIncrement = 0.f;
                if (conveyorTimer <= 0.f) conveyorTimer = 0.f;
                if (conveyorTimer >= 200.f) conveyorTimer = 200.f;
                OnConveyorEnd(dirRight);
            }
        }
        if (lastConveyorTimer != conveyorTimer) {
            cupBgControl->GetAnimationFamily(0)->SetAnimation(0, conveyorTimer);
            lastConveyorTimer = conveyorTimer;
        }
        if (conveyorBlockedFrames) conveyorBlockedFrames--;
    }

    void MenuPageHandler::MenuSingleCupBasePage::OnConveyorEnd(bool directionRight) {
        u32 size;
        void(*UIManipulatorSetCursor)(u32*, int) = (decltype(UIManipulatorSetCursor))GameFuncs::UIManipulatorSetCursor;
        float(*AnimationItemGetCurrentFrame)(void*) = (decltype(AnimationItemGetCurrentFrame))GameFuncs::AnimationItemGetCurrentFrame;
		CourseManager::getCupTranslatetable(&size);
        conveyorTimer = 100.f;
        SetCupButtonVisible(8, false);
        SetCupButtonVisible(9, false);
        conveyorState = ConveyorState::STOPPED;
        UpdateButtonEnabledState();
        if (directionRight) {
            startingButtonID--;
            if (startingButtonID < 0) {
				startingButtonID = (size / 2) - 1;
			}
        } else {
            startingButtonID++;
            if (startingButtonID >= (size / 2)) {
				startingButtonID = 0;
			}
        }
        UpdateCupButtonState(0);

        int lastSelectedbutton = this->gameSection->GetLastSelectedButton();
        if (lastSelectedbutton < 0 || lastSelectedbutton == 10)
            return;
        VisualControl::GameVisualControl* prevSelectedButtonControl = this->buttonList[lastSelectedbutton];
        if (lastSelectedbutton == 8) {
            if (directionRight) {
                lastSelectedbutton = 0;
            } else {
                lastSelectedbutton = 3;
            }
        } else if (lastSelectedbutton == 9) {
            if (directionRight) {
                lastSelectedbutton = 4;
            } else {
                lastSelectedbutton = 7;
            }
        } else {
            if (directionRight) {
                if (lastSelectedbutton != 3 && lastSelectedbutton != 7) lastSelectedbutton++;
                else forceSelectOn = true;
            } else {
                if (lastSelectedbutton != 0 && lastSelectedbutton != 4) lastSelectedbutton--;
                else forceSelectOn = true;
            }
        }
        VisualControl::GameVisualControl* newSelectedButtonControl = this->buttonList[lastSelectedbutton];

        float prevButtonBackgroundLoopFrame = AnimationItemGetCurrentFrame(prevSelectedButtonControl->GetAnimationFamily(4)->GetAnimationItem(0));
        float prevButtonIconLoopFrame = AnimationItemGetCurrentFrame(prevSelectedButtonControl->GetAnimationFamily(3)->GetAnimationItem(0));
        int *currentManipulatorBlockedFrames = (int*)((u32)(gameSection->GetUIManipulator()) + 0x108);
        int prevManipulatorBlockedFrames = *currentManipulatorBlockedFrames;

        u32* buttonSoundPlayer = ((u32*)newSelectedButtonControl + 0x134/4);
        u32 buttonSoundPlayerBackup = *buttonSoundPlayer;
        *buttonSoundPlayer = 0;

        UIManipulatorSetCursor(this->gameSection->GetUIManipulator(), lastSelectedbutton);

        *buttonSoundPlayer = buttonSoundPlayerBackup;

        // Disable background fadeout
        prevSelectedButtonControl->GetAnimationFamily(1)->ChangeAnimation(4, 2.f);
        newSelectedButtonControl->GetAnimationFamily(1)->ChangeAnimation(3, 2.f); // This needs to be set to 3, otherwise the game will overwrite the next to change animation

        newSelectedButtonControl->GetAnimationFamily(4)->ChangeAnimation(0, prevButtonBackgroundLoopFrame);
        newSelectedButtonControl->GetAnimationFamily(3)->ChangeAnimation(0, prevButtonIconLoopFrame);

        *currentManipulatorBlockedFrames = prevManipulatorBlockedFrames;
    }

    void MenuPageHandler::MenuSingleCupBasePage::OnPagePreStep(GameSequenceSection* own) {
        u32(*getSelectedCupCourse)(u32 moflexHandle) = (decltype(getSelectedCupCourse))GameFuncs::moflexGetSelectedCupCourse;
        void(*buttonKeyHandlerCommon)(VisualControl::GameVisualControl*, int, Key) = (decltype(buttonKeyHandlerCommon))GameFuncs::buttonKeyHandlerCommon;

        u32* ownU32 = (u32*)own;
        u8* ownU8 = (u8*)own;
        MenuSingleCupGPPage* obj = ((MenuSingleCupGPPage*)own->vtable->userData);
        obj->CalcConveyor();
        int lastSelectedButton = own->GetLastSelectedButton();
        
        if (obj->prevSelectedButton != lastSelectedButton || obj->forceSelectOn) {
            if (lastSelectedButton == 10) {
                ButtonHandler_SelectOn(own, 9);
            } else {
                selectedCupIcon = lastSelectedButton;
                if (lastSelectedButton >= 0)
                    ButtonHandler_SelectOn(own, ((u32*)(obj->buttonList[lastSelectedButton]))[0x214/4]);
                
                obj->forceSelectOn = false;
            }
            obj->prevSelectedButton = lastSelectedButton;
        }
        if (lastSelectedButton == 10)
            return;
        if (ownU8[0x14/1] == 5 && lastSelectedButton >= 0) {
            u32 selectedTrack;
            int buttonID = ((u32*)(obj->buttonList[lastSelectedButton]))[0x214/4];
            if (buttonID < 8) {
                u32 selectedCupCourse = getSelectedCupCourse(ownU32[0x294/4]);
                u32 selectedButtonID = ((u32*)(own->GetButtonArray(0x2E0)[lastSelectedButton]))[0x214/4];
                if (selectedCupCourse > 0 && selectedButtonID == (selectedCupCourse - 1) / 8 ) {
                    u32 viewIndex = selectedCupCourse % 8;
                    if (viewIndex == 1 || viewIndex == 2) {
                        selectedTrack = 0;
                    } else if (viewIndex == 3 || viewIndex == 4) {
                        selectedTrack = 1;
                    } else if (viewIndex == 5 || viewIndex == 6) {
                        selectedTrack = 2;
                    } else if (viewIndex == 7 || viewIndex == 0) {
                        selectedTrack = 3;
                    } else {
                        selectedTrack = -1;
                    }
                }
            } else {
                obj->ctPreviewFrameCounter++;
                if (obj->ctPreviewFrameCounter >= MarioKartTimer::ToFrames(0, 4)) {
                    obj->ctPreviewFrameCounter = 0;
                    obj->ctPreviewCurrTrack++;
                    if (obj->ctPreviewCurrTrack > 3) obj->ctPreviewCurrTrack = 0;
                    obj->ctPreview.SetPreview(buttonID, obj->ctPreviewCurrTrack, false);
                }
                selectedTrack = obj->ctPreviewCurrTrack;
            }
            VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
            for (int i = 0; i < 4; i++) {
                if (i == selectedTrack)
                    courseButtonDmySelectOn(courseButtonDummies[i]);
                else
                    courseButtonDmySelectOff(courseButtonDummies[i]);
            }
        }

        if (g_getCTModeVal == CTMode::ONLINE_COM && CourseManager::isRandomTracksForcedComm && obj->hasRandomButton && ownU8[0x14/1] == 5 && !obj->autoButtonPressed) {
            buttonKeyHandlerCommon((VisualControl::GameVisualControl*)ownU32[0x314/4], 0, Key::A);
            obj->autoButtonPressed = true;
        }
    }

    void MenuPageHandler::MenuSingleCupGPPage::OnPagePreStep(GameSequenceSection* own) {
        u32(*getSelectedCupCourse)(u32 moflexHandle) = (decltype(getSelectedCupCourse))GameFuncs::moflexGetSelectedCupCourse;
        void(*controlSliderStepH)(void* controlSlider) = (decltype(controlSliderStepH))GameFuncs::controlSliderStepH;
        void(*controlSliderStepV)(void* controlSlider) = (decltype(controlSliderStepV))GameFuncs::controlSliderStepV;
        void(*basePageCalcNormalControl)(GameSequenceSection*) = (decltype(basePageCalcNormalControl))GameFuncs::basePageCalcNormalControl;
        void(*buttonKeyHandlerCommon)(VisualControl::GameVisualControl*, int, Key) = (decltype(buttonKeyHandlerCommon))GameFuncs::buttonKeyHandlerCommon;

        u32* ownU32 = (u32*)own;
        u8* ownU8 = (u8*)own;
        MenuSingleCupGPPage* obj = ((MenuSingleCupGPPage*)own->vtable->userData);
        obj->CalcConveyor();
        int lastSelectedButton = own->GetLastSelectedButton();
        if (obj->prevSelectedButton != lastSelectedButton || obj->forceSelectOn) {
            if (lastSelectedButton >= 0)
            {
                selectedCupIcon = lastSelectedButton;
                ButtonHandler_SelectOn(own, ((u32*)(obj->buttonList[lastSelectedButton]))[0x214/4]);
            }
            obj->prevSelectedButton = lastSelectedButton;
            obj->forceSelectOn = false;
        }
        if (ownU8[0x14/1] && lastSelectedButton >= 0) {
            u32 selectedTrack;
            int buttonID = ((u32*)(obj->buttonList[lastSelectedButton]))[0x214/4];
            if (buttonID < 8) {
                u32 selectedCupCourse = getSelectedCupCourse(ownU32[0x294/4]);
                u32 selectedButtonID = ((u32*)(own->GetButtonArray(0x2E0)[lastSelectedButton]))[0x214/4];
                if (selectedCupCourse > 0 && selectedButtonID == (selectedCupCourse - 1) / 8 ) {
                    u32 viewIndex = selectedCupCourse % 8;
                    if (viewIndex == 1 || viewIndex == 2) {
                        selectedTrack = 0;
                    } else if (viewIndex == 3 || viewIndex == 4) {
                        selectedTrack = 1;
                    } else if (viewIndex == 5 || viewIndex == 6) {
                        selectedTrack = 2;
                    } else if (viewIndex == 7 || viewIndex == 0) {
                        selectedTrack = 3;
                    } else {
                        selectedTrack = -1;
                    }
                }
            } else {
                obj->ctPreviewFrameCounter++;
                if (obj->ctPreviewFrameCounter >= MarioKartTimer::ToFrames(0, 4)) {
                    obj->ctPreviewFrameCounter = 0;
                    obj->ctPreviewCurrTrack++;
                    if (obj->ctPreviewCurrTrack > 3) obj->ctPreviewCurrTrack = 0;
                    obj->ctPreview.SetPreview(MissionHandler::isMissionMode ? -2 : buttonID, obj->ctPreviewCurrTrack, false);
                }
                selectedTrack = obj->ctPreviewCurrTrack;
            }
            VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
            for (int i = 0; i < 4; i++) {
                if (i == selectedTrack && !MissionHandler::isMissionMode)
                    courseButtonDmySelectOn(courseButtonDummies[i]);
                else
                    courseButtonDmySelectOff(courseButtonDummies[i]);
            }
        }
        if (ownU8[0x318/1]) {
            u8 status = ownU8[0x8F/1];
            if (status == 2 || status == 5)
                ownU32[0x98/4] = 0;
            if (ownU32[0x314/4]) {
                ownU32[0x314/4]--;
            } else {
                SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
                void* defaultSlider = controlSliderArray[0];
                controlSliderStepH(defaultSlider);
                controlSliderStepV(defaultSlider);
                ((u32*)defaultSlider)[0]++;
                if (((u32*)defaultSlider)[0xC/4] && ((u8*)defaultSlider)[0x1866/1]) {
                    ownU8[0x318/1] = 0;
                }
            }
        }
        if (ownU8[0x8F/1] != 0 && ownU8[0x8F/1] != 1) {
            basePageCalcNormalControl(own);
        }
        SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
        void* defaultSlider = controlSliderArray[0];
        if (VersusHandler::IsVersusMode && VersusHandler::IsRandomCourseMode() && ownU8[0x14/1] == 5 && ((u32*)defaultSlider)[0xC/4] && ((u8*)defaultSlider)[0x1866/1] && (ownU8[0x8F/1] == 0 || ownU8[0x8F/1] == 1) && !obj->autoButtonPressed) {
            if (obj->comesFromOK && obj->backButtonControl)
                buttonKeyHandlerCommon(obj->backButtonControl, 0, Key::B);
            else
                buttonKeyHandlerCommon(own->GetButtonArray(0x2E0)[lastSelectedButton], 0, Key::A);
            obj->autoButtonPressed = true;
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::ButtonHandler_SelectOn(GameSequenceSection* own, int buttonID) {
        u32* ownU32 = (u32*)own;
        void(*MoflexReset)(bool enable) = (decltype(MoflexReset))GameFuncs::MoflexReset;
        void(*MoflexEnablePane)(VisualControl::GameVisualControl*, bool enable) = (decltype(MoflexEnablePane))GameFuncs::MoflexEnablePane;
        void(*MoflexInit)(const SafeStringBase&,int) = (decltype(MoflexInit))GameFuncs::MoflexInit;
        void(*MoflexUpdateFrame)(u32, u32*) = (decltype(MoflexUpdateFrame))GameFuncs::MoflexUpdateFrame;
        MenuSingleCupBasePage* obj = ((MenuSingleCupBasePage*)own->vtable->userData);

        bool isCupLocked = buttonID < 0 || buttonID > 7 || MissionHandler::isMissionMode;
        
        if (isCupLocked) {
            MoflexReset(false);
        }
        VisualControl::GameVisualControl* omakaseView = (VisualControl::GameVisualControl*)ownU32[0x2DC/4];
        u32 omakaseRootHandle = (u32)(omakaseView->vtable->getRootPane(omakaseView));
        omakaseView->GetNwlytControl()->vtable->setVisibleImpl(omakaseView->GetNwlytControl(), omakaseRootHandle, isCupLocked);
        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2C0/4]; 
        MoflexEnablePane(movieView, !isCupLocked);
        
        obj->ctPreviewCurrTrack = (isCupLocked && obj->ctPreviewChildTrack) ? obj->ctPreviewChildTrack - 1 : 0;
        obj->ctPreviewChildTrack = 0;
        obj->ctPreviewFrameCounter = 0;
        if (!isCupLocked)
            obj->ctPreview.SetPreview(-1, 0, false);
        else
            obj->ctPreview.SetPreview((buttonID == 9 || MissionHandler::isMissionMode) ? -2 : buttonID, obj->ctPreviewCurrTrack, false);

        VisualControl::GameVisualControl** courseButtonDummies = (VisualControl::GameVisualControl**)(ownU32 + 0x2C8/4);
        for (int i = 0; i < 4; i++) {
            if (i == obj->ctPreviewCurrTrack && buttonID != 9)
                courseButtonDmySelectOn(courseButtonDummies[i]);
            else
                courseButtonDmySelectOff(courseButtonDummies[i]);
        }
        changeCup(own, buttonID & 0xFF);
        if (!isCupLocked) {
            if (!((u8*)(ownU32[0x294/4]))[0x16B8/1]) {
                MoflexInit(SafeStringBase("CourseSelectRace"), 0);
            }
            u32** moflexUpdaters = (u32**)(ownU32 + 0x29C/4);
            u32* currUpdater = moflexUpdaters[buttonID];
            if (!currUpdater)
                currUpdater = moflexUpdaters[0];
            MoflexUpdateFrame(ownU32[0x294/4], currUpdater);
        }
    }

    void MenuPageHandler::MenuSingleCupGPPage::ButtonHandler_OK(GameSequenceSection* own, int buttonID) {
        u32* ownU32 = (u32*)own;
        MenuSingleCupGPPage* obj = (MenuSingleCupGPPage*)own->vtable->userData;

        void(*controlSliderStartH)(void* controlSlider) = (decltype(controlSliderStartH))GameFuncs::controlSliderStartH;
        void(*controlSliderStartV)(void* controlSlider) = (decltype(controlSliderStartV))GameFuncs::controlSliderStartV;
        void(*garageDirectorFade)(u32 garageDirector, bool fade) = (decltype(garageDirectorFade))GameFuncs::garageDirectorFade;
        
        if (buttonID == 9)
            return;
        
        if (buttonID != 8) {
            if (!((u8*)own)[0x2BC/1]) {
                GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 0xF, 1);
            }
            ownU32[0x318/4] = 1;
            ownU32[0x314/4] = 0;
            SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
            ((u8*)controlSliderArray[0])[0x1838] = 1;
            ((u8*)controlSliderArray[1])[0x1838] = 0;
            ((u8*)controlSliderArray[2])[0x1838] = 0;
            ((u8*)controlSliderArray[0])[0x1858] = 0;
            ((u8*)controlSliderArray[0])[0x4] = 0;
            ((u32*)controlSliderArray[0])[0] = 0;
            controlSliderStartH(controlSliderArray[0]);
            controlSliderStartH(controlSliderArray[0]);
            if (VersusHandler::IsVersusMode) {
                VersusHandler::OnCupSelect(buttonID);
                buttonID = VERSUSCUPID;
            }
            else if (MissionHandler::isMissionMode) {
                MissionHandler::OnCupSelect(buttonID);
                buttonID = MISSIONCUPID;
            }
            UserCTHandler::UpdateCurrentCustomCup(buttonID);
            MarioKartFramework::BasePageSetCup(buttonID & 0xFF);
        } else {
            if (((u8*)own)[0x8F/1]) {
                ownU32[0x318/4] = 1;
                ownU32[0x314/4] = 0;
                SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
                ((u8*)controlSliderArray[0])[0x1838] = 2;
                ((u8*)controlSliderArray[1])[0x1838] = 0;
                ((u8*)controlSliderArray[2])[0x1838] = 0;
                ((u8*)controlSliderArray[0])[0x1858] = 1;
                ((u8*)controlSliderArray[0])[0x4] = 1;
                ((u32*)controlSliderArray[0])[0] = 0;
                controlSliderStartH(controlSliderArray[0]);
                controlSliderStartH(controlSliderArray[0]);
                obj->comesFromOK = true;
                obj->autoButtonPressed = false;
            }
            u32 garageDirector = MarioKartFramework::getGarageDirector();
            garageDirectorFade(garageDirector, true);
            GarageRequestChangeState(garageDirector, 0xB, 0);
        }
    }

    void MenuPageHandler::MenuSingleCupBasePage::ButtonHandler_OK(GameSequenceSection* own, int buttonID) {
        u32* ownU32 = (u32*)own;
        
        if (buttonID == 9 || buttonID == 8)
            return;
        
        if (VersusHandler::IsVersusMode) {
            VersusHandler::OnCupSelect(buttonID);
            buttonID = VERSUSCUPID;
        }
        else if (MissionHandler::isMissionMode) {
            MissionHandler::OnCupSelect(buttonID);
            buttonID = MISSIONCUPID;
        }
        UserCTHandler::UpdateCurrentCustomCup(buttonID);
        MarioKartFramework::BasePageSetCup(buttonID & 0xFF);

        if (!((u8*)own)[0x2BC/1]) {
            GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 0xF, 1);
        }
    }

    void MenuPageHandler::MenuSingleCupGPPage::OnPageComplete(GameSequenceSection* own) {
        void(*SequenceStartFadeout)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceStartFadeout))GameFuncs::SequenceStartFadeout;
        void(*SequenceReserveFadeIn)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceReserveFadeIn))GameFuncs::SequenceReserveFadeIn;
        
        u32* ownU32 = (u32*)own;
        if (ownU32[0x4C/4] != ownU32[0x5C/4]) {
            ((void(*)())(UserCTHandler::BaseMenuPage_applySetting_GP))();
            SequenceStartFadeout(1, 30, 2);
            SequenceReserveFadeIn(1, 30, 2);
            GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 0x10, 1);
        }
    }

    void MenuPageHandler::MenuMultiCupGPPage::OnPageComplete(GameSequenceSection* own) {
        ((MenuMultiCupGPPage*)own->vtable->userData)->OnPageCompleteBackup(own);

        void(*SequenceStartFadeout)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceStartFadeout))GameFuncs::SequenceStartFadeout;
        void(*SequenceReserveFadeIn)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceStartFadeout))GameFuncs::SequenceReserveFadeIn;
        
        SequenceStartFadeout(1, 30, 2);
        SequenceReserveFadeIn(1, 30, 2);
        ((u8*)own)[0x320/1] = 1;
    }

    void MenuPageHandler::MenuSingleCupBasePage::OnPageExit(GameSequenceSection* own, bool isCupBase) {
        u32* ownU32 = (u32*)own;
        ((MenuMultiCupGPPage*)own->vtable->userData)->isInPage = false;
        void(*MoflexReset)(bool enable) = (decltype(MoflexReset))GameFuncs::MoflexReset;

        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2C0/4];
        bool somethingequal = ownU32[0x4C/4] == ownU32[0x5C/4];
        if (somethingequal) {
            MoflexReset(true);
        }
        ((u32*)movieView)[0xAC/4] = somethingequal;
        ((MenuSingleCupGPPage*)own->vtable->userData)->ctPreview.Unload();
        int uiManipulatorLastButton =  own->GetLastSelectedButton();
        if (uiManipulatorLastButton >= 0)
            ((MenuSingleCupGPPage*)own->vtable->userData)->selectedCupIcon = uiManipulatorLastButton;
        if (!isCupBase) MoflexReset(true);
    }

    void MenuPageHandler::MenuSingleCourseBasePage::OnPageEnter(GameSequenceSection* own) {
        void(*moflexUpdateCourse)(GameSequenceSection*) = (decltype(moflexUpdateCourse))GameFuncs::moflexUpdateCourse;
        void(*uiMovieViewAnimOut)(VisualControl::GameVisualControl*) = (decltype(uiMovieViewAnimOut))GameFuncs::uiMovieViewAnimOut;
        void(*MoflexEnablePane)(VisualControl::GameVisualControl*, bool enable) = (decltype(MoflexEnablePane))GameFuncs::MoflexEnablePane;
        void(*MenuCourseNameSet)(VisualControl::GameVisualControl*, u32 cupID, u32& engineLevel, bool isMirror, bool unk0, bool unk1) = (decltype(MenuCourseNameSet))GameFuncs::MenuCourseNameSet;
        
        CRaceInfo* raceInfo = MarioKartFramework::getRaceInfo(true);
        int cupID = MarioKartFramework::BasePageGetCup();
        MenuSingleCourseBasePage* obj = (MenuSingleCourseBasePage*)own->vtable->userData;
        u32* ownU32 = (u32*)own;

        obj->ctPreview.Load();

        VisualControl::GameVisualControl* movieView1 = (VisualControl::GameVisualControl*)ownU32[0x2C0/4];
        VisualControl::GameVisualControl* movieView2 = (VisualControl::GameVisualControl*)ownU32[0x2C4/4];
        VisualControl::GameVisualControl* omakaseView1 = (VisualControl::GameVisualControl*)ownU32[0x2EC/4];
        VisualControl::GameVisualControl* omakaseView2 = (VisualControl::GameVisualControl*)ownU32[0x2F0/4];

        ((u8*)movieView1)[0xAD/1] = 1;
        moflexUpdateCourse(own);

        if (ownU32[0x48/4] != 1) {
            uiMovieViewAnimOut(movieView1);
            uiMovieViewAnimOut(movieView2);
        }
        
        bool isCupLocked = cupID < 0 || cupID > 7;

        u32 omakase1RootHandle = (u32)(omakaseView1->vtable->getRootPane(omakaseView1));
        omakaseView1->GetNwlytControl()->vtable->setVisibleImpl(omakaseView1->GetNwlytControl(), omakase1RootHandle, isCupLocked);
        u32 omakase2RootHandle = (u32)(omakaseView2->vtable->getRootPane(omakaseView2));
        omakaseView2->GetNwlytControl()->vtable->setVisibleImpl(omakaseView2->GetNwlytControl(), omakase2RootHandle, isCupLocked);
        MoflexEnablePane(movieView1, !isCupLocked);
        MoflexEnablePane(movieView2, !isCupLocked);
        if (isCupLocked)
            obj->ctPreview.SetPreview(MissionHandler::isMissionMode ? -2 : cupID, own->GetLastSelectedButton(), false);
        else
            obj->ctPreview.SetPreview(-1, 0, false);

        for (int i = 0; i < 4; i++) {
            u32 nameID = CourseManager::getCourseGlobalIDName(cupID, i);
            u32 courseID; CourseManager::getGPCourseID(&courseID, cupID, i);
            bool courseLocked = std::find(blockedCourses.begin(), blockedCourses.end(), courseID) != blockedCourses.end();

            VisualControl::GameVisualControl** buttonArray1 = (VisualControl::GameVisualControl**)(ownU32 + 0x2CC/4);
            VisualControl::GameVisualControl** buttonArray2 = (VisualControl::GameVisualControl**)(ownU32 + 0x2DC/4);

            VisualControl::Message message;
            string16 nameReplacement;
            Language::MsbtHandler::GetMessageFromList(message, (Language::MsbtHandler::MessageDataList*)buttonArray1[i]->GetMessageDataList(), nameID);
            if (courseLocked) {
                nameReplacement = message.data;
                nameReplacement = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 0, 0)) + nameReplacement;
                message.data = nameReplacement.c_str();
            }
            buttonArray1[i]->GetNwlytControl()->vtable->replaceMessageImpl(buttonArray1[i]->GetNwlytControl(), ((u32*)buttonArray1[i])[0x7C/4], message, nullptr, nullptr);
            Language::MsbtHandler::GetMessageFromList(message, (Language::MsbtHandler::MessageDataList*)buttonArray2[i]->GetMessageDataList(), nameID);
            if (courseLocked) {
                nameReplacement = message.data;
                nameReplacement = Language::MsbtHandler::ControlString::GenColorControlString(Language::MsbtHandler::ControlString::DashColor::CUSTOM, Color(255, 0, 0)) + nameReplacement;
                message.data = nameReplacement.c_str();
            }
            buttonArray2[i]->GetNwlytControl()->vtable->replaceMessageImpl(buttonArray2[i]->GetNwlytControl(), ((u32*)buttonArray2[i])[0x94/4], message, nullptr, nullptr);

            ((u32*)buttonArray2[i])[0x230 / 4] = courseLocked ? -1 : 0;
            ((u8*)buttonArray2[i])[0x224] = courseLocked ? 0 : 90;
            ((u8*)buttonArray2[i])[0x225] = courseLocked ? 91 : 0;
        }

        if (((u8*)own)[0x2BC/1]) {
            MenuCourseNameSet((VisualControl::GameVisualControl*)ownU32[0x2C8/4], cupID, raceInfo->engineLevel, raceInfo->isMirror, 0, 0);
        }

        GarageRequestChangeState(MarioKartFramework::getGarageDirector(), 0xC, ownU32[0x48/4] != 1 ? 1 : 0);
    }

    void MenuPageHandler::MenuSingleCoursePage::buttonHandler_SelectOn(GameSequenceSection* own, int buttonID) {
        MenuSingleCoursePage* obj = (MenuSingleCoursePage*)own->vtable->userData;
        u32 cupID = MarioKartFramework::BasePageGetCup();
        if (cupID > 7) {
            obj->ctPreview.SetPreview(cupID, buttonID, false);
            u32 courseID;
            CourseManager::getGPCourseID(&courseID, cupID, buttonID);
            ((u32*)own)[0x31C/4] = courseID;
            ((u32*)own)[0x314/4] = 0;
        } else
            obj->buttonHandler_SelectOnbackup(own, buttonID);
    }

    void MenuPageHandler::MenuSingleCourseBasePage::OnPageExit(GameSequenceSection* own) {
        void(*MoflexReset)(bool enable) = (decltype(MoflexReset))GameFuncs::MoflexReset;
        u32* ownU32 = (u32*)own;
        MenuSingleCoursePage* obj = (MenuSingleCoursePage*)own->vtable->userData;
        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2C0/4];
        bool somethingequal = ownU32[0x4C/4] == ownU32[0x5C/4];
        if (somethingequal) {
            MoflexReset(true);
        }
        ((u32*)movieView)[0xAC/4] = somethingequal;
        obj->ctPreview.Unload();
        u32 lastTrack = own->GetLastSelectedButton();
        u32 cupID = MarioKartFramework::BasePageGetCup();
        if (lastTrack >= 0 && lastTrack <= 4 && cupID > 7) {
            obj->parent[0]->ctPreviewChildTrack = lastTrack + 1;
            obj->parent[0]->ctPreviewChildAnim = obj->ctPreview.GetTarget();
        }
    }

    void MenuPageHandler::MenuCourseVoteBase::OnRevealCourseMoflex(GameSequenceSection* own) {
        void(*MoflexEnablePane)(VisualControl::GameVisualControl*, bool enable) = (decltype(MoflexEnablePane))GameFuncs::MoflexEnablePane;
        u32* ownU32 = (u32*)own;
        MenuCourseVoteBase* obj = (MenuCourseVoteBase*)own->vtable->userData;
        
        u32* menuData = (u32*)MarioKartFramework::getMenuData();
        u8* networkmenu = (u8*)MarioKartFramework::getNetworkSelectMenuProcess();
        SeadArrayPtr<u8*>& networkBuffers = *(SeadArrayPtr<u8*>*)(menuData + 0x72C/4);
        u32 courseID = networkBuffers[networkmenu[0x6B/1]][4];

        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2CC/4];
        VisualControl::GameVisualControl* omakaseView = (VisualControl::GameVisualControl*)ownU32[0x2D0/4];

        u32 omakaseRootHandle = (u32)(omakaseView->vtable->getRootPane(omakaseView));
        omakaseView->GetNwlytControl()->vtable->setVisibleImpl(omakaseView->GetNwlytControl(), omakaseRootHandle, false);

        if (courseID < CUSTOMTRACKLOWER) {
            MoflexEnablePane(movieView, ((u8*)own)[0x2C5/1] == 0);
        } else {
            u32 myomakaseRootHandle = (u32)(obj->myOmakase->vtable->getRootPane(obj->myOmakase));
            obj->myOmakase->GetNwlytControl()->vtable->setVisibleImpl(obj->myOmakase->GetNwlytControl(), myomakaseRootHandle, true);
            obj->ctPreveiw.Load();
            obj->ctPreveiw.SetPreview(courseID, true);
        }
    }

    void MenuPageHandler::MenuCourseVoteBase::InitControl(GameSequenceSection* own) {
        u32* ownU32 = (u32*)own;
        MenuCourseVoteBase* obj = (MenuCourseVoteBase*)own->vtable->userData;
        obj->initControlBackup(own);

        obj->myOmakase = obj->ctPreveiw.GetControl(own, false);
        u32 myomakaseRootHandle = (u32)(obj->myOmakase->vtable->getRootPane(obj->myOmakase));
        obj->myOmakase->GetNwlytControl()->vtable->setVisibleImpl(obj->myOmakase->GetNwlytControl(), myomakaseRootHandle, false);

        VisualControl::GameVisualControl* movieView = (VisualControl::GameVisualControl*)ownU32[0x2CC/4];

        void* movieRootPane = movieView->vtable->getRootPane(movieView);
        ((u32*)obj->myOmakase)[0xA8/4] = (u32)movieRootPane;
        void* omakaseViewRootPane = obj->myOmakase->vtable->getRootPane(obj->myOmakase);
        Vector3* movieRootPanePos = (Vector3*)((u32)movieRootPane + 0x28);
        Vector3* omakaseViewRootPanePos = (Vector3*)((u32)omakaseViewRootPane + 0x28);
        ((float*)obj->myOmakase)[0xAC/4] = omakaseViewRootPanePos->y - movieRootPanePos->y;
    }

    void MenuPageHandler::MenuCourseVoteBase::OnPageExit(GameSequenceSection* own) {
        MenuCourseVoteBase* obj = (MenuCourseVoteBase*)own->vtable->userData;
        obj->onPageExitBackup(own);
        obj->ctPreveiw.Unload();
        u32 myomakaseRootHandle = (u32)(obj->myOmakase->vtable->getRootPane(obj->myOmakase));
        obj->myOmakase->GetNwlytControl()->vtable->setVisibleImpl(obj->myOmakase->GetNwlytControl(), myomakaseRootHandle, false);
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadSingleModeMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuSingleMode) {
            menuSingleMode = new MenuSingleModePage();
            return menuSingleMode->Load(sectionDirector);
        }
        return nullptr;
    }

    void MenuPageHandler::MenuTitlePage::OnInitControl(GameSequenceSection* own, VisualControl::GameVisualControl* buttons[3]) {
        #if CITRA_MODE == 1
        VisualControl::NwlytControlSight* buttonNwlyt = buttons[2]->GetNwlytControl(); 
        buttonNwlyt->vtable->setVisibleImpl(buttonNwlyt, (u32)buttonNwlyt->vtable->getRootPane(buttonNwlyt), false);

        void* singleButtonRoot = buttons[0]->vtable->getRootPane(buttons[0]);
        void* multiButtonRoot = buttons[1]->vtable->getRootPane(buttons[1]);
        
        Vector3* singleButtonRootPanePos = (Vector3*)((u32)singleButtonRoot + 0x28);
        Vector3* multiButtonRootPanePos = (Vector3*)((u32)multiButtonRoot + 0x28);

        singleButtonRootPanePos->y -= 22;
        multiButtonRootPanePos->y -= 40;


        own->GetControlMove()->keyHandler = HandleCursor;
        #endif
    }

    #if CITRA_MODE == 1
    static const s8 g_moveCursorTableTitle[4][4] = { // Down, Right, Up, Left
        {1, -1, 3, -1},
        {3, -1, 0, -1},
        {-1, -1, -1, -1},
        {0, -1, 1, -1},
    };
    #endif
    int MenuPageHandler::MenuTitlePage::HandleCursor(CursorMove* move, CursorItem* item) {
        #if CITRA_MODE == 1
        CursorMove::KeyType k = move->GetDir(item);

        if (k & CursorMove::KeyType::KEY_NONE)
            return -1;
        
        k = (CursorMove::KeyType)(k & 0xF);
        if (!k) return -1;
        int ret = item->elementID;
        const s8 (*moveCursorTable)[4] = g_moveCursorTableTitle;

        while (k) {
            int prevRet = ret;
            if (k & CursorMove::KeyType::KEY_UP) {
                ret = moveCursorTable[ret][0];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_UP);
            } else if (k & CursorMove::KeyType::KEY_RIGHT) {
                ret = moveCursorTable[ret][1];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_RIGHT);
            } else if (k & CursorMove::KeyType::KEY_DOWN) {
                ret = moveCursorTable[ret][2];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_DOWN);
            } else if (k & CursorMove::KeyType::KEY_LEFT) {
                ret = moveCursorTable[ret][3];
                k = (CursorMove::KeyType)(k & ~CursorMove::KeyType::KEY_LEFT);
            }
        }
        return ret;
        #else
        return -1;
        #endif
    }

    void MenuPageHandler::MenuEndingPage::InitControl(GameSequenceSection* own) {
        // 0x26C -> EndingView 0x274 staffroll
        MenuEndingPage* obj = ((MenuEndingPage*)own->vtable->userData);
        obj->staffroll = new StaffRoll(obj);
        if (obj->staffroll)
            obj->staffroll->InitControl();
    }

    void MenuPageHandler::MenuEndingPage::onPageEnter(GameSequenceSection* own) {
        MenuEndingPage* obj = ((MenuEndingPage*)own->vtable->userData);
        if (obj->staffroll)
            obj->staffroll->onPageEnter();
    }
    void MenuPageHandler::MenuEndingPage::onPagePreStep(GameSequenceSection* own) {
        MenuEndingPage* obj = ((MenuEndingPage*)own->vtable->userData);
        if (obj->staffroll)
            obj->staffroll->onPagePreStep();
    }
    void MenuPageHandler::MenuEndingPage::onPageComplete(GameSequenceSection* own) {
        void(*SequenceStartFadeout)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceStartFadeout))MenuPageHandler::GameFuncs::SequenceStartFadeout;
        SequenceStartFadeout(0, 30, 2);
        MenuEndingPage* obj = ((MenuEndingPage*)own->vtable->userData);
        if (obj->staffroll)
            obj->staffroll->onPageComplete();
    }
    void MenuPageHandler::MenuEndingPage::onPageExit(GameSequenceSection* own) {
        MenuEndingPage* obj = ((MenuEndingPage*)own->vtable->userData);
        if (obj->staffroll)
            obj->staffroll->onPageExit();
    }
    
    void MenuPageHandler::MenuSingleCharaPage::OnInitControl(GameSequenceSection* own) {
        initControlBackup(own);

        u32* ownU32 = (u32*)own;
        if (!controlVtable) {
            controlVtable = (VisualControl::GameVisualControlVtable*)operator new(sizeof(VisualControl::GameVisualControlVtable));
            memcpy(controlVtable, (u32*)GameFuncs::omakaseViewVtable, sizeof(VisualControl::GameVisualControlVtable));
            controlVtable->onReset = (decltype(controlVtable->onReset))VisualControl::nullFunc;
        }
        VisualControl::GameVisualControl* charmngr = VisualControl::Build(ownU32, "chara_mngr", "omakase_T", &VisualControl::AnimationDefine::empty, controlVtable, VisualControl::ControlType::BASEMENUVIEW_CONTROL);
        SeadArrayPtr<void*>& controlSliderArray = *(SeadArrayPtr<void*>*)(ownU32 + 0x26C/4);
        ((void(*)(void*, VisualControl::GameVisualControl*))GameFuncs::ControlSliderSetSlideH)(controlSliderArray[0], charmngr);

        string16 msg;
        Utils::ConvertUTF8ToUTF16(msg, NAME("charman"));
        u32 textElementHandle = charmngr->GetNwlytControl()->vtable->getElementHandle(charmngr->GetNwlytControl(), SafeStringBase("T_mngr"), 0);
        charmngr->GetNwlytControl()->vtable->replaceMessageImpl(charmngr->GetNwlytControl(), textElementHandle, VisualControl::Message(msg.c_str()), nullptr, nullptr);
    }

    void MenuPageHandler::MenuSingleCharaPage::OnPageEnter(GameSequenceSection* own) {
        isInSingleCharaPage = true;
        pageEnterBackup(own);
    }

    void MenuPageHandler::MenuSingleCharaPage::OnPageExit(GameSequenceSection* own) {
        pageExitBackup(own);
        isInSingleCharaPage = false;
    }

    static bool g_pageprestepcallbackadded = false;
    static void OnPagePreStepCallback() {
	    extern MenuEntry *customCharactersEntry;
        *(PluginMenu::GetRunningInstance()) -= OnPagePreStepCallback;
		Process::Pause();
        SoundEngine::PlayMenuSound(SoundEngine::Event::ACCEPT);
        CharacterManager::characterManagerSettings(customCharactersEntry);
		Process::Play();
        g_pageprestepcallbackadded = false;
    }

    void MenuPageHandler::MenuSingleCharaPage::OnPagePreStep(GameSequenceSection* own) {
        pagePreStepBackup(own);
        if (Controller::IsKeyPressed(Key::Select) && !g_pageprestepcallbackadded) {
            g_pageprestepcallbackadded = true;
           *(PluginMenu::GetRunningInstance()) += OnPagePreStepCallback;
        }
    }

    void MenuPageHandler::InitHooksFromSingleCupGP(u32 sectionVtable) {
    }

    void MenuPageHandler::InitHooksFromSingleCup(u32 sectionVtable) {
    }

    void MenuPageHandler::InitHooksFromSingleCourse(u32 sectionVtable) {
    }

    void MenuPageHandler::InitHooksFromSingleChara(u32 sectionVtable) {
    }

    VisualControl::GameVisualControl* MenuPageHandler::MenuSingleCourseBasePage::GenerateCTPreview(GameSequenceSection* own, bool isTopScreen) {
        return ((MenuSingleCourseBasePage*)own->vtable->userData)->ctPreview.GetControl(own, isTopScreen);
    }

    void MenuPageHandler::OnMenuSingleModeDeallocate(GameSequenceSection* own) {
        if (!menuSingleMode) return;
        MenuSingleModePage* page = (MenuSingleModePage*)own->vtable->userData;
        page->Deallocate();
        delete menuSingleMode;
        menuSingleMode = nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadSingleCupGPMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuSingleCupGP) {
            menuSingleCupGP = new MenuSingleCupGPPage();
            return menuSingleCupGP->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadMultiCupGPMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuMultiCupGP) {
            menuMultiCupGP = new MenuMultiCupGPPage();
            return menuMultiCupGP->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadSingleCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuSingleCup) {
            menuSingleCup = new MenuSingleCupPage();
            return menuSingleCup->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadMultiCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuMultiCup) {
            menuMultiCup = new MenuMultiCupPage();
            return menuMultiCup->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadWifiCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuWifiCup) {
            menuWifiCup = new MenuWifiCupPage();
            return menuWifiCup->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadSingleCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuSingleCourse) {
            menuSingleCourse = new MenuSingleCoursePage();
            return menuSingleCourse->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadMultiCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuMultiCourse) {
            menuMultiCourse = new MenuMultiCoursePage();
            return menuMultiCourse->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadWifiCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuWifiCourse) {
            menuWifiCourse = new MenuWifiCoursePage();
            return menuWifiCourse->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadMultiCourseVoteMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuMultiCourseVote) {
            menuMultiCourseVote = new MenuMultiCourseVote();
            return menuMultiCourseVote->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadWifiCourseVoteMenu(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuWifiCourseVote) {
            menuWifiCourseVote = new MenuWifiCourseVote();
            return menuWifiCourseVote->Load(sectionDirector);
        }
        return nullptr;
    }

    MenuPageHandler::GameSequenceSection* MenuPageHandler::LoadEndingPage(ExecutableSectionClassInfo* own, void* sectionDirector) {
        if (!menuEndingPage) {
            menuEndingPage = new MenuEndingPage();
            return menuEndingPage->Load(sectionDirector);
        }
        return nullptr;
    }

    void MenuPageHandler::OnMenuSingleCupGPDeallocate(GameSequenceSection* own) {
        if (!menuSingleCupGP) return;
        MenuSingleCupGPPage* page = (MenuSingleCupGPPage*)own->vtable->userData;
        page->Deallocate();
        delete menuSingleCupGP;
        menuSingleCupGP = nullptr;
    }

    void MenuPageHandler::OnMenuMultiCupGPDeallocate(GameSequenceSection* own) {
        if (!menuMultiCupGP) return;
        MenuMultiCupGPPage* page = (MenuMultiCupGPPage*)own->vtable->userData;
        page->Deallocate();
        delete menuMultiCupGP;
        menuMultiCupGP = nullptr;
    }

    void MenuPageHandler::OnMenuSingleCupDeallocate(GameSequenceSection* own) {
        if (!menuSingleCup) return;
        MenuSingleCupPage* page = (MenuSingleCupPage*)own->vtable->userData;
        page->Deallocate();
        delete menuSingleCup;
        menuSingleCup = nullptr;
    }

    void MenuPageHandler::OnMenuMultiCupDeallocate(GameSequenceSection* own) {
        if (!menuMultiCup) return;
        MenuMultiCupPage* page = (MenuMultiCupPage*)own->vtable->userData;
        page->Deallocate();
        delete menuMultiCup;
        menuMultiCup = nullptr;
    }

    void MenuPageHandler::OnMenuWifiCupDeallocate(GameSequenceSection* own) {
        if (!menuWifiCup) return;
        MenuWifiCupPage* page = (MenuWifiCupPage*)own->vtable->userData;
        page->Deallocate();
        delete menuWifiCup;
        menuWifiCup = nullptr;
    }

    void MenuPageHandler::OnMenuSingleCourseDeallocate(GameSequenceSection* own) {
        if (!menuSingleCourse) return;
        MenuSingleCoursePage* page = (MenuSingleCoursePage*)own->vtable->userData;
        page->Deallocate();
        delete menuSingleCourse;
        menuSingleCourse = nullptr;
    }

    void MenuPageHandler::OnMenuMultiCourseDeallocate(GameSequenceSection* own) {
        if (!menuMultiCourse) return;
        MenuMultiCoursePage* page = (MenuMultiCoursePage*)own->vtable->userData;
        page->Deallocate();
        delete menuMultiCourse;
        menuMultiCourse = nullptr;
    }

    void MenuPageHandler::OnMenuWifiCourseDeallocate(GameSequenceSection* own) {
        if (!menuWifiCourse) return;
        MenuWifiCoursePage* page = (MenuWifiCoursePage*)own->vtable->userData;
        page->Deallocate();
        delete menuWifiCourse;
        menuWifiCourse = nullptr;
    }

    void MenuPageHandler::OnMenuMultiCourseVoteDeallocate(GameSequenceSection* own) {
        if (!menuMultiCourseVote) return;
        MenuMultiCourseVote* page = (MenuMultiCourseVote*)own->vtable->userData;
        page->Deallocate();
        delete menuMultiCourseVote;
        menuMultiCourseVote = nullptr;
    }

    void MenuPageHandler::OnMenuWifiCourseVoteDeallocate(GameSequenceSection* own) {
        if (!menuWifiCourseVote) return;
        MenuWifiCourseVote* page = (MenuWifiCourseVote*)own->vtable->userData;
        page->Deallocate();
        delete menuWifiCourseVote;
        menuWifiCourseVote = nullptr;
    }

    void MenuPageHandler::OnMenuEndingPageDeallocate(GameSequenceSection* own) {
        if (!menuEndingPage) return;
        MenuEndingPage* page = (MenuEndingPage*)own->vtable->userData;
        page->Deallocate();
        delete page->staffroll;
        page->staffroll = nullptr;
        delete menuEndingPage;
        menuEndingPage = nullptr;
    }

    void MenuPageHandler::dashSectionDefinePageClassInfoList(void* dashSectionManager, void* sectionclassinfolist) {
        ExecutableSectionClassInfo* (*SectionClassInfoBaseSubObject)(ExecutableSectionClassInfo*) = (decltype(SectionClassInfoBaseSubObject))GameFuncs::SectionClassInfoBaseSubObject;
        void (*SectionClassInfoListAddSectionClassInfo)(void* classinfolist, ExecutableSectionClassInfo* info) = (decltype(SectionClassInfoListAddSectionClassInfo))GameFuncs::SectionClassInfoListAddSectionClassInfo;
        ExecutableSectionClassInfoVtable** vtableList = (ExecutableSectionClassInfoVtable**)classInfoVtableList;
        for (int i = 0; i < sizeof(ExecutableSectionClassInfoVtableList) / sizeof(ExecutableSectionClassInfoVtable*); i++) {
            ExecutableSectionClassInfo* info = (ExecutableSectionClassInfo*)GameAlloc::game_operator_new_autoheap(0x24);
            info = SectionClassInfoBaseSubObject(info);
            info->vtable = vtableList[i];
            SectionClassInfoListAddSectionClassInfo(sectionclassinfolist, info);
        }
    }

    void MenuPageHandler::InitHooksFromDefinePageClassInfoList(u32 funcaddr) {
    }   
}
