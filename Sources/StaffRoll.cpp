/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StaffRoll.cpp
Open source lines: 574/574 (100.00%)
*****************************************************/

#include "StaffRoll.hpp"
#include "MarioKartFramework.hpp"
#include "Math.hpp"
#include "TextFileParser.hpp"
#include "CourseCredits.hpp"
#include "CharacterHandler.hpp"
#include "MusicSlotMngr.hpp"
#include "SaveHandler.hpp"

namespace CTRPluginFramework {

    constexpr MarioKartTimer StaffRoll::mainRollDuration;

    StaffRoll::StaffRollImage::StaffRollImage() {
        heapReturnAlloc = GameAlloc::game_operator_new_autoheap(0x4050);
        bclimData = (u8*)(((u32)heapReturnAlloc + 0x10) & ~0xF);
        memset(bclimData, 0xFF, 0x4000);
        const u8 bclimFooter[] = {
            0x43, 0x4C, 0x49, 0x4D, 0xFF, 0xFE, 0x14, 0x00, 0x00, 0x00, 0x02, 0x02, 0x28, 0x40, 0x00, 0x00,
            0x01, 0x00, 0x00, 0x00, 0x69, 0x6D, 0x61, 0x67, 0x10, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00,
            0x0A, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00
        };
        memcpy(bclimData + 0x4000, bclimFooter, sizeof(bclimFooter));
        sarc = new ExtraResource::StreamedSarc("/CTGP-7/resources/creditsImages.sarc");
        fileReplaceTask = new Task(ReplaceTaskFunc, this, Task::Affinity::SysCore);
        if (sarc->processed) {
            for (int i = 0; i < sarc->GetFileCount() - 2; i++) {
                randomImageList.push_back((u8)i);
            }
            random_shuffle_custom(randomImageList.begin(), randomImageList.end(), [](int i) {
				return Utils::Random(0, i - 1);
			});
        }

        constexpr u32 maxImages = 12;
        if (randomImageList.size() > maxImages) {
            randomImageList.resize(maxImages);
        }
    }

    StaffRoll::StaffRollImage::~StaffRollImage() {
        fileReplaceTask->Wait();
        GameAlloc::game_operator_delete(heapReturnAlloc);
        delete sarc;
        delete fileReplaceTask;
    }

    void StaffRoll::StaffRollImage::LoadControl(MenuPageHandler::GameSequenceSection* page) {
        control = new VisualControl("credits_roll_pr", true, (void*)MenuPageHandler::GameFuncs::creditsRollStaffTextVtable);
        control->SetUserData(this);
        control->SetOnCalcCallback(OnCalc);
        control->Load((u32)page, "omakase_T", VisualControl::ControlType::BASEMENUVIEW_CONTROL);
        elementHandle = control->GetLayoutElementHandle("Image");
        SetAlpha(0.f);
    }

    s32 StaffRoll::StaffRollImage::ReplaceTaskFunc(void* userData) {
        StaffRoll::StaffRollImage* own = (StaffRoll::StaffRollImage*)userData;
        std::string file;
        {
            Lock lock(own->fileReplaceMutex);
            file = own->fileToReplace;
        }
        ExtraResource::SARC::FileInfo fInfo;
        fInfo.fileSize = 0x4000;
        u32* vaddr = own->control->GetGameVisualControl()->GetRawTextureVAddr(own->elementHandle);
        own->sarc->ReadFile(vaddr, fInfo, file, true);
        own->fileIsReplacing = false;
        return 0;
    }

    void StaffRoll::StaffRollImage::OnCalc(VisualControl::GameVisualControl* obj) {
        StaffRollImage* own = (StaffRollImage*)obj->GetVisualControl()->GetUserData();

        if (own->fadeTotFrames >= 0) {
            float val = own->fadeLinear(++own->fadeCurrFrames);
            if (own->fadeCurrFrames >= own->fadeTotFrames) {
                own->ClearFade();
            }
            own->SetAlpha(val);
        }

        Vector3 finalPos = own->position + Vector3(0.f, -44.f, 0.f);
        obj->UpdatePosRaw(own->elementHandle, finalPos);
        obj->UpdateScaleRaw(own->elementHandle, own->scale);
        obj->CalcAnim(own->elementHandle);
    }

