/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MenuPage.hpp
Open source lines: 768/768 (100.00%)
*****************************************************/

#pragma once
#include "rt.hpp"
#include "CTRPluginFramework.hpp"
#include "VisualControl.hpp"
#include "DataStructures.hpp"
#include "CTPreview.hpp"
#include "MarioKartTimer.hpp"

namespace CTRPluginFramework {

    class StaffRoll;

    class MenuPageHandler {
    public:
        
        struct CursorItem
        {
            u32 unknown1;
            u32 unknown2;
            u32 elementID;
        };
        
        struct CursorMove {
            enum KeyType {
                KEY_RIGHT = (1 << 0),
                KEY_LEFT = (1 << 1),
                KEY_UP = (1 << 2),
                KEY_DOWN = (1 << 3),
                KEY_NONE = 0x10000000
            };
            u32 cursorType;
            u32 unknown1;
            CursorMove* own;
            int(*keyHandler)(CursorMove* own, CursorItem* cursorItem);
            u32 keyHandlerArg;
            u32 itemCount;
            u32 unknown2;
            bool allowHorizontalLoop;
            bool allowVerticalLoop;
            u32 prevButton;

            KeyType GetDir(CursorItem* cursorItem) {
                return ((KeyType(*)(CursorMove*, CursorItem*))GameFuncs::CursorMoveGetDir)(this, cursorItem);
            } 
        };

        struct GameSequenceSection;

        struct ExecutableSectionClassInfo;

        struct ExecutableSectionClassInfoVtable {
            void (*getDTIClassInfo)(ExecutableSectionClassInfo* own);
            void (*getDTIClass)(ExecutableSectionClassInfo* own);
            u32 null0;
            u32 (*convertMode)(ExecutableSectionClassInfo* own, const SafeStringBase* mode);
            u32 (*convertEnterCode)(ExecutableSectionClassInfo* own, const SafeStringBase* mode);
            char* (*convertReturnCode)(ExecutableSectionClassInfo* own, int code);
            GameSequenceSection* (*constructSection)(ExecutableSectionClassInfo* own, void* sectionDirector);
        };

        struct ExecutableSectionClassInfo {
            ExecutableSectionClassInfoVtable* vtable;
        };

