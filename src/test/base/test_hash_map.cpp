#include <test/test.h>
#include <base/hash_map.h>

struct int_hash {
	uhash operator()(int i) {
		return i * 13214;
	}
};

test_case(hash_map_ints)
{
	hash_map<int, int, int_hash> map;

	for (int i = 0; i < 1000; i++) {
		map[i] = i * i;
	}

	for (int i = 0; i < 1000; i++) {
		test_assert(map[i] == i * i, "Stupid square test");
	}

	hash_map<int, int, int_hash> copy = map;

	for (int i = 0; i < 1000; i++) {
		test_assert(map[i] == i * i, "Stupid square test after copy (original instance)");
		test_assert(copy[i] == i * i, "Stupid square test after copy (clone)");
	}

	auto it = map.find(5000);
	test_assert(it == map.end(), "Trying to find element that doesn't exist");


	hash_map<int, int, int_hash> moved = std::move(copy);
	for (int i = 0; i < 1000; i++) {
		test_assert(moved[i] == i * i, "Stupid square test after move");
	}
}

test_case(hash_map_iteration)
{
	hash_map<int, int, int_hash> map;
	bool visited[400] = { };

	for (int i = 0; i < 400; i++) {
		map.insert(i, i * i);
	}

	for (const auto &pair : map) {
		visited[pair.key] = true;
		int i = pair.key;
		test_assert(pair.val == i * i, "Pair value is correct");
	}

	for (int i = 0; i < 400; i++) {
		test_assert(visited[i], "Visited every element");
	}
}
