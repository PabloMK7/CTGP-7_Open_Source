/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: str16utils.hpp
Open source lines: 107/107 (100.00%)
*****************************************************/

#pragma once
#include "types.h"

static inline int strsize16(const char16_t* src) {
	int ret = 0;
	while (*src++) ret++;
	return ret;
}

static inline bool strcmp16(const char16_t* src, const char16_t* pat) {
	while (*src && *pat) {
		if (*src != *pat)
			return false;
		src++;
		pat++;
	}
	return (!(*pat));
}

static inline bool strfind16(char16_t* src, const char16_t* pat, int* pos = nullptr) {
	char16_t* origSrc = src;
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

static inline void strcpy16(char16_t* dst, const char* src) {
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

static inline void strcpy16(char16_t* dst, const char16_t* src) {
	while (*src) {
		*dst++ = *src++;
	}
	*dst = '\0';
}

static inline void strcpy16n(char16_t* dst, const char* src, size_t dstsize) {
	if (!dstsize) return;
	u32 maxchars = (dstsize >> 2) - 1;
	while (*src && maxchars) {
		*dst++ = *src++;
		maxchars--;
	}
	*dst = '\0';
}

static inline void strcpy16n(char16_t* dst, const char16_t* src, size_t dstsize) {
	if (!dstsize) return;
	u32 maxchars = (dstsize >> 2) - 1;
	while (*src && maxchars) {
		*dst++ = *src++;
		maxchars--;
	}
	*dst = '\0';
}

static inline void strcat16n(char16_t* out, const char* cat, size_t outsize) {
	if (!outsize) return;
	while (*out) {
		out++; outsize-=2;
	}
	strcpy16n(out, cat, outsize);
}

static inline void strcat16n(char16_t* out, const char16_t* cat, size_t outsize) {
	if (!outsize) return;
	while (*out) {
		out++; outsize-=2;
	}
	strcpy16n(out, cat, outsize);
}

static inline std::u16string to_u16string(u32 value) {
	std::string v = std::to_string(value);
	std::u16string ret;
	ret.resize(v.size());
	const char* src = v.data();
	char16_t* dst = ret.data();
	for (int i = 0; i < v.size(); i++) {
		*dst++ = *src++;
	}
	return ret;
}

namespace CTRPluginFramework {
	int utf8len(const char* str);
	std::string limitUtf8(const std::string& str, size_t amount);
}
