/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: socCustom.h
Open source lines: 54/54 (100.00%)
*****************************************************/

#pragma once

#include <sys/socket.h>
#include <sys/iosupport.h>
#include <errno.h>
#include "3ds.h"
#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

static int     soc_open_cust(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int     soc_close_cust(struct _reent *r, void *fd);
static ssize_t soc_write_cust(struct _reent *r, void *fd, const char *ptr, size_t len);
static ssize_t soc_read_cust(struct _reent *r, void *fd, char *ptr, size_t len);

#define SYNC_ERROR ENODEV

extern Handle	SOCU_handle;
extern Handle	socMemhandle;

static inline int
soc_get_fd(int fd)
{
    __handle *handle = __get_handle(fd);
    if(handle == NULL)
        return -ENODEV;
    if(strcmp(devoptab_list[handle->device]->name, "soc") != 0)
        return -ENOTSOCK;
    return *(Handle*)handle->fileStruct;
}

s32 _net_convert_error(s32 sock_retval);

ssize_t soc_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

ssize_t soc_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

void socCustomInit(const Handle* handle);
void socCustomExit();

#ifdef __cplusplus
}
#endif