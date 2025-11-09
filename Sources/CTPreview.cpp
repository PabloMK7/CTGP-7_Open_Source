/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CTPreview.cpp
Open source lines: 193/193 (100.00%)
*****************************************************/

#include "CTPreview.hpp"
#include "MenuPage.hpp"
#include "CourseManager.hpp"
#include "UserCTHandler.hpp"
#include "VersusHandler.hpp"
#include "PointsModeHandler.hpp"

namespace CTRPluginFramework {

    VisualControl::GameVisualControlVtable* CTPreview::controlVtable = nullptr;
    VisualControl::AnimationDefineVtable CTPreview::animDefineVtable = {CTPreview::defineAnimation, 0, 0};
    CTPreview* CTPreview::globalLoaded = nullptr;
    Task* CTPreview::fileReplaceTask = nullptr;

    CTPreview::ReplaceTarget::ReplaceTarget(ExtraResource::MultiSARC& sarc, const std::string& base, int targetID) {
        ExtraResource::SARC::FileInfo fInfo;
        target_tl = sarc.GetFile(base + "_TL" + std::to_string(targetID) + ".bclim", &fInfo);
        size_tl = fInfo.fileSize;
        target_tr = sarc.GetFile(base + "_TR" + std::to_string(targetID) + ".bclim", &fInfo);
        size_tr = fInfo.fileSize;
        target_bl = sarc.GetFile(base + "_BL" + std::to_string(targetID) + ".bclim", &fInfo);
        size_bl = fInfo.fileSize;
        target_br = sarc.GetFile(base + "_BR" + std::to_string(targetID) + ".bclim", &fInfo);
        size_br = fInfo.fileSize;
        loaded = target_tl && target_tr && target_bl && target_br && size_tl && size_tr && size_bl && size_br;
    }

    bool CTPreview::ReplaceTarget::Replace(ExtraResource::StreamedSarc& sarc, const std::string& courseName, volatile bool* shouldCancel) {
        if (!loaded)
            return false;
        ExtraResource::SARC::FileInfo fInfo;
        bool ret = true;
        if (!*shouldCancel && ret) {
            fInfo.fileSize = size_tl;
            ret = sarc.ReadFile(target_tl, fInfo, courseName + "_TL" + ".bclim");
        }
        if (!*shouldCancel && ret) {
            fInfo.fileSize = size_tr;
            ret = sarc.ReadFile(target_tr, fInfo, courseName + "_TR" + ".bclim");
        }
        if (!*shouldCancel && ret) {
            fInfo.fileSize = size_bl;
            ret = sarc.ReadFile(target_bl, fInfo, courseName + "_BL" + ".bclim");
        }
        if (!*shouldCancel && ret) {
            fInfo.fileSize = size_br;
            ret = sarc.ReadFile(target_br, fInfo, courseName + "_BR" + ".bclim");
        }
        return ret;
    }

    CTPreview::CTPreview() {}

    void CTPreview::defineAnimation(VisualControl::AnimationDefine* anim) {
        anim->InitAnimationFamilyList(1);
        
        anim->InitAnimationFamily(0, "G_movie", 1);
        anim->InitAnimation(0, "tex_pat", VisualControl::AnimationDefine::AnimationKind::NOPLAY);
    }

    VisualControl::GameVisualControl* CTPreview::GetControl(void* menupage, bool isTopScreen) {
        u32 omakaseViewVtable = MenuPageHandler::GameFuncs::omakaseViewVtable;
        if (!controlVtable) {
            controlVtable = (VisualControl::GameVisualControlVtable*)operator new(sizeof(VisualControl::GameVisualControlVtable));
            memcpy(controlVtable, (u32*)omakaseViewVtable, sizeof(VisualControl::GameVisualControlVtable));
            controlVtable->onReset = (decltype(controlVtable->onReset))VisualControl::nullFunc;
        }
        VisualControl::GameVisualControl* ret = VisualControl::Build((u32*)menupage, "course_pr", isTopScreen ? "omakase_T" : "omakase_B", &animDefineVtable, controlVtable, VisualControl::ControlType::BASEMENUVIEW_CONTROL);
        control.push_back(ret);
        return ret;
    }

