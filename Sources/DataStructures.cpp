/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: DataStructures.cpp
Open source lines: 38/38 (100.00%)
*****************************************************/

#include "DataStructures.hpp"
#include "ExtraResource.hpp"
#include "MarioKartFramework.hpp"

namespace CTRPluginFramework {
    void (*SndLfoSin::CalcImpl)(SndLfoSin* myself) = nullptr;

    ActorArray::ActorArray(u32 elCount) {
        array.SetBuffer(elCount, (u32**)GameAlloc::game_operator_new_autoheap(elCount * 4));
        array.Fill(0);
        count = 0;
    }

    CustomFieldObjectCreateArgs::CustomFieldObjectCreateArgs(u32 gobjID) : args(), gObjParams(), gObjEntry() {
        gObjEntry.objID = gobjID;
        gObjParams.objEntry = &gObjEntry;
        args.params = &gObjParams;
        args.objFlowEntries = (SeadArrayPtr<void*>*)(MarioKartFramework::getObjectDirector() + 0x28);
    }

    KartButtonData KartButtonData::GetFromVehicle(u32 vehicle) {
        KartButtonData ret;
        ret.raw = *(u32*)(vehicle + 0xFC);
        u32 keyData = ((Key***)vehicle)[0xE4/4][0x8/4][0x110/4];
        if (keyData & 0x8 || keyData & 0x2000)
            ret.item = true;
        return ret;
    }
}