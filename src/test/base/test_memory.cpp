#include <test/test.h>
#include <base/memory.h>

test_case(test_mem_alloc)
{
	void *pointers[1024];

	for (uint32_t i = 0; i < 1024; i++) {
		void *ptr = mem::alloc(1024, 64);
		test_assert((uintptr_t)ptr % 64 == 0, "Pointer is correctly aligned");
		pointers[i] = ptr;
	}

	for (uint32_t i = 0; i < 1024; i++) {
		mem::free(pointers[i]);
	}
}

test_case(test_mem_size)
{
	void *pointer = mem::alloc(1024, 8);
	size_t size = mem::get_size(pointer);
	test_assert(size == 1024, "Size is correct");
	mem::free(pointer);
}
