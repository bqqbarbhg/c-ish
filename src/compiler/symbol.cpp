#include "symbol.h"
#include <base/linear_allocator.h>
#include <base/hash_map.h>
#include <mutex>

uint32_t symbol_hash(const char *str, uint32_t length)
{
	uint32_t hash = symbol_hash_init();
	for (uint32_t i = 0; i < length; i++) {
		hash = symbol_hash_feed(hash, str[i]);
	}
	return hash;
}

typedef hash_set_base<symbol> symbol_set;
typedef typename symbol_set::key_val key_val;

constexpr uint32_t symbol_page_bits = 10;
symbol_string **g_symbol_pointers[32 - symbol_page_bits];
symbol_set g_symbol_set;
linear_allocator g_string_data;
uint32_t g_symbol_counter;
std::mutex g_mutex;

static uint32_t get_page_size(uint32_t page)
{
	if (page > 1) {
		return 1 << (symbol_page_bits + page - 1);
	} else {
		return 1 << symbol_page_bits;
	}
}

static void get_page_offset(uint32_t index, uint32_t *page, uint32_t *offset)
{
	uint32_t ix = page >> symbol_page_bits;
	if (ix) {
		uint32_t msb = find_msb(ix);
		*page = msb + 1;
		*offset = ix & (1 << (msb + symbol_page_bits)) - 1;
	} else {
		*page = 0;
		*offset = ix;
	}
}

struct symbol_cmp
{
	const char *data;
	size_t length;

	symbol_cmp(const char *data, size_t length)
		: data(data)
		, length(length)
	{
	}

	bool operator==(const symbol& sym)
	{
		symbol_string *s = get_symbol(sym);
		if (s->length != length)
			return false;

		return !memcmp(s->data, data);
	}
};

static symbol make_string(uint32_t index, const char *str, uint32_t length)
{
	uint32_t ix = ++g_symbol_counter;

	symbol_string *s = g_string_data.alloc(sizeof(uint32_t) + length + 1, alignof(uint32_t));
	s->length = length;
	memcpy(s->data, str, length);
	s->data[length] = '\0';

	uint32_t page, offset;
	get_page_offset(ix, &page, &offset);

	symbol_string **pdata = g_symbol_pointers[page];
	if (pdata == nullptr) {
		uint32_t size = get_page_size(page);
		pdata = (symbol_string**)malloc(size * sizeof(symbol_string*));
		g_symbol_pointers[page] = pdata;
	}
	pdata[offset] = s;

	symbol sym;
	sym.index = ix;
	return sym;
}

static symbol intern_lockfree(const char *str, uint32_t length, uint32_t hash)
{
	p_debug_assert(hash == symbol_hash(str, length));

	key_val kv;
	if (g_symbol_set.insert_with_hash_ptr(symbol_cmp(str, length), hash, &kv)) {
		return kv.key = make_string(str, length);
	} else {
		return kv.key;
	}
}

void intern_symbols(size_t count, symbol *symbols, const char **strs, const uint32_t *lengths, const uint32_t *hashes)
{
	std::lock_guard<std::mutex> mg(g_mutex);
	for (size_t i = 0; i < count; i++) {
		symbols[i] = intern_lockfree(strs[i], lengths[i], hashes[i]);
	}
}

symbol intern_symbol(const char *str, uint32_t length, uint32_t hash)
{
	std::lock_guard<std::mutex> mg(g_mutex);
	return intern_lockfree(str, length, hash);
}

symbol intern_symbol(const char *str, uint32_t length)
{
	return intern_symbol(str, length, symbol_hash(str, length));
}

symbol_string *get_symbol(symbol sym)
{
	return nullptr;
}

