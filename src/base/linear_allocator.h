#pragma once

#include <base/base.h>
#include <base/memory.h>

struct linear_allocator : mem_allocator
{
	linear_allocator(const linear_allocator&) = delete;
	linear_allocator &operator=(const linear_allocator&) = delete;

	linear_allocator();
	~linear_allocator();

	virtual void *allocator_allocate(size_t size, size_t alignment) override;
	virtual void allocator_free(void *) override;

	void *alloc(size_t size, size_t alignment)
	{
		size_t alloc_pos = align_up(pos, alignment);
		if (alloc_pos + size > capacity) {
			alloc_pos = grow(size, alignment);
		}

		void *mem = (char*)memory + alloc_pos;
		pos = alloc_pos + size;
		return mem;
	}

	size_t grow(size_t size, size_t alignment);

	void reset();

	void *memory;
	size_t pos;
	size_t capacity;
	mem_allocator *ator;
};

