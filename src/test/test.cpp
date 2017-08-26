#include "test.h"
#include <stdint.h>
#include <stdio.h>
#include <base/memory.h>
#include <base/hash_map.h>

#include <string.h>

uhash int_hash(uint32_t i)
{
	uint64_t j = i * 2654435761U;
	return (uhash)(~j ^ (j >> 32));
}

uhash int_hash(uint64_t i)
{
	uint64_t j = i * 11400714819323198549ULL;
	return (uhash)(~j ^ (j >> 32));
}

struct pointer_hash
{
	uhash operator()(void *ptr)
	{
		return int_hash((uintptr_t)ptr);
	}
};

struct test_allocator : mem_allocator
{
	hash_map<void*, size_t, pointer_hash> allocs;
	mem_allocator *inner;

	test_allocator()
	{
		allocs.ator = inner = mem_get_standard_allocator();
	}

	virtual void *allocator_allocate(size_t size, size_t alignment) override
	{
		void *mem = inner->allocator_allocate(size, alignment);
		allocs.insert(mem, size);
		return mem;
	}

	virtual void allocator_free(void *pointer) override
	{
		bool found = allocs.erase(pointer);
		p_assert(found);
		inner->allocator_free(pointer);
	}
};

test_case_struct g_tests[1024];
uint32_t g_num_tests = 0;

const char *test_tag = "";

operator_counts counts;

test_case_struct::test_case_struct(const char *name, const char *tag, void (*func)())
	: name(name)
	, tag(tag)
	, func(func)
{
	if (tag && strlen(tag)) {
		char *mem = (char*)malloc(strlen(name) + strlen(tag) + 2);
		sprintf(mem, "%s/%s", name, tag);
		this->name = mem;
	}
}

const char *g_fail_file;
const char *g_fail_expr;
const char *g_fail_msg;
int g_fail_line;
bool g_did_fail;
size_t g_num_asserts;

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
	test_allocator ator;
	mem_allocator *prev = mem_set_default_allocator(&ator);

	uint32_t num = g_num_tests;
	uint32_t num_fail = 0;
	for (uint32_t i = 0; i < num; i++) {
		auto &test = g_tests[i];

		ator.allocs.clear();
		g_did_fail = false;
		g_num_asserts = 0;
		counts.reset();

		test.func();

		if (!(ator.allocs.count == 0)) {
			test_fail(__FILE__, __LINE__, "ator.allocs.count == 0" , "Test should not leak memory");
		}

		if (g_did_fail) {
			printf("%-6zu fail %s\n", g_num_asserts, test.name);
		} else {
			printf("%-8zu ok %s\n", g_num_asserts, test.name);
		}

		if (g_did_fail) {
			fprintf(stderr, "%s fail: %s (%s)\n", test.name, g_fail_msg, g_fail_expr);
			num_fail++;
		}
	}

	mem_allocator *should_be_ator = mem_set_default_allocator(prev);
	p_assert(should_be_ator == &ator);

	printf("Tests passed: %u/%u\n", num - num_fail, num);

	return num_fail == 0;
}

