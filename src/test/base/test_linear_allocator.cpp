#include <test/test.h>
#include <base/linear_allocator.h>

#include <stdlib.h>

test_case(test_linear_allocator_simple)
{
	linear_allocator a;

	void *ptr = a.alloc(1024, 8);
	test_assert((uintptr_t)ptr % 8 == 0, "Pointer is correctly aligned");
}

test_case(test_linear_allocator)
{
	linear_allocator a;

	for (uint32_t i = 0; i < 1024; i++) {
		void *ptr = a.alloc(1024, 8);
		test_assert((uintptr_t)ptr % 8 == 0, "Pointer is correctly aligned");
	}
}

