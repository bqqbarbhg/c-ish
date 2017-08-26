
struct operator_counter {
	uint32_t value;

	struct hash {
		uhash operator()(const operator_counter& c) {
			hash_count++;
			return c.value * 13213;
		}
	};

	bool operator==(const operator_counter &oc) const
	{
		return value == oc.value;
	}

	operator_counter()
		: value(0x1234abcd)
	{
		counts.ctor++;
		counts.defa++;
	}

	explicit operator_counter(uint32_t v)
		: value(v)
	{
		counts.ctor++;
	}

	operator_counter(const operator_counter &o)
		: value(o.value)
	{
		counts.ctor++;
		counts.copy++;
	}

	operator_counter(operator_counter &&o)
		: value(o.value)
	{
		o.value = ~0U;
		counts.ctor++;
		counts.move++;
	}

	~operator_counter()
	{
		counts.dtor++;
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

test_case(hash_map_erase_simple)
{
	hash_map<int, int, int_hash> map;
	map.insert(1, 10);
	map.insert(2, 10);
	map.insert(3, 10);

	test_assert(map.find(1) != map.end(), "Found #1");
	test_assert(map.find(2) != map.end(), "Found #2");
	test_assert(map.find(3) != map.end(), "Found #3");
	test_assert(map.count == 3, "Left: 3");

	map.erase(1);

	test_assert(map.find(1) == map.end(), "Gone  #1");
	test_assert(map.find(2) != map.end(), "Found #2");
	test_assert(map.find(3) != map.end(), "Found #3");
	test_assert(map.count == 2, "Left: 2");

	map.erase(2);

	test_assert(map.find(1) == map.end(), "Gone  #1");
	test_assert(map.find(2) == map.end(), "Gone  #2");
	test_assert(map.find(3) != map.end(), "Found #3");
	test_assert(map.count == 1, "Left: 1");

	map.erase(3);

	test_assert(map.find(1) == map.end(), "Gone  #1");
	test_assert(map.find(2) == map.end(), "Gone  #2");
	test_assert(map.find(3) == map.end(), "Gone  #3");
	test_assert(map.count == 0, "Left: 0");
}

test_case(hash_map_erase)
{
	hash_map<int, int, int_hash> map;

	for (int i = 0; i < 10; i++) {
		map.insert(i, i * 10);
	}

	for (int j = 0; j < 10; j++) {
		bool found = map.erase(j);
		test_assert(found, "Found key to erase");
		test_assert(map.count == 10 - j - 1, "Count is correct");

		for (int i = 0; i < j + 1; i++) {
			auto it = map.find(i);
			test_assert(it == map.end(), "Did not find deleted keys");
		}
		for (int i = j + 1; i < 10; i++) {
			auto it = map.find(i);
			test_assert(it != map.end(), "Found the rest of the keys");
		}
	}
}

test_case(hash_map_non_pod_value)
{
	hash_count = 0;

	{
		hash_map<int, operator_counter, int_hash> map;
		map.insert(1, operator_counter(5));
		test_assert(map[1].value == 5, "Retrieve stored non-pod");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 1, "No extra moves");
	test_assert(hash_count == 0, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_value_create)
{
	hash_count = 0;

	{
		hash_map<int, operator_counter, int_hash> map;
		test_assert(map[5].value == 0x1234abcd, "Default created value is good");
	}

	test_assert(counts.defa == 1, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 0, "No extra moves");
	test_assert(hash_count == 0, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_key)
{
	hash_count = 0;

	{
		hash_map<operator_counter, int, operator_counter::hash> map;
		map.insert(operator_counter(5), 1);
		test_assert(map[operator_counter(5)] == 1, "Retrieve with non-pod");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 1, "No extra moves");
	test_assert(hash_count == 2, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_both)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.insert(operator_counter(5), operator_counter(3));
		test_assert(map[operator_counter(5)] == operator_counter(3), "Retrieve with non-pod");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 2, "No extra moves");
	test_assert(hash_count == 2, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_copy_move_permutations)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.reserve(4);

		{
			operator_counter k(1), v(2);
			map.insert(k, v);
			test_assert(k.value == 1 && v.value == 2, "Copy both");
		}

		{
			operator_counter k(3), v(4);
			map.insert(k, std::move(v));
			test_assert(k.value == 3 && v.value == ~0U, "Moved value");
		}

		{
			operator_counter k(5), v(6);
			map.insert(std::move(k), v);
			test_assert(k.value == ~0U && v.value == 6, "Moved key");
		}

		{
			operator_counter k(7), v(8);
			map.insert(std::move(k), std::move(v));
			test_assert(k.value == ~0U && v.value == ~0U, "Moved both");
		}

		test_assert(map[operator_counter(1)].value == 2, "Lookup copy, copy");
		test_assert(map[operator_counter(3)].value == 4, "Lookup copy, move");
		test_assert(map[operator_counter(5)].value == 6, "Lookup move, copy");
		test_assert(map[operator_counter(7)].value == 8, "Lookup move, move");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 4, "No extra copies");
	test_assert(counts.move == 4, "No extra moves");
	test_assert(hash_count == 8, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_rehash)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.reserve(32);

		for (uint32_t i = 0; i < 32; i++) {
			map.insert(operator_counter(i), operator_counter(i * 2));
		}

		for (uint32_t i = 0; i < 32; i++) {
			auto it = map.find(operator_counter(i));
			test_assert(it != map.end() && it->val.value == i * 2, "Found value");
		}

		test_assert(counts.copy == 0, "No extra copies");
		test_assert(counts.move == 64, "No extra moves");

		map.reserve(128);

		for (uint32_t i = 0; i < 32; i++) {
			auto it = map.find(operator_counter(i));
			test_assert(it != map.end() && it->val.value == i * 2, "Found value");
		}

		test_assert(counts.copy == 0, "No extra copies");
		test_assert(counts.move == 128, "No extra moves");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(hash_count == 32 * 3, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_copy)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		hash_map<operator_counter, operator_counter, operator_counter::hash> copy;
		map.reserve(32);

		for (uint32_t i = 0; i < 32; i++) {
			map.insert(operator_counter(i), operator_counter(i * 2));
		}

		test_assert(counts.copy == 0, "No extra copies");
		test_assert(counts.move == 64, "No extra moves");

		copy = map;

		for (uint32_t i = 0; i < 32; i++) {
			auto it = copy.find(operator_counter(i));
			test_assert(it != copy.end() && it->val.value == i * 2, "Found value");
		}

		test_assert(counts.copy == 64, "No extra copies");
		test_assert(counts.move == 64, "No extra moves");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(hash_count == 32 * 2, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_move)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		hash_map<operator_counter, operator_counter, operator_counter::hash> move;
		map.reserve(32);

		for (uint32_t i = 0; i < 32; i++) {
			map.insert(operator_counter(i), operator_counter(i * 2));
		}

		test_assert(counts.copy == 0, "No extra copies");
		test_assert(counts.move == 64, "No extra moves");

		move = std::move(map);

		for (uint32_t i = 0; i < 32; i++) {
			auto it = move.find(operator_counter(i));
			test_assert(it != move.end() && it->val.value == i * 2, "Found value");
		}

		test_assert(counts.copy == 0, "No extra copies");
		test_assert(counts.move == 64, "No extra moves");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(hash_count == 32 * 2, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_insert_over)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.insert(operator_counter(1), operator_counter(2));
		map.insert(operator_counter(1), operator_counter(3));

		test_assert(map[operator_counter(1)].value == 3, "Value overwritten");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 3, "No extra moves");
	test_assert(hash_count == 3, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_erase)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.insert(operator_counter(1), operator_counter(10));
		map.insert(operator_counter(2), operator_counter(20));

		map.erase(operator_counter(1));
		map.erase(operator_counter(2));
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(hash_count == 4, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_non_pod_clear)
{
	hash_count = 0;

	{
		hash_map<operator_counter, operator_counter, operator_counter::hash> map;
		map.insert(operator_counter(1), operator_counter(2));
		map.insert(operator_counter(3), operator_counter(4));

		map.clear();

		test_assert(map.count == 0, "Empty after clear");
		test_assert(map.find(operator_counter(1)) == map.end(), "Can't find after clear");
		test_assert(map.find(operator_counter(3)) == map.end(), "Can't find after clear");
	}

	test_assert(counts.defa == 0, "No extra default initialization");
	test_assert(counts.copy == 0, "No extra copies");
	test_assert(counts.move == 4, "No extra moves");
	test_assert(hash_count == 4, "No extra hashes");
	test_assert(counts.ctor == counts.dtor, "Objects destroyed");
}

test_case(hash_map_pod_clear)
{
	hash_map<int, int, int_hash> map;
	map.insert(1, 2);
	map.insert(3, 4);

	map.clear();

	test_assert(map.count == 0, "Empty after clear");
	test_assert(map.find(1) == map.end(), "Can't find after clear");
	test_assert(map.find(3) == map.end(), "Can't find after clear");
}

test_case(hash_map_count)
{
	hash_map<int, int, int_hash> map;

	map.insert(1, 10);
	map.insert(2, 20);
	map.insert(3, 30);
	map.insert(4, 40);

	test_assert(map.count == 4, "Count is correct");
}

