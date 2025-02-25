/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: DataStructures.cpp
Open source lines: 66/66 (100.00%)
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

    void SeadRandom::Init(u32 seed)
    {
        elements[0] = multValue * (seed ^ (seed >> 30)) + 1;
        elements[1] = multValue * (elements[0] ^ (elements[0] >> 30)) + 1;
        elements[2] = multValue * (elements[1] ^ (elements[1] >> 30)) + 1;
        elements[3] = multValue * (elements[2] ^ (elements[2] >> 30)) + 1;
    }
    u32 SeadRandom::Get()
    {
        u32 el0 = elements[0];
        u32 el1 = elements[1];
        u32 el2 = elements[2];
        u32 el3 = elements[3];
        u32 ret = el0 ^ (el0 << 11) ^ ((el0 ^ (el0 << 11)) >> 8) ^ el3 ^ (el3 >> 19);
        elements[0] = el1;
        elements[1] = el2;
        elements[2] = el3;
        elements[3] = ret;
        return ret;
    }
    u32 SeadRandom::Get(u32 low, u32 high)
    {
        u32 random = Get();
        u32 size = high - low;
        u32 chosen = (u32)(((u64)random * (u64)size) >> 32);
        return chosen + low;
    }
}