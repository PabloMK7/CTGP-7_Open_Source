/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: foolsday.cpp
Open source lines: 221/248 (89.11%)
*****************************************************/

#include "foolsday.hpp"
#include <3ds.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <cstdlib>
#include "main.hpp"
#include "MarioKartFramework.hpp"
#include "Lang.hpp"
#include "SaveHandler.hpp"
#include "entrystructs.hpp"
#include "ExtraResource.hpp"
#include "LED_Control.hpp"
#include "AsyncRunner.hpp"

namespace CTRPluginFramework {

    bool g_isFoolActive = false;
    static ExtraResource::SARC* foolResources;
    static void* foolResourcesBuff;
    static Sound* sirenSound;
    static Sound* crashSound;
    static RGBLedPattern sirenLedPat;
    bool checkFoolsDay() {
        time_t unixTime = time(NULL);
        struct tm* timeStruct = gmtime((const time_t *)&unixTime);
        int day = timeStruct->tm_mday;
        int month = timeStruct->tm_mon + 1;
        if (day == 1 && month == 4) {
            if (SaveHandler::saveData.flags1.readyToFool) {
				SaveHandler::saveData.flags1.readyToFool = false;
                g_isFoolActive = true;
                return true;
            }
            return false;
        } else {
			SaveHandler::saveData.flags1.readyToFool = true;
            return false;
        }
    }

    /*void applyFoolsJokeMsbt() {
        std::vector<u16*> shuffle;
        for (int i = 0; i < *Language::commonMsbt->msbtTablePtr; i++)
        {
            u16* ptr = Language::commonMsbt->getTextFromID(i, false);
            if (ptr && *ptr != 0)
                shuffle.push_back(ptr);
        }
        for (int i = 0; i < *Language::menuMsbt->msbtTablePtr; i++)
        {
            u16* ptr = Language::menuMsbt->getTextFromID(i, false);
            if (ptr && *ptr != 0)
                shuffle.push_back(ptr);
        }
        for (int i = 0; i < *Language::raceMsbt->msbtTablePtr; i++)
        {
            u16* ptr = Language::raceMsbt->getTextFromID(i, false);
            if (ptr && *ptr != 0)
                shuffle.push_back(ptr);
        }

        for (int i = 0; i < *Language::commonMsbt->msbtTablePtr; i++)
        {
            u32 id = Utils::Random(0, shuffle.size() - 1);
            Language::commonMsbt->replaceIngameText(i, false, shuffle[id]);
        }
        for (int i = 0; i < *Language::menuMsbt->msbtTablePtr; i++)
        {
            u32 id = Utils::Random(0, shuffle.size() - 1);
            Language::menuMsbt->replaceIngameText(i, false, shuffle[id]);
        }
        for (int i = 0; i < *Language::raceMsbt->msbtTablePtr; i++)
        {
            u32 id = Utils::Random(0, shuffle.size() - 1);
            Language::raceMsbt->replaceIngameText(i, false, shuffle[id]);
        }
    }*/
    static u64  g_foolTextSeed;
    void setFoolsSeed() {
        g_foolTextSeed = Utils::Random() | (((u64)Utils::Random()) << 32);
    }
    const char16_t* getFoolsText() {
        const char16_t* piracyStr[] = {
            u"CHECKSUM FAILED",
            u"PIRACY IS NO FUN",
            u"POWER OFF NOW",
            u"REPORT STOLEN SOFTWARE"
        };
        g_foolTextSeed = rol<u64>(g_foolTextSeed, 3);
        return piracyStr[g_foolTextSeed & 3];
    }
    void applyRaceJoke() {
        std::string text = "It is a serious crime\nto copy videogames.\n\nPOWER OFF THE SYSTEM NOW\nAND REPORT STOLEN SOFTWARE\n\nNintendo    Retro Studios";
        MarioKartFramework::openDialog(DialogFlags::Mode::NOBUTTON, text);
    }
    void loadJokeResources()
    {
        File sarcFile("/CTGP-7/resources/fiximg.bin");
        if (!sarcFile.IsOpen())
            panic("fiximg.bin missing or corrupted.");
        foolResourcesBuff = ::operator new(sarcFile.GetSize());
        sarcFile.Read(foolResourcesBuff, sarcFile.GetSize());
        foolResources = new ExtraResource::SARC((u8*)foolResourcesBuff, false);
        if (!foolResources->processed)
            panic("fiximg.bin missing or corrupted.");
        ExtraResource::SARC::FileInfo finfo;
        sirenSound = new Sound(foolResources->GetFile("siren.bcwav", &finfo), 1);
        crashSound = new Sound(foolResources->GetFile("crash.bcwav", &finfo), 1);
        LED::GeneratePattern(sirenLedPat, Color(0xFF, 0x00, 0xFF), LED_PatType::WAVE_ASC, Seconds(1), Seconds(0), 0x20, 0.f, 0.f, 0.5f);
    }
    void playSirenJoke()
    {
        LED::PlayLEDPattern(sirenLedPat, Seconds(4));
        DirectSoundModifiers modifiers;
        modifiers.forceSpeakerOutput = true;
        modifiers.ignoreVolumeSlider = true;
        modifiers.leftChannelVolume  = 0.5f;
        sirenSound->PlayDirectly(0, -1, 0, 0, modifiers);
    }

#define PA_PTR(addr)            (void *)((u32)(addr) | 1 << 31)
#define REG32(addr)             (*(vu32 *)(PA_PTR(addr)))
    static void toggleLeds()
    {
        
        u8 result;
        MCUHWC_ReadRegister(0x28, &result, 1);
        result = ~result;
        MCUHWC_WriteRegister(0x28, &result, 1);
        
    }