        struct ExecutableSectionClassInfoVtableList {
            ExecutableSectionClassInfoVtable* exec_titledemopage;
            ExecutableSectionClassInfoVtable* exec_demopage;
            ExecutableSectionClassInfoVtable* exec_winningrunpage;
            ExecutableSectionClassInfoVtable* exec_baseracepage;
            ExecutableSectionClassInfoVtable* exec_racepage;
            ExecutableSectionClassInfoVtable* exec_trophypage;
            ExecutableSectionClassInfoVtable* exec_endingpage;
            ExecutableSectionClassInfoVtable* exec_thankyoupage;
            ExecutableSectionClassInfoVtable* exec_menutitle;
            ExecutableSectionClassInfoVtable* exec_menusinglemode;
            ExecutableSectionClassInfoVtable* exec_menusingleclass;
            ExecutableSectionClassInfoVtable* exec_menusinglechara;
            ExecutableSectionClassInfoVtable* exec_menusinglekart;
            ExecutableSectionClassInfoVtable* exec_menusinglecup;
            ExecutableSectionClassInfoVtable* exec_menusinglecupgp;
            ExecutableSectionClassInfoVtable* exec_menusinglecourse;
            ExecutableSectionClassInfoVtable* exec_menusinglecoursebattle;
            ExecutableSectionClassInfoVtable* exec_menusinglesetting;
            ExecutableSectionClassInfoVtable* exec_menusingleghost;
            ExecutableSectionClassInfoVtable* exec_menusingleghostload;
            ExecutableSectionClassInfoVtable* exec_timeattackchart;
            ExecutableSectionClassInfoVtable* exec_MenuUpBarController;
            ExecutableSectionClassInfoVtable* exec_menumultigroup;
            ExecutableSectionClassInfoVtable* exec_menumultiwaiting;
            ExecutableSectionClassInfoVtable* exec_menumultidlpsequence;
            ExecutableSectionClassInfoVtable* exec_menumultimode;
            ExecutableSectionClassInfoVtable* exec_menumulticlass;
            ExecutableSectionClassInfoVtable* exec_menumultichara;
            ExecutableSectionClassInfoVtable* exec_menumultikart;
            ExecutableSectionClassInfoVtable* exec_menumultivssetting;
            ExecutableSectionClassInfoVtable* exec_menumultibattlesetting;
            ExecutableSectionClassInfoVtable* exec_menumulticup;
            ExecutableSectionClassInfoVtable* exec_menumulticupgp;
            ExecutableSectionClassInfoVtable* exec_menumulticourse;
            ExecutableSectionClassInfoVtable* exec_menumulticoursebattle;
            ExecutableSectionClassInfoVtable* exec_menumulticoursevote;
            ExecutableSectionClassInfoVtable* exec_menumulticoursedl;
            ExecutableSectionClassInfoVtable* exec_menuwificonnect;
            ExecutableSectionClassInfoVtable* exec_menuwifimodeopponent;
            ExecutableSectionClassInfoVtable* exec_menuwifimode;
            ExecutableSectionClassInfoVtable* exec_menuwifichara;
            ExecutableSectionClassInfoVtable* exec_menuwifikart;
            ExecutableSectionClassInfoVtable* exec_menuwifiwaiting;
            ExecutableSectionClassInfoVtable* exec_menuwifiwaitingdecide;
            ExecutableSectionClassInfoVtable* exec_menuwificonfirm;
            ExecutableSectionClassInfoVtable* exec_menuwificup;
            ExecutableSectionClassInfoVtable* exec_menuwificourse;
            ExecutableSectionClassInfoVtable* exec_menuwificoursebattle;
            ExecutableSectionClassInfoVtable* exec_menuwificoursevote;
            ExecutableSectionClassInfoVtable* exec_menuwififriend;
            ExecutableSectionClassInfoVtable* exec_menucommunityenter;
            ExecutableSectionClassInfoVtable* exec_menucommunitytop;
            ExecutableSectionClassInfoVtable* exec_menucommunitylobby;
            ExecutableSectionClassInfoVtable* exec_menucommunitysearch;
            ExecutableSectionClassInfoVtable* exec_menucommunitysearchconfirm;
            ExecutableSectionClassInfoVtable* exec_menucommunitycreatemark;
            ExecutableSectionClassInfoVtable* exec_menucommunitycreatename;
            ExecutableSectionClassInfoVtable* exec_menucommunitycreaterule;
            ExecutableSectionClassInfoVtable* exec_menucommunitycreatecomment;
            ExecutableSectionClassInfoVtable* exec_menucommunitycreatefinish;
            ExecutableSectionClassInfoVtable* exec_menucommunitychara;
            ExecutableSectionClassInfoVtable* exec_menuchanneltop;
            ExecutableSectionClassInfoVtable* exec_menuchannelshowmii;
            ExecutableSectionClassInfoVtable* exec_menuchannelcecmii;
            ExecutableSectionClassInfoVtable* exec_menuchannelceclist;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetuptop;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetupmii;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetupcomment;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetupmachine;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetupgp;
            ExecutableSectionClassInfoVtable* exec_menuchannelsetupsetting;
            ExecutableSectionClassInfoVtable* exec_commonsystemdialog;
            ExecutableSectionClassInfoVtable* exec_bgpage;
            ExecutableSectionClassInfoVtable* exec_faderpage;
            ExecutableSectionClassInfoVtable* exec_mmencheckpage;
            ExecutableSectionClassInfoVtable* exec_timerpage;
        };

        static ExecutableSectionClassInfoVtableList* classInfoVtableList;

