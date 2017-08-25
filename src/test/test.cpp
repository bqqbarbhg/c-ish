#include "test.h"
#include <stdint.h>
#include <stdio.h>

test_case_struct g_tests[1024];
uint32_t g_num_tests = 0;

operator_counts counts;

test_case_struct::test_case_struct(const char *name, void (*func)())
	: name(name)
	, func(func)
{
}

const char *g_fail_file;
const char *g_fail_expr;
const char *g_fail_msg;
int g_fail_line;
bool g_did_fail;

int add_test(const test_case_struct &s)
{
	g_tests[g_num_tests++] = s;
	return g_num_tests;
}

void test_fail(const char *file, int line, const char *expr, const char *msg)
{
	g_did_fail = true;
	g_fail_file = file;
	g_fail_line = line;
	g_fail_expr = expr;
	g_fail_msg = msg;
}

bool run_tests()
{
	uint32_t num = g_num_tests;
	uint32_t num_fail = 0;
	for (uint32_t i = 0; i < num; i++) {
		auto &test = g_tests[i];
		g_did_fail = false;
		counts.reset();
		test.func();
		if (g_did_fail) {
			fprintf(stderr, "%s fail: %s (%s)\n", test.name, g_fail_msg, g_fail_expr);
			num_fail++;
		}
	}

	printf("Tests passed: %u/%u\n", num - num_fail, num);

	return num_fail == 0;
}

