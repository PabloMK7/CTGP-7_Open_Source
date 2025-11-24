/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: httpcPatch.c
Open source lines: 165/165 (100.00%)
*****************************************************/

#include <string.h>
#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/sslc.h>
#include <httpcPatch.h>
#include <3ds/ipc.h>
#include <3ds/services/httpc.h>

extern Handle __httpc_servhandle;
static int __httpcPatch_refcount;

u32 *__httpcPatch_sharedmem_addr;
static u32 __httpcPatch_sharedmem_size;
static Handle __httpcPatch_sharedmem_handle;

static Result HTTPC_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1,1,4); // 0x10044
	cmdbuf[1]=sharedmem_size; // POST buffer size (page aligned)
	cmdbuf[2]=IPC_Desc_CurProcessId();
	cmdbuf[4]=IPC_Desc_SharedHandles(1);
	cmdbuf[5]=sharedmem_handle;// POST buffer memory block handle

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPC_Finalize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x39,0,0); // 0x390000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchInit(u32 sharedmem_addr, u32 sharedmem_size)
{
	Result ret=0;

	if (AtomicPostIncrement(&__httpcPatch_refcount)) return 0;

	ret = srvGetServiceHandle(&__httpc_servhandle, "http:C");
	if (R_SUCCEEDED(ret))
	{
		__httpcPatch_sharedmem_size = sharedmem_size;
		__httpcPatch_sharedmem_handle = 0;

		if(__httpcPatch_sharedmem_size)
		{
			__httpcPatch_sharedmem_addr = (u32*)sharedmem_addr;
			if(__httpcPatch_sharedmem_addr==NULL)ret = -1;

			if (R_SUCCEEDED(ret))
			{
				memset(__httpcPatch_sharedmem_addr, 0, __httpcPatch_sharedmem_size);
				ret = svcCreateMemoryBlock(&__httpcPatch_sharedmem_handle, (u32)__httpcPatch_sharedmem_addr, __httpcPatch_sharedmem_size, (MemPerm)0, (MemPerm)3);
			}
		}

		if (R_SUCCEEDED(ret))ret = HTTPC_Initialize(__httpc_servhandle, __httpcPatch_sharedmem_size, __httpcPatch_sharedmem_handle);
		if (R_FAILED(ret)) svcCloseHandle(__httpc_servhandle);
	}
	if (R_FAILED(ret)) AtomicDecrement(&__httpcPatch_refcount);

	if (R_FAILED(ret) && __httpcPatch_sharedmem_handle)
	{
		svcCloseHandle(__httpcPatch_sharedmem_handle);
		__httpcPatch_sharedmem_handle = 0;
		__httpcPatch_sharedmem_size = 0;

		__httpcPatch_sharedmem_addr = NULL;
	}

	return ret;
}

void httpcPatchExit(void)
{
	if (AtomicDecrement(&__httpcPatch_refcount)) return;

	HTTPC_Finalize(__httpc_servhandle);

	svcCloseHandle(__httpc_servhandle);

	if(__httpcPatch_sharedmem_handle)
	{
		svcCloseHandle(__httpcPatch_sharedmem_handle);
		__httpcPatch_sharedmem_handle = 0;
		__httpcPatch_sharedmem_size = 0;

		__httpcPatch_sharedmem_addr = NULL;
	}
}

Result httpcPatchSetPostDataType(httpcContext *context, HTTPC_PostDataType type)
{
    u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x15, 2, 0);
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=type;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))
	{
		return ret;
	}
	return cmdbuf[1];
}

Result httpcPatchSendPostDataRawTimeout(httpcContext *context, const u32* data, u32 len, u64 timeout)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1B, 4, 2);
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=len;
    cmdbuf[3]=timeout & 0xffffffff;
	cmdbuf[4]=(timeout >> 32) & 0xffffffff;
	cmdbuf[5]=IPC_Desc_Buffer(len, IPC_BUFFER_R);
	cmdbuf[6]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))
	{
		return ret;
	}
	return cmdbuf[1];
}

Result httpcPatchNotifyFinishSendPostData(httpcContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1D, 1, 0);
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))
	{
		return ret;
	}
	return cmdbuf[1];
}