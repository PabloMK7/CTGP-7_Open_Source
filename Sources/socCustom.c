/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: socCustom.c
Open source lines: 119/119 (100.00%)
*****************************************************/

#include "socCustom.h"

static devoptab_t
    soc_devoptab_cust =
    {
    .name         = "soc",
    .structSize   = sizeof(Handle),
    .open_r       = soc_open_cust,
    .close_r      = soc_close_cust,
    .write_r      = soc_write_cust,
    .read_r       = soc_read_cust,
    .seek_r       = NULL,
    .fstat_r      = NULL,
    .stat_r       = NULL,
    .link_r       = NULL,
    .unlink_r     = NULL,
    .chdir_r      = NULL,
    .rename_r     = NULL,
    .mkdir_r      = NULL,
    .dirStateSize = 0,
    .diropen_r    = NULL,
    .dirreset_r   = NULL,
    .dirnext_r    = NULL,
    .dirclose_r   = NULL,
    .statvfs_r    = NULL,
    .ftruncate_r  = NULL,
    .fsync_r      = NULL,
    .deviceData   = 0,
    .chmod_r      = NULL,
    .fchmod_r     = NULL,
    };

static int
soc_open_cust(struct _reent *r,
         void          *fileStruct,
         const char    *path,
         int           flags,
         int           mode)
{
	r->_errno = ENOSYS;
	return -1;
}

static int
soc_close_cust(struct _reent *r,
          void           *fd)
{
	Handle sockfd = *(Handle*)fd;

	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,1,2); // 0xB0042
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = IPC_Desc_CurProcessId();

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret =_net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

static ssize_t
soc_write_cust(struct _reent *r,
          void          *fd,
          const char    *ptr,
          size_t        len)
{
	Handle sockfd = *(Handle*)fd;
	return soc_sendto(sockfd, ptr, len, 0, NULL, 0);
}

static ssize_t
soc_read_cust(struct _reent *r,
         void          *fd,
         char          *ptr,
         size_t        len)
{
	Handle sockfd = *(Handle*)fd;
	return soc_recvfrom(sockfd, ptr, len, 0, NULL, 0);
}

void socCustomInit(const Handle* handle) {
    int dev = FindDevice("soc:");
	if(dev >= 0)
		return;

    SOCU_handle = *handle;
    AddDevice(&soc_devoptab_cust);
}

void socCustomExit() {
    SOCU_handle = 0;

    int dev = FindDevice("soc:");
    if(dev >= 0)
        RemoveDevice("soc:");
}