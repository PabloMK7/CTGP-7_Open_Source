/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SequenceHandler.cpp
Open source lines: 61/61 (100.00%)
*****************************************************/

#include "SequenceHandler.hpp"
#include "MarioKartFramework.hpp"

namespace CTRPluginFramework {
    std::vector<std::pair<u32, SequenceHandler::BinarySequence>> SequenceHandler::registeredSequences;
    std::vector<std::pair<u32, std::pair<u32, u32>>> SequenceHandler::flowPatches;

    void SequenceHandler::registerBinarySequence(BinarySequence sequence) {
        if (!sequence) return;
        u32 sequenceID = ((u32*)sequence)[1];
        int i = 0;
        for (i = 0; i < registeredSequences.size(); i++) {
            if (registeredSequences[i].first == sequenceID) {
                registeredSequences[i].second = sequence;
            }
        }
        if (i >= registeredSequences.size())
            registeredSequences.push_back(std::make_pair(sequenceID, sequence));
        applyFlowPatches(sequenceID);
    }
    SequenceHandler::BinarySequence SequenceHandler::getBinarySequenceByID(u32 sequenceID)
    {
        if (sequenceID == rootSequenceID) return MarioKartFramework::getRootSequence();
        for (int i = 0; i < registeredSequences.size(); i++)
            if (registeredSequences[i].first == sequenceID)
                return registeredSequences[i].second;
        return 0;
    }
    void SequenceHandler::addFlowPatch(u32 sequenceID, u32 offset, u16 subEntry, u16 seqEntry)
    {
        u32 value = subEntry | (seqEntry << 16);
        int i = 0;
        for (i = 0; i < flowPatches.size(); i++) {
            if (flowPatches[i].first == sequenceID && flowPatches[i].second.first == offset) {
                flowPatches[i].second.second = value;
            }
        }
        if (i >= flowPatches.size()) flowPatches.push_back(std::make_pair(sequenceID, std::make_pair(offset, value)));
        applyFlowPatches(sequenceID);
    }
    void SequenceHandler::applyFlowPatches(u32 sequenceID)
    {
        for (int i = 0; i < flowPatches.size(); i++) {
            if (flowPatches[i].first != sequenceID) continue;
            BinarySequence seq = getBinarySequenceByID(sequenceID);
            if (!seq) continue;
            u32 offset = flowPatches[i].second.first;
            u32 value = flowPatches[i].second.second;
            *((u32*)(seq + offset)) = value;
        }
    }
}