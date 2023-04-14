/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: acta.h
Open source lines: 33/33 (100.00%)
*****************************************************/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <3ds/types.h>

#define ACT_CURRENT_ACCOUNT 0xfe

Result actAInit();
void actAExit();
Result ACTA_CreateLocalAccount();
Result ACTA_GetAccountInfo(void *out, u32 out_size, u32 block_id, u8 account_index);
Result ACTA_GetAccountCount(u32 *out);
Result ACTA_GetFriendLocalAccountId(u32 *out, u32 index);
Result ACTA_GetPersistentId(u32 *out, u32 index);
Result ACTA_GetAccountIndexOfFriendAccountId(u32 *index, u32 friend_account_id);
Result ACTA_ResetAccount(u8 account_index, bool format_nnid);
Result ACTA_SetDefaultAccount(u8 account_index);

#ifdef __cplusplus
}
#endif