        struct GameSequenceSectionVtable { // NOTE: Return types are not correct
            void (*getDTIClassInfo)(GameSequenceSection* own);
            void (*getDTIClass)(GameSequenceSection* own);
            void (*_destructor)(GameSequenceSection* own);
            void (*_deallocating)(GameSequenceSection* own);
            void (*create)(GameSequenceSection* own, const void* arg);
            void (*init)(GameSequenceSection* own);
            void (*calc)(GameSequenceSection* own);
            void (*render)(GameSequenceSection* own);
            void (*renderMainL)(GameSequenceSection* own);
            void (*renderMainR)(GameSequenceSection* own);
            void (*renderSub)(GameSequenceSection* own);
            u32 null1;
            void (*accept)(GameSequenceSection* own, void* visitor);
            void (*callbackInvokeEventID)(GameSequenceSection* own, int event);
            u32 null2;
            void (*createOuter)(GameSequenceSection* own, const void* unk);
            void (*initOuter)(GameSequenceSection* own);
            u32 null3;
            void (*destroy)(GameSequenceSection* own);
            void (*getSectionType)(GameSequenceSection* own);
            void (*isCompletable)(GameSequenceSection* own);
            void (*isSyncFadein)(GameSequenceSection* own);
            void (*getFadeDelay)(GameSequenceSection* own);
            void (*updateState)(GameSequenceSection* own);
            void (*step)(GameSequenceSection* own);
            void (*ready)(GameSequenceSection* own);
            void (*enter)(GameSequenceSection* own, u32& fadeKind, u32 unkwnown);
            void (*standby)(GameSequenceSection* own);
            void (*start)(GameSequenceSection* own);
            void (*complete)(GameSequenceSection* own);
            void (*cancel)(GameSequenceSection* own, u32& fadeKind, u32 unknown);
            void (*finish)(GameSequenceSection* own, u32& fadeKind, u32 unknown);
            void (*reenter)(GameSequenceSection* own);
            void (*exit)(GameSequenceSection* own);
            void (*clear)(GameSequenceSection* own);
            void (*sceneStart)(GameSequenceSection* own, int code);
            void (*sceneFinish)(GameSequenceSection* own, int code);
            void (*onDestroy)(GameSequenceSection* own);
            void (*onReady)(GameSequenceSection* own);
            void (*onClear)(GameSequenceSection* own);
            void (*pageStep)(GameSequenceSection* own, bool unknown);
            void (*onCreate)(GameSequenceSection* own);
            void (*onGenerateControl)(GameSequenceSection* own, void* controlInitializer);
            void (*onPagePreStep)(GameSequenceSection* own);
            void (*onPagePostStep)(GameSequenceSection* own);
            void (*onPageEnter)(GameSequenceSection* own);
            void (*onPageStandby)(GameSequenceSection* own);
            void (*onPageStart)(GameSequenceSection* own);
            void (*onPageComplete)(GameSequenceSection* own);
            void (*onPageFinish)(GameSequenceSection* own);
            void (*onPageCancel)(GameSequenceSection* own);
            void (*onPageReenter)(GameSequenceSection* own);
            void (*onPageExit)(GameSequenceSection* own);
            void (*canFinishFadein)(GameSequenceSection* own);
            void (*canFinishFadeout)(GameSequenceSection* own);
            void (*completePage)(GameSequenceSection* own, int code);
            void (*getFadeinType)(GameSequenceSection* own, int unknown);
            void (*getFadeoutType)(GameSequenceSection* own, int unknown);
            void (*initControl)(GameSequenceSection* own);
            void (*calcItemIcon)(GameSequenceSection* own);
            void (*onPageFadein)(GameSequenceSection* own);
            void (*onPageFadeout)(GameSequenceSection* own);
            void (*enterCursor)(GameSequenceSection* own, int cursor);
            void (*buttonHandler_SelectOn)(GameSequenceSection* own, int id);
            void (*buttonHandler_OK)(GameSequenceSection* own, int id);
            void (*inputHandler)(GameSequenceSection* own, int unk1, int unk2);
            void (*procOpenMenu)(GameSequenceSection* own);
            void (*procCloseMenu)(GameSequenceSection* own);
            void (*procExitMenu)(GameSequenceSection* own);
            void (*onMenuEnter)(GameSequenceSection* own);
            void (*onMenuStart)(GameSequenceSection* own);
            void (*onMenuComplete)(GameSequenceSection* own);
            void (*onMenuExit)(GameSequenceSection* own);
            void (*onSliderSetting)(GameSequenceSection* own, bool unk1, bool unk2);
            void (*canStartSlideIn)(GameSequenceSection* own);
            void (*canStartSlideOut)(GameSequenceSection* own);
            void (*onTimeUp)(GameSequenceSection* own, int unknown);
            void (*onTimeUpComplete)(GameSequenceSection* own, int unknown);
            void (*onTimeUpCompleteStep)(GameSequenceSection* own, int unknown);
            void (*getBackEnterCode)(GameSequenceSection* own);
            void (*getBackReturnCode)(GameSequenceSection* own);
            // This are only present in network pages, but I'll place them here anyways
            void (*onPagePreStepCore)(GameSequenceSection* own);
            void (*onPageEnterCore)(GameSequenceSection* own);
            void (*onPageCompleteCore)(GameSequenceSection* own);
            void (*onCompleteNetwork)(GameSequenceSection* own);
            void* userData;
            u32 null5;
        };

