/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: httpcPatch.c
Open source lines: 741/741 (100.00%)
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

Handle __httpcPatch_servhandle;
static int __httpcPatch_refcount;

u32 *__httpcPatch_sharedmem_addr;
static u32 __httpcPatch_sharedmem_size;
static Handle __httpcPatch_sharedmem_handle;

static Result HTTPCPATCH_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle);
static Result HTTPCPATCH_Finalize(Handle handle);

static Result HTTPCPATCH_CreateContext(Handle handle, HTTPCPATCH_RequestMethod method, const char* url, Handle* contextHandle);
static Result HTTPCPATCH_CloseContext(Handle handle, Handle contextHandle);

static Result HTTPCPATCH_InitializeConnectionSession(Handle handle, Handle contextHandle);
static Result HTTPCPATCH_SetProxyDefault(Handle handle, Handle contextHandle);

Result httpcPatchInit(u32 sharedmem_addr, u32 sharedmem_size)
{
	Result ret=0;

	if (AtomicPostIncrement(&__httpcPatch_refcount)) return 0;

	ret = srvGetServiceHandle(&__httpcPatch_servhandle, "http:C");
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

		if (R_SUCCEEDED(ret))ret = HTTPCPATCH_Initialize(__httpcPatch_servhandle, __httpcPatch_sharedmem_size, __httpcPatch_sharedmem_handle);
		if (R_FAILED(ret)) svcCloseHandle(__httpcPatch_servhandle);
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

	HTTPCPATCH_Finalize(__httpcPatch_servhandle);

	svcCloseHandle(__httpcPatch_servhandle);

	if(__httpcPatch_sharedmem_handle)
	{
		svcCloseHandle(__httpcPatch_sharedmem_handle);
		__httpcPatch_sharedmem_handle = 0;
		__httpcPatch_sharedmem_size = 0;

		__httpcPatch_sharedmem_addr = NULL;
	}
}

Result httpcPatchOpenContext(httpcPatchContext *context, HTTPCPATCH_RequestMethod method, const char* url, u32 use_defaultproxy)
{
	Result ret=0;

	ret = HTTPCPATCH_CreateContext(__httpcPatch_servhandle, method, url, &context->httphandle);
	if(R_FAILED(ret))return ret;

	ret = srvGetServiceHandle(&context->servhandle, "http:C");
	if(R_FAILED(ret)) {
		HTTPCPATCH_CloseContext(__httpcPatch_servhandle, context->httphandle);
		return ret;
        }

	ret = HTTPCPATCH_InitializeConnectionSession(context->servhandle, context->httphandle);
	if(R_FAILED(ret)) {
		svcCloseHandle(context->servhandle);
		HTTPCPATCH_CloseContext(__httpcPatch_servhandle, context->httphandle);
		return ret;
        }

	if(use_defaultproxy==0)return 0;

	ret = HTTPCPATCH_SetProxyDefault(context->servhandle, context->httphandle);
	if(R_FAILED(ret)) {
		svcCloseHandle(context->servhandle);
		HTTPCPATCH_CloseContext(__httpcPatch_servhandle, context->httphandle);
		return ret;
        }

	return 0;
}

Result httpcPatchCloseContext(httpcPatchContext *context)
{
	Result ret=0;

	svcCloseHandle(context->servhandle);
	ret = HTTPCPATCH_CloseContext(__httpcPatch_servhandle, context->httphandle);

	return ret;
}

