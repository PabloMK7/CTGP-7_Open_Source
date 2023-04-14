/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: StaffRoll.hpp
Open source lines: 203/203 (100.00%)
*****************************************************/

#pragma once

#include "CTRPluginFramework.hpp"
#include "MenuPage.hpp"
#include "Math.hpp"
#include "MarioKartTimer.hpp"
#include "TextFileParser.hpp"

namespace CTRPluginFramework {
    class StaffRoll
    {
    public:
        class StaffRollImage {
        public:
            StaffRollImage();
            ~StaffRollImage();
            void LoadControl(MenuPageHandler::GameSequenceSection* page);

            static s32 ReplaceTaskFunc(void* userData);

            static void OnCalc(VisualControl::GameVisualControl* obj);
            void PreStep();

            void SetFade(float from, float to, int frames) {
                fadeLinear = Linear(Vector2(0, from), Vector2(frames, to));
                fadeCurrFrames = 0;
                fadeTotFrames = frames;
            }
            void ClearFade() {
                fadeCurrFrames = fadeTotFrames = -1;
            }
            
            void StartFileReplace(const std::string& file) {
                {
                    Lock lock(fileReplaceMutex);
                    fileToReplace = file;
                }
                fileIsReplacing = true;
                bool res = fileReplaceTask->Start();
            }

            u8* GetBclimData() {
                return bclimData;
            }

            enum class State {
                WAIT,
                RUN,
                ENDED,
            };

            void PrepareNext() {
                needsPrepare = true;
            }
            void PrepareCTGP7() {
                needsPrepareCTGP7 = true;
                PrepareNext();
            }
            void Start();
            void End() {state = State::ENDED;}
            void SetTotalTime(const MarioKartTimer& timer);
            void SetAlpha(float alpha);
        private:
            u32 elementHandle;
            Linear fadeLinear;
            Linear translationXLinear;
            Linear translationYLinear;
            int fadeCurrFrames = -1;
            int fadeTotFrames = -1;
            Vector3 position{};
            Vector3 scale{1.f,1.f,1.f};
            u8* bclimData = nullptr;
            void* heapReturnAlloc = nullptr;
            VisualControl* control = nullptr;

            State state = State::WAIT;
            int substate = 0;
            MarioKartTimer timer{};
            std::vector<u8> randomImageList{};
            MarioKartTimer totalTimePerImage{};
            MarioKartTimer fadeTimePerImage{};
            u32 currentImage = 0;
            bool needsPrepare = false;
            bool needsPrepareCTGP7 = false;

            ExtraResource::StreamedSarc* sarc;
            Task* fileReplaceTask = nullptr;
            Mutex fileReplaceMutex;
            std::string fileToReplace{};
            bool fileIsReplacing = false;
        };
        class StaffRollText {
        public:
            class StaffRollTextHandler {
            public:
                StaffRollTextHandler() : parent(nullptr), elementHandle(0) {}
                StaffRollTextHandler(StaffRollText* p, u32 handle) : parent(p), elementHandle(handle) {}

                void PreStep();

                void SetFade(float from, float to, int frames) {
                    fadeLinear = Linear(Vector2(0, from), Vector2(frames, to));
                    fadeCurrFrames = 0;
                    fadeTotFrames = frames;
                }
                void ClearFade() {
                    fadeCurrFrames = fadeTotFrames = -1;
                }
                void SetAlpha(float alpha);

                void ClearText();
                void AddText(const std::string& text);

            private:
                StaffRollText* parent;
                u32 elementHandle;

                Linear fadeLinear;
                int fadeCurrFrames = -1;
                int fadeTotFrames = -1;
                std::string currText;
                bool textChanged = false;
            };
            StaffRollText(MenuPageHandler::GameSequenceSection* page);
            
            enum class TextMode {
                RED,
                WHITE
            };

            void ClearText() {whiteHandler.ClearText(); redHandler.ClearText();}
            void AddLine(const std::string& text, TextMode mode = TextMode::WHITE);
            void PreStep() {whiteHandler.PreStep(); redHandler.PreStep();}

            VisualControl* control;
            StaffRollTextHandler whiteHandler;
            StaffRollTextHandler redHandler;
        };

        enum class RollState {
            WAIT,
            START,
            MAIN_ROLL,
            SPECIAL_THANKS,
            LOGO_THANKS,
            FINISHED
        };

        StaffRoll(MenuPageHandler::MenuEndingPage* ending_page) : page(ending_page) {};
        ~StaffRoll() {delete text; delete image;}

        void InitControl();
        void onPageEnter();
        void onPagePreStep();
        void onPageComplete();
        void onPageExit();

        StaffRollImage* GetStaffRollImage() {
            return image;
        }

        class MainRollPage {
        public:
            bool hasContinuation = false;
            std::vector<std::pair<StaffRollText::TextMode, std::string>> entries;
        };
        static constexpr MarioKartTimer mainRollDuration{2, 36, 700};

    private:
        void GotoState(RollState state) {
            currState = state;
            substate = 0;
            currStateTimer = 0;
        }

        void LoadMainRoll();
        

        MenuPageHandler::MenuEndingPage* page;
        StaffRollText* text = nullptr;
        StaffRollImage* image = nullptr;
        RollState currState = RollState::WAIT;
        MarioKartTimer timer{0};
        MarioKartTimer currStateTimer{0};
        int substate = 0;
        
        std::vector<MainRollPage> mainRollPages;
        MainRollPage specialThanksPage{};
        MarioKartTimer timePerRollPage{};
        MarioKartTimer timePerRollFade{};
        int currRollPage = 0;
        MarioKartTimer currRollTimer{};
    };
}