        struct GameSequenceSection {
            GameSequenceSectionVtable* vtable;

            SeadArrayPtr<VisualControl::GameVisualControl*>& GetButtonArray(u32 offset) {
                return *(SeadArrayPtr<VisualControl::GameVisualControl*>*)((u32)this + offset);
            }

            u32* GetUIManipulator() {
                return ((u32***)this)[0x88/4][0];
            }

            u32 GetLastSelectedButton() {
                return GetUIManipulator()[0x100/4];
            }

            void SetLastSelectedButton(u32 button) {
                GetUIManipulator()[0x100/4] = button;
            }

            CursorMove* GetControlMove() {
                return (CursorMove*)(GetUIManipulator() + 0xDC/4);
            } 
        };

        class MenuSingleModePage {
            public:
                
                MenuSingleModePage() {}

                GameSequenceSection* gameSection;
                VisualControl::GameVisualControl* buttonList[6];
                GameSequenceSectionVtable vtable;

                GameSequenceSection* Load(void* sectionDirector);

                void Deallocate() {deallocatingBackup(gameSection);}
                static bool DefineAnimation(VisualControl::AnimationDefine* anDefine);

            private:
                GameSequenceSection* MenuSingleModePageCons(GameSequenceSection* own);
                static int HandleCursor(CursorMove* move, CursorItem* item);
                static void OnPageEnter(GameSequenceSection* own);
                static void OnSelect(GameSequenceSection* own, int selected);
                static void OnOK(GameSequenceSection* own, int selected);
                static void InitControl(GameSequenceSection* own);
                void (*deallocatingBackup)(GameSequenceSection*);
                void (*buttonOKBackup)(GameSequenceSection*, int);
                static bool IsButtonConstructing;
        };

        class MenuSingleCupBasePage {
            public:
                enum class ConveyorState {
                    STOPPED,
                    GOING,
                };
                GameSequenceSection* gameSection;
                GameSequenceSectionVtable vtable;
                GameSequenceSection* MenuSingleCupBasePageCons(GameSequenceSection* own);
                bool isInPage = false;

                static RT_HOOK onPageEnterHook;
                static RT_HOOK onPagePreStepHook;
                static RT_HOOK initControlHook;
                static RT_HOOK buttomHandlerOKHook;

                VisualControl::GameVisualControl* buttonList[10];
                bool buttonEnabledState[10] = {0};
                VisualControl::GameVisualControl* cupBgControl;
                VisualControl::GameVisualControl* backButtonControl = nullptr;

                float conveyorTimer = 100.f;
                float conveyorIncrement = 0.f;
                float lastConveyorTimer = 0.f;
                const float valueConveyorFrames = 8.f;
                const int valueConveyorBlockedFrames = 4;
                int conveyorBlockedFrames = 0;
                int prevSelectedButton = -1;
                bool forceSelectOn = false;
                bool hasRandomButton = false;
                bool comesFromOK = false;
                bool autoButtonPressed = false;
                ConveyorState conveyorState = ConveyorState::STOPPED;
                bool StartConveyor(bool direction);
                void CalcConveyor();
                void OnConveyorEnd(bool direction);
                void SetCupButtonVisible(int buttonID, bool isVisible);
                void SetCupButtonBG(int buttonID, u32 paneID);
                void SetButtonEnabledState(int buttonID, bool state);
                void UpdateButtonEnabledState();
                void UpdateCupButtonState(int mode); // 0 -> Normal, 1 -> Right Scroll, 2 -> Left Scroll
                static int HandleCursor(CursorMove* move, CursorItem* item, bool isMulti);
                static int startingButtonID;
                static int selectedCupIcon;
                static void CupSelectBGControlAnimationDefine(VisualControl::AnimationDefine* obj);
                static const VisualControl::AnimationDefineVtable cupSelectBGAnimationDefineVtable;
                