    void StaffRoll::StaffRollImage::PreStep() {
        if (!sarc->processed)
            return;
        
        if (needsPrepare && !fileIsReplacing) {
            std::string next;
            if (needsPrepareCTGP7) {
                next = "ctgp7.bclim";
                needsPrepareCTGP7 = false;
                constexpr float scalingFactor = 364.f/256.f;
                position = Vector3();
                scale = Vector3(scalingFactor, scalingFactor, 1.f);
            } else {
                next = std::to_string(randomImageList[currentImage]) + ".bclim";
            }
            
            StartFileReplace(next);
            needsPrepare = false;
        }
        switch (state)
        {
        case State::RUN:
        {
            ++timer;
            if (substate == 0) {
                u32 random = Utils::Random();
                u32 mode = random & 3;
                float fromX, fromY, toX, toY;
                if (mode == 0) { // Left to right
                    fromX = -200 + 128 + 10;
                    toX = 200 - 128 - 10;
                    fromY = ((((random >> 8) & 0x1FF) / 512.f) * 92.f) - 46.f;
                    toY = ((((random >> 17) & 0x1FF) / 512.f) * 92.f) - 46.f;
                } else if (mode == 1) { // Right to left
                    fromX = 200 - 128 - 10;
                    toX = -200 + 128 + 10;
                    fromY = ((((random >> 8) & 0x1FF) / 512.f) * 92.f) - 46.f;
                    toY = ((((random >> 17) & 0x1FF) / 512.f) * 92.f) - 46.f;
                } else if (mode == 2) { // Top to bottom
                    fromX = ((((random >> 8) & 0x1FF) / 512.f) * 124.f) - 62.f;
                    toX = ((((random >> 17) & 0x1FF) / 512.f) * 124.f) - 62.f;
                    fromY = 120 - 64 - 10;
                    toY = -120 + 64 + 10;
                } else if (mode == 3) { // Bottom to top
                    fromX = ((((random >> 8) & 0x1FF) / 512.f) * 124.f) - 62.f;
                    toX = ((((random >> 17) & 0x1FF) / 512.f) * 124.f) - 62.f;
                    fromY =-120 + 64 + 10;
                    toY = 120 - 64 - 10; 
                }
                translationXLinear = Linear(Vector2(0, fromX), Vector2(totalTimePerImage.GetFrames(), toX));
                translationYLinear = Linear(Vector2(0, fromY), Vector2(totalTimePerImage.GetFrames(), toY));
                substate = 1;
            }
            if (substate == 1 && !fileIsReplacing) {
                SetFade(0.f, 1.f, fadeTimePerImage.GetFrames());
                substate = 2;
            }
            if (substate == 2 && timer > totalTimePerImage - fadeTimePerImage) {
                SetFade(1.0, 0.f, fadeTimePerImage.GetFrames());
                substate = 3;
            }
            if (timer > totalTimePerImage) {
                currentImage++;
                ClearFade();
                SetAlpha(0.f);
                if (currentImage >= randomImageList.size()) {
                    state = State::ENDED;
                } else {                    
                    PrepareNext();
                    timer = 0;
                    substate = 0;
                }
            } else {
                position.x = translationXLinear(timer.GetFrames());
                position.y = translationYLinear(timer.GetFrames());
            } 
        }
        break;
        default:
            break;
        }
    }

    void StaffRoll::StaffRollImage::Start() {
        state = State::RUN;
        timer = 0;
    }

    void StaffRoll::StaffRollImage::SetTotalTime(const MarioKartTimer& timer) {
        totalTimePerImage = timer / randomImageList.size();
        if (totalTimePerImage > MarioKartTimer(0, 5, 0)) {
            fadeTimePerImage = MarioKartTimer(0, 2, 0);
        } else {
            fadeTimePerImage = MarioKartTimer(totalTimePerImage.GetFrames() * (3.f/5.f));
        }
    }

