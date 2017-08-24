#pragma once

struct test_case_struct
{
	const char *name;
	void (*func)();

	test_case_struct() { }
	test_case_struct(const char *name, void (*func)());
};

int add_test(const test_case_struct &s);
void test_fail(const char *file, int line, const char *expr, const char *msg);

#define test_case(name) \
	void test_##name(); \
	int dummy_##name = add_test(test_case_struct(#name, test_##name)); \
	void test_##name()

#define test_assert(x, msg) do { if (!(x)) { \
	test_fail(__FILE__, __LINE__, #x, msg); \
	return; } } while (0)

bool run_tests();