Result httpcPatchCancelConnection(httpcPatchContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchDownloadData(httpcPatchContext *context, u8* buffer, u32 size, u32 *downloadedsize)
{
	Result ret=0;
	Result dlret=HTTPCPATCH_RESULTCODE_DOWNLOADPENDING;
	u32 pos=0, sz=0;
	u32 dlstartpos=0;
	u32 dlpos=0;

	if(downloadedsize)*downloadedsize = 0;

	ret=httpcPatchGetDownloadSizeState(context, &dlstartpos, NULL);
	if(R_FAILED(ret))return ret;

	while(pos < size && dlret==HTTPCPATCH_RESULTCODE_DOWNLOADPENDING)
	{
		sz = size - pos;

		dlret=httpcPatchReceiveData(context, &buffer[pos], sz);

		ret=httpcPatchGetDownloadSizeState(context, &dlpos, NULL);
		if(R_FAILED(ret))return ret;

		pos = dlpos - dlstartpos;
	}

	if(downloadedsize)*downloadedsize = pos;

	return dlret;
}

static Result HTTPCPATCH_Initialize(Handle handle, u32 sharedmem_size, Handle sharedmem_handle)
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

static Result HTTPCPATCH_Finalize(Handle handle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x39,0,0); // 0x390000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPCPATCH_CreateContext(Handle handle, HTTPCPATCH_RequestMethod method, const char* url, Handle* contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 l=strlen(url)+1;

	cmdbuf[0]=IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1]=l;
	cmdbuf[2]=method;
	cmdbuf[3]=IPC_Desc_Buffer(l,IPC_BUFFER_R);
	cmdbuf[4]=(u32)url;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	if(contextHandle)*contextHandle=cmdbuf[2];

	return cmdbuf[1];
}

static Result HTTPCPATCH_InitializeConnectionSession(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,1,2); // 0x80042
	cmdbuf[1]=contextHandle;
	cmdbuf[2]=IPC_Desc_CurProcessId();

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPCPATCH_SetProxyDefault(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xE,1,0); // 0xE0040
	cmdbuf[1]=contextHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

static Result HTTPCPATCH_CloseContext(Handle handle, Handle contextHandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1]=contextHandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(handle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchAddRequestHeaderField(httpcPatchContext *context, const char* name, const char* value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	int value_len=strlen(value)+1;

	cmdbuf[0]=IPC_MakeHeader(0x11,3,4); // 0x1100C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=value_len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(value_len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchAddPostDataAscii(httpcPatchContext *context, const char* name, const char* value)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;
	int value_len=strlen(value)+1;

	cmdbuf[0]=IPC_MakeHeader(0x12,3,4); // 0x1200C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=value_len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(value_len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchAddPostDataBinary(httpcPatchContext *context, const char* name, const u8* value, u32 len)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;

	cmdbuf[0]=IPC_MakeHeader(0x13, 3, 4); // 0x1300C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=len;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len,3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(len,IPC_BUFFER_R);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchAddPostDataRaw(httpcPatchContext *context, const u32* data, u32 len)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x14, 2, 2); // 0x140082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=len;
	cmdbuf[3]=IPC_Desc_Buffer(len, IPC_BUFFER_R);
	cmdbuf[4]=(u32)data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))
	{
		return ret;
	}
	return cmdbuf[1];
}

Result httpcPatchBeginRequest(httpcPatchContext *context)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchReceiveData(httpcPatchContext *context, u8* buffer, u32 size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xB,2,2); // 0xB0082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=size;
	cmdbuf[3]=IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4]=(u32)buffer;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchReceiveDataTimeout(httpcPatchContext *context, u8* buffer, u32 size, u64 timeout)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xC,4,2); // 0xC0102
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=size;
	cmdbuf[3]=timeout & 0xffffffff;
	cmdbuf[4]=(timeout >> 32) & 0xffffffff;
	cmdbuf[5]=IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[6]=(u32)buffer;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchGetRequestState(httpcPatchContext *context, HTTPCPATCH_RequestStatus* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = (HTTPCPATCH_RequestStatus)cmdbuf[2];

	return cmdbuf[1];
}

Result httpcPatchGetDownloadSizeState(httpcPatchContext *context, u32* downloadedsize, u32* contentsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	if(downloadedsize)*downloadedsize = cmdbuf[2];
	if(contentsize)*contentsize = cmdbuf[3];

	return cmdbuf[1];
}
Result httpcPatchGetResponseHeader(httpcPatchContext *context, const char* name, char* value, u32 valuebuf_maxsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	int name_len=strlen(name)+1;

	cmdbuf[0]=IPC_MakeHeader(0x1E,3,4); // 0x1E00C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=name_len;
	cmdbuf[3]=valuebuf_maxsize;
	cmdbuf[4]=IPC_Desc_StaticBuffer(name_len, 3);
	cmdbuf[5]=(u32)name;
	cmdbuf[6]=IPC_Desc_Buffer(valuebuf_maxsize, IPC_BUFFER_W);
	cmdbuf[7]=(u32)value;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchGetResponseStatusCode(httpcPatchContext *context, u32* out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1]=context->httphandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}


