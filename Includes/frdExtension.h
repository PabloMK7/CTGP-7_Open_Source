/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: frdExtension.h
Open source lines: 21/21 (100.00%)
*****************************************************/

#pragma once
#include <3ds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

Result FRD_GetMyPassword(char *password, size_t max_size);

#ifdef __cplusplus
}
#endif