/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CrashReport.cpp
Open source lines: 266/267 (99.63%)
*****************************************************/

#include "CrashReport.hpp"
#include "MarioKartFramework.hpp"
#include "CourseManager.hpp"
#include "base64.hpp"
#include "main.hpp"
#include "csvc.h"
#include "ExtraResource.hpp"
#include "CharacterHandler.hpp"

extern "C" char* __text_end__;

namespace CTRPluginFramework {

    CrashReport::StateID CrashReport::stateID = CrashReport::StateID::STATE_UNINITIALIZED;
    CpuRegisters CrashReport::abortRegs;
    ERRF_ExceptionInfo CrashReport::abortExcep;
    bool CrashReport::fromAbort = false;
    const char* CrashReport::textCrash = nullptr;
    Sound* CrashReport::oofSound = nullptr;

    u32 __attribute__((no_instrument_function)) CrashReport::SaveStackPointer() {
#ifndef _MSC_VER
        __asm__ __volatile__(
            "MOV R0, SP \n"
            "BX LR \n"
        );
#endif
    }

    void CrashReport::OnAbort() {
    #if CITRA_MODE == 1
        panic("abort() called");
    #else
        u32 sp = SaveStackPointer();
        {
            memset(&abortRegs, 0, sizeof(CpuRegisters));
            memset(&abortExcep, 0, sizeof(ERRF_ExceptionInfo));
            abortRegs.sp = sp;
            abortExcep.type = (ERRF_ExceptionType)ExceptionType::EXTYPE_ABORT;
            fromAbort = true;
        }
        // Trigger exception
        *(u32*)nullptr = 0;
    #endif
        for (;;);
    }

    void CrashReport::populateReportData(const ERRF_ExceptionInfo* excep, const CpuRegisters* regs)
    {
        reportData.ctgp7ver = MarioKartFramework::ctgp7ver;
        reportData.regRev = (MarioKartFramework::region << 4) | MarioKartFramework::revision;
        memset(&reportData.padding, 0xFF, sizeof(reportData.padding));
        if (!textCrash) {
            reportData.magic = QRREPORTMAGIC | BINARYREPORTVERSION;
            memset(&reportData.binaryData.registerInfo, 0xFF, sizeof(reportData.binaryData.registerInfo));
            memset(&reportData.binaryData.gameState, 0xFF, sizeof(reportData.binaryData.gameState));
            reportData.exceptionType = excep->type;
            switch (excep->type)
            {
            case ExceptionType::EXTYPE_PREFETCH:
            case ExceptionType::EXTYPE_DATA:
                reportData.binaryData.registerInfo.far = excep->far;
            case ExceptionType::EXTYPE_UNDINS:
            case ExceptionType::EXTYPE_ABORT:
                populateRegisterInfo(regs);
            default:
                break;
            }
            reportData.binaryData.gameState.stateArgs0 = stateID;
            switch (stateID)
            {
            case CTRPluginFramework::CrashReport::STATE_MENU:
                reportData.binaryData.gameState.stateArgs1 = MarioKartFramework::lastLoadedMenu & 0xFFFFFF;
                break;
            case CTRPluginFramework::CrashReport::STATE_RACE:
                reportData.binaryData.gameState.stateArgs1 = CourseManager::lastLoadedCourseID & 0xFFFFFF;
                break;
            case CrashReport::STATE_UNINITIALIZED:
            case CrashReport::STATE_PATCHPROCESS:
            case CrashReport::STATE_INITIALIZE:
            case CrashReport::STATE_MAIN:
            case CrashReport::STATE_TROPHY:
            default:
                break;
            }
        } else {
            reportData.magic = QRREPORTMAGIC | TEXTREPORTVERSION;
            strncpy(reportData.textData, textCrash, sizeof(CrashReport::QRData::BinaryException));
        }
    }

    void CrashReport::DrawTopScreen(Screen& scr)
    {
        scr.DrawRect(20, 20, 360, 200, Color::Black);
        int posY = 25;
        scr.Draw("An exception occurred!", 24, posY, Color::Red);
        const char** crashMessage = getRandomCrahMsg();
        posY += 10 * 2;
        if (!CharacterHandler::potentialCrashReason.empty()) {
            scr.Draw("Reason:", 25, posY, Color::Gray);
            posY += 10 * 1;
            scr.Draw(" Custom character", 25, posY, Color::Gray);
            posY += 10 * 1;
            scr.Draw(" " + CharacterHandler::potentialCrashReason.substr(0, 19), 25, posY, Color::Gray);
            posY += 10 * 3;
        } else {
            scr.Draw(crashMessage[0], 25, posY, Color::Gray);
            posY += 10 * 1;
            scr.Draw(crashMessage[1], 25, posY, Color::Gray);
            posY += 10 * 4;
        }
        
        scr.Draw("B: Home menu.", 25, posY, Color::White);
        posY += 10 * 1;
        scr.Draw("X: Reboot.", 25, posY, Color::White);
        posY += 10 * 1;
        scr.Draw("Y: Extended info.", 25, posY, Color::White);
        posY += 10 * 4;
        scr.Draw("Take a good quality", 25, posY, Color::Gray);
        posY += 10 * 1;
        scr.Draw("picture of the QR", 25, posY, Color::Gray);
        posY += 10 * 1;
        scr.Draw("code and ask for", 25, posY, Color::Gray);
        posY += 10 * 1;
        scr.Draw("help in the CTGP-7", 25, posY, Color::Gray);
        posY += 10 * 1;
        scr.Draw("discord server:", 25, posY, Color::Gray);
        posY += 10 * 1;
        scr.Draw("invite.gg/ctgp7", 25, posY, Color::White);
        DrawQRCode(scr, &reportData);
    }

