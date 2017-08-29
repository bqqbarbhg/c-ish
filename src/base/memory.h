#pragma once

#include "base.h"

namespace mem {

// General purpose memory allocation
//
// Overhead introduced by this API:
// * Around 16 bytes per allocation + overhead of the actual allocator
// * Indirect call per allocation or free
//
// The maximum supported total allocation size is UINT32_MAX (around 4GB)
//
// The allocated memory contains the information necessary to be able to
// free itself, so the pointers can be passed further without the allocator!

// Memory allocation interface:
//
// size: Number of bytes to allocate
// alignment: Alignment of the pointer, must be power of 2
// header: Number of bytes to allocate _before_ the alignment, making the effective
//         allocation size `size + header` bytes. The alignment before the header
//         is guaranteed to not alias with any other memory.
//
// Example:
//
//     void *ptr = mem::alloc(500, 64, 16);
//
// * `ptr` points to 516 bytes (500 + 16)
// * `ptr + 16` is aligned to 64 bytes
// * `ptr[-48 .. 515]` is guaranteed not to alias with other memory
// * `ptr[516 .. 576]` is _not_! Size needs to be aligned manually if this is desired

// The `_using` variants specify an allocator, if it's null they use the thread's default
// allocator. Thus, `mem::alloc(...)` is just a shorthand for `mem::alloc_using(nullptr, ...)`
struct allocator;

// C malloc semantics:
// * Returns nullptr on error
void *alloc(size_t size, size_t alignment = 8, size_t header = 0);
void *alloc_using(allocator *alloc, size_t size, size_t alignment = 8, size_t header = 0);

// C free semantics:
// * Null pointer `mem::free(nullptr);` is a safe no-op
// Note: The allocator that the pointer was allocated with must be still valid.
void free(void *pointer);

// Retrieve the size in bytes of the pointer allocated with `mem::alloc/realloc/_using`
// Returns the total user visible size of the allocation (size + header)
size_t get_size(const void *pointer);

// Retrieves the default default allocator
allocator *get_standard_allocator();

// Index of the running thread compatible with the one that is passed to the allocator interface
uint32_t get_thread_index();

// The following functions returns the previous allocator, which allows using them in
// a stack-like manner:
//
//     mem::allocator *prev = mem::set_default_allocator_for_this_thread(allocator_to_push);
//     func();
//     mem::set_default_allocator_for_this_thread(prev);
//

// Allocations done using `mem::alloc` or with null allocator will use this
allocator *set_default_allocator_for_this_thread(allocator *alloc);
allocator *get_default_allocator_for_this_thread();

// New threads will use this allocator as the default allocator
allocator *set_default_allocator_for_new_threads(allocator *alloc);
allocator *get_default_allocator_for_new_threads();

// Memory allocator interface
// Do not use this directly for allocating memory, except when delegating in allocators
struct allocator
{
	// Fundamental methods, must be implemented:

	// Allocate `size` bytes with the address of the first byte aligned by `alignment`
	// * thread: Thread index of the allocating thread
	virtual void *allocator_allocate(uint32_t thread, size_t size, size_t alignment) = 0;

	// Free previously allocated pointer. `size` and `alignment` are the same as when allocated.
	// * thread: Thread index of the _calling_ thread, not the one which allocated the pointer!
	virtual void allocator_free(uint32_t thread, void *pointer, size_t size, size_t alignment) = 0;
};

}
