#include <test/test.h>
#include <base/bit_math.h>

test_case(find_msb_singe_bit)
{
	for (uint32_t i = 0; i < 31; i++) {
		test_assert(find_msb(1 << i) == i, "Single bit MSB");
	}
}

test_case(find_msb_small)
{
	for (uint32_t i = 1; i < 65536; i++) {
		uint32_t msb = find_msb(i);
		uint32_t mask = (1 << msb << 1) - 1;
		test_assert((~mask & i) == 0, "No bits over MSB set");
		test_assert(((1 << msb) & i) != 0, "MSB is actually set");
	}
}