    void StaffRoll::StaffRollImage::SetAlpha(float alpha) {
        alpha = std::max(std::min(alpha, 1.f), 0.f);
        auto nwlyt = control->GetGameVisualControl()->GetNwlytControl();
        nwlyt->vtable->setAlphaImpl(nwlyt, elementHandle, alpha * 255);
    }

    void StaffRoll::StaffRollText::StaffRollTextHandler::PreStep() {
        if (fadeTotFrames >= 0) {
            float val = fadeLinear(++fadeCurrFrames);
            if (fadeCurrFrames >= fadeTotFrames) {
                ClearFade();
            }
            SetAlpha(val);
        }
        if (textChanged) {
            textChanged = false;
            std::u16string out;
            Utils::ConvertUTF8ToUTF16(out, currText);
            auto nwlyt = parent->control->GetGameVisualControl()->GetNwlytControl();
            nwlyt->vtable->replaceMessageImpl(nwlyt, elementHandle, VisualControl::Message(out.c_str()), nullptr, nullptr);
        }
        parent->control->GetGameVisualControl()->CalcAnim(elementHandle);
    }

    void StaffRoll::StaffRollText::StaffRollTextHandler::ClearText() {
        auto nwlyt = parent->control->GetGameVisualControl()->GetNwlytControl();
        nwlyt->vtable->replaceMessageImpl(nwlyt, elementHandle, VisualControl::Message(u""), nullptr, nullptr);
        currText.clear();
        textChanged = false;
    }

    void StaffRoll::StaffRollText::StaffRollTextHandler::AddText(const std::string& text) {
        currText += text;
        textChanged = true;
    }

    void StaffRoll::StaffRollText::StaffRollTextHandler::SetAlpha(float alpha) {
        alpha = std::max(std::min(alpha, 1.f), 0.f);
        auto nwlyt = parent->control->GetGameVisualControl()->GetNwlytControl();
        nwlyt->vtable->setAlphaImpl(nwlyt, elementHandle, alpha * 255);
    }

    StaffRoll::StaffRollText::StaffRollText(MenuPageHandler::GameSequenceSection* page) {
        control = new VisualControl("staff_csrl", false, (void*)MenuPageHandler::GameFuncs::creditsRollStaffTextVtable);
        control->Load((u32)page, "staff_csrl", VisualControl::ControlType::BASEMENUVIEW_CONTROL);
        whiteHandler = StaffRollTextHandler(this, control->GetLayoutElementHandle("StaffWhite"));
        redHandler = StaffRollTextHandler(this, control->GetLayoutElementHandle("StaffRed"));
        ClearText();
        whiteHandler.SetAlpha(1.f);
        redHandler.SetAlpha(1.f);
        whiteHandler.ClearFade();
        redHandler.ClearFade();
    }

    void StaffRoll::StaffRollText::AddLine(const std::string& text, TextMode mode) {
        if (mode == TextMode::RED) {
            redHandler.AddText(text + "\n");
            whiteHandler.AddText("\n");
        } else if (mode == TextMode::WHITE) {
            redHandler.AddText("\n");
            whiteHandler.AddText(text + "\n");
        }
    }

    void StaffRoll::InitControl() {
        text = new StaffRollText(page->gameSection);
        image = new StaffRollImage();
        image->LoadControl(page->gameSection);
        LoadMainRoll();
    }

    void StaffRoll::onPageEnter() {
        void(*SequenceStartFadein)(int faderType, u32 frames, int faderScreen) = (decltype(SequenceStartFadein))MenuPageHandler::GameFuncs::StartFadeIn;
        u8* isMusicPlaying = (u8*)page->gameSection + 0x2B1;
        *isMusicPlaying = false;

        SequenceStartFadein(0, 30, 2);
    }

