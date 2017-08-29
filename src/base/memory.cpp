#include "memory.h"
#include <stdlib.h>
#include <mutex>
#include <atomic>
#include <algorithm>

namespace mem {

namespace {

#if p_compiler == p_msvc

struct stdlib_allocator : allocator
{
	virtual void *allocator_allocate(uint32_t thread, size_t size, size_t alignment) override
	{
		if (alignment > 8)
			return ::_aligned_malloc(size, alignment);
		else
			return ::malloc(size);
	}
	virtual void allocator_free(uint32_t thread, void *pointer, size_t size, size_t alignment) override
	{
		if (alignment > 8)
			::_aligned_free(pointer);
		else
			return ::free(pointer);
	}
};

#else

struct stdlib_allocator : allocator
{
	virtual void *allocator_allocate(uint32_t thread, size_t size, size_t alignment) override
	{
		return ::aligned_alloc(alignment, size);
	}
	virtual void allocator_free(uint32_t thread, void *pointer, size_t size, size_t alignment) override
	{
		::free(pointer);
	}
};

#endif

struct thread_data
{
	allocator *default_allocator;
	uint32_t thread_index;
};

stdlib_allocator g_stdlib_allocator;
std::atomic<allocator*> g_default_thread_allocator;
thread_local thread_data t_thread_data;
std::atomic<uint32_t> g_thread_index_counter;

thread_data *get_thread_data()
{
	thread_data *td = &t_thread_data;

	if (td->thread_index == 0) {
		allocator *alloc = g_default_thread_allocator.load();
		td->default_allocator = alloc ? alloc : &g_stdlib_allocator;
		td->thread_index = g_thread_index_counter.fetch_add(1) + 1;
	}

	return td;
}

struct block_header
{
	allocator *alloc;
	uint32_t size;
	uint16_t alignment;
	uint16_t offset;
};

static_assert(alignof(block_header) == alignof(void*), "block_header must be pointer-aligned");

}

void *alloc(size_t size, size_t alignment, size_t header)
{
	return alloc_using(nullptr, size, alignment, header);
}

void *alloc_using(allocator *alloc, size_t size, size_t alignment, size_t header)
{
	p_assert((alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");
	p_assert((header & (alignof(void*) - 1)) == 0 && "Header must be pointer aligned");

	thread_data *td = get_thread_data();
	if (!alloc) {
		alloc = td->default_allocator;
	}

	// Reserve space for the user header and allocator index
	size_t actual_alignment = at_least(alignment, alignof(block_header));
	header = align_up(header, alignof(block_header));
	size_t prefix_size = align_up(header + sizeof(block_header), actual_alignment);
	size_t actual_size = prefix_size + size;
	char *ptr = (char*)alloc->allocator_allocate(td->thread_index, actual_size, actual_alignment);
	if (!ptr) return nullptr;
	p_assert(((uintptr_t)ptr & (alignment - 1)) == 0);

	block_header *hd = (block_header*)(ptr + prefix_size - header - sizeof(block_header));

	size_t offset = (char*)hd - (char*)ptr;
	p_assert(actual_size <= UINT32_MAX);
	p_assert(offset <= UINT16_MAX);
	p_assert(actual_alignment <= UINT16_MAX);

	hd->alloc = alloc;
	hd->size = (uint32_t)actual_size;
	hd->alignment = (uint16_t)actual_alignment;
	hd->offset = (uint16_t)offset;

	return (char*)hd + sizeof(block_header);
}

void free(void *pointer)
{
	if (pointer == nullptr)
		return;

	thread_data *td = get_thread_data();
	char *base = (char*)pointer - sizeof(block_header);
	block_header *hd = (block_header*)base;
	hd->alloc->allocator_free(td->thread_index, base - hd->offset, hd->size, hd->alignment);
}

size_t get_size(const void *pointer)
{
	char *base = (char*)pointer - sizeof(block_header);
	block_header *hd = (block_header*)base;
	return hd->size - hd->offset - sizeof(block_header);
}

allocator *get_standard_allocator()
{
	return &g_stdlib_allocator;
}

uint32_t get_thread_index()
{
	thread_data *td = get_thread_data();
	return td->thread_index;
}

allocator *get_default_allocator_for_this_thread()
{
	thread_data *td = get_thread_data();
	return td->default_allocator;
}

allocator *get_default_allocator_for_new_threads()
{
	return g_default_thread_allocator;
}

allocator *set_default_allocator_for_this_thread(allocator *alloc)
{
	thread_data *td = get_thread_data();
	allocator *prev = td->default_allocator;
	td->default_allocator = alloc;
	return prev;
}

allocator *set_default_allocator_for_new_threads(allocator *alloc)
{
	return g_default_thread_allocator.exchange(alloc);
}

}
