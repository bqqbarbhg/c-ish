#pragma once

#include "base.h"

static uint32_t find_msb(uint32_t val);

#if p_compiler == p_msvc

#include <intrin.h>

static inline uint32_t find_msb(uint32_t val)
{
	p_assert(val != 0);
	unsigned long index;
	_BitScanReverse(&index, (unsigned long)val);
	return (uint32_t)index;
}

#else

#endif