    void StaffRoll::onPagePreStep() {
        void(*BasicSoundStartPrepared)(u32 handle) = (decltype(BasicSoundStartPrepared))MenuPageHandler::GameFuncs::BasicSoundStartPrepared;
        bool(*UItstDemoButton)() = (decltype(UItstDemoButton))MenuPageHandler::GameFuncs::UItstDemoButton;
        ++timer;
        ++currStateTimer;

        switch (currState)
        {
        case RollState::WAIT:
        {
            u8* isMusicPlaying = (u8*)page->gameSection + 0x2B1;
            if (!*isMusicPlaying) {
                u32 sndEngine = MarioKartFramework::getSndEngine();
                u32**** somethingHandle = (u32****)sndEngine;
                if (somethingHandle[0x84/4][0x14/4]) {
                    *isMusicPlaying = true;
                    u32 handle = somethingHandle[0x84/4][0x14/4][0x2C/4][0x8/4];
                    if (handle) BasicSoundStartPrepared(handle);
                    timer = 0;
                    GotoState(RollState::START);
                }
            }
        }
        break;
        case RollState::START:
        {
            
            if (timer > MarioKartTimer(0, 2, 650) && substate == 0) {
                text->AddLine("CTGP-7", StaffRollText::TextMode::RED);
                text->AddLine("Staff Credits", StaffRollText::TextMode::WHITE);
                text->whiteHandler.SetAlpha(0.f);
                substate++;
            }
            if (timer > MarioKartTimer(0, 7, 980) && substate == 1) {
                text->whiteHandler.SetFade(0, 1, MarioKartTimer::ToFrames(0, 0, 800));
                substate++;
            }
            if (timer > MarioKartTimer(0, 15, 330) && substate == 2) {
                text->whiteHandler.SetFade(1, 0, MarioKartTimer::ToFrames(0, 0, 130));
                text->redHandler.SetFade(1, 0, MarioKartTimer::ToFrames(0, 0, 130));
                substate++;
            }
            if (timer > MarioKartTimer(0, 16, 000) && substate == 3) {
                GotoState(RollState::MAIN_ROLL);
                image->Start();
                currRollPage = -1;
                currRollTimer = MarioKartTimer(0xFFFFFFFF);
                text->whiteHandler.ClearFade();
                text->redHandler.ClearFade();
                text->whiteHandler.SetAlpha(0.f);
                text->redHandler.SetAlpha(0.f);
            }
        }
        break;
        case RollState::MAIN_ROLL:
        {
            if (currRollPage < (int)mainRollPages.size()) {
                if (currRollTimer >= timePerRollPage && currRollPage < (int)mainRollPages.size() - 1) {
                    substate = 0;
                    currRollPage++;
                    bool wasContinuing = currRollPage == 0 ? false : mainRollPages[currRollPage - 1].hasContinuation;
                    currRollTimer = 0;
                    text->ClearText();
                    for (int i = 0; i < mainRollPages[currRollPage].entries.size(); i++) {
                        text->AddLine(mainRollPages[currRollPage].entries[i].second, mainRollPages[currRollPage].entries[i].first);
                    }
                    if (!wasContinuing) text->redHandler.SetFade(0.f, 1.f, timePerRollFade.GetFrames());
                    text->whiteHandler.SetFade(0.f, 1.f, timePerRollFade.GetFrames());
                }
                if (currRollTimer >= timePerRollPage - timePerRollFade && substate == 0) {
                    substate = 1;
                    if (!mainRollPages[currRollPage].hasContinuation) text->redHandler.SetFade(1.f, 0.f, timePerRollFade.GetFrames());
                    text->whiteHandler.SetFade(1.f, 0.f, timePerRollFade.GetFrames());
                }
                ++currRollTimer;
            }
            if (currStateTimer >= mainRollDuration) {
                text->whiteHandler.ClearFade();
                text->redHandler.ClearFade();
                text->whiteHandler.SetAlpha(0.f);
                text->redHandler.SetAlpha(0.f);
                GotoState(RollState::SPECIAL_THANKS);
            }
        }
        break;
        case RollState::SPECIAL_THANKS:
        {
            if (substate == 0) {
                image->End();
                image->PrepareCTGP7();
                text->ClearText();
                for (int i = 0; i < specialThanksPage.entries.size(); i++) {
                    text->AddLine(specialThanksPage.entries[i].second, specialThanksPage.entries[i].first);
                }
                text->redHandler.SetFade(0.f, 1.f, MarioKartTimer::ToFrames(0, 2, 0));
                text->whiteHandler.SetFade(0.f, 1.f, MarioKartTimer::ToFrames(0, 2, 0));
                substate++;
            }
            if (currStateTimer > MarioKartTimer(0, 9, 690) && substate == 1) {
                text->redHandler.SetFade(1.f, 0.f, MarioKartTimer::ToFrames(0, 2, 0));
                text->whiteHandler.SetFade(1.f, 0.f, MarioKartTimer::ToFrames(0, 2, 0));
                substate++;
            }
            if (currStateTimer > MarioKartTimer(0, 13, 800) && substate == 2) {
                text->whiteHandler.ClearFade();
                text->redHandler.ClearFade();
                text->whiteHandler.SetAlpha(0.f);
                text->redHandler.SetAlpha(0.f);
                GotoState(RollState::LOGO_THANKS);
            }
        }
        break;
        case RollState::LOGO_THANKS:
        {
            if (substate == 0) {
                image->SetAlpha(1.f);
                substate = 1;
                text->ClearText();
                text->AddLine("Thanks for playing", StaffRollText::TextMode::WHITE);
            }
            if (currStateTimer > MarioKartTimer(0, 3, 330) && substate == 1) {
                text->whiteHandler.SetFade(0.f, 1.f, MarioKartTimer::ToFrames(0, 0, 130));
                substate = 2;
            }
            if (currStateTimer > MarioKartTimer(0, 7, 900) && substate == 2) {
                text->whiteHandler.SetFade(1.0, 0.f, MarioKartTimer::ToFrames(0, 1, 0));
                image->SetFade(1.0, 0.f, MarioKartTimer::ToFrames(0, 1, 0));
                GotoState(RollState::FINISHED);
                SaveHandler::saveData.flags1.creditsWatched = true;
                SaveHandler::SaveSettingsAll();
            }
        }
        break;
        default:
            break;
        }
   
        if ((UItstDemoButton() && SaveHandler::saveData.flags1.creditsWatched) || timer > MarioKartTimer(3, 15, 500)) {
            page->vtable.completePage(page->gameSection, 0);
        }
        text->PreStep();
        image->PreStep();
    }