                CTPreview ctPreview;
                int ctPreviewFrameCounter;
                int ctPreviewCurrTrack;
                int ctPreviewChildTrack = 0;
                int ctPreviewChildAnim;
                
                static void InitControl(GameSequenceSection* own);
                static void OnPageEnter(GameSequenceSection* own);
                static void EnterCursor(GameSequenceSection* own, int cursor);
                static void changeCup(GameSequenceSection* own, int cupID);
                static void OnPagePreStep(GameSequenceSection* own);
                static void ButtonHandler_OK(GameSequenceSection* own, int buttonID);
                static void ButtonHandler_SelectOn(GameSequenceSection* own, int buttonID);
                static void OnPageExit(GameSequenceSection* own, bool isCupBase);
                void (*deallocatingBackup)(GameSequenceSection*);
                void Deallocate() {deallocatingBackup(gameSection);}
        };

        class MenuSingleCupGPPage : public MenuSingleCupBasePage{
            public:
                
                GameSequenceSection* Load(void* sectionDirector);
                static MenuSingleCupGPPage* GetInstace() {return menuSingleCupGP;}

                static RT_HOOK onPagePreStepHook;
                static RT_HOOK buttonHandlerOKHook;
                
                static void OnPagePreStep(GameSequenceSection* own);
                static void ButtonHandler_OK(GameSequenceSection* own, int buttonID);
                static void OnPageComplete(GameSequenceSection* own);

            private:
                GameSequenceSection* MenuSingleCupGPPageCons(GameSequenceSection* own);
        };

        class MenuMultiCupGPPage : public MenuSingleCupBasePage{
            public:
                
                GameSequenceSection* Load(void* sectionDirector);
                static MenuMultiCupGPPage* GetInstace() {return menuMultiCupGP;}

                static void OnPageComplete(GameSequenceSection* own);
                void (*OnPageCompleteBackup)(GameSequenceSection* own);

            private:
                GameSequenceSection* MenuMultiCupGPPageCons(GameSequenceSection* own);
        };

        class MenuSingleCupPage : public MenuSingleCupBasePage {
            public:
                
                GameSequenceSection* Load(void* sectionDirector);
                static MenuSingleCupPage* GetInstace() {return menuSingleCup;}

            private:
                GameSequenceSection* MenuSingleCupPageCons(GameSequenceSection* own);
        };

        class MenuMultiCupPage : public MenuSingleCupBasePage{
            public:
                
                GameSequenceSection* Load(void* sectionDirector);
                static MenuMultiCupPage* GetInstace() {return menuMultiCup;}

                static void InitControl(GameSequenceSection* own);
                static void OnPageEnter(GameSequenceSection* own);
                void(*onPageEnterBackup)(GameSequenceSection* own);

            private:
                GameSequenceSection* MenuMultiCupPageCons(GameSequenceSection* own);
        };

        class MenuWifiCupPage : public MenuSingleCupBasePage{
            public:
                
                GameSequenceSection* Load(void* sectionDirector);
                static MenuWifiCupPage* GetInstace() {return menuWifiCup;}

                static void OnPageEnter(GameSequenceSection* own);

            private:
                GameSequenceSection* MenuWifiCupPageCons(GameSequenceSection* own);
        };

        class MenuSingleCourseBasePage {
            public:
                GameSequenceSection* gameSection;
                GameSequenceSectionVtable vtable;

                MenuSingleCupBasePage** parent;
                
                CTPreview ctPreview;
                static RT_HOOK onPageEnterHook;
                static RT_HOOK coursePageInitOmakaseTHook;
                static RT_HOOK coursePageInitOmakaseBHook;

                static void ClearBlockedCourses();
                static void AddBlockedCourse(u32 course);
                static bool IsBlockedCourse(u32 course);
                static std::vector<u32> blockedCourses;

                static void OnPageEnter(GameSequenceSection* own);
                static void OnPageExit(GameSequenceSection* own);

                void (*deallocatingBackup)(GameSequenceSection*);
                void Deallocate() {deallocatingBackup(gameSection);}
                
                static VisualControl::GameVisualControl* GenerateCTPreview(GameSequenceSection* own, bool isTopScreen);
        };