    static Clock exitbromjokeclock;
    static bool firstPart = true;
    static Process::ExceptionCallbackState foolExceptCallback(ERRF_ExceptionInfo* excep, CpuRegisters* regs)
    {
        if (firstPart)
        {
            u32 brightTop, brightBot;
            mcuHwcInit();
            DirectSoundModifiers modifiers;
            modifiers.forceSpeakerOutput = true;
            modifiers.ignoreVolumeSlider = true;
            modifiers.leftChannelVolume = 0.5f;
            crashSound->PlayDirectly(0, -1, 0, 0, modifiers);
            Sleep(Seconds(0.5f));
            REG32(0x10202204) = 0x01000000; // Fill color top
            REG32(0x10202A04) = 0x01000000; // Fill color bot
            Sleep(Seconds(0.5f));
            toggleLeds();
            brightTop = REG32(0x10202240);
            brightBot = REG32(0x10202A40);
            for (int i = 0; i < 40; i++)
            {
                REG32(0x10202240) = 0;
                REG32(0x10202A40) = 0;
                Sleep(Seconds(0.1f));
            }
            Screen top = OSD::GetTopScreen();
            Screen bot = OSD::GetBottomScreen();
            top.DrawRect(0, 0, 400, 240, Color(0, 0, 255), true);
            bot.DrawRect(0, 0, 320, 240, Color(0, 0, 255), true);
            ExtraResource::SARC::FileInfo finfo;
            u8* bootText = foolResources->GetFile("brick.bin", &finfo);
            for (int i = 0; i < finfo.fileSize; i += 2)
            {
                top.DrawPixel(bootText[i], bootText[i + 1], Color(255, 255, 0));
                bot.DrawPixel(bootText[i], bootText[i + 1], Color(255, 255, 0));
            }
            OSD::SwapBuffers();
            toggleLeds();
            REG32(0x10202204) = 0; // Fill color top
            REG32(0x10202A04) = 0; // Fill color bot
            REG32(0x10202240) = brightTop;
            REG32(0x10202A40) = brightBot;
            exitbromjokeclock.Restart();
            firstPart = false;
        }
        else if (exitbromjokeclock.HasTimePassed(Seconds(5.f)))
        {
            mcuHwcExit();
            return Process::ExceptionCallbackState::EXCB_RETURN_HOME;
        }
        return Process::ExceptionCallbackState::EXCB_LOOP;
    }

    static Clock playMemEraseClock;
    static bool showNewDialog = true;
    static void playMemEraseJokeMenuCallback()
    {
        
        if (showNewDialog)
        {
            if (MarioKartFramework::isDialogOpened())
                return;
            showNewDialog = false;
            MarioKartFramework::openDialog(DialogFlags::Mode::LOADING, "STARTING EMERGENCY\nMEMORY DESTRUCTION");
        }
        if (!playMemEraseClock.HasTimePassed(Seconds(3.f)))
            return;
        Process::exceptionCallback = foolExceptCallback;
        // Trigger exception
        *(u32*)1 = 1;
    }
    void playMemEraseJoke()
    {
        MarioKartFramework::closeDialog();
        AsyncRunner::StartAsync(playMemEraseJokeMenuCallback);
        playMemEraseClock.Restart();
    }
}