    void StaffRoll::onPageComplete() {
        
    }

    void StaffRoll::onPageExit() {
        
    }

    void StaffRoll::LoadMainRoll() {
        constexpr u32 itemsPerPage = 10;

        std::vector<std::string> keys;
        std::vector<std::vector<std::string>> items;
        std::vector<std::string> special_thanks;
        {
            File creditsFile("/CTGP-7/resources/credits.ini", File::READ);
            LineReader reader(creditsFile);
            std::string line;
            bool special_thanks_mode = false;

            for (; reader(line);)
            {
                if (line.empty() || line[0] == '#')
                    continue;
                
                if (line[0] == '%') { // RED
                    line = line.substr(1);
                    TextFileParser::Trim(line);
                    TextFileParser::ProcessText(line);
                    keys.push_back(line);
                    items.push_back(std::vector<std::string>());
                } else if (line == "<pg>") { // Add empty key signaling page break
                    keys.push_back("");
                    items.push_back(std::vector<std::string>());
                } else if (line == "<br>") {
                    if (special_thanks_mode) {
                        special_thanks.push_back("");
                    } else {
                        items.back().push_back("");
                    }
                } else if (line == "<sp>") {
                    special_thanks_mode = true;
                } else if (line == "</sp>") {
                    special_thanks_mode = false;
                } else if (line == "<course>") {
                    auto courseAuthors = CourseCredits::GetAllAuthors();
                    std::copy(courseAuthors.begin(), courseAuthors.end(), std::back_inserter(items.back()));
                } else if (line == "<music>") {
                    auto musicAuthors = MusicSlotMngr::GetAllAuthors();
                    std::copy(musicAuthors.begin(), musicAuthors.end(), std::back_inserter(items.back()));
                } else if (line == "<chars>") {
                    auto charAuthors = CharacterHandler::GetAllAuthors();
                    std::copy(charAuthors.begin(), charAuthors.end(), std::back_inserter(items.back()));
                } else {
                    TextFileParser::Trim(line);
                    TextFileParser::ProcessText(line);
                    if (special_thanks_mode) {
                        special_thanks.push_back(line);
                    } else {
                        items.back().push_back(line);
                    }
                }
            }
            if (special_thanks.size() > itemsPerPage - 1)
                special_thanks.resize(itemsPerPage - 1);
        }
        {
            MainRollPage currRollPage;
            u32 currKey = 0;
            while (currKey < keys.size()) {
                std::vector<int> keysInPage;
                u32 currLineAmount = 0;
                bool needsDivision = false;
                while (true) {
                    if (keys[currKey].empty()) { // Page break
                        currKey++;
                        break;
                    }
                    u32 currSize = items[currKey].size() + 1;
                    if (currLineAmount + currSize > itemsPerPage) {
                        if (keysInPage.empty()) { // Will be divided
                            needsDivision = true;
                            keysInPage.push_back(currKey);
                            currKey++;
                        }
                        break;
                    } else {
                        keysInPage.push_back(currKey);
                        currLineAmount += currSize + 1;
                    }
                    currKey++;
                    if (currKey >= keys.size())
                        break;
                }
                if (keysInPage.empty())
                    continue;
                if (needsDivision) {
                    u32 keyIndex = keysInPage[0];
                    currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::RED, keys[keyIndex]));
                    u32 addedEntries = 0;
                    for (int j = 0; j < items[keyIndex].size(); j++) {
                        currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::WHITE, items[keyIndex][j]));
                        addedEntries++;
                        if (addedEntries >= itemsPerPage - 1) {
                            currRollPage.hasContinuation = true;
                            mainRollPages.push_back(currRollPage);
                            currRollPage = MainRollPage();
                            currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::RED, keys[keyIndex]));
                            addedEntries = 0;
                        }
                    }
                    if (addedEntries != 0) {
                        while (addedEntries <  itemsPerPage - 1) {
                            currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::WHITE, ""));
                            addedEntries++;
                        }
                        mainRollPages.push_back(currRollPage);
                    }
                    currRollPage = MainRollPage();
                } else {
                    for (int i = 0; i < keysInPage.size(); i++) {
                        u32 keyIndex = keysInPage[i];
                        currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::RED, keys[keyIndex]));
                        for (int j = 0; j < items[keyIndex].size(); j++) {
                            currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::WHITE, items[keyIndex][j]));
                        }
                        if (i < keysInPage.size() - 1)
                            currRollPage.entries.push_back(std::make_pair(StaffRollText::TextMode::WHITE, ""));
                    }
                    mainRollPages.push_back(currRollPage);
                    currRollPage = MainRollPage();
                }                
            }
            if (special_thanks.size() > 0) {
                specialThanksPage.entries.push_back(std::make_pair(StaffRollText::TextMode::RED, "SPECIAL THANKS"));
                for (int i = 0; i < special_thanks.size(); i++) {
                    specialThanksPage.entries.push_back(std::make_pair(StaffRollText::TextMode::WHITE, special_thanks[i]));
                }
            }
            
        }
        {
            if (mainRollPages.size() == 0) {
                timePerRollPage = timePerRollFade = MarioKartTimer(0xFFFFFFFF);
            } else {
                timePerRollPage = mainRollDuration / (u32)mainRollPages.size();
                if (timePerRollPage > MarioKartTimer(0, 8, 0)) {
                    timePerRollFade = MarioKartTimer(0, 2, 500);
                } else {
                    timePerRollFade = MarioKartTimer(timePerRollPage.GetFrames() * (2.5f/8.f));
                }
            }
            image->SetTotalTime(mainRollDuration - MarioKartTimer(0, 0, 20));
            image->PrepareNext();
        }
    }
}