        class MenuSingleCoursePage : public MenuSingleCourseBasePage {
            public:
                GameSequenceSection* Load(void* sectionDirector);
                static MenuSingleCoursePage* GetInstace() {return menuSingleCourse;}

                static void buttonHandler_SelectOn(GameSequenceSection* own, int buttonID);

            private:
                void(*buttonHandler_SelectOnbackup)(GameSequenceSection* own, int buttonID);
        };

        class MenuMultiCoursePage : public MenuSingleCourseBasePage {
            public:
                GameSequenceSection* Load(void* sectionDirector);
                static MenuMultiCoursePage* GetInstace() {return menuMultiCourse;}

                static void buttonHandler_SelectOn(GameSequenceSection* own, int buttonID);

            private:
                void(*buttonHandler_SelectOnbackup)(GameSequenceSection* own, int buttonID);
        };

        class MenuWifiCoursePage : public MenuSingleCourseBasePage {
            public:
                GameSequenceSection* Load(void* sectionDirector);
                static MenuWifiCoursePage* GetInstace() {return menuWifiCourse;}

                static void buttonHandler_SelectOn(GameSequenceSection* own, int buttonID);

            private:
                void(*buttonHandler_SelectOnbackup)(GameSequenceSection* own, int buttonID);
        };

        class MenuSingleCourseBattle {
            public:
                static RT_HOOK onPageEnterHook;
                static void OnPageEnter(GameSequenceSection* own);
        };

        class MenuCourseVoteBase {
            public:

                GameSequenceSection* gameSection;
                GameSequenceSectionVtable vtable;

                CTPreview ctPreveiw;
                VisualControl::GameVisualControl* myOmakase;

                static void OnRevealCourseMoflex(GameSequenceSection* own);

                static void InitControl(GameSequenceSection* own);
                static void OnPageExit(GameSequenceSection* own);
                void(*initControlBackup)(GameSequenceSection* own);
                void(*onPageExitBackup)(GameSequenceSection* own);
                
                void (*deallocatingBackup)(GameSequenceSection*);
                void Deallocate() {deallocatingBackup(gameSection);}
        };

        class MenuMultiCourseVote : public MenuCourseVoteBase {
            public:
                GameSequenceSection* Load(void* sectionDirector);
        };

        class MenuWifiCourseVote : public MenuCourseVoteBase {
            public:
                GameSequenceSection* Load(void* sectionDirector);
        };

        class MenuTitlePage {
            public:
                static void OnInitControl(GameSequenceSection* own, VisualControl::GameVisualControl* buttons[3]);
                static int HandleCursor(CursorMove* move, CursorItem* item);
        };

        class MenuEndingPage {
            public:
                GameSequenceSection* gameSection;
                GameSequenceSectionVtable vtable;
                StaffRoll* staffroll = nullptr;

                GameSequenceSection* Load(void* sectionDirector);

                static void InitControl(GameSequenceSection* own);
                static void onPageEnter(GameSequenceSection* own);
                static void onPagePreStep(GameSequenceSection* own);
                static void onPageComplete(GameSequenceSection* own);
                static void onPageExit(GameSequenceSection* own);

                void (*deallocatingBackup)(GameSequenceSection*);
                void Deallocate() {deallocatingBackup(gameSection);}

                static bool loadCTGPCredits;
                static MenuEndingPage* GetInstance() {
                    return menuEndingPage;
                }
                static bool IsLoaded() {
                    return GetInstance() != nullptr;
                }
            private:
                
        };

        class MenuCharaBasePage {
            public:
                static EDriverID GetSelectedDriverID(GameSequenceSection* own) {
                    return (EDriverID)(((u32*)own)[0x354/4]);
                }
                static EDriverID GetDriverIDFromButton(GameSequenceSection* own, int button) {
                    SeadArray<EDriverID, 0x11>* arr = (SeadArray<EDriverID, 0x11>*)(((u32*)own) + 0x2A8/4);
                    return (*arr)[button];
                }
                static VisualControl::GameVisualControl* GetButtonFromDriverID(GameSequenceSection* own, EDriverID driverID) {
                    if (driverID == EDriverID::DRIVER_MIIM || driverID == EDriverID::DRIVER_MIIF) {
                        return (VisualControl::GameVisualControl*)((u32*)own)[0x350/4];
                    }

                    SeadArray<EDriverID, 0x11>* arr = (SeadArray<EDriverID, 0x11>*)(((u32*)own) + 0x2A8/4);
                    for (int i = 0; i < arr->count; i++) {
                        if ((*arr)[i] == driverID) {
                            return own->GetButtonArray(0x2F8)[i];
                        }
                    }
                    return nullptr;
                }
                static u64 GetSelectedCustomCharacterID(EDriverID driverID);
                void UpdateEntriesString();
                void UpdateDriverIcon(EDriverID driverID, bool forceReset = false);

