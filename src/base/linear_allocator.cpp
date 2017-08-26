#include "linear_allocator.h"
#include <stdlib.h>

linear_allocator::linear_allocator()
	: memory(nullptr)
	, pos(0)
	, capacity(0)
	, ator(nullptr)
{
}

linear_allocator::~linear_allocator()
{
	reset();
}

constexpr size_t initial_capacity = 16U * 1024U;

size_t linear_allocator::grow(size_t size, size_t alignment)
{
	size_t new_cap = capacity ? capacity * 2 : initial_capacity;
	new_cap = at_least(new_cap, size + alignment + 8);

	void *new_mem = mem_alloc_using(ator, new_cap, 64, sizeof(void*));
	p_assert(new_mem != nullptr);

	size_t new_pos = align_up(sizeof(void*), alignment);

	*(void**)new_mem = memory;

	memory = new_mem;
	capacity = new_cap;
	pos = new_pos;

	return new_pos;
}

void linear_allocator::reset()
{
	void *mem = memory;
	while (mem) {
		void *to_free = mem;
		mem = *(void**)mem;
		mem_free(to_free);
	}

	memory = nullptr;
	pos = 0;
	capacity = 0;
}

void *linear_allocator::allocator_allocate(size_t size, size_t alignment)
{
	return alloc(size, alignment);
}
void linear_allocator::allocator_free(void *)
{
}

