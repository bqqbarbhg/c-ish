#include <test/test.h>
#include <base/memory.h>

test_case(test_mem_alloc)
{
	void *pointers[1024];

	for (uint32_t i = 0; i < 1024; i++) {
		void *ptr = mem_alloc(1024, 8);
		test_assert((uintptr_t)ptr % 8 == 0, "Pointer is correctly aligned");
		pointers[i] = ptr;
	}

	for (uint32_t i = 0; i < 1024; i++) {
		mem_free(pointers[i]);
	}
}

