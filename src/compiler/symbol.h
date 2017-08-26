#pragma once

#include <base/base.h>
#include <string.h>

struct symbol
{
	uint32_t index;
};

struct symbol_string
{
	uint32_t length;
	char data[1];
};

uint32_t symbol_hash(const char *str, uint32_t length);

constexpr uint32_t symbol_hash_init()
{
	return 2166136261U;
}

constexpr inline uint32_t symbol_hash_feed(uint32_t hash, unsigned char byte)
{
	return (hash ^ byte) * 16777619U;
}

symbol intern_symbol(const char *str, uint32_t length, uint32_t hash);
symbol intern_symbol(const char *str, uint32_t length);
static inline symbol intern_symbol(const char *str)
{
	return intern_symbol(str, (uint32_t)strlen(str));
}

symbol_string *get_symbol(symbol sym);

struct symbol_cahce
{
	uint64_t storage[4];

	symbol intern(const char *str, uint32_t length, uint32_t hash);
	symbol intern(const char *str, uint32_t length);
	inline symbol intern(const char *str)
	{
		return intern(str, (uint32_t)strlen(str));
	}

	bool intern_deferred(symbol *sym, const char *str, uint32_t length, uint32_t hash);
};

