/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: IntelliSense.h
Open source lines: 32/32 (100.00%)
*****************************************************/

#pragma once
// Define unknown GCC stuff
#define __attribute__(x)
#define __asm__
#define __asm__(x)
#define __volatile__(x)
#define __ATOMIC_SEQ_CST 1
#define __atomic_add_fetch(x,y,z) (*x)
#define __atomic_sub_fetch(x,y,z) (*x)
#define __atomic_fetch_add(x,y,z) (*x)
#define __atomic_fetch_sub(x,y,z) (*x)

// The file <sys/lock.h> cannot be found by VS, so we define the needed stuff that couldn't be loaded
typedef unsigned int _LOCK_T; 
typedef unsigned int _LOCK_RECURSIVE_T;
typedef unsigned int ssize_t;
struct _reent {
	unsigned int x;
};
void* memalign(size_t alignment, size_t size);
void free(void* ptr);
// Other definitions
#define M_PI 3.14