Result httpcPatchGetResponseStatusCodeTimeout(httpcPatchContext *context, u32* out, u64 timeout)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x23,3,0); // 0x2300C0
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=timeout & 0xffffffff;
	cmdbuf[3]=(timeout >> 32) & 0xffffffff;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	*out = cmdbuf[2];

	return cmdbuf[1];
}

Result httpcPatchAddTrustedRootCA(httpcPatchContext *context, const u8 *cert, u32 certsize)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x24,2,2); // 0x240082
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchAddDefaultCert(httpcPatchContext *context, SSLC_DefaultRootCert certID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x25,2,0); // 0x250080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSelectRootCertChain(httpcPatchContext *context, u32 RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x26,2,0); // 0x260080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=RootCertChain_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetClientCert(httpcPatchContext *context, const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x27,3,4); // 0x2700C4
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=privk_size;
	cmdbuf[4]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[5]=(u32)cert;
	cmdbuf[6]=IPC_Desc_Buffer(privk_size, IPC_BUFFER_R);
	cmdbuf[7]=(u32)privk;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetClientCertDefault(httpcPatchContext *context, SSLC_DefaultClientCert certID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x28,2,0); // 0x280080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetClientCertContext(httpcPatchContext *context, u32 ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x29,2,0); // 0x290080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=ClientCert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetSSLOpt(httpcPatchContext *context, u32 options)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2B,2,0); // 0x2B0080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=options;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetSSLClearOpt(httpcPatchContext *context, u32 options)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2C,2,0); // 0x2C0080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=options;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchCreateRootCertChain(u32 *RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2D,0,0); // 0x2D0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && RootCertChain_contexthandle)*RootCertChain_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcPatchDestroyRootCertChain(u32 RootCertChain_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2E,1,0); // 0x2E0040
	cmdbuf[1]=RootCertChain_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchRootCertChainAddCert(u32 RootCertChain_contexthandle, const u8 *cert, u32 certsize, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x2F,2,2); // 0x2F0082
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=certsize;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcPatchRootCertChainAddDefaultCert(u32 RootCertChain_contexthandle, SSLC_DefaultRootCert certID, u32 *cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x30,2,0); // 0x300080
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && cert_contexthandle)*cert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcPatchRootCertChainRemoveCert(u32 RootCertChain_contexthandle, u32 cert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x31,2,0); // 0x310080
	cmdbuf[1]=RootCertChain_contexthandle;
	cmdbuf[2]=cert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchOpenClientCertContext(const u8 *cert, u32 certsize, const u8 *privk, u32 privk_size, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x32,2,4); // 0x320084
	cmdbuf[1]=certsize;
	cmdbuf[2]=privk_size;
	cmdbuf[3]=IPC_Desc_Buffer(certsize, IPC_BUFFER_R);
	cmdbuf[4]=(u32)cert;
	cmdbuf[5]=IPC_Desc_Buffer(privk_size, IPC_BUFFER_R);
	cmdbuf[6]=(u32)privk;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && ClientCert_contexthandle)*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcPatchOpenDefaultClientCertContext(SSLC_DefaultClientCert certID, u32 *ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x33,1,0); // 0x330040
	cmdbuf[1]=certID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret) && ClientCert_contexthandle)*ClientCert_contexthandle = cmdbuf[2];

	return ret;
}

Result httpcPatchCloseClientCertContext(u32 ClientCert_contexthandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x34,1,0); // 0x340040
	cmdbuf[1]=ClientCert_contexthandle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__httpcPatch_servhandle)))return ret;

	return cmdbuf[1];
}

Result httpcPatchSetKeepAlive(httpcPatchContext *context, HTTPCPATCH_KeepAlive option)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x37,2,0); // 0x370080
	cmdbuf[1]=context->httphandle;
	cmdbuf[2]=option;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(context->servhandle)))return ret;

	return cmdbuf[1];
}