                static RT_HOOK destructorHook;
                static RT_HOOK initControlHook;
                static RT_HOOK pageEnterHook;
                static RT_HOOK pageExitHook;
                static RT_HOOK pagePreStepHook;
                static RT_HOOK buttonHandlerSelectOnHook;
                static RT_HOOK buttonHandlerOKHook;
                static void (*deallocateBackup)(GameSequenceSection* own);
                static void (*multiOnTimeUpCompleteStepBackup)(GameSequenceSection* own, int unk);
                
                static void OnDestruct(GameSequenceSection* own);
                static void OnDeallocate(GameSequenceSection* own);
                static void OnInitControl(GameSequenceSection* own);
                static void OnPageEnter(GameSequenceSection* own);
                static void OnPageExit(GameSequenceSection* own);
                static void OnPagePreStep(GameSequenceSection* own);
                static void OnButtonHandlerSelectOn(GameSequenceSection* own, int buttonID);
                static void OnButtonHandlerOK(GameSequenceSection* own, int buttonID);
                static void OnTimeUpCompleteStep(GameSequenceSection* own, int unknown);
            private:
                VisualControl::GameVisualControl* charCountControl = nullptr;
                GameSequenceSection* own = nullptr;
                static VisualControl::GameVisualControlVtable* controlVtable;
                friend class CharacterHandler;
                static std::array<int, EDriverID::DRIVER_SIZE> currentChoices;
                MarioKartTimer loadAllIconsDelay;
                MarioKartTimer coolDownScrollTimer;
                MarioKartTimer reloadCharacterModelTimer;
        };

        static GameSequenceSection* LoadSingleModeMenu(ExecutableSectionClassInfo* own, void* sectionDirector);

        static GameSequenceSection* LoadSingleCupGPMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadMultiCupGPMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadSingleCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadMultiCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadWifiCupMenu(ExecutableSectionClassInfo* own, void* sectionDirector);

        static GameSequenceSection* LoadSingleCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadMultiCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadWifiCourseMenu(ExecutableSectionClassInfo* own, void* sectionDirector);

        static GameSequenceSection* LoadMultiCourseVoteMenu(ExecutableSectionClassInfo* own, void* sectionDirector);
        static GameSequenceSection* LoadWifiCourseVoteMenu(ExecutableSectionClassInfo* own, void* sectionDirector);

        static GameSequenceSection* LoadEndingPage(ExecutableSectionClassInfo* own, void* sectionDirector);
        class GameFuncs
        {
        public:
            static u32 SectionClassInfoBaseSubObject;
            static u32 SectionClassInfoListAddSectionClassInfo;

            static u32 SingleModeVtable;
            static u32 BaseMenuPageInitSlider;
            static u32 SetupControlMovieView;
            static u32 SetupControlMenuButton;
            static u32 SetupControlBackButton;
            static u32 MovieViewSetMoviePane;
            static u32 BaseMenuButtonControlSetTex;
            static u32 ControlSliderSetSlideH;
            static u32 initAnimationTouchSelect;

            static u32 CaptionAnimationDefineVtable;
            static u32 BaseMenuViewVtable;

            static u32 CursorMoveGetDir;

            static u32 MenuCaptionAnimIn;
            static u32 StartFadeIn;
            static u32 SetPlayerMasterID;
            static u32 SetCameraTargetPlayer;

            static u32 movieBasePageCons;
            static u32 menuSingleCupGPVtable;
            static u32 SequenceBasePageInitMenu;
            static u32 menuMultiCupGPVtable;
            static u32 menuSingleCupVtable;
            static u32 menuMultiCupVtable;
            static u32 MenuWifiCupVtable;

