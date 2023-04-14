/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraUIElements.hpp
Open source lines: 205/205 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "VisualControl.hpp"
#include "MarioKartTimer.hpp"

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
}