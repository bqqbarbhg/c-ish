#pragma once

#include <base/base.h>

struct test_case_struct
{
	const char *name;
	const char *tag;
	void (*func)();

	test_case_struct() { }
	test_case_struct(const char *name, const char *tag, void (*func)());
};

int add_test(const test_case_struct &s);
void test_fail(const char *file, int line, const char *expr, const char *msg);

extern const char *test_tag;

#define test_case(name) \
	void test_##name(); \
	int dummy_##name = add_test(test_case_struct(#name, test_tag, test_##name)); \
	void test_##name()

extern size_t g_num_asserts;

#define test_assert(x, msg) do { g_num_asserts++; if (!(x)) { \
	test_fail(__FILE__, __LINE__, #x, msg); \
	return; } } while (0)

bool run_tests();

struct operator_counts {
	uint32_t ctor = 0;
	uint32_t dtor = 0;
	uint32_t copy = 0;
	uint32_t move = 0;
	uint32_t defa = 0;

	void reset()
	{
		*this = operator_counts();
	}
};

extern operator_counts counts;
