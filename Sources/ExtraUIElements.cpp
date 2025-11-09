/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraUIElements.cpp
Open source lines: 1260/1260 (100.00%)
*****************************************************/

#include "ExtraUIElements.hpp"
#include "SaveHandler.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "UserCTHandler.hpp"
#include "MusicSlotMngr.hpp"
#include "MissionHandler.hpp"
#include "PointsModeHandler.hpp"
#include "str16utils.hpp"

namespace CTRPluginFramework {

    SpeedometerController::Speedometer* SpeedometerController::speedometer;

    void SpeedometerController::Speedometer::SetVehicleMove() {
        currentVehicleMove = MarioKartFramework::getVehicle(MarioKartFramework::masterPlayerID);
    }

    float SpeedometerController::Speedometer::GetSpeed() {
        return *(float*)(currentVehicleMove + 0x32C + 0xC00);
    }

    void SpeedometerController::Load() {
        if (!SaveHandler::saveData.speedometer.enabled)
            return;
        SpeedMode mode = (SpeedMode)SaveHandler::saveData.speedometer.mode;
        SpeedUnit unit = (SpeedUnit)SaveHandler::saveData.speedometer.unit;
        if (speedometer)
            return;
        switch (mode)
        {
        case SpeedMode::NUMERIC:
            speedometer = new NumericSpeedometer(unit);
            break;
        case SpeedMode::GRAPHICAL:
            speedometer = new GraphicalSpeedometer(unit);
            break;
        case SpeedMode::MKDD:
            speedometer = new DoubleDashSpeedometer(unit);
            break;
        default:
            break;
        }
    }

    void SpeedometerController::OnDeallocate(VisualControl* v) {
        v->Deallocate();
        if (speedometer)
            delete speedometer;
        speedometer = nullptr;
    }

    SpeedometerController::NumericSpeedometer::NumericSpeedometer(SpeedUnit unit) : Speedometer() {
        SetUnit(unit);
        vControl = new VisualControl("SpdNuControl", true);
        vControl->SetUserData(this);
        vControl->SetDeallocateCallback(SpeedometerController::OnDeallocate);
        vControl->SetAnimationDefineCallback(DefineAnimation);
        vControl->SetOnCreateCallback(Create);
        vControl->SetOnCalcCallback(Calc);
        vControl->LoadRace();
        number1Visible = number2Visible = number3Visible = true;
        prevNumber[0] = prevNumber[1] = prevNumber[2] = prevNumber[3] = 0;
        counter = 0;
        SetVehicleMove();
    }

    SpeedometerController::NumericSpeedometer::~NumericSpeedometer() {
        delete vControl;
    }

    void SpeedometerController::NumericSpeedometer::DefineAnimation(VisualControl::AnimationDefine* animDefine) {
        animDefine->InitAnimationFamilyList(4);
        
        // First number
        animDefine->InitAnimationFamily(0, "G_00", 1);
        animDefine->InitAnimation(0, "texture_pattern_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        // Second number
        animDefine->InitAnimationFamily(1, "G_01", 1);
        animDefine->InitAnimation(0, "texture_pattern_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        // Third number
        animDefine->InitAnimationFamily(2, "G_02", 1);
        animDefine->InitAnimation(0, "texture_pattern_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        // Forth number
        animDefine->InitAnimationFamily(3, "G_03", 1);
        animDefine->InitAnimation(0, "texture_pattern_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    void SpeedometerController::NumericSpeedometer::Create(VisualControl::GameVisualControl* control, void* createArg) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        SpeedometerController::NumericSpeedometer* own = (SpeedometerController::NumericSpeedometer*)control->GetVisualControl()->GetUserData();

        own->rootHandle = layout->vtable->getElementHandle(layout, SafeStringBase("R_speed"), 0);
        own->number0Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_speed-00"), 0);
        own->number1Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_speed-01"), 0);
        own->number2Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_speed-02"), 0);
        own->number3Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_speed-03"), 0);

        u32 textHandle = layout->vtable->getElementHandle(layout, SafeStringBase("T_unit"), 0);
        const char16_t* units[] = {u"km/h", u"mph"};
        VisualControl::Message msg(units[(u32)own->GetUnit()]);
        layout->vtable->replaceMessageImpl(layout, textHandle, msg, nullptr, nullptr);
    }

    void SpeedometerController::NumericSpeedometer::Calc(VisualControl::GameVisualControl* control) {
        SpeedometerController::NumericSpeedometer* own = (SpeedometerController::NumericSpeedometer*)control->GetVisualControl()->GetUserData();
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        own->counter++;
        if (own->counter & 1) return;
        int speed = own->GetSpeed() * own->GetSpeedUnitMultiplier();
        if (speed < 0) speed = -speed;
        if (speed > 9999) speed = 9999;

        float units = speed % 10;
        speed /= 10;
        float dec = speed % 10;
        speed /= 10;
        float cent = speed % 10;
        speed /= 10;
        float mil = speed % 10;

        if (own->prevNumber[0] != units) { control->GetAnimationFamily(0)->SetAnimation(0, units); own->prevNumber[0] = units; }
        if (own->prevNumber[1] != dec) { control->GetAnimationFamily(1)->SetAnimation(0, dec); own->prevNumber[1] = dec; }
        if (own->prevNumber[2] != cent) { control->GetAnimationFamily(2)->SetAnimation(0, cent); own->prevNumber[2] = cent; }
        if (own->prevNumber[3] != mil) { control->GetAnimationFamily(3)->SetAnimation(0, mil); own->prevNumber[3] = mil; }

        if (!mil && !cent && !dec) {if ( own->number1Visible) {own->number1Visible = false; layout->vtable->setVisibleImpl(layout, own->number1Handle, false);}}
        else if (!own->number1Visible) {own->number1Visible = true; layout->vtable->setVisibleImpl(layout, own->number1Handle, true);}

        if (!mil && !cent) {if (own->number2Visible) {own->number2Visible = false; layout->vtable->setVisibleImpl(layout, own->number2Handle, false);}}
        else if (!own->number2Visible) {own->number2Visible = true; layout->vtable->setVisibleImpl(layout, own->number2Handle, true);}

        if (!mil) {if (own->number3Visible) {own->number3Visible = false; layout->vtable->setVisibleImpl(layout, own->number3Handle, false);}}
        else if (!own->number3Visible) {own->number3Visible = true; layout->vtable->setVisibleImpl(layout, own->number3Handle, true);}

        control->GetNwlytAnimator()->ApplyAnimation();
    }

    void SpeedometerController::NumericSpeedometer::OnPauseEvent(bool pauseOpen) {
        VisualControl::NwlytControlSight* layout = vControl->GetGameVisualControl()->GetNwlytControl();
        layout->vtable->setVisibleImpl(layout, rootHandle, !pauseOpen);
    }

    SpeedometerController::GraphicalSpeedometer::GraphicalSpeedometer(SpeedUnit unit) : Speedometer() {
        SetUnit(unit);
        vControl = new VisualControl("SpdGrControl", true);
        vControl->SetUserData(this);
        vControl->SetDeallocateCallback(SpeedometerController::OnDeallocate);
        vControl->SetAnimationDefineCallback(DefineAnimation);
        vControl->SetOnCreateCallback(Create);
        vControl->SetOnCalcCallback(Calc);
        vControl->LoadRace();
        counter = 0;
        SetVehicleMove();
    }

    SpeedometerController::GraphicalSpeedometer::~GraphicalSpeedometer() {
        delete vControl;
    }

    void SpeedometerController::GraphicalSpeedometer::DefineAnimation(VisualControl::AnimationDefine* animDefine) {
        animDefine->InitAnimationFamilyList(1);
        
        // First number
        animDefine->InitAnimationFamily(0, "G_needle", 1);
        animDefine->InitAnimation(0, "speed_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    void SpeedometerController::GraphicalSpeedometer::Create(VisualControl::GameVisualControl* control, void* createArg) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        SpeedometerController::GraphicalSpeedometer* own = (SpeedometerController::GraphicalSpeedometer*)control->GetVisualControl()->GetUserData();

        own->rootHandle = layout->vtable->getElementHandle(layout, SafeStringBase("R_speed"), 0);
        own->rotationHandle = layout->vtable->getElementHandle(layout, SafeStringBase("N_rot"), 0);

        u32 textHandle = layout->vtable->getElementHandle(layout, SafeStringBase("T_unit"), 0);
        const char16_t* units[] = {u"km/h", u"mph"};
        VisualControl::Message msg(units[(u32)own->GetUnit()]);
        layout->vtable->replaceMessageImpl(layout, textHandle, msg, nullptr, nullptr);
        if (own->GetUnit() == SpeedUnit::MPH) {
            const char* elements[] = {"T_01", "T_02", "T_03", "T_04", "T_05", "T_06"};
            const char16_t* units[] = {u"15", u"30", u"45", u"60", u"75", u"90"};
            for (int i = 0; i < 6; i++) {
                textHandle = layout->vtable->getElementHandle(layout, SafeStringBase(elements[i]), 0);
                msg = VisualControl::Message(units[i]);
                layout->vtable->replaceMessageImpl(layout, textHandle, msg, nullptr, nullptr);
            }
        }
    }

    void SpeedometerController::GraphicalSpeedometer::Calc(VisualControl::GameVisualControl* control) {
        SpeedometerController::GraphicalSpeedometer* own = (SpeedometerController::GraphicalSpeedometer*)control->GetVisualControl()->GetUserData();
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();

        int speed = own->GetSpeed() * own->GetSpeedUnitMultiplier();
        if (speed < 0) speed = -speed;

        const float maxSpeed[] = {140.f, 105.f};

        control->GetAnimationFamily(0)->SetAnimation(0, (speed / maxSpeed[(u32)own->GetUnit()]) * 180.f);

        control->CalcAnim(own->rotationHandle);
    }

    void SpeedometerController::GraphicalSpeedometer::OnPauseEvent(bool pauseOpen) {
        VisualControl::NwlytControlSight* layout = vControl->GetGameVisualControl()->GetNwlytControl();
        layout->vtable->setVisibleImpl(layout, rootHandle, !pauseOpen);
    }

    SpeedometerController::DoubleDashSpeedometer::DoubleDashSpeedometer(SpeedUnit unit) : Speedometer() {
        SetUnit(unit);
        vControl = new VisualControl("SpdDDControl", true);
        vControl->SetUserData(this);
        vControl->SetDeallocateCallback(SpeedometerController::OnDeallocate);
        vControl->SetAnimationDefineCallback(DefineAnimation);
        vControl->SetOnCreateCallback(Create);
        vControl->SetOnResetCallback(Reset);
        vControl->SetOnCalcCallback(Calc);
        vControl->LoadRace();
        counter = 0;
        scroll = false;
        cancelScroll = false;
        currScroll = 0;
        number1Visible = number2Visible = true;
        playedThunder = false;
        prevNumber[0] = prevNumber[1] = prevNumber[2] = 0;
        state = prevState = State::HIDDEN;
        for (int i = 0; i < 7; i++)
            tabColor[i] = TabColor::TAB_GREY;
        SetVehicleMove();
    }

    SpeedometerController::DoubleDashSpeedometer::~DoubleDashSpeedometer() {
        delete vControl;
    }

    void SpeedometerController::DoubleDashSpeedometer::DefineAnimation(VisualControl::AnimationDefine* animDefine) {
        const char* tabgroups[] = {"G_tab_00", "G_tab_01", "G_tab_02", "G_tab_03", "G_tab_04", "G_tab_05", "G_tab_06"};
        animDefine->InitAnimationFamilyList(20);
        
        animDefine->InitAnimationFamily(0, "G_unit", 1);
        animDefine->InitAnimation(0, "unit_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        for (int i = 0; i < 3; i++) {
            const char* groups[] = {"G_num_00", "G_num_01", "G_num_02"};
            animDefine->InitAnimationFamily(i + 1, groups[i], 1);
            animDefine->InitAnimation(0, "speed_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        }
        
        for (int i = 0; i < 7; i++) {
            animDefine->InitAnimationFamily(i + 4, tabgroups[i], 1);
            animDefine->InitAnimation(0, "color_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        }

        animDefine->InitAnimationFamily(11, "G_unit", 1);
        animDefine->InitAnimation(1, "in", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        animDefine->InitAnimationFamily(12, "G_spd_00", 4);
        animDefine->InitAnimation(0, "in", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        for (int i = 0; i < 7; i++) {
            animDefine->InitAnimationFamily(i + 13, tabgroups[i], 1);
            animDefine->InitAnimation(1, "in", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        }
    }

    void SpeedometerController::DoubleDashSpeedometer::Create(VisualControl::GameVisualControl* control, void* createArg) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        SpeedometerController::DoubleDashSpeedometer* own = (SpeedometerController::DoubleDashSpeedometer*)control->GetVisualControl()->GetUserData();

        own->rootHandle = layout->vtable->getElementHandle(layout, SafeStringBase("R_speed"), 0);
        own->number1Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_num_01"), 0);
        own->number2Handle = layout->vtable->getElementHandle(layout, SafeStringBase("P_num_02"), 0);
    }

    void SpeedometerController::DoubleDashSpeedometer::Reset(VisualControl::GameVisualControl* control) {
        SpeedometerController::DoubleDashSpeedometer* own = (SpeedometerController::DoubleDashSpeedometer*)control->GetVisualControl()->GetUserData();

        control->GetAnimationFamily(0)->SetAnimation(0, (float)own->GetUnit()); // Unit
        own->StopScroll(true);
        own->SetNumSpeed(0);
        own->state = State::HIDDEN;
        own->playedThunder = false;

        for (int i = 0; i < 7; i++) {
            own->ChangeTabColor(i, TabColor::TAB_GREY);
            control->GetAnimationFamily(i + 13)->SetAnimation(0, 0.f);
        }
        control->GetAnimationFamily(11)->SetAnimation(0, 0.f);
        control->GetAnimationFamily(12)->SetAnimation(0, 0.f);

        control->GetNwlytAnimator()->ApplyAnimation();
        control->AnimOff();
    }

    void SpeedometerController::DoubleDashSpeedometer::Calc(VisualControl::GameVisualControl* control) {
        SpeedometerController::DoubleDashSpeedometer* own = (SpeedometerController::DoubleDashSpeedometer*)control->GetVisualControl()->GetUserData();
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        own->counter++;
        if (own->state == State::HIDDEN) {
            own->appearingCounter = 0;
        } else if (own->state == State::APPEARING) {

            const u8 offset[] = {0, 10, 20, 30, 40, 50, 60};
            for (int i = 0; i < 7; i++) {
                if (own->appearingCounter >= offset[i] && own->appearingCounter <= offset[i] + 20) 
                    control->GetAnimationFamily(i + 13)->SetAnimation(0, own->appearingCounter - offset[i]);
            }
            
            if (own->appearingCounter >= 70 && own->appearingCounter <= 90) {
                control->GetAnimationFamily(11)->SetAnimation(0, own->appearingCounter - 70);
                control->GetAnimationFamily(12)->SetAnimation(0, own->appearingCounter - 70);
            } else if (own->appearingCounter == 92) {
                layout->vtable->setVisibleImpl(layout, own->number1Handle, true);
                layout->vtable->setVisibleImpl(layout, own->number2Handle, true);
                control->CalcAnim(own->rootHandle);
                layout->vtable->setVisibleImpl(layout, own->number1Handle, false);
                layout->vtable->setVisibleImpl(layout, own->number2Handle, false);
                own->state = own->prevState = State::NORMAL;
                return;
            }
            own->appearingCounter+=2;
            control->CalcAnim(own->rootHandle);
        } else {
            if (own->counter & 1) return;
            int speed = own->GetSpeed() * own->GetSpeedUnitMultiplier();
            if (speed < 0) speed = -speed;

            u32* vehicleMove = own->GetVehicleMove();
            u32 boostState = vehicleMove[(0xC00 + 0x398) / 4];
            u32 boostTimer = vehicleMove[(0xC00 + 0x39C) / 4];
            u32 starTimer = vehicleMove[(0xC00 + 0x3F4) / 4];
            u32 thunderTimer = vehicleMove[(0x1000) / 4];
            u32 megaTimer = MarioKartFramework::megaMushTimers[vehicleMove[0x21]];
            bool isBullet = vehicleMove[(0xC30) / 4] & 0x400000;

            if (own->state == State::NORMAL) {
                if ((boostState == 2 || boostState == 7 || boostState == 6 || boostState == 4) && boostTimer) // Boost start, mushroom, boost pad, star ring
                    own->state = State::SPEED;
            }
            if (starTimer && (own->state == State::NORMAL || own->state == State::SPEED || own->state == State::MEGA))
                own->state = State::STAR;
            if (megaTimer && (own->state == State::NORMAL || own->state == State::SPEED))
                own->state = State::MEGA;
            if (isBullet)
                own->state = State::BULLET;
            if (thunderTimer && !own->playedThunder) {
                own->playedThunder = true;
                own->state = State::THUNDER;
                own->thunderCounter = 20;
            } else if (!thunderTimer) own->playedThunder = false;
            
            if (own->state != own->prevState && own->state != State::NORMAL && own->prevState != State::NORMAL) {
                own->StopScroll(true);
            }

            own->prevState = own->state;

            switch (own->state)
            {
            case State::NORMAL:
                if (speed <= 999) {
                    if (own->scroll) {
                        own->StopScroll(false);
                    } else {
                        const float singleSpeedVal[2] = {80.f / 7.f, 50.f / 7.f};
                        float singleSpeed = singleSpeedVal[(u32)own->GetUnit()];
                        float currSpeed = singleSpeed;

                        for (int i = 0; i < 7; i++) {
                            if (speed > currSpeed)
                                own->ChangeTabColor(i, i + 1);
                            else
                                own->ChangeTabColor(i, 0);
                            currSpeed += singleSpeed;
                        }
                    }
                } else {
                    if (own->scroll)
                        own->StopScroll(true);
                    
                    for (int i = 0; i < 7; i++)
                    {
                        own->ChangeTabColor(i, TabColor::TAB_RED);
                    }
                }
                break;
            case State::SPEED:
                if (!own->scroll) {
                    for (int i = 0; i < 7; i++) {
                        own->ChangeTabColor(i, i + 1);
                    }
                    own->StartScroll();
                }
                if (boostTimer == 0)
                    own->state = State::NORMAL;
                break;
            case State::STAR:
                if (!own->scroll) {
                    const int starColors[7] = {TabColor::TAB_ORANGE, TabColor::TAB_YELLOW, TabColor::TAB_YELLOW, TabColor::TAB_ORANGE, TabColor::TAB_PINK, TabColor::TAB_PINK, TabColor::TAB_PINK};
                    for (int i = 0; i < 7; i++) {
                        own->ChangeTabColor(i, starColors[i]);
                    }
                    own->StartScroll();
                }
                if (starTimer == 0)
                    own->state = State::NORMAL;
                break;
            case State::MEGA:
                if (!own->scroll) {
                    const int megaColors[7] = {TabColor::TAB_ORANGE, TabColor::TAB_ORANGE, TabColor::TAB_ORANGE, TabColor::TAB_ORANGE, TabColor::TAB_ORANGE, TabColor::TAB_RED, TabColor::TAB_RED};
                    for (int i = 0; i < 7; i++) {
                        own->ChangeTabColor(i, megaColors[i]);
                    }
                    own->StartScroll();
                }
                if (megaTimer == 0)
                    own->state = State::NORMAL;
                break;
            case State::BULLET:
                if (!own->scroll) {
                    const int starColors[7] = {TabColor::TAB_YELLOW, TabColor::TAB_ORANGE, TabColor::TAB_ORANGERED, TabColor::TAB_RED, TabColor::TAB_GREY, TabColor::TAB_BLACK, TabColor::TAB_BLACK};
                    for (int i = 0; i < 7; i++) {
                        own->ChangeTabColor(i, starColors[i]);
                    }
                    own->StartScroll();
                }
                if (!isBullet)
                    own->state = State::NORMAL;
                break;
            case State::THUNDER:
                own->StopScroll(true);
                for (int i = 0; i < 7; i++) {
                    own->ChangeTabColor(i, TabColor::TAB_YELLOW);
                }
                own->SetNumError();
                if (own->thunderCounter == 0) {
                    own->state = State::NORMAL;
                }
                own->thunderCounter--;
                break;
            default:
                break;
            }
            
            if (own->state != State::THUNDER) own->SetNumSpeed(speed);
            own->CalcScrollingAnim();
            control->GetNwlytAnimator()->ApplyAnimation();
        }
    }

    void SpeedometerController::DoubleDashSpeedometer::OnPauseEvent(bool pauseOpen) {
        VisualControl::NwlytControlSight* layout = vControl->GetGameVisualControl()->GetNwlytControl();
        layout->vtable->setVisibleImpl(layout, rootHandle, !pauseOpen);
    }

    void SpeedometerController::DoubleDashSpeedometer::OnRaceStart() {
        state = State::APPEARING;
    }

    void SpeedometerController::DoubleDashSpeedometer::SetNumSpeed(int speed) {
        if (speed > 999) {
            SetNumError();
            return;
        }
        VisualControl::GameVisualControl* control = vControl->GetGameVisualControl();
        VisualControl::NwlytControlSight* layout = vControl->GetGameVisualControl()->GetNwlytControl();

        int auxSpeed = speed;
        speed /= 10;
        float units = auxSpeed - (speed * 10);
        auxSpeed = speed;
        speed /= 10;
        float dec = auxSpeed - (speed * 10);
        auxSpeed = speed;
        speed /= 10;
        float cent = auxSpeed - (speed * 10);

        if (prevNumber[0] != units) { control->GetAnimationFamily(1)->SetAnimation(0, units); prevNumber[0] = units; }
        if (prevNumber[1] != dec) { control->GetAnimationFamily(2)->SetAnimation(0, dec); prevNumber[1] = dec; }
        if (prevNumber[2] != cent) { control->GetAnimationFamily(3)->SetAnimation(0, cent); prevNumber[2] = cent; }

        if (!cent && !dec) {if ( number1Visible) {number1Visible = false; layout->vtable->setVisibleImpl(layout, number1Handle, false);}}
        else if (!number1Visible) {number1Visible = true; layout->vtable->setVisibleImpl(layout, number1Handle, true);}

        if (!cent) {if (number2Visible) {number2Visible = false; layout->vtable->setVisibleImpl(layout, number2Handle, false);}}
        else if (!number2Visible) {number2Visible = true; layout->vtable->setVisibleImpl(layout, number2Handle, true);}
    }

    void SpeedometerController::DoubleDashSpeedometer::SetNumError() {
        VisualControl::GameVisualControl* control = vControl->GetGameVisualControl();
        VisualControl::NwlytControlSight* layout = vControl->GetGameVisualControl()->GetNwlytControl();
        float units = 10.f;

        for (int i = 0; i < 3; i++) {
            if (prevNumber[i] != units) { control->GetAnimationFamily(i + 1)->SetAnimation(0, units); prevNumber[i] = units; }
        }

        if (!number1Visible) {number1Visible = true; layout->vtable->setVisibleImpl(layout, number1Handle, true);}
        if (!number2Visible) {number2Visible = true; layout->vtable->setVisibleImpl(layout, number2Handle, true);}
    }

    void SpeedometerController::DoubleDashSpeedometer::CalcScrollingAnim() {
        if (!scroll) return;
        u32 offset = currScroll;
        for (int i = 0; i < 7; i++) {
            vControl->GetGameVisualControl()->GetAnimationFamily(offset + 4)->SetAnimation(0, tabColor[i]);
            offset++;
            if (offset >= 7) offset = 0;
        }
        currScroll++;
        if (currScroll >= 7) currScroll = 0;
        if (cancelScroll && currScroll == 0) StopScroll(true);
    }

    void SpeedometerController::DoubleDashSpeedometer::StopScroll(bool force) {
        if (scroll) {
            if (force) {
                scroll = false;
                cancelScroll = false;
                for (int i = 0; i < 7; i++)
                    tabColor[i] = -1;
            } else 
                cancelScroll = true;
        }
    }

    extern bool g_isAutoAccel;
    VisualControl* AutoAccelerationController::autoAccelControl = nullptr;
    bool AutoAccelerationController::prevIsAutoAccel = false;
    u32 AutoAccelerationController::rootHandle = 0;

    void AutoAccelerationController::initAutoAccelIcon() {
		autoAccelControl = new VisualControl("AcceControl", false);
		autoAccelControl->SetAnimationDefineCallback([](VisualControl::AnimationDefine* animDefine){
			animDefine->InitAnimationFamilyList(1);

			animDefine->InitAnimationFamily(0, "G_icon_00", 1);
        	animDefine->InitAnimation(0, "icon_00", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
		});
        autoAccelControl->SetOnCreateCallback([](VisualControl::GameVisualControl* control, void* createArg) {
            VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
            rootHandle = layout->vtable->getElementHandle(layout, SafeStringBase("R_accel"), 0);
        });
		autoAccelControl->SetOnResetCallback([](VisualControl::GameVisualControl* control){
			prevIsAutoAccel = g_isAutoAccel;
			control->GetAnimationFamily(0)->SetAnimation(0, g_isAutoAccel ? 0 : 1);
		});
		autoAccelControl->SetOnCalcCallback([](VisualControl::GameVisualControl* control){
			if (prevIsAutoAccel != g_isAutoAccel)
			{
				control->GetAnimationFamily(0)->SetAnimation(0, g_isAutoAccel ? 0 : 1);
				prevIsAutoAccel = g_isAutoAccel;
			}
		});
		autoAccelControl->LoadRace();
	}
    
    void AutoAccelerationController::OnPauseEvent(bool pauseOpen) {
        VisualControl::NwlytControlSight* layout = autoAccelControl->GetGameVisualControl()->GetNwlytControl();
        layout->vtable->setVisibleImpl(layout, rootHandle, !pauseOpen);
    }

    VisualControl* MusicCreditsController::musicCreditsControl = nullptr;
    u32 MusicCreditsController::rootHandle = 0;
    MarioKartTimer MusicCreditsController::timer;
    bool MusicCreditsController::hasData = false;
    constexpr MarioKartTimer MusicCreditsController::animTimer;

    void MusicCreditsController::Init() {
		musicCreditsControl = new VisualControl("MsCrControl", false);
        musicCreditsControl->SetAnimationDefineCallback([](VisualControl::AnimationDefine* animDefine){
			animDefine->InitAnimationFamilyList(1);

			animDefine->InitAnimationFamily(0, "G_credits", 1);
        	animDefine->InitAnimation(0, "out", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
		});
        musicCreditsControl->SetOnCreateCallback([](VisualControl::GameVisualControl* control, void* createArg) {
            VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
            rootHandle = layout->vtable->getElementHandle(layout, SafeStringBase("R_credits"), 0);
            hasData = false;

            std::string name;
            std::string author;

            if (!MissionHandler::isMissionMode || !MissionHandler::LoadCustomMusicNameAuthor(name, author)) {
                u32 courseID = CourseManager::lastLoadedCourseID;
                if (UserCTHandler::IsUsingCustomCup()) courseID = UserCTHandler::GetCurrentCourseOrigSlot();
                auto it = MusicSlotMngr::entryMap.find(courseID);
                if (it != MusicSlotMngr::entryMap.end() && !it->second.musicName.empty()) { 
                    name = it->second.musicName;
                    author = it->second.GetAuthorString();
                }
            }            

            if (!name.empty()) {
                u32 nameTextHandle = layout->vtable->getElementHandle(layout, SafeStringBase("T_name"), 0);
                std::u16string out;
                Utils::ConvertUTF8ToUTF16(out, name);
                VisualControl::Message nameMsg(out.c_str());
                layout->vtable->replaceMessageImpl(layout, nameTextHandle, nameMsg, nullptr, nullptr);
                out.clear();

                u32 authorTextHandle = layout->vtable->getElementHandle(layout, SafeStringBase("T_author"), 0);
                Utils::ConvertUTF8ToUTF16(out, author);
                VisualControl::Message authorMsg(out.c_str());
                layout->vtable->replaceMessageImpl(layout, authorTextHandle, authorMsg, nullptr, nullptr);
                hasData = true;
            }

        });
		musicCreditsControl->SetOnResetCallback([](VisualControl::GameVisualControl* control){
            VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
			timer = 0;
            Vector3* rootHandlePos = (Vector3*)((u32)rootHandle + 0x28);
            rootHandlePos->y = 0.f;
            layout->vtable->setVisibleImpl(layout, rootHandle, hasData);
		});
		musicCreditsControl->SetOnCalcCallback([](VisualControl::GameVisualControl* control){
			if (timer == 0)
                return;

            if (timer < animTimer) {
                float prog = (1.f - (timer.GetFrames() / (float)animTimer.GetFrames())) * 100.f;
                control->GetAnimationFamily(0)->SetAnimation(0, prog);
                if (timer == 1) {
                    VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
                    layout->vtable->setVisibleImpl(layout, rootHandle, false);
                }
            }
            timer--;
		});
		musicCreditsControl->LoadRace();
	}

    ExtendedItemBoxController::AmountMode ExtendedItemBoxController::nextAmountMode = ExtendedItemBoxController::AmountMode::AMOUNT_INVALID;
    RT_HOOK ExtendedItemBoxController::ItemBoxControlOnCalcHook = { 0 };
    RT_HOOK ExtendedItemBoxController::ItemBoxControlOnCreateHook = { 0 };
    u32 ExtendedItemBoxController::progBarHandle = 0;
    u32 ExtendedItemBoxController::progWinHandle = 0;
    u32 ExtendedItemBoxController::progBGHandle = 0;
    u32 ExtendedItemBoxController::amountHandle = 0;
    float ExtendedItemBoxController::prevProgressbarValue = 1.01f;
    float ExtendedItemBoxController::prevVisibilityValue = 0.f;
    float ExtendedItemBoxController::targetProgressValue = 1.f;
    float ExtendedItemBoxController::currentProgressValue = 1.f;
    bool ExtendedItemBoxController::prevIsBoxVisible = false;
    bool ExtendedItemBoxController::prevIsBoxDecided = true;
    int ExtendedItemBoxController::visibilityAnimFrames = 0;
    int ExtendedItemBoxController::konohaTimer = -1;
    float ExtendedItemBoxController::killerProgress = -1.f;

    void ExtendedItemBoxController::DefineAnimation(VisualControl::AnimationDefine* animationDefine) {
        animationDefine->InitAnimationFamilyList(7);

        animationDefine->InitAnimationFamily(0, "G_loop", 1);
        animationDefine->InitAnimation(0, "loop", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        animationDefine->InitAnimationFamily(1, "G_inOut", 4);
        animationDefine->InitAnimationStopByRate(0, "inOut", 0.f);
        animationDefine->InitAnimation(1, "inOut", VisualControl::AnimationDefine::AnimationKind::NEXT);
        animationDefine->InitAnimationStopByRate(2, "inOut", 1.f);
        animationDefine->InitAnimationReverse(3, "inOut", VisualControl::AnimationDefine::AnimationKind::ONCE);

        animationDefine->InitAnimationFamily(2, "G_sc", 3);
        animationDefine->InitAnimationStopByRate(0, "sc", 0.f);
        animationDefine->InitAnimation(1, "sc", VisualControl::AnimationDefine::AnimationKind::ONCE);
        animationDefine->InitAnimation(2, "item_flash", VisualControl::AnimationDefine::AnimationKind::ONCE);

        animationDefine->InitAnimationFamily(3, "G_team_color", 1);
        animationDefine->InitAnimation(0, "team_color", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        animationDefine->InitAnimationFamily(4, "G_prog", 1);
        animationDefine->InitAnimation(0, "prog", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        animationDefine->InitAnimationFamily(5, "G_barInOut", 1);
        animationDefine->InitAnimation(0, "barInOut", VisualControl::AnimationDefine::AnimationKind::NOPLAY);

        animationDefine->InitAnimationFamily(6, "G_amount", 1);
        animationDefine->InitAnimation(0, "amount", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    void ExtendedItemBoxController::OnCreate(VisualControl::GameVisualControl* visualControl, void* createArg) {
        ((void(*)(VisualControl::GameVisualControl*, void*))ItemBoxControlOnCreateHook.callCode)(visualControl, createArg);

        VisualControl::NwlytControlSight* layout = visualControl->GetNwlytControl();
        progWinHandle = layout->vtable->getElementHandle(layout, SafeStringBase("P_prog-00"), 0);
        progBarHandle = layout->vtable->getElementHandle(layout, SafeStringBase("P_bar-00"), 0);
        progBGHandle = layout->vtable->getElementHandle(layout, SafeStringBase("P_progBG-00"), 0);
        amountHandle = layout->vtable->getElementHandle(layout, SafeStringBase("P_am-00"), 0);
        if (amountHandle) layout->vtable->setVisibleImpl(layout, amountHandle, false);

        prevProgressbarValue = 1.01f;
        prevVisibilityValue = 0.f;
        targetProgressValue = 1.f;
        currentProgressValue = 1.f;
        prevIsBoxDecided = true;
        prevIsBoxVisible = false;
        visibilityAnimFrames = 0;
        konohaTimer = -1;
        killerProgress = -1.f;
        nextAmountMode = AmountMode::AMOUNT_INVALID;
    }

    void ExtendedItemBoxController::OnCalc(VisualControl::GameVisualControl* visualControl) {
        ((void(*)(VisualControl::GameVisualControl*))ItemBoxControlOnCalcHook.callCode)(visualControl);
        bool isBoxVisible = ((u8*)visualControl)[0x91];
        bool isBoxDecided = ((u8*)visualControl)[0x92];

        if (isBoxVisible) {
            if (nextAmountMode != AmountMode::AMOUNT_INVALID) {
                ChangeAmountMode(visualControl, nextAmountMode);
                nextAmountMode = AmountMode::AMOUNT_INVALID;
            }
            EItemSlot currItem = GetCurrentItem(visualControl);
            if (!prevIsBoxVisible) {
                ChangeVisibilityState(false);
                prevProgressbarValue = 1.01f;
                SetProgressBar(visualControl, 1.f);
                targetProgressValue = 1.f;
                currentProgressValue = 1.f;
                konohaTimer = -1;
                killerProgress = -1.f;
            } else if (!prevIsBoxDecided && isBoxDecided) {
                if (currItem == EItemSlot::ITEM_KONOHA || currItem == EItemSlot::ITEM_FLOWER || currItem == EItemSlot::ITEM_KINOKOP || currItem == EItemSlot::ITEM_KILLER)
                    ChangeVisibilityState(true);
            }
            if (currItem == EItemSlot::ITEM_KONOHA) {
                float progress = (konohaTimer < 0) ? 1.f : ((600.f - konohaTimer) / 600.f);
                ChangeProgress(progress);
            } else if (currItem == EItemSlot::ITEM_FLOWER) {
                u32 kartItem = ((u32**)visualControl)[0x9C/4][0];
                int flowerUses = ((int*)kartItem)[0x4C/4];
                int flowerTimer = ((int*)kartItem)[0x50/4];
                float progressUses = (10.f - flowerUses) / 10.f;
                float progressTimer = (flowerTimer < 0) ? 1.f : ((600.f - flowerTimer) / 600.f);
                ChangeProgress(progressUses * progressTimer);
            } else if (currItem == EItemSlot::ITEM_KINOKOP) {
                u32 kartItem = ((u32**)visualControl)[0x9C/4][0];
                int goldenTimer = ((int*)kartItem)[0x54/4];
                float progress = (goldenTimer < 0) ? 1.f : ((450.f - goldenTimer) / 450.f);
                ChangeProgress(progress);
            } else if (currItem == EItemSlot::ITEM_KILLER) {
                ChangeProgress((killerProgress < 0.f) ? 1.f : killerProgress);
            }
        }
        
        CalcVisibility(visualControl);
        CalcProgress(visualControl);
        prevIsBoxVisible = isBoxVisible;
        prevIsBoxDecided = isBoxDecided;
    }

    void ExtendedItemBoxController::SetProgressBar(VisualControl::GameVisualControl* visualControl, float progress) {
        if (progress > prevProgressbarValue)
            return;
        visualControl->GetAnimationFamily(4)->SetAnimation(0, progress * 60.f);
        if (progBarHandle) visualControl->CalcAnim(progBarHandle);
        prevProgressbarValue = progress;
    }

    void ExtendedItemBoxController::SetVisibility(VisualControl::GameVisualControl* visualControl, float progress) {
        if (progress == prevVisibilityValue)
            return;
        visualControl->GetAnimationFamily(5)->SetAnimation(0, progress * 60.f);
        if (progBarHandle) visualControl->CalcAnim(progBarHandle);
        if (progWinHandle) visualControl->CalcAnim(progWinHandle);
        if (progBGHandle) visualControl->CalcAnim(progBGHandle);
        prevVisibilityValue = progress;
    }

    void ExtendedItemBoxController::ChangeVisibilityState(bool visible) {
        if (visible) {
            visibilityAnimFrames = 1;
        } else {
            visibilityAnimFrames = 0;
        }
    }

    void ExtendedItemBoxController::ChangeProgress(float progress) {
        if (progress < targetProgressValue)
            targetProgressValue = progress;
    }

    void ExtendedItemBoxController::CalcVisibility(VisualControl::GameVisualControl* visualControl) {
        if (visibilityAnimFrames) {
            SetVisibility(visualControl, visibilityAnimFrames / 10.f);
            if (visibilityAnimFrames < 10) visibilityAnimFrames++;
        } else {
            SetVisibility(visualControl, 0.f);
        }
    }

    void ExtendedItemBoxController::CalcProgress(VisualControl::GameVisualControl* visualControl) {
        if (currentProgressValue > targetProgressValue) {
            currentProgressValue -= ((1 / 60.f) * 3/4.f);
            if (currentProgressValue < targetProgressValue)
                currentProgressValue = targetProgressValue;
        }
        SetProgressBar(visualControl, currentProgressValue);
    }

    void ExtendedItemBoxController::ChangeAmountMode(VisualControl::GameVisualControl* visualControl, AmountMode mode)
    {
        VisualControl::NwlytControlSight* layout = visualControl->GetNwlytControl();
        if (!amountHandle) return;

        switch (mode)
        {
        case AmountMode::AMOUNT_NONE:
            layout->vtable->setVisibleImpl(layout, amountHandle, false);
            break;
        case AmountMode::AMOUNT_2X:
            layout->vtable->setVisibleImpl(layout, amountHandle, true);
            visualControl->GetAnimationFamily(6)->SetAnimation(0, 0.f);
            visualControl->GetAnimationFamily(3)->SetAnimation(0, 0.f);
            visualControl->CalcAnim(amountHandle);
            break;
        case AmountMode::AMOUNT_3X:
            layout->vtable->setVisibleImpl(layout, amountHandle, true);
            visualControl->GetAnimationFamily(6)->SetAnimation(0, 1.f);
            visualControl->GetAnimationFamily(3)->SetAnimation(0, 0.f);
            visualControl->CalcAnim(amountHandle);
            break;
        case AmountMode::AMOUNT_FRENZY:
            layout->vtable->setVisibleImpl(layout, amountHandle, true);
            visualControl->GetAnimationFamily(6)->SetAnimation(0, 2.f);
            visualControl->GetAnimationFamily(3)->SetAnimation(0, 3.f);
            visualControl->CalcAnim(amountHandle);
            break;
        default:
            break;
        }
    }

    std::array<std::u16string, 4> PointsModeDisplayController::ComboCongratController::congratNames{};
    std::u16string PointsModeDisplayController::ComboCongratController::comboName{};

    PointsModeDisplayController::PointsModeDisplayController() {
        vControl = new VisualControl("PointsMCtrl", false);
        vControl->SetUserData(this);
        vControl->SetDeallocateCallback(OnDeallocate);
        vControl->SetAnimationDefineCallback([](VisualControl::AnimationDefine* animationDefine) {
            reinterpret_cast<PointsModeDisplayController*>(animationDefine->GetVisualControl()->GetUserData())->DefineAnimation(animationDefine);
        });
        vControl->SetOnCreateCallback([](VisualControl::GameVisualControl* v, void* args) {
            reinterpret_cast<PointsModeDisplayController*>(v->GetVisualControl()->GetUserData())->Create(v, args);
        });
        vControl->SetOnResetCallback([](VisualControl::GameVisualControl* v) {
            reinterpret_cast<PointsModeDisplayController*>(v->GetVisualControl()->GetUserData())->Reset(v);
        });
        vControl->SetOnCalcCallback([](VisualControl::GameVisualControl* v) {
            reinterpret_cast<PointsModeDisplayController*>(v->GetVisualControl()->GetUserData())->Calc(v);
        });
        vControl->LoadRace();
    }

    void PointsModeDisplayController::OnDeallocate(VisualControl* v) {
        v->Deallocate();
        PointsModeHandler::DestroyDisplayController();
    }

    void PointsModeDisplayController::DefineAnimation(VisualControl::AnimationDefine* animationDefine) {
        animationDefine->InitAnimationFamilyList(AnimFamilyID::SIZE);

        animationDefine->InitAnimationFamily(AnimFamilyID::ALL_IN, "G_root", 3);
        animationDefine->InitAnimationStopByRate(0, "in", 0.f);
        animationDefine->InitAnimation(1, "in", VisualControl::AnimationDefine::AnimationKind::NEXT);
        animationDefine->InitAnimationStopByRate(2, "in", 1.f);

        numberController.DefineAnimation(animationDefine);
        reasonController.DefineAnimation(animationDefine);
        comboCongratController.DefineAnimation(animationDefine);
    }

    void PointsModeDisplayController::Create(VisualControl::GameVisualControl* control, void* createArgs) {
        rootHandle = control->GetNwlytControl()->vtable->getElementHandle(control->GetNwlytControl(), SafeStringBase("R_center"), 0);

        numberController.Create(control, createArgs);
        reasonController.Create(control, createArgs);
        comboCongratController.Create(control, createArgs);
    }
    
    void PointsModeDisplayController::Reset(VisualControl::GameVisualControl* control) {
        control->GetAnimationFamily(AnimFamilyID::ALL_IN)->SetAnimation(0, 0.f);

        currState = State::HIDDEN;
        stateTimer = 0;

        numberController.Reset(control);
        reasonController.Reset(control);
        comboCongratController.Reset(control);
    }
    
    void PointsModeDisplayController::Calc(VisualControl::GameVisualControl* control) {
        PointsModeHandler::OnVisualElementCalc();

        switch (currState)
        {
        case State::APPEARING:
        {
            if (++stateTimer >= MarioKartTimer(0, 3, 250)) {
                control->GetAnimationFamily(AnimFamilyID::ALL_IN)->ChangeAnimation(1, 0.f);
                currState = State::SHOWED;
                stateTimer = 0;
            }
        }
        break;
        case State::SHOWED:
        {
        }
        break;
        
        default:
            break;
        }

        numberController.Calc(control);
        reasonController.Calc(control);
        comboCongratController.Calc(control);
    }

    void PointsModeDisplayController::NumberController::GotoNumber(u32 number, const MarioKartTimer& timer) {
        targetGotoNumber = number;
        targetGotoTimer = timer;
        fromGotoNumber = requestedNumber;
        fromGotoTimer = MarioKartTimer(1);
        StartBounce();
    }

    void PointsModeDisplayController::NumberController::DefineAnimation(VisualControl::AnimationDefine* animationDefine) {
        const char* const groups[] = {"G_num_00", "G_num_01", "G_num_02", "G_num_03", "G_num_04"};
        for (int i = AnimFamilyID::NUM_0_PAT; i <= AnimFamilyID::NUM_4_PAT; i++) {
            animationDefine->InitAnimationFamily(i, groups[i - AnimFamilyID::NUM_0_PAT], 1);
            animationDefine->InitAnimation(0, "point_val", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        }
        for (int i = AnimFamilyID::NUM_0_PAT; i <= AnimFamilyID::NUM_4_PAT; i++) {
            animationDefine->InitAnimationFamily(i, groups[i - AnimFamilyID::NUM_0_PAT], 1);
            animationDefine->InitAnimation(0, "point_col", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
        }
        for (int i = AnimFamilyID::NUM_0_BNC; i <= AnimFamilyID::NUM_4_BNC; i++) {
            animationDefine->InitAnimationFamily(i, groups[i - AnimFamilyID::NUM_0_BNC], 2);
            animationDefine->InitAnimationStopByRate(0, "point_bnc", 0.f);
            animationDefine->InitAnimation(1, "point_bnc", VisualControl::AnimationDefine::AnimationKind::LOOP);
        }
    }

    void PointsModeDisplayController::NumberController::Create(VisualControl::GameVisualControl* control, void* createArgs) {
        const char* const num_names[] = {"LNumber_0_00", "LNumber_0_01", "LNumber_0_02", "LNumber_0_03", "LNumber_0_04"};
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();

        for (int i = 0; i < NUMBER_AMOUNT; i++) {
            numberHandle[i] = layout->vtable->getElementHandle(layout, SafeStringBase(num_names[i]), 0);
        }
    }
    
    void PointsModeDisplayController::NumberController::Reset(VisualControl::GameVisualControl* control) {
        currentColor = requestColor = ColorState::ORANGE;
        currBounceState = nextBounceState = BounceState::IDLE;
        bounceTimer = 0;
        forcedGrey = false;
        for (int i = 0; i < NUMBER_AMOUNT; i++) {
            control->GetAnimationFamily(AnimFamilyID::NUM_0_COL + i)->SetAnimation(0, 0.f);
            control->GetAnimationFamily(AnimFamilyID::NUM_0_BNC + i)->SetAnimation(0, 0.f);
        }
        currentNumber = requestedNumber = 0;
        for (int i = 0; i < NUMBER_AMOUNT; i++) {
            numberValue[i] = 0;
            numberHidden[i] = i != 0;
            control->GetNwlytControl()->vtable->setVisibleImpl(control->GetNwlytControl(), numberHandle[i], i == 0);
            control->GetAnimationFamily(AnimFamilyID::NUM_0_PAT + i)->SetAnimation(0, 0);
        }
    }
    
    void PointsModeDisplayController::NumberController::Calc(VisualControl::GameVisualControl* control) {
        ColorState nextColor = forcedGrey ? ColorState::GREY : requestColor;
        if (currentColor != nextColor) {
            currentColor = nextColor;
            for (int i = 0; i < NUMBER_AMOUNT; i++) {
                control->GetAnimationFamily(AnimFamilyID::NUM_0_COL + i)->SetAnimation(0, static_cast<float>(currentColor));
            }
        }

        switch (currBounceState)
        {
        case BounceState::STARTING:
        {
            requestColor = ColorState::PINK;
            bool allBouncing = true;
            for (int i = 0; i < NUMBER_AMOUNT; i++) {
                int trigger_frame = 2 * i;
                if (!isBouncing[i] && trigger_frame == bounceTimer) {
                    control->GetAnimationFamily(AnimFamilyID::NUM_0_BNC + i)->ChangeAnimation(1, 0.f);
                    isBouncing[i] = true;
                }
                allBouncing &= isBouncing[i];
            }
            if (allBouncing) {
                currBounceState = nextBounceState;
                nextBounceState = BounceState::IDLE;
            }
            bounceTimer++;
            break;
        }
        case BounceState::STOPPING:
        {
            bool anyBouncing = false;
            for (int i = 0; i < NUMBER_AMOUNT; i++) {
                VisualControl::AnimationFamily* family = control->GetAnimationFamily(AnimFamilyID::NUM_0_BNC + i);
                if ((i == 0 || !isBouncing[0]) && isBouncing[i] && family->GetAnimationRate() == 0.f) {
                    family->ChangeAnimation(0, 0.f);
                    isBouncing[i] = false;
                }
                anyBouncing |= isBouncing[i];
            }
            if (!anyBouncing) {
                requestColor = ColorState::ORANGE;
                currBounceState = nextBounceState;
                nextBounceState = BounceState::IDLE;
            }
            break;
        }
        default:
            currBounceState = nextBounceState;
            break;
        }

        if (fromGotoTimer != 0) {
            float progress = (float)fromGotoTimer.GetFrames() / (float)targetGotoTimer.GetFrames();
            s32 difference = (s32)targetGotoNumber - (s32)fromGotoNumber;
            SetNumber((u32)(progress * difference + (s32)fromGotoNumber));
            if (fromGotoTimer++ >= targetGotoTimer) {
                fromGotoTimer = 0;
                SetNumber(targetGotoNumber);
                EndBounce();
            }
        }

        if (requestedNumber != currentNumber) {
            currentNumber = requestedNumber;
            SetNumberImpl(control, requestedNumber);
        }
    }

    void PointsModeDisplayController::NumberController::SetNumberImpl(VisualControl::GameVisualControl* control, u32 number) {
        if (number > 99999) number = 99999;
        u8 numberNew[NUMBER_AMOUNT] = { 0 };
        for (int i = 0; i < NUMBER_AMOUNT; i++) {
            numberNew[i] = number % 10;
            number /= 10;
        }
        bool hideZeroes = true;
        for (int i = NUMBER_AMOUNT - 1; i >= 0; i--) {
            if (numberNew[i] != numberValue[i]) {
                numberValue[i] = numberNew[i];
                control->GetAnimationFamily(AnimFamilyID::NUM_0_PAT + i)->SetAnimation(0, numberNew[i]);
            }
            if (numberNew[i] != 0 || i == 0) hideZeroes = false;
            if (numberNew[i] == 0 && hideZeroes) {
                if (!numberHidden[i]) {
                    numberHidden[i] = true;
                    control->GetNwlytControl()->vtable->setVisibleImpl(control->GetNwlytControl(), numberHandle[i], false);
                }
            } else if (numberHidden[i]) {
                numberHidden[i] = false;
                control->GetNwlytControl()->vtable->setVisibleImpl(control->GetNwlytControl(), numberHandle[i], true);
            }
        }
    }

    void PointsModeDisplayController::ReasonController::DefineAnimation(VisualControl::AnimationDefine* animationDefine) {
        animationDefine->InitAnimationFamily(AnimFamilyID::REASON_IN, "G_reason", 4);
        animationDefine->InitAnimationStopByRate(0, "reason_in", 0.f);
        animationDefine->InitAnimation(1, "reason_in", VisualControl::AnimationDefine::AnimationKind::NEXT);
        animationDefine->InitAnimationStopByRate(2, "reason_in", 1.f);
        animationDefine->InitAnimationReverse(3, "reason_in", VisualControl::AnimationDefine::AnimationKind::ONCE);
    }

    void PointsModeDisplayController::ReasonController::Create(VisualControl::GameVisualControl* control, void* createArgs) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        reasonHandle = layout->vtable->getElementHandle(layout, SafeStringBase("PM_reason"), 0);
        amountHandle = layout->vtable->getElementHandle(layout, SafeStringBase("PM_points"), 0);
    }
    
    void PointsModeDisplayController::ReasonController::Reset(VisualControl::GameVisualControl* control) {
        requestedAmount = currentAmount = 0;
        requestedReason = u"";
        appearRequest = AppearRequest::NONE;
        SetReasonImpl(control, u"");
        SetAmountImpl(control, 0);
        appearTimer = 0;
        control->GetAnimationFamily(AnimFamilyID::REASON_IN)->SetAnimation(0, 0.f);
    }
    
    void PointsModeDisplayController::ReasonController::Calc(VisualControl::GameVisualControl* control) {
        if (!requestedReason.empty()) {
            SetReasonImpl(control, requestedReason);
            requestedReason = u"";
        }
        if (requestedAmount != currentAmount) {
            currentAmount = requestedAmount;
            SetAmountImpl(control, currentAmount);
        }
        if (appearRequest == AppearRequest::APPEAR) {
            control->GetAnimationFamily(AnimFamilyID::REASON_IN)->ChangeAnimation(1, 0.f);
            appearRequest = AppearRequest::NONE;
        } else if (appearRequest == AppearRequest::DISAPPEAR) {
            control->GetAnimationFamily(AnimFamilyID::REASON_IN)->ChangeAnimation(3, 0.f);
            appearRequest = AppearRequest::NONE;
        }
        if (appearTimer != 0) {
            if (--appearTimer == 0) {
                SetReason(u"", 0);
            }
        }
    }

    void PointsModeDisplayController::ReasonController::SetReason(const std::u16string& reason, int amount, const MarioKartTimer& time) {
        if (reason.empty()) {
            Disappear();
        } else {
            appearTimer = time;
            SetReason(reason);
            SetAmount(amount);
            Appear();
        }
    }

    void PointsModeDisplayController::ReasonController::SetReasonImpl(VisualControl::GameVisualControl* control, const std::u16string& reason) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        layout->vtable->replaceMessageImpl(layout, reasonHandle, VisualControl::Message(reason.c_str()), nullptr, nullptr);
    }
    void PointsModeDisplayController::ReasonController::SetAmountImpl(VisualControl::GameVisualControl* control, int amount) {
        std::u16string reason_16;
        Utils::ConvertUTF8ToUTF16(reason_16, Utils::Format("%+d", amount));
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        layout->vtable->replaceMessageImpl(layout, amountHandle, VisualControl::Message(reason_16.c_str()), nullptr, nullptr);
    }

    void PointsModeDisplayController::ComboCongratController::DefineAnimation(VisualControl::AnimationDefine* animationDefine) {
        animationDefine->InitAnimationFamily(AnimFamilyID::COMBO_IN, "G_combo", 4);
        animationDefine->InitAnimationStopByRate(0, "combo_in", 0.f);
        animationDefine->InitAnimation(1, "combo_in", VisualControl::AnimationDefine::AnimationKind::NEXT);
        animationDefine->InitAnimationStopByRate(2, "combo_in", 1.f);
        animationDefine->InitAnimationReverse(3, "combo_in", VisualControl::AnimationDefine::AnimationKind::ONCE);

        animationDefine->InitAnimationFamily(AnimFamilyID::CONGRAT_IN, "G_congrat", 4);
        animationDefine->InitAnimationStopByRate(0, "congrat_in", 0.f);
        animationDefine->InitAnimation(1, "congrat_in", VisualControl::AnimationDefine::AnimationKind::NEXT);
        animationDefine->InitAnimationStopByRate(2, "congrat_in", 1.f);
        animationDefine->InitAnimationReverse(3, "congrat_in", VisualControl::AnimationDefine::AnimationKind::ONCE);

        animationDefine->InitAnimationFamily(AnimFamilyID::CONGRAT_COL, "G_congrat", 1);
        animationDefine->InitAnimation(0, "congrat_col", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    void PointsModeDisplayController::ComboCongratController::Create(VisualControl::GameVisualControl* control, void* createArgs) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        comboHandle = layout->vtable->getElementHandle(layout, SafeStringBase("PM_combo"), 0);
        congratHandle = layout->vtable->getElementHandle(layout, SafeStringBase("PM_congrat"), 0);
    }
    
    void PointsModeDisplayController::ComboCongratController::Reset(VisualControl::GameVisualControl* control) {
        requestedCombo = requestedCongrat = u"";
        SetCongratComboImpl(control, u"", true);
        SetCongratComboImpl(control, u"", false);

        currentColor = requestedColor = CongratColor::NONE;
        control->GetAnimationFamily(AnimFamilyID::CONGRAT_COL)->SetAnimation(0, static_cast<float>(CongratColor::GREEN));

        isComboAppeared = false;
        comboAppearRequest = AppearRequest::NONE; congratAppearRequest = AppearRequest::NONE;
        control->GetAnimationFamily(AnimFamilyID::COMBO_IN)->SetAnimation(0, 0.f);
        control->GetAnimationFamily(AnimFamilyID::CONGRAT_IN)->SetAnimation(0, 0.f);
    }
    
    void PointsModeDisplayController::ComboCongratController::Calc(VisualControl::GameVisualControl* control) {
        if (!requestedCombo.empty()) {
            SetCongratComboImpl(control, requestedCombo, true);
            requestedCombo = u"";
        }
        if (!requestedCongrat.empty()) {
            SetCongratComboImpl(control, requestedCongrat, false);
            requestedCongrat = u"";
        }
        if (requestedColor != currentColor) {
            currentColor = requestedColor;
            control->GetAnimationFamily(AnimFamilyID::CONGRAT_COL)->SetAnimation(0, static_cast<float>(currentColor));
        } else {
            if (comboAppearRequest == AppearRequest::APPEAR) {
                comboAppearRequest = AppearRequest::NONE;
                control->GetAnimationFamily(AnimFamilyID::COMBO_IN)->ChangeAnimation(1, 0.f);
            } else if (comboAppearRequest == AppearRequest::DISAPPEAR) {
                comboAppearRequest = AppearRequest::NONE;
                control->GetAnimationFamily(AnimFamilyID::COMBO_IN)->ChangeAnimation(3, 0.f);
            }
            if (congratAppearRequest == AppearRequest::APPEAR) {
                congratAppearRequest = AppearRequest::NONE;
                control->GetAnimationFamily(AnimFamilyID::CONGRAT_IN)->ChangeAnimation(1, 0.f);
            } else if (congratAppearRequest == AppearRequest::DISAPPEAR) {
                congratAppearRequest = AppearRequest::NONE;
                control->GetAnimationFamily(AnimFamilyID::CONGRAT_IN)->ChangeAnimation(3, 0.f);
            }
        }
    }

    void PointsModeDisplayController::ComboCongratController::SetCongrat(CongratState state) {
        if (state == CongratState::NONE) {
            DisappearCongrat();
        } else {
            SetColor(static_cast<CongratColor>(state));
            SetCongratText(congratNames[static_cast<int>(state) - 1]);
            AppearCongrat();
        }
    }

    void PointsModeDisplayController::ComboCongratController::SetCombo(u32 combo) {
        if (combo == 0) {
            if (isComboAppeared) {
                isComboAppeared = false;
                DisappearCombo();
            }
        } else {
            isComboAppeared = true;
            SetComboText(comboName + to_u16string(combo));
            AppearCombo();
        }
    }

    void PointsModeDisplayController::ComboCongratController::InitializeText()
    {
        Utils::ConvertUTF8ToUTF16(congratNames[0], NAME("sc_at_nice"));
        Language::RemoveEntry("sc_at_nice");
        Utils::ConvertUTF8ToUTF16(congratNames[1], NAME("sc_at_great"));
        Language::RemoveEntry("sc_at_great");
        Utils::ConvertUTF8ToUTF16(congratNames[2], NAME("sc_at_excel"));
        Language::RemoveEntry("sc_at_excel");
        Utils::ConvertUTF8ToUTF16(congratNames[3], NAME("sc_at_fanta"));
        Language::RemoveEntry("sc_at_fanta");
        Utils::ConvertUTF8ToUTF16(comboName, NAME("sc_at_combo"));
        Language::RemoveEntry("sc_at_combo");
    }

    void PointsModeDisplayController::ComboCongratController::SetCongratComboImpl(VisualControl::GameVisualControl* control, const std::u16string& text, bool isCombo) {
        VisualControl::NwlytControlSight* layout = control->GetNwlytControl();
        layout->vtable->replaceMessageImpl(layout, isCombo ? comboHandle : congratHandle, VisualControl::Message(text.c_str()), nullptr, nullptr);
    }
}