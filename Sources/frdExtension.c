/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: frdExtension.c
Open source lines: 40/40 (100.00%)
*****************************************************/

#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/synchronization.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/srv.h>
#include <3ds/services/frd.h>
#include <3ds/util/utf.h>

Result FRDExt_GetMyPassword(char *password, size_t max_size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbuf = getThreadStaticBuffers();

	cmdbuf[0] = IPC_MakeHeader(0x10,1,0); // 0x100040
	cmdbuf[1] = max_size;

	u32 staticbufBackup[2];
	memcpy(staticbufBackup, staticbuf, 8);

	staticbuf[0] = IPC_Desc_StaticBuffer(max_size, 0);
	staticbuf[1] = (u32)password;

	ret = svcSendSyncRequest(*frdGetSessionHandle());

	memcpy(staticbuf, staticbufBackup, 8);

	return R_SUCCEEDED(ret) ? (Result)cmdbuf[1] : ret;
}