    void CTPreview::Load() {
        if (loaded)
            return;
        
        if (globalLoaded) {
            globalLoaded->Unload();
            globalLoaded = nullptr;
        }
        
        sarc = new ExtraResource::StreamedSarc("/CTGP-7/resources/coursePreviews.sarc");
        if (!sarc->processed) {
            delete sarc;
            return;
        }
        replaceTargets[0] = new ReplaceTarget(*ExtraResource::mainSarc, "UI/menu.szs/course_movie", 0);
        if (!replaceTargets[0]->IsLoaded()) {
            delete replaceTargets[0];
            delete sarc;
            return;
        }
        replaceTargets[1] = new ReplaceTarget(*ExtraResource::mainSarc, "UI/menu.szs/course_movie", 1);
        if (!replaceTargets[1]->IsLoaded()) {
            delete replaceTargets[0];
            delete replaceTargets[1];
            delete sarc;
            return;
        }
        if (!fileReplaceTask)
            fileReplaceTask = new Task(fileReplaceFunc, nullptr, Task::Affinity::AppCore);
        loaded = true;
        globalLoaded = this;
    }

    void CTPreview::Unload() {
        if (!loaded)
            return;
        clearTaskData();
        setCancelTask(true);
        fileReplaceTask->Wait();
        delete sarc;
        delete replaceTargets[0];
        delete replaceTargets[1];
        loaded = false;
        if (globalLoaded == this) {
            globalLoaded = nullptr;
        }
    }

    void CTPreview::SetPreview(int cup, int track, bool throughBlack) {
        if (!loaded)
            return;
        if (cup == USERCUPID || cup == POINTSRANDOMCUPID || cup == POINTSWEEKLYCHALLENGECUPID || cup == VERSUSCUPID || cup == -1) {
            setCancelTask(true);
            clearTaskData();
            ChangeAnim(0);
            return;
        } else if (cup == -2) {
            setTaskData("omakase");
            setCancelTask(true);
            if (throughBlack)
                ChangeAnim(0);
            fileReplaceTask->Start(this);
        } else {
            u32 courseID;
            CourseManager::getGPCourseID(&courseID, cup, track);
            setTaskData(CourseManager::getCourseData(courseID)->name);
            setCancelTask(true);
            if (throughBlack)
                ChangeAnim(0);
            fileReplaceTask->Start(this);
        }
    }

    void CTPreview::SetPreview(u32 courseID, bool throughBlack) {
        if (!loaded)
            return;
        setTaskData(CourseManager::getCourseData(courseID)->name);
        setCancelTask(true);
        if (throughBlack)
            ChangeAnim(0);
        fileReplaceTask->Start(this);
    }

    void CTPreview::SetTarget(bool target) {
        if (!loaded)
            return;
        currentTarget = target;
        ChangeAnim((currentTarget ? 1 : 0) + 1);
    }

    s32 CTPreview::fileReplaceFunc(void* args) {
        CTPreview* own = static_cast<CTPreview*>(args);
        std::string toReplace;
        while (own->fetchTaskData(toReplace)) {
            if (own->replaceTargets[(!own->currentTarget) ? 1 : 0]->Replace(*own->sarc, toReplace, &own->cancelReplaceTask))
            {
                if (!own->cancelReplaceTask) {
                    own->SetTarget(!own->currentTarget);
                }
            } else {
                own->ChangeAnim(0);
            }
        }
        return 0;
    }

    void CTPreview::ChangeAnim(int ID) {
        Lock lock(taskDataMutex);
        for (auto it = control.begin(); it != control.end(); it++)
            (*it)->GetAnimationFamily(0)->SetAnimation(0, ID);
    }
}