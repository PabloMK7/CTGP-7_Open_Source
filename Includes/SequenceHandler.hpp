/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: SequenceHandler.hpp
Open source lines: 29/29 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {
	class SequenceHandler
	{
		
	public:
		using BinarySequence = u32;
		static const u32 rootSequenceID = 0x14E548E6;
		static void registerBinarySequence(BinarySequence sequence);
		static void addFlowPatch(u32 sequenceID, u32 offset, u16 seqEntry, u16 subEntry);

	private:
		static void applyFlowPatches(u32 sequenceID);
		static BinarySequence getBinarySequenceByID(u32 sequenceID);
		static std::vector<std::pair<u32, BinarySequence>> registeredSequences;
		static std::vector<std::pair<u32, std::pair<u32, u32>>> flowPatches;
	};
}