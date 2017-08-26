#include "symbol.h"

uint32_t symbol_hash(const char *str, uint32_t length)
{
	uint32_t hash = symbol_hash_init();
	for (uint32_t i = 0; i < length; i++) {
		hash = symbol_hash_feed(hash, str[i]);
	}
	return hash;
}

symbol intern_symbol(const char *str, uint32_t length, uint32_t hash)
{
	p_debug_assert(hash == symbol_hash(str, length));

	symbol dummy;
	return dummy;
}

symbol intern_symbol(const char *str, uint32_t length)
{
	return intern_symbol(str, length, symbol_hash(str, length));
}

constexpr uint32_t symbol_page_bits = 10;

symbol_string *g_symbol_pointers[32 - symbol_page_bits];

symbol_string *get_symbol(symbol sym)
{
	return nullptr;
}

