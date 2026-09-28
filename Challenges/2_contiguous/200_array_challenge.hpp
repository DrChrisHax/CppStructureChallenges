// REQUIRED
//
// Challenge 200: std::array
//
// std::array is a C array wrapped in a struct. That's the whole trick, and this challenge is about seeing it.
//
// The size is part of the type. Array<int, 3> and Array<int, 4> are different types, and N is known at compile
// time, so the size isn't stored anywhere: Size() just returns N.
//
// The elements live inside the object. There's no pointer and no heap allocation: a local Array<int, 3> sits entirely
// on the stack, and sizeof(Array<int, 3>) is exactly sizeof(int[3]). Compare that with vector (challenge 210), which
// is a pointer to a heap buffer plus a size and a capacity.
//
// Wrapping the C array in a struct gives it value semantics for free. A plain C array can't be copied with =, can't
// be assigned, can't be returned from a function, and decays to a pointer when you pass it. A struct that contains
// one can do all of that: the compiler-generated copy constructor and copy assignment copy the C array element by
// element. You write none of that code, and the tests check it works.
//
// It's an aggregate: a plain struct with public members and no constructors. Aggregates can be brace-initialized
// member by member, which is how `Array<int, 3> a = {1, 2, 3};` fills Elements without any constructor. So keep
// Elements public, and don't add constructors, a base class, or private members, or that line stops compiling.
// Elements is public only because aggregates need it to be; code that uses an Array goes through the member
// functions. (libstdc++ calls it _M_elems.)
//
// Everything is constexpr, like std::array. The tests use an Array in a constant expression.
//
// What to implement:
// - operator[]: index into Elements, with no bounds check. There's a const and a non-const version: the const one
//   returns const T&, so a const Array can be read but not written.
// - At: the same, but with a bounds check. It throws std::out_of_range when the index is out of range. This check is
//   the one thing At adds over a C array.
// - Data: a pointer to the first element. A C array already converts to one.
// - Size: returns N.
// - begin and end: a pointer already is an iterator. It supports ++, --, +, -, and comparisons, and the elements sit
//   next to each other in memory, so Iterator is just T*. begin() points at the first element and end() points one
//   past the last. They're lowercase, unlike everything else, because range-for looks for members with exactly those
//   names. Once they work, range-for and std::sort work too.
// - Swap: swap the two arrays element by element. Because the data is inline, there's no pointer to trade, so swapping
//   two arrays is O(N). Swapping two vectors is O(1). Write Swap's noexcept yourself: it should be noexcept exactly
//   when swapping two T's is (challenge 4).
//
// Left out on purpose: std::array also has front, back, empty, max_size, fill, the cbegin/rbegin family, comparison
// operators, get for structured bindings, and std::to_array. They repeat the ideas above. One detail worth knowing:
// std::array<T, 0> is legal even though a C array T[0] isn't, so standard libraries special-case N == 0 (libstdc++
// swaps in an empty struct as the storage). That case isn't part of this challenge.
//
// Resources:
// - https://en.cppreference.com/w/cpp/container/array
// - https://en.cppreference.com/w/cpp/language/aggregate_initialization
// - https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/array  (the real thing, see _M_elems)

#pragma once

#include <cstddef>

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "test.hpp"

namespace challenges::array {

template <typename T, std::size_t N>
struct Array {
    using ValueType = T;
    using SizeType = std::size_t;
    using Iterator = T*;
    using ConstIterator = const T*;

    constexpr T& operator[](SizeType index) {
        // Your code here.
        Test::Todo();
    }

    constexpr const T& operator[](SizeType index) const {
        // Your code here.
        Test::Todo();
    }

    constexpr T& At(SizeType index) {
        // Your code here.
        Test::Todo();
    }

    constexpr const T& At(SizeType index) const {
        // Your code here.
        Test::Todo();
    }

    constexpr T* Data() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr const T* Data() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr SizeType Size() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator begin() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr ConstIterator begin() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator end() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr ConstIterator end() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr void Swap(Array& other) {
        // Your code here.
        Test::Todo();
    }

    T Elements[N];
};

}  // namespace challenges::array