            static u32 MenuSingleCourseCons;
            static u32 MenuMultiCourseCons;
            static u32 MenuWifiCourseCons;
            
            static u32 MenuMultiCourseVoteCons;
            static u32 MenuWifiCourseVoteCons;

            static u32 setupOmakaseView;
            static u32 setupCourseName;
            static u32 setupCourseButtonDummy;
            static u32 setupRaceDialogButton;
            static u32 setupBackButtonBothControl;

            static u32 baseMenuButtonControlSetCursor;
            static u32 cursorMoveSetType;
            static u32 cupSelectBGVtable;
            static u32 setDelayH;
            static u32 cupButtonVtable;
            static u32 cupCursorAnimVtable;
            static u32 cupCursorVtable;
            static u32 setupOKButton2;

            static u32 moflexUpdateCup;
            static u32 MoflexEnablePane;
            static u32 MoflexReset;

            static u32 MenuCourseNameSet;

            static u32 UIManipulatorSetCursor;
            static u32 AnimationItemGetCurrentFrame;

            static u32 moflexGetSelectedCupCourse;
            static u32 buttonKeyHandlerCommon;
            static u32 controlSliderStepH;
            static u32 controlSliderStepV;
            static u32 basePageCalcNormalControl;

            static u32 MoflexInit;
            static u32 MoflexUpdateFrame;

            static u32 controlSliderStartH;
            static u32 controlSliderStartV;
            static u32 garageDirectorFade;

            static u32 SequenceStartFadeout;
            static u32 SequenceReserveFadeIn;

            static u32 moflexUpdateCourse;
            static u32 uiMovieViewAnimOut;

            static u32 omakaseViewVtable;
            static u32 BaseMenuButtonControlCons;

            static u32 creditsRollStaffTextVtable;
            static u32 BasicSoundStartPrepared;
            static u32 UItstDemoButton;
            static u32 BasePageCons;
            static u32 EndingPageVtable;
        };
        
        static void InitHooksFromSingleCupGP(u32 sectionVtable);
        static void InitHooksFromSingleCup(u32 sectionVtable);
        static void InitHooksFromSingleCourse(u32 sectionVtable);
        static void InitHooksFromSingleChara(u32 sectionVtable);

        static void dashSectionDefinePageClassInfoList(void* dashSectionManager, void* sectionclassinfolist);
        static void InitHooksFromDefinePageClassInfoList(u32 funcAddr);
    private:
        static MenuSingleModePage* menuSingleMode;

        static MenuSingleCupGPPage* menuSingleCupGP;
        static MenuMultiCupGPPage* menuMultiCupGP;
        static MenuSingleCupPage* menuSingleCup;
        static MenuMultiCupPage* menuMultiCup;
        static MenuWifiCupPage* menuWifiCup;

        static MenuSingleCoursePage* menuSingleCourse;
        static MenuMultiCoursePage* menuMultiCourse;
        static MenuWifiCoursePage* menuWifiCourse;

        static MenuMultiCourseVote* menuMultiCourseVote;
        static MenuWifiCourseVote* menuWifiCourseVote;

        static MenuEndingPage* menuEndingPage;

        static void OnMenuSingleModeDeallocate(GameSequenceSection* own);

        static void OnMenuSingleCupGPDeallocate(GameSequenceSection* own);
        static void OnMenuMultiCupGPDeallocate(GameSequenceSection* own);
        static void OnMenuSingleCupDeallocate(GameSequenceSection* own);
        static void OnMenuMultiCupDeallocate(GameSequenceSection* own);
        static void OnMenuWifiCupDeallocate(GameSequenceSection* own);

        static void OnMenuSingleCourseDeallocate(GameSequenceSection* own);
        static void OnMenuMultiCourseDeallocate(GameSequenceSection* own);
        static void OnMenuWifiCourseDeallocate(GameSequenceSection* own);

        static void OnMenuMultiCourseVoteDeallocate(GameSequenceSection* own);
        static void OnMenuWifiCourseVoteDeallocate(GameSequenceSection* own);

        static void OnMenuEndingPageDeallocate(GameSequenceSection* own);

        static RT_HOOK trophyPageSelectNextSceneHook;
        static RT_HOOK trophyPageSelectNextSceneHook2;
        static RT_HOOK thankyouPageInitControlHook;
    };
}