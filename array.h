/*
  This file is part of the Fairytale project

  Copyright (C) 2018 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef ARRAY_H
#define ARRAY_H

// Template Array class from paq8px, slightly modified, see https://github.com/hxim/paq8px

#include "common.h"

template <class T, const int Align = 16> class Array {
private:
	size_t used_size;
	size_t reserved_size;
	char* ptr; // Address of allocated memory (may not be aligned)
	T* data;	 // Aligned base address of the elements, (ptr <= T)
	void create(size_t requested_size);
	inline size_t padding() const { return Align - 1; }
	inline size_t allocated_bytes() const { return (reserved_size == 0) ? 0 : reserved_size * sizeof(T) + padding(); }

public:
	explicit Array(size_t requested_size) { create(requested_size); }
	~Array();
	T& operator[](const size_t i) { return data[i]; }
	const T& operator[](const size_t i) const { return data[i]; }
	size_t size() const { return used_size; }
	void resize(size_t new_size);
	void pop_back() {
		assert(used_size > 0);
		--used_size;
	}																			 // decrement size
	void push_back(const T& x);						 // increment size, append x
	Array(const Array&) { assert(false); } //prevent copying - this method must be public (gcc must see it but actually won't use it)
private:
	Array& operator=(const Array&){}; //prevent assignment
};

template <class T, const int Align> void Array<T, Align>::create(size_t requested_size) {
	assert((Align & (Align - 1)) == 0);
	used_size = reserved_size = requested_size;
	if (requested_size == 0) {
		data = 0;
		ptr = 0;
		return;
	}
	size_t bytes_to_allocate = allocated_bytes();
	ptr = (char*)calloc(bytes_to_allocate, 1);
	if (ptr == nullptr)
		throw std::bad_alloc();
	uint64_t pad = padding();
	data = (T*)(((uintptr_t)ptr + pad) & ~(uintptr_t)pad);
	assert(ptr <= (char*)data && (char*)data <= ptr + Align);
	assert(((uintptr_t)data & (Align - 1)) == 0); //aligned as expected?
}

template <class T, const int Align> void Array<T, Align>::resize(size_t new_size) {
	if (new_size <= reserved_size) {
		used_size = new_size;
		return;
	}
	char* old_ptr = ptr;
	T* old_data = data;
	size_t old_size = used_size;
	create(new_size);
	if (old_size > 0) {
		assert(old_ptr && old_data);
		memcpy(data, old_data, sizeof(T) * old_size);
	}
	if (old_ptr) {
		free(old_ptr);
		old_ptr = 0;
	}
}

template <class T, const int Align> void Array<T, Align>::push_back(const T& x) {
	if (used_size == reserved_size) {
		size_t old_size = used_size;
		size_t new_size = used_size * 2 + 16;
		resize(new_size);
		used_size = old_size;
	}
	data[used_size++] = x;
}

template <class T, const int Align> Array<T, Align>::~Array() {
	free(ptr);
	used_size = reserved_size = 0;
	data = 0;
	ptr = 0;
}


#endif