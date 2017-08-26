#include "memory.h"
#include <stdlib.h>
#include <mutex>
#include <atomic>

#if p_compiler == p_msvc

struct stdlib_allocator : mem_allocator
{
	virtual void *allocator_allocate(size_t size, size_t alignment) override
	{
		return _aligned_malloc(size, alignment);
	}
	virtual void allocator_free(void *pointer) override
	{
		_aligned_free(pointer);
	}
};

#else

struct stdlib_allocator : mem_allocator
{
	virtual void *allocator_allocate(size_t size, size_t alignment) override
	{
		return aligned_alloc(alignment, size);
	}
	virtual void allocator_free(void *pointer) override
	{
		free(pointer);
	}
};

#endif

stdlib_allocator g_stdlib_allocator;

// Data that is lock-free shared between threads
// Note: This data should be read-only except in exceptional cases:
// - Changing the default allocator
struct mem_shared
{
	std::atomic<mem_allocator*> default_allocator;
	char cacheline_pad[64];

	mem_shared()
	{
		default_allocator.store(&g_stdlib_allocator);
	}
};

static mem_shared g_shared;

struct mem_header
{
	mem_allocator *alloc;
	size_t offset;
};

void *mem_alloc(size_t size, size_t alignment, size_t header)
{
	return mem_alloc_using(nullptr, size, alignment, header);
}

void *mem_alloc_using(mem_allocator *alloc, size_t size, size_t alignment, size_t header)
{
	p_assert((alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");
	if (alloc == nullptr) {
		alloc = g_shared.default_allocator;
	}

	// Reserve space for the user header and allocator index
	size_t actual_alignment = at_least(alignment, alignof(mem_allocator*));
	header = align_up(header, alignof(mem_header));
	size_t prefix_size = align_up(header + sizeof(mem_header), actual_alignment);
	char *ptr = (char*)alloc->allocator_allocate(prefix_size + size, actual_alignment);
	if (!ptr) return nullptr;

	mem_header *hd = (mem_header*)(ptr + prefix_size - header - sizeof(mem_header));
	hd->alloc = alloc;
	hd->offset = (char*)hd - (char*)ptr;

	return (char*)hd + sizeof(mem_header);
}

void mem_free(void *pointer)
{
	char *base = (char*)pointer - sizeof(mem_header);
	mem_header *hd = (mem_header*)base;
	hd->alloc->allocator_free(base - hd->offset);
}

mem_allocator *mem_get_standard_allocator()
{
	return &g_stdlib_allocator;
}

mem_allocator *mem_get_default_allocator()
{
	return g_shared.default_allocator.load();
}

mem_allocator *mem_set_default_allocator(mem_allocator *alloc)
{
	return g_shared.default_allocator.exchange(alloc);
}

