#pragma once

#include "base.h"

struct mem_allocator
{
	virtual void *allocator_allocate(size_t size, size_t alignment) = 0;
	virtual void allocator_free(void *) = 0;
};

void *mem_alloc(size_t size, size_t alignment, size_t header = 0);
void *mem_alloc_using(mem_allocator *alloc, size_t size, size_t alignment, size_t header = 0);
void mem_free(void *pointer);

mem_allocator *mem_get_standard_allocator();
mem_allocator *mem_get_default_allocator();
mem_allocator *mem_set_default_allocator(mem_allocator *alloc);