    void CrashReport::DrawQRCode(Screen& scr, QRData* qrData)
    {
        std::string base64Data = base64_encode((u8*)qrData, sizeof(QRData));
        qrcodegen::QrCode qrcode = qrcodegen::QrCode::encodeText(base64Data.c_str(), qrcodegen::QrCode::Ecc::MEDIUM);
        int screenX = 172 + 20, screenY = 12 + 20, endScreenX = 350 + 20, endScreenY = 190 + 20;
        int qrSize = qrcode.getSize();
        if (qrSize > endScreenY - screenY) return;
        float pixelsPerModule = (endScreenY - screenY) / (float)qrSize;
        float drawStartX = screenX, drawStartY = screenY;
        int qrx = 0, qry = 0;
        scr.DrawRect(160 + 20, 0 + 20, 200, 200, Color::White);
        while (drawStartX < endScreenX && qrx < qrSize)
        { 
            qry = 0;
            drawStartY = screenY;
            while (drawStartY < endScreenY && qry < qrSize)
            {
                u32 drawSizeX = (u32)(drawStartX + pixelsPerModule) - (u32)drawStartX;
                u32 drawSizeY = (u32)(drawStartY + pixelsPerModule) - (u32)drawStartY;
                scr.DrawRect(drawStartX, drawStartY, drawSizeX, drawSizeY, qrcode.getModule(qrx, qry) ? Color::Black : Color::White);
                qry++;
                drawStartY += pixelsPerModule;
            }
            qrx++;
            drawStartX += pixelsPerModule;
        }
    }

    Process::ExceptionCallbackState CrashReport::CTGPExceptCallback(ERRF_ExceptionInfo* excep, CpuRegisters* regs)
    {
#ifdef RELEASE_BUILD
        if (stateID == StateID::STATE_UNINITIALIZED || stateID == StateID::STATE_PATCHPROCESS || stateID == StateID::STATE_INITIALIZE) return Process::EXCB_DEFAULT_HANDLER;
        static bool first = true;
        if (first) {
            first = false;
            CrashTaskData taskData;
            if (fromAbort) {
                fromAbort = false;
                excep = &abortExcep;
                regs = &abortRegs;
            }
            taskData.excep = excep;
            taskData.regs = regs;
            // Task topTask([](void* data) {
            CrashTaskData* cd = static_cast<CrashTaskData*>(&taskData);
            CrashReport cr;
            cr.populateReportData(cd->excep, cd->regs);
            Screen top = OSD::GetTopScreen();
            top.Fade(0.3f);
            cr.DrawTopScreen(top);
            if (oofSound) oofSound->Play();
            // return (s32)0;
         // }, &taskData);
         // topTask.Start();
         // topTask.Wait();
            OSD::SwapBuffers();
        }
        Controller::Update();
        if (Controller::IsKeyPressed(Key::B)) return Process::EXCB_RETURN_HOME;
        else if (Controller::IsKeyPressed(Key::X)) return Process::EXCB_REBOOT;
        else if (Controller::IsKeyPressed(Key::Y)) return Process::EXCB_DEFAULT_HANDLER;
        else return Process::EXCB_LOOP;
#else
        return Process::EXCB_DEFAULT_HANDLER;
#endif // RELEASE_BUILD
    }

    const char** CrashReport::getRandomCrahMsg()
    {
        static const char* messages[][2] = {
            {"This didn't go", "to plan..."},
            {"Would you give me", "a hug?"},
            {"This is so", "embarassing..."},
            {"Blame PabloMK7", "for this!"},
            {"Try our sister mod:", "CPGT-7!"},
            {"(Insert intelligent", "quote.)"},
            {"You just got", "CTGP-7'd."},
            {"OK, this just", "happened..."},
            {"I wonder what", "Ganon is up to!"},
            {"Look at this fancy", "crash screen!"},
            {"Who let the", "bugs out?"},
            {"You fell in", "my trap! >:D"},
            {"I'm CTGP-7, and I", "like to crash. ^.^"},
            {"I'm a secret!", "Well, not really..."},
            {"Bam!", "Game crash!"},
            {"Come here", "fishy fishy."}
        };
        u32 rnd = svcGetSystemTick();
        return messages[(rnd & 0xF00) >> 8];
    }
    void CrashReport::populateRegisterInfo(const CpuRegisters* regs)
    {
        u32 pluginTextStart = 0x07000100;
        u32 pluginTextEnd = (u32)&__text_end__;
        u32 gameTextStart = 0x100000;
        u32 gameTextEnd = Process::GetTextSize() + gameTextStart;
        s64 info;

        reportData.binaryData.registerInfo.lr = regs->lr;
        reportData.binaryData.registerInfo.sp = regs->sp;
        reportData.binaryData.registerInfo.pc = regs->pc;

        u32 stackdata = regs->sp;
        u32 stackOffset = 0;
 
        for (int i = 0; i < (sizeof(reportData.binaryData.registerInfo.callStack) / sizeof(u32)); i++) {
            u32 value;
            while (Process::Read32(stackdata + stackOffset, value) && stackOffset < 0x7000) {
                stackOffset += 4;
                if ((value >= gameTextStart && value <= gameTextEnd) || (value >= pluginTextStart && value <= pluginTextEnd)) {
                    reportData.binaryData.registerInfo.callStack[i] = value;
                    break;
                }
            }
        }
    }

    void CrashReport::Initialize() {
        u8* oofData = ExtraResource::mainSarc->GetFile("Plugin/oof.bcwav", nullptr);
        if (oofData) {
            oofSound = new Sound(oofData);
        }
    }
}

