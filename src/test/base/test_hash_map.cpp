#include <test/test.h>
#include <base/hash_map.h>
#include <stdint.h>

uint32_t hash_count;

namespace ok {

struct int_hash {
	uhash operator()(int i) {
		return i * 13213;
	}
};

#undef test_tag
#define test_tag "ok"

#include "test_hash_map_impl.h"

}

namespace bad {

struct int_hash {
	uhash operator()(int i) {
		return 0;
	}
};

#undef test_tag
#define test_tag "bad"

#include "test_hash_map_impl.h"

}

namespace identity {

struct int_hash {
	uhash operator()(int i) {
		return i;
	}
};

#undef test_tag
#define test_tag "identity"

#include "test_hash_map_impl.h"

}


