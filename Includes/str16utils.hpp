/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: str16utils.hpp
Open source lines: 94/94 (100.00%)
*****************************************************/

#pragma once
#include "types.h"

static inline int strsize16(const u16* src) {
	int ret = 0;
	while (*src++) ret++;
	return ret;
}

static inline bool strcmp16(const u16* src, const u16* pat) {
	while (*src && *pat) {
		if (*src != *pat)
			return false;
		src++;
		pat++;
	}
	return (!(*pat));
}

static inline bool strfind16(u16* src, const u16* pat, int* pos = nullptr) {
	u16* origSrc = src;
	while (*src) {
		if ((*src == *pat) && strcmp16(src, pat)) {
			if (pos) *pos = ((int)src - (int)origSrc) / sizeof(u16);
			return true;
		}
		src++;
	}
	if (pos) *pos == -1;
	return false;
}

static inline void strcpy16(u16* dst, const u8* src) {
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

static inline void strcpy16(u16* dst, const u16* src) {
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

static inline void strcpy16n(u16* dst, const u8* src, size_t dstsize) {
	if (!dstsize) return;
	u32 maxchars = (dstsize >> 2) - 1;
	while (*src && maxchars) {
		*dst++ = *src++;
		maxchars--;
	}
	*dst = '\0';
}

static inline void strcpy16n(u16* dst, const u16* src, size_t dstsize) {
	if (!dstsize) return;
	u32 maxchars = (dstsize >> 2) - 1;
	while (*src && maxchars) {
		*dst++ = *src++;
		maxchars--;
	}
	*dst = '\0';
}

static inline void strcat16n(u16* out, const u8* cat, size_t outsize) {
	if (!outsize) return;
	while (*out) {
		out++; outsize-=2;
	}
	strcpy16n(out, cat, outsize);
}

static inline void strcat16n(u16* out, const u16* cat, size_t outsize) {
	if (!outsize) return;
	while (*out) {
		out++; outsize-=2;
	}
	strcpy16n(out, cat, outsize);
}

namespace CTRPluginFramework {
	int utf8len(const char* str);
}
