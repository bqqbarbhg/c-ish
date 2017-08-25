#pragma once

#include "base.h"
#include <stdlib.h>
#include <string.h>
#include <new>
#include <utility>
#include <type_traits>

typedef unsigned usize;
typedef unsigned uhash;

constexpr usize align_up(usize val, usize alignment)
{
	return val + ((alignment - (val & (alignment - 1))) & (alignment - 1));
}

#if __GNUG__ && __GNUC__ < 5
#define c_trivially_copyable(x) (__has_trivial_copy(x))
#else
#define c_trivially_copyable(x) (std::is_trivially_copyable<x>::value)
#endif

inline usize next_pow2(usize val)
{
	usize x = val - 1;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

template <typename T>
void cswap(T &a, T &b)
{
	T tmp(std::move(a));
	a.~T();
	new (&a) T(std::move(b));
	b.~T();
	new (&b) T(std::move(tmp));
}

struct always_false
{
	template <typename T>
	bool operator==(const T& t) const
	{
		return false;
	}
};

struct hash_base
{
	hash_base()
		: count(0)
		, capacity(0)
		, hbuf(nullptr)
		, kvbuf(nullptr)
	{
	}

	hash_base(hash_base &&hb)
		: count(hb.count)
		, capacity(hb.capacity)
		, hbuf(hb.hbuf)
		, kvbuf(hb.kvbuf)
	{

		hb.count = 0;
		hb.capacity = 0;
		hb.hbuf = nullptr;
		hb.kvbuf = nullptr;
	}

	hash_base(usize count, usize capacity)
		: count(count)
		, capacity(capacity)
	{
	}

	~hash_base()
	{
		if (kvbuf)
			free(kvbuf);
	}

	usize count;
	usize capacity;
	uhash *hbuf;
	void  *kvbuf;

	// -- Iterators

	template <typename It>
	struct iterator_base
	{
		const hash_base *base;
		usize index;

		iterator_base()
		{
		}

		iterator_base(const hash_base *base, usize index)
			: base(base)
			, index(index)
		{
		}

		bool operator!=(const It &rhs) const
		{
			c_assert(base == rhs.base);
			return index != rhs.index;
		}

		bool operator==(const It &rhs) const
		{
			c_assert(base == rhs.base);
			return index == rhs.index;
		}

		It& operator++()
		{
			uhash *const hb = base->hbuf;
			usize const cap = base->capacity;
			usize ix = index + 1;

			while (ix < cap && hb[ix] == 0)
				ix++;

			index = ix;
			return static_cast<It&>(*this);
		}

		It& operator--()
		{
			uhash *const hb = base->hbuf;
			usize ix = index - 1;

			while (ix >= 0 && hb[ix] == 0)
				ix--;

			index = ix;
			return static_cast<It&>(*this);
		}

		It operator++(int)
		{
			It copy = static_cast<It&>(*this);
			++copy;
			return copy;
		}

		It operator--(int)
		{
			It copy = static_cast<It&>(*this);
			--copy;
			return copy;
		}
	};

	usize find_first_used_slot(usize begin = 0)
	{
		uhash *const hb = hbuf;
		usize const cap = capacity;
		usize i;
		for (i = begin; i < cap; i++) {
			if (hb[i] != 0)
				break;
		}
		return i;
	}
};

template <typename KeyVal>
struct hash_container : hash_base
{
	typedef KeyVal key_val;

	hash_container()
		: hash_base()
	{
	}

	hash_container(const hash_container &rhs)
		: hash_base(rhs.count, rhs.capacity)
	{
		if (rhs.count) {
			size_t alloc_size = (sizeof(key_val) + sizeof(uhash)) * capacity + sizeof(uhash);
			void *alloc = malloc(alloc_size);
			kvbuf = alloc;
			hbuf = (uhash*)((char*)kvbuf + align_up(sizeof(key_val) * capacity, alignof(usize)));

			if (c_trivially_copyable(key_val)) {
				memcpy(alloc, rhs.kvbuf, alloc_size);
			} else {
				uhash *const hb = hbuf;
				key_val *const kvb = (key_val*)kvbuf;
				const uhash *const rhb = rhs.hbuf;
				const key_val *const rkvb = (const key_val*)rhs.kvbuf;
				for (usize i = 0; i < capacity; i++) {
					uhash hash = hb[i] = rhb[i];
					if (hash) {
						new (&kvb[i]) key_val(rkvb[i]);
					}
				}
			}
		} else {
			hbuf = nullptr;
			kvbuf = nullptr;
		}
	}

	hash_container(hash_container &&rhs)
		: hash_base(std::move(rhs))
	{
	}

	~hash_container()
	{
		if (!c_trivially_copyable(key_val)) {
			uhash *const hb = hbuf;
			key_val *const kvb = (key_val*)kvbuf;
			for (usize i = 0; i < capacity; i++) {
				if (hb[i]) {
					kvb[i].~key_val();
				}
			}
		}
	}

	hash_container &operator=(const hash_container &rhs)
	{
		this->~hash_container();
		new (this) hash_container(rhs);
		return *this;
	}


	hash_container &operator=(hash_container &&rhs)
	{
		this->~hash_container();
		new (this) hash_container(std::move(rhs));
		return *this;
	}

	// -- Fundamental operations

	// Rehash the container
	void rehash_impl(usize new_size)
	{
		uhash *const hb = hbuf;
		key_val *const kvb = (key_val*)kvbuf;
		usize const old_cap = capacity;

		capacity = new_size ? new_size * 2 : 8;

		void *alloc = malloc((sizeof(key_val) + sizeof(uhash)) * capacity + sizeof(uhash));
		kvbuf = alloc;
		hbuf = (uhash*)((char*)kvbuf + align_up(sizeof(key_val) * capacity, alignof(usize)));
		memset(hbuf, 0, sizeof(uhash) * capacity);

		for (usize i = 0; i < old_cap; i++) {
			uhash const hval = hb[i];
			if (hval != 0) {
				key_val *kv;
				bool created = insert_with_hash_ptr(always_false(), hval, kv);
				c_assert(created);
				new (kv) key_val(std::move(kvb[i]));
				kvb[i].~key_val();
			}
		}

		if (kvb)
			free((void*)kvb);
	}

	// Erase with slot index
	void erase_slot(usize slot_index)
	{
		uhash *const hb = hbuf;
		key_val *const kvb = (key_val*)kvbuf;
		usize const mask = capacity - 1;

		c_assert(slot_index < capacity);
		c_assert(hb[slot_index] != 0);

		count--;

		kvb[slot_index].~key_val();

		usize index = slot_index;

		// Shift entries back until there's an empty or zero scan distance cell
		for (;;) {
			usize const next_index = (index + 1) & mask;
			uhash hval = hb[next_index];

			if (hval == 0 || ((hval - index) & mask) == 0)
			{
				hb[index] = 0;
				return;
			}

			hb[index] = hval;
			new (&kvb[index]) key_val(std::move(kvb[next_index]));
			kvb[next_index].~key_val();

			index = next_index;
		}
	}

	template <typename K>
	bool insert_with_hash_ptr(const K &key, uhash hash_or_zero, key_val *&kv)
	{
		if (count * 2 >= capacity)
			rehash_impl(capacity);

		uhash const hash = hash_or_zero ? hash_or_zero : 1;
		uhash *const hb = hbuf;
		key_val *const kvb = (key_val*)kvbuf;
		usize const mask = capacity - 1;

		usize index = hash & mask;
		usize scan = 0;

		alignas(key_val) char swapbuf[sizeof(key_val)];
		uhash swaphash;

		// Scan before any swapping has happened
		for (;;) {
			uhash &href = hb[index];
			uhash const hval = href;
			key_val &kvref = kvb[index];

			// Found the key
			if (hval == hash && key == kvref.key) {
				kv = &kvref;
				return false;
			}

			// Found an empty cell
			if (hval == 0) {
				kv = &kvref;
				href = hash;
				count++;
				return true;
			}

			usize const sref = (index - hval) & mask;

			// Current slot has shorter scan distance -> insert here
			if (sref < scan) {
				new (swapbuf) key_val(std::move(kvref));
				swaphash = hval;
				kvref.~key_val();
				kv = &kvref;
				href = hash;
				scan = sref;
				count++;
				break;
			}

			index = (index + 1) & mask;
			scan++;
		}

		// Element was swapped, find it a new place
		for (;;) {

			uhash &href = hb[index];
			uhash const hval = href;
			key_val &kvref = kvb[index];

			// Found an empty slot, finish
			if (hval == 0) {
				new (&kvref) key_val(std::move(*(key_val*)swapbuf));
				href = swaphash;
				(*(key_val*)swapbuf).~key_val();
				return true;
			}

			usize const sref = (index - hval) & mask;

			// Current slot has shorter scan distance -> swap
			if (sref < scan) {
				cswap(*(key_val*)swapbuf, kvref);
				cswap(swaphash, href);
				scan = sref;
			}

			index = (index + 1) & mask;
			scan++;
		}
	}

	template <typename K>
	usize find_slot_with_hash(const K &key, uhash hash_or_zero)
	{
		uhash const hash = hash_or_zero ? hash_or_zero : 1;
		uhash *const hb = hbuf;
		key_val *const kvb = (key_val*)kvbuf;
		usize const mask = capacity - 1;

		if (capacity == 0)
			return 0;

		usize index = hash & mask;
		usize scan = 0;

		// Scan before any swapping has happened
		for (;;) {
			uhash const hval = hb[index];
			key_val &kvref = kvb[index];

			// Found the key
			if (hval == hash && key == kvref.key) {
				return index;
			}

			// Found an empty cell
			if (hval == 0) {
				return capacity;
			}

			usize const sref = (index - hval) & mask;

			// Current slot has shorter scan distance -> would have been swapped
			if (sref < scan) {
				return capacity;
			}

			index = (index + 1) & mask;
			scan++;
		}
	}

	void reserve(usize size)
	{
		usize const pow2 = next_pow2(size + 1);
		if (pow2 * 2 > capacity)
			rehash_impl(pow2);
	}
};

template <typename Key>
struct set_key_val
{
	Key key;
};

template <typename Key, typename Val>
struct map_key_val
{
	Key key;
	Val val;
};

template <typename T>
struct default_hash;

template <typename Key, typename Val, typename Hash = default_hash<Key>>
struct hash_map : hash_container<map_key_val<Key, Val>>
{
	typedef hash_container<map_key_val<Key, Val>> base;
	typedef typename base::key_val key_val;
	typedef map_key_val<const Key, Val> value_type;

	struct iterator : hash_base::iterator_base<iterator>
	{
		iterator() : hash_base::iterator_base<iterator>() { }
		iterator(const hash_base *h, usize i) : hash_base::iterator_base<iterator>(h, i) { }

		value_type *operator->() { return &((value_type*)this->base->kvbuf)[this->index]; }
		value_type &operator*()  { return ((value_type*)this->base->kvbuf)[this->index]; }
	};

	struct const_iterator : hash_base::iterator_base<const_iterator>
	{
		const_iterator() : hash_base::iterator_base<const_iterator>() { }
		const_iterator(iterator it) : hash_base::iterator_base<const_iterator>(it.base, it.index) { }
		const_iterator(const hash_base *h, usize i) : hash_base::iterator_base<const_iterator>(h, i) { }

		const value_type *operator->() { return &((const value_type*)this->base->kvbuf)[this->index]; }
		const value_type &operator*()  { return ((const value_type*)this->base->kvbuf)[this->index]; }
	};

	template <typename K, typename V>
	bool insert_impl(K &&key, V &&value)
	{
		key_val *kv;
		bool inserted = base::insert_with_hash_ptr(key, Hash()(key), kv);
		if (inserted) {
			new (&kv->key) Key(std::forward<typename std::remove_reference<K>::type>(key));
		} else {
			kv->val.~Val();
		}
		new (&kv->val) Val(std::forward<typename std::remove_reference<V>::type>(value));
		return inserted;
	}

	template <typename K>
	bool insert_ptr_impl(K &&key, key_val *&kv)
	{
		bool inserted = base::insert_with_hash_ptr(key, Hash()(key), kv);
		if (inserted) {
			new (&kv->key) Key(std::forward<typename std::remove_reference<K>::type>(key));
			new (&kv->val) Val();
		}
		return inserted;
	}

	bool insert(const Key  &key, const Val  &val) { return insert_impl(          key,            val); }
	bool insert(const Key  &key,       Val &&val) { return insert_impl(          key,  std::move(val)); }
	bool insert(      Key &&key, const Val  &val) { return insert_impl(std::move(key),           val); }
	bool insert(      Key &&key,       Val &&val) { return insert_impl(std::move(key), std::move(val)); }

	bool insert_ptr(const Key  &key, const Val *&val) { key_val *kv; bool i = insert_ptr_impl(key,            kv); val = kv->val; return i; }
	bool insert_ptr(      Key &&key, const Val *&val) { key_val *kv; bool i = insert_ptr_impl(std::move(key), kv); val = kv->val; return i; }

	Val& operator[](const Key  &key) { key_val *kv; insert_ptr_impl(key,            kv); return kv->val; }
	Val& operator[](      Key &&key) { key_val *kv; insert_ptr_impl(std::move(key), kv); return kv->val; }

	iterator erase(const_iterator it)
	{
		usize const slot = it.index;
		base::erase_slot(slot);
		return iterator(this, base::find_first_used_slot(slot));
	}

	iterator erase(const Key &key)
	{
		usize slot = base::find_slot_with_hash(key, Hash()(key));
		base::erase_slot(slot);
		return iterator(this, base::find_first_used_slot(slot));
	}

	iterator find(const Key &key)
	{
		usize slot = base::find_slot_with_hash(key, Hash()(key));
		return iterator(this, slot);
	}

	const_iterator find(const Key &key) const
	{
		usize slot = base::find_slot_with_hash(key, Hash()(key));
		return const_iterator(this, slot);
	}


	const_iterator begin() const { return const_iterator(this, base::find_first_used_slot()); }
	iterator begin() { return iterator(this, base::find_first_used_slot()); }
	const_iterator end() const { return const_iterator(this, base::capacity); }
	iterator end() { return iterator(this, base::capacity); }
};

template <typename Key, typename Hash = default_hash<Key>>
struct hash_set : hash_container<set_key_val<Key>>
{
	typedef hash_container<set_key_val<Key>> base;
	typedef typename base::key_val key_val;
	typedef Key value_type;

	struct const_iterator : hash_base::iterator_base<const_iterator>
	{
		const_iterator() : hash_base::iterator_base<const_iterator>() { }
		const_iterator(const hash_base *h, usize i) : hash_base::iterator_base<const_iterator>(h, i) { }

		const value_type *operator->() { return &((key_val*)this->base->kvbuf)[this->index].key; }
		const value_type &operator*()  { return ((key_val*)this->base->kvbuf)[this->index].key; }
	};

	typedef const_iterator iterator;

	template <typename K>
	bool insert_impl(K &&key)
	{
		key_val *kv;
		bool inserted = base::insert_with_hash_ptr(key, Hash()(key), kv);
		if (inserted) {
			new (&kv->key) Key(std::forward<typename std::remove_reference<K>::type>(key));
		}
		return inserted;
	}

	bool insert(const Key  &key) { return insert_impl(          key); }
	bool insert(      Key &&key) { return insert_impl(std::move(key)); }

	iterator erase(const_iterator it)
	{
		usize const slot = it.index;
		base::erase_slot(slot);
		return iterator(this, base::find_first_used_slot(slot));
	}

	iterator erase(const Key &key)
	{
		usize slot = base::find_slot_with_hash(key, Hash()(key));
		base::erase_slot(slot);
		return iterator(this, base::find_first_used_slot(slot));
	}

	const_iterator find(const Key &key) const
	{
		usize slot = base::find_slot_with_hash(key, Hash()(key));
		return const_iterator(this, slot);
	}

	const_iterator begin() const { return const_iterator(this, base::find_first_used_slot()); }
	iterator begin() { return iterator(this, base::find_first_used_slot()); }
	const_iterator end() const { return const_iterator(this, base::capacity); }
	iterator end() { return iterator(this, base::capacity); }
};

