/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CTPreview.hpp
Open source lines: 72/72 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "VisualControl.hpp"
#include "ExtraResource.hpp"

namespace CTRPluginFramework {
    class CTPreview {
    public:
        class ReplaceTarget {
        public:
            ReplaceTarget(ExtraResource::MultiSARC& sarc, const std::string& baseName, int targetID);
            bool Replace(ExtraResource::StreamedSarc& sarc, const std::string& courseName, volatile bool* shouldCancel);
            bool IsLoaded() {return loaded;}
        private:
            bool loaded = false;
            void* target_tl;
            void* target_tr;
            void* target_bl;
            void* target_br;
            u32 size_tl;
            u32 size_tr;
            u32 size_bl;
            u32 size_br;
        };
        CTPreview();
        ~CTPreview() {Unload();}

        VisualControl::GameVisualControl* GetControl(void* menupage, bool isTopScreen = true);
        void Load();
        void Unload();
        void Wait() {if (!loaded) return; fileReplaceTask->Wait();}

        void SetPreview(int cup, int track, bool throughBlack);
        void SetPreview(u32 courseID, bool throughBlack);

        void SetTarget(bool target);
        int GetTarget() {return currentTarget;}
    private:
        bool loaded = false;
        static bool globalLoaded;
        std::vector<VisualControl::GameVisualControl*> control;
        static VisualControl::GameVisualControlVtable* controlVtable;
        static VisualControl::AnimationDefineVtable animDefineVtable;
        ExtraResource::StreamedSarc* sarc;
        static Task* fileReplaceTask;
        ReplaceTarget* replaceTargets[2];
        static s32 fileReplaceFunc(void* args);
        static void defineAnimation(VisualControl::AnimationDefine* animDefine);
        void ChangeAnim(int ID);
        // Task args
        volatile bool cancelReplaceTask = false;
        bool currentTarget = false;
        Mutex taskDataMutex;

        bool taskDataQueued = false;
        std::string replaceName;
        //
        void setCancelTask(bool cancel) {Lock lock(taskDataMutex); cancelReplaceTask = cancel;}
        bool fetchTaskData(std::string& out) {Lock lock(taskDataMutex); bool ret = taskDataQueued; out = replaceName; taskDataQueued = false; cancelReplaceTask = false; return ret;}
        void clearTaskData() {Lock lock(taskDataMutex); taskDataQueued = false;}
        void setTaskData(const std::string& in) {Lock lock(taskDataMutex); replaceName = in; taskDataQueued = true;}
    };
}