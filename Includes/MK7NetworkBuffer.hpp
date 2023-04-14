/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MK7NetworkBuffer.hpp
Open source lines: 68/68 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"

namespace CTRPluginFramework {

	struct MK7NetworkBuffer
	{
		u8	bufferType;
		u8* dataPtr;
		u32 dataSize;
		u32 currentSize;

		MK7NetworkBuffer() { ConstructorImpl(this, 0, nullptr, 0);}

		MK7NetworkBuffer(u8 buffertype, u8* dataPtr, u32 dataSize) {ConstructorImpl(this, bufferType, dataPtr, dataSize);}

		static MK7NetworkBuffer* ConstructorImpl(MK7NetworkBuffer* obj, u8 buffertype, u8* dataPtr, u32 dataSize)
		{
			if (!obj)
				return obj;
			obj->bufferType = buffertype;
			obj->dataPtr = dataPtr;
			obj->dataSize = dataSize;
			obj->currentSize = 0;
			if (dataPtr) memset(dataPtr, 0, dataSize);
			return obj;
		}

		inline void Clear() { ClearImpl(this); }

		static void ClearImpl(MK7NetworkBuffer* obj)
		{
			if (!obj || !obj->dataPtr)
				return;
			obj->currentSize = 0;
			memset(obj->dataPtr, 0, obj->dataSize);
		}

		inline void Set(void* newData, u32 newDataSize) { SetImpl(this, (u8*)newData, newDataSize); }

		static void SetImpl(MK7NetworkBuffer* obj, u8* newData, u32 newDataSize)
		{
			if (!obj || !obj->dataPtr || !newData || newDataSize > obj->dataSize)
				return;
			memcpy(obj->dataPtr, newData, newDataSize);
			obj->currentSize = newDataSize;
		}

		inline void Add(void* newData, u32 newDataSize) { AddImpl(this, (u8*)newData, newDataSize); }

		static void AddImpl(MK7NetworkBuffer* obj, u8* newData, u32 newDataSize)
		{
			if (!obj || !obj->dataPtr || !newData || obj->currentSize + newDataSize > obj->dataSize)
				return;
			memcpy(obj->dataPtr + obj->currentSize, newData, newDataSize);
			obj->currentSize += newDataSize;
		}
	};
}