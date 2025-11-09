/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraUIElements.hpp
Open source lines: 523/523 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "VisualControl.hpp"
#include "MarioKartTimer.hpp"
#include "rt.hpp"
#include "ItemHandler.hpp"
#include "array"

namespace CTRPluginFramework {
    

    class SpeedometerController
    {
    public:
        enum class SpeedUnit {
            KMPH = 0,
            MPH = 1,
        };
        enum class SpeedMode {
            NUMERIC = 0,
            GRAPHICAL = 1,
            MKDD = 2
        };
    private:
        class Speedometer
        {
        public:
            Speedometer() : speedUnit(SpeedUnit::KMPH), speedValue(0), currentVehicleMove(0) {}
            virtual ~Speedometer() {};

            void SetUnit(SpeedUnit mode) {speedUnit = mode;}
            SpeedUnit GetUnit() {return speedUnit;}

            void SetVehicleMove();
            u32* GetVehicleMove() {return (u32*)currentVehicleMove;}
            float GetSpeed();
            float GetSpeedUnitMultiplier() {const float vals[2] = {10.f, 6.21371f}; return vals[(int)speedUnit];}

            virtual void OnPauseEvent(bool pauseOpen) = 0;
            virtual void OnRaceStart() {}
        private:
            SpeedUnit speedUnit;
            float speedValue;
            u32 currentVehicleMove;
        };

        class NumericSpeedometer : public Speedometer
        {
        private:
            VisualControl* vControl;
            u32 rootHandle;
            u32 number0Handle;
            u32 number1Handle;
            u32 number2Handle;
            u32 number3Handle;

            bool number1Visible;
            bool number2Visible;
            bool number3Visible;
            float prevNumber[4];
            u32 counter;

            static void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            static void Create(VisualControl::GameVisualControl* control, void* createArgs);
            static void Calc(VisualControl::GameVisualControl* control);
        public:
            void OnPauseEvent(bool pauseOpen) override;

            NumericSpeedometer(SpeedUnit unit);
            ~NumericSpeedometer();            
        };

        class GraphicalSpeedometer : public Speedometer
        {
        private:
            VisualControl* vControl;
            u32 rootHandle;
            u32 rotationHandle;

            u32 counter;

            static void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            static void Create(VisualControl::GameVisualControl* control, void* createArgs);
            static void Calc(VisualControl::GameVisualControl* control);
        public:
            void OnPauseEvent(bool pauseOpen) override;

            GraphicalSpeedometer(SpeedUnit unit);
            ~GraphicalSpeedometer();
        };

        class DoubleDashSpeedometer : public Speedometer
        {
        private:
            VisualControl* vControl;
            u32 rootHandle;
            u32 number1Handle;
            u32 number2Handle;

            enum TabColor {
                TAB_GREY = 0,
                TAB_BLUE,
                TAB_GREEN,
                TAB_GREENYELLOW,
                TAB_YELLOW,
                TAB_ORANGE,
                TAB_ORANGERED,
                TAB_RED,
                TAB_BLACK,
                TAB_PINK
            };

            enum class State {
                HIDDEN = 0,
                APPEARING = 1,
                NORMAL = 2,
                SPEED = 3,
                MEGA = 4,
                STAR = 5,
                BULLET = 6,
                THUNDER = 7
            };

            State state;
            State prevState;
            bool number1Visible;
            bool number2Visible;
            float prevNumber[3];

            bool scroll;
            bool cancelScroll;
            u32 currScroll;
            void StartScroll() {if (!scroll) {scroll = true; currScroll = 0; cancelScroll = false;}}
            void StopScroll(bool force);

            int tabColor[7];
            void ChangeTabColor(int tab, int color) {
                if (tabColor[tab] != color) {
                    tabColor[tab] = color;
                    if (!scroll) vControl->GetGameVisualControl()->GetAnimationFamily(tab + 4)->SetAnimation(0, color);
                }
            }

            u32 counter;
            u32 appearingCounter;
            u32 thunderCounter;
            bool playedThunder;

            static void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            static void Create(VisualControl::GameVisualControl* control, void* createArgs);
            static void Reset(VisualControl::GameVisualControl* control);
            static void Calc(VisualControl::GameVisualControl* control);

            void SetNumSpeed(int speed);
            void SetNumError();
            void CalcScrollingAnim();
        public:
            void OnPauseEvent(bool pauseOpen) override;
            void OnRaceStart() override;

            DoubleDashSpeedometer(SpeedUnit unit);
            ~DoubleDashSpeedometer();
        }; 


        SpeedometerController();
        ~SpeedometerController();
        static Speedometer* speedometer;

        friend NumericSpeedometer;
        static void OnDeallocate(VisualControl* control);
    public:
        static void Load();
        static void OnPauseEvent(bool pauseOpen) {if (speedometer) speedometer->OnPauseEvent(pauseOpen);}
        static void OnRaceStart() {if (speedometer) speedometer->OnRaceStart();}
    };

    class AutoAccelerationController {
    public:
        static void initAutoAccelIcon();
        static void OnPauseEvent(bool pauseOpen);
        static VisualControl* autoAccelControl;
        static bool prevIsAutoAccel;
        static u32 rootHandle;
    };

    class MusicCreditsController {
    public:
        static void Init();
        static VisualControl* musicCreditsControl;
        static u32 rootHandle;
        static MarioKartTimer timer;
        static bool hasData;
        static constexpr MarioKartTimer animTimer{0, 0, 400};
        static void OnRaceStart() {
            timer = MarioKartTimer(0, 3, 500) + animTimer;
        }
    };

    class ExtendedItemBoxController {
    public:
        enum class AmountMode {
            AMOUNT_NONE,
            AMOUNT_2X,
            AMOUNT_3X,
            AMOUNT_FRENZY,

            AMOUNT_INVALID,
        };
        static AmountMode nextAmountMode;
        static RT_HOOK ItemBoxControlOnCalcHook;
        static RT_HOOK ItemBoxControlOnCreateHook;
        static u32 progBarHandle;
        static u32 progWinHandle;
        static u32 progBGHandle;
        static u32 amountHandle;
        static float prevProgressbarValue;
        static float prevVisibilityValue;
        static float targetProgressValue;
        static float currentProgressValue;
        static bool prevIsBoxVisible;
        static bool prevIsBoxDecided;
        static int visibilityAnimFrames;
        static int konohaTimer;
        static float killerProgress;

        static void SetNextAmountMode(AmountMode mode) {
            nextAmountMode = mode;
        }
        static void NotifyItemBoxStarted() {
            prevIsBoxVisible = false;
            prevIsBoxDecided = false;
        }

        static void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
        static void OnCreate(VisualControl::GameVisualControl* visualControl, void* createArg);
        static void OnCalc(VisualControl::GameVisualControl* visualControl);
    
    private:
        static void SetProgressBar(VisualControl::GameVisualControl* visualControl, float progress);
        static void SetVisibility(VisualControl::GameVisualControl* visualControl, float progress);

        static void ChangeVisibilityState(bool visible);
        static void ChangeProgress(float progress);
        static void CalcVisibility(VisualControl::GameVisualControl* visualControl);
        static void CalcProgress(VisualControl::GameVisualControl* visualControl);

        static void ChangeAmountMode(VisualControl::GameVisualControl* visualControl, AmountMode mode);

        static EItemSlot GetCurrentItem(VisualControl::GameVisualControl* visualControl) {
            // 0x9C -> kart item proxy
            return ((EItemSlot****)visualControl)[0x9C/4][0][0x34/4][0x30/4];
        }
    };

    class PointsModeDisplayController {
    public:
        enum class CongratState : u8 {
            NONE = 0,
            NICE = 1,
            GREAT = 2,
            EXCELLENT = 3,
            FANTASTIC = 4,
        };

        PointsModeDisplayController();
        ~PointsModeDisplayController() {
            delete vControl;
        }
        void OnRaceStart() {
            currState = State::APPEARING;
        }

        // Number
        void GotoNumber(u32 number, const MarioKartTimer& timer = MarioKartTimer(0, 2, 0)) {
            numberController.GotoNumber(number, timer);
        }
        void SetNumberDisabled(bool disabled) {
            numberController.SetDisabled(disabled);
        }

        // Reason
        void SetReason(const std::u16string& reason, int amount, const MarioKartTimer& time = MarioKartTimer(0, 3, 0)) {
            reasonController.SetReason(reason, amount, time);
        }

        // Combo
        void SetCombo(u32 combo) {
            comboCongratController.SetCombo(combo);
        }

        // Congrat
        void SetCongrat(CongratState state) {
            comboCongratController.SetCongrat(state);
        }

        static void InitializeText() {
            ComboCongratController::InitializeText();
        }

    private:
        enum class AppearRequest {
            NONE = 0,
            APPEAR = 1,
            DISAPPEAR = 2,
        };
        struct AnimFamilyID {
            enum {
                ALL_IN,

                NUM_0_PAT,
                NUM_1_PAT,
                NUM_2_PAT,
                NUM_3_PAT,
                NUM_4_PAT,

                NUM_0_COL,
                NUM_1_COL,
                NUM_2_COL,
                NUM_3_COL,
                NUM_4_COL,

                NUM_0_BNC,
                NUM_1_BNC,
                NUM_2_BNC,
                NUM_3_BNC,
                NUM_4_BNC,

                REASON_IN,
                COMBO_IN,
                CONGRAT_IN,
                CONGRAT_COL,

                SIZE,
            };
        };

        enum class State {
            HIDDEN,
            APPEARING,
            SHOWED,
        };
        State currState = State::HIDDEN;
        MarioKartTimer stateTimer = 0;

        u32 rootHandle;

        VisualControl* vControl;
        static void OnDeallocate(VisualControl* v);

        void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
        void Create(VisualControl::GameVisualControl* control, void* createArgs);
        void Reset(VisualControl::GameVisualControl* control);
        void Calc(VisualControl::GameVisualControl* control);

        class NumberController {
        public:
            static constexpr int NUMBER_AMOUNT = 5;
            enum class ColorState {
                ORANGE,
                PINK,
                GREY,
            };
        private:
            enum class BounceState {
                IDLE,
                STARTING,
                STOPPING,
            };
            BounceState currBounceState = BounceState::IDLE;
            BounceState nextBounceState = BounceState::IDLE;
            int bounceTimer = 0;
            bool isBouncing[NUMBER_AMOUNT] = { 0 };
            
            ColorState currentColor = ColorState::ORANGE;
            ColorState requestColor = ColorState::ORANGE;
            bool forcedGrey = false;

            u32 requestedNumber = 0, currentNumber = 0, fromGotoNumber = 0, targetGotoNumber = 0;
            u32 numberHandle[NUMBER_AMOUNT] = { 0 };
            u8 numberValue[NUMBER_AMOUNT] = { 10, 10, 10, 10, 10 };
            bool numberHidden[NUMBER_AMOUNT] = { 0 };
            MarioKartTimer fromGotoTimer = 0, targetGotoTimer;

            void SetNumberImpl(VisualControl::GameVisualControl* control, u32 number);

            void StartBounce() {
                if (currBounceState != BounceState::STARTING) {
                    bounceTimer = 0;
                    nextBounceState = BounceState::STARTING;
                }
            }

            void EndBounce() {
                if (currBounceState != BounceState::STOPPING) {
                    nextBounceState = BounceState::STOPPING;
                }
            }

            void SetNumber(u32 number) {
                requestedNumber = number;
            }

        public:
            void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            void Create(VisualControl::GameVisualControl* control, void* createArgs);
            void Reset(VisualControl::GameVisualControl* control);
            void Calc(VisualControl::GameVisualControl* control);

            void GotoNumber(u32 number, const MarioKartTimer& timer = MarioKartTimer(0, 2, 0));
            void SetDisabled(bool disabled) {forcedGrey = disabled;}
        };
        NumberController numberController;

        class ReasonController {
        private:
            u32 reasonHandle;
            u32 amountHandle;

            void SetReasonImpl(VisualControl::GameVisualControl* control, const std::u16string& reason);
            void SetAmountImpl(VisualControl::GameVisualControl* control, int amount);

            void SetReason(const std::u16string& reason) {
                if (reason.empty())
                    requestedReason = u" ";
                else
                    requestedReason = reason;
            }
            void SetAmount(int amount) {
                requestedAmount = amount;
            }
            void Appear() {
                appearRequest = AppearRequest::APPEAR;
            }
            void Disappear() {
                appearRequest = AppearRequest::DISAPPEAR;
            }

            AppearRequest appearRequest;
            std::u16string requestedReason;
            int requestedAmount, currentAmount;
            MarioKartTimer appearTimer = 0;
        public:
            void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            void Create(VisualControl::GameVisualControl* control, void* createArgs);
            void Reset(VisualControl::GameVisualControl* control);
            void Calc(VisualControl::GameVisualControl* control);

            void SetReason(const std::u16string& reason, int amount, const MarioKartTimer& time = MarioKartTimer(0, 3, 0));
        };
        ReasonController reasonController;

        class ComboCongratController {
        public:
            enum class CongratColor : s32 {
                NONE = 0,
                GREEN = 1,
                YELLOW = 2,
                PINK = 3,
                RAINBOW = 4,
            };
        private:
            static std::array<std::u16string, 4> congratNames;
            static std::u16string comboName;

            u32 comboHandle;
            u32 congratHandle;

            std::u16string requestedCombo;
            std::u16string requestedCongrat;

            CongratColor currentColor, requestedColor;

            bool isComboAppeared = false;
            AppearRequest comboAppearRequest = AppearRequest::NONE;
            AppearRequest congratAppearRequest = AppearRequest::NONE;

            void SetComboText(const std::u16string& combo) {
                if (combo.empty())
                    requestedCombo = u" ";
                else
                    requestedCombo = combo;
            }
            void SetCongratText(const std::u16string& congrat) {
                if (congrat.empty())
                    requestedCongrat = u" ";
                else
                    requestedCongrat = congrat;
            }

            void SetColor(CongratColor color) {
                requestedColor = color;
            }

            void AppearCombo() {comboAppearRequest = AppearRequest::APPEAR;}
            void DisappearCombo() {comboAppearRequest = AppearRequest::DISAPPEAR;}
            void AppearCongrat() {congratAppearRequest = AppearRequest::APPEAR;}
            void DisappearCongrat() {congratAppearRequest = AppearRequest::DISAPPEAR;}

            void SetCongratComboImpl(VisualControl::GameVisualControl* control, const std::u16string& text, bool isCombo);
        public:
            void DefineAnimation(VisualControl::AnimationDefine* animationDefine);
            void Create(VisualControl::GameVisualControl* control, void* createArgs);
            void Reset(VisualControl::GameVisualControl* control);
            void Calc(VisualControl::GameVisualControl* control);

            void SetCongrat(CongratState state);
            void SetCombo(u32 combo);

            static void InitializeText();
        };
        ComboCongratController comboCongratController;
    };
}