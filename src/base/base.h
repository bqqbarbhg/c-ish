#pragma once

#ifdef _MSC_VER
#define c_assert(x) do { if (!(x)) __debugbreak(); } while (0)
#else
#include <assert.h>
#define c_assert(x) assert(x)
#endif

