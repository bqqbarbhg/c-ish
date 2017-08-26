#pragma once

#include <stdint.h>

#define p_msvc 1
#define p_gcc 2
#define p_generic 3

#if defined(_MSC_VER)
	#define p_compiler p_msvc
#elif defined(__GNUC__)
	#define p_compiler p_gcc
#else
	#define p_compiler p_generic
#endif

#define p_debug 1
#define p_release 2
#define p_final 3

#if defined(DEBUG)
#define p_build p_debug
#elif defined(FINAL)
#define p_build p_final
#else
#define p_build p_release
#endif

#if __GNUG__ && __GNUC__ < 5
#define p_trivially_copyable(x) (__has_trivial_copy(x))
#else
#define p_trivially_copyable(x) (std::is_trivially_copyable<x>::value)
#endif

#if p_build != p_final
	#if p_compiler == p_msvc
		#define p_assert(x) do { if (!(x)) __debugbreak(); } while (0)
	#else
		#include <assert.h>
		#define p_assert(x) assert(x)
	#endif
#else
	#define p_assert(x) ((void)0)
#endif

#if p_build == p_debug
	#define p_debug_assert(x) p_assert(x)
#else
	#define p_debug_assert(x) ((void)0)
#endif

constexpr uint32_t align_up(uint32_t val, uint32_t alignment)
{
	return val + ((alignment - (val & (alignment - 1))) & (alignment - 1));
}

constexpr uint64_t align_up(uint64_t val, uint64_t alignment)
{
	return val + ((alignment - (val & (alignment - 1))) & (alignment - 1));
}

constexpr inline uint32_t at_least(uint32_t a, uint32_t b) { return a > b ? a : b; }
constexpr inline uint32_t at_most(uint32_t a, uint32_t b) { return a < b ? a : b; }
constexpr inline uint64_t at_least(uint64_t a, uint64_t b) { return a > b ? a : b; }
constexpr inline uint64_t at_most(uint64_t a, uint64_t b) { return a < b ? a : b; }

