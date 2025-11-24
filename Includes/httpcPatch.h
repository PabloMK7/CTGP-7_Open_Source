/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: httpcPatch.h
Open source lines: 39/39 (100.00%)
*****************************************************/


#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"
#include "3ds/services/sslc.h"
#include "3ds/services/httpc.h"

typedef enum {
    HTTP_POSTDATATYPE_ASCIIFORM = 0x0,
    HTTP_POSTDATATYPE_MULTIPARTFORM = 0x1,
    HTTP_POSTDATATYPE_RAW = 0x2,
} HTTPC_PostDataType;

/// Initializes HTTPCPATCH. For HTTP GET the sharedmem_size can be zero. The sharedmem contains data which will be later uploaded for HTTP POST. sharedmem_size should be aligned to 0x1000-bytes.
Result httpcPatchInit(u32 sharedmem_addr, u32 sharedmem_size);

/// Exits HTTPCPATCH.
void httpcPatchExit(void);

Result httpcPatchSetPostDataType(httpcContext *context, HTTPC_PostDataType type);

Result httpcPatchSendPostDataRawTimeout(httpcContext *context, const u32* data, u32 len, u64 timeout);

Result httpcPatchNotifyFinishSendPostData(httpcContext *context);

#ifdef __cplusplus
}
#endif