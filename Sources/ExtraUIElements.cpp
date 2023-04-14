/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: ExtraUIElements.cpp
Open source lines: 645/645 (100.00%)
*****************************************************/

#include "ExtraUIElements.hpp"
#include "SaveHandler.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "UserCTHandler.hpp"
#include "MusicSlotMngr.hpp"
#include "MissionHandler.hpp"

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
        const u16* units[] = {(u16*)u"km/h", (u16*)u"mph"};
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
        const u16* units[] = {(u16*)u"km/h", (u16*)u"mph"};
        VisualControl::Message msg(units[(u32)own->GetUnit()]);
        layout->vtable->replaceMessageImpl(layout, textHandle, msg, nullptr, nullptr);
        if (own->GetUnit() == SpeedUnit::MPH) {
            const char* elements[] = {"T_01", "T_02", "T_03", "T_04", "T_05", "T_06"};
            const u16* units[] = {(u16*)u"15", (u16*)u"30", (u16*)u"45", (u16*)u"60", (u16*)u"75", (u16*)u"90"};
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
                string16 out;
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
}