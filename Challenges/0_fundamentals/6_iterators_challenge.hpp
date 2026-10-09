// REQUIRED
//
// Challenge 6: Iterators
//
// An iterator is a small object that remembers a position in a container and knows two things: how to read the
// element at that position, and how to move to another position. Every algorithm in <algorithm> is written against
// iterators instead of containers, so one std::find works on a vector, a list, a map, and everything you build later.
// Every container in this project gets an iterator. This challenge teaches the parts they all share, on the simplest
// container there is: a fixed array. How an iterator moves through a specific structure (following list nodes,
// jumping between deque blocks, climbing a tree) is its own challenge right after that structure.
//
// Half-open ranges. A container hands out two iterators: begin() at the first element, and end() one PAST the last.
// The range is [begin, end): begin is included, end is not. end() is a position you can compare against, never one
// you can read, so dereferencing end() is undefined behavior. An empty range has begin() == end(). Every loop is
// "while (it != end) { use *it; ++it; }", and that's exactly what range-for turns into:
//
//   for (int x : arr) { ... }
//
// becomes roughly
//
//   auto it = arr.begin();
//   auto last = arr.end();
//   for (; it != last; ++it) { int x = *it; ... }
//
// So range-for needs member functions named exactly begin and end, plus operator!= (C++20 writes it for you from
// operator==), operator++, and operator* on the iterator. The names begin and end break the PascalCase style rule because
// the language looks for those exact names.
//
// Categories. Not every structure can move the same way. The standard sorts iterators into a ladder, and each step
// adds one ability on top of the one below:
// - Input: read the current element and step forward, once. Copies of an input iterator can't walk the range
//   separately (think of reading from a network stream). Mentioned only; you won't write one here.
// - Forward: like input, but multi-pass: copy the iterator, walk both copies, and they see the same elements. Must be
//   default-constructible. A singly linked list stops here: a node has no pointer back.
// - Bidirectional: adds -- to step backward. A doubly linked list or a tree stops here.
// - Random access: adds jumping n steps in O(1) (+=, -=, +, -), indexing it[n], ordering with <, and the distance
//   between two iterators (it2 - it1). An array, a vector, or a deque gets here.
// - Contiguous: random access, plus a promise that the elements sit next to each other in memory, so &*it + 1 is the
//   next element. Only array-like containers qualify.
//
// The category is a promise about COST. A linked list could implement += n with a loop, but then std::sort would
// believe it can jump in O(1) and quietly become far slower. Pick the strongest category your structure can do in
// O(1) per operation, and no stronger.
//
// The member types. Algorithms learn about an iterator by reading types it declares. These names are fixed by the
// standard (std::iterator_traits and the C++20 concepts look for exactly these spellings), so they're another
// exception to the PascalCase rule:
// - value_type: the element type, without const. Even a const iterator over ints has value_type int.
// - difference_type: the signed type for "how far apart are two iterators". Use std::ptrdiff_t. It's signed because
//   it2 - it1 can be negative.
// - reference: what *it returns. T& normally, const T& for a const iterator.
// - pointer: what it-> returns. T* normally, const T* for a const iterator.
// - iterator_category: the category tag for older (pre-C++20) code, e.g. std::bidirectional_iterator_tag.
// - iterator_concept: the category tag the C++20 concepts read first. Set it to the same tag.
// Don't inherit from std::iterator to get these. It was deprecated in C++17. Just write the usings.
//
// Why a plain pointer already works: T* has no member types, so std::iterator_traits has a partial specialization for
// T* that fills them in (value_type T, reference T&, contiguous category). That's why challenges 200 and 201 could
// use T* as their iterator with no class at all.
//
// Checking yourself. C++20 turned each category into a concept: std::forward_iterator, std::bidirectional_iterator,
// std::random_access_iterator, std::contiguous_iterator. Put static_assert(std::random_access_iterator<YourIterator>)
// right under your class and the compiler checks every requirement for you. If it fails, the error lists which requirement
// is missing. The tests here do the same, one requirement at a time, so their failure messages point at the piece
// you're missing. You'll use these static_asserts in every container challenge from here on.
//
// Toolbox: std::conditional_t<Condition, A, B> (from <type_traits>) is A when Condition is true, B otherwise. It picks
// a type at compile time, the way the ternary operator picks a value.
// https://en.cppreference.com/w/cpp/types/conditional
//
// What to implement: ArrayIterator, a random access iterator over the provided Array, plus Array's begin and end.
// An array can jump in O(1), so its iterator gets the full random access set from the start. You'll meet the weaker
// categories when you build the structures that need them (a linked list stops at bidirectional). Every member is
// stubbed below so you can see the full set; each one starts with Test::Todo() so its test shows TODO instead of FAIL
// until you fill it in. Store the position as a pointer to the FIRST element plus an index (the constructor stub takes
// exactly those two). The pointer never moves; every operator works on the index. Don't make the iterator a T* in
// disguise: most structures later on can't use raw pointer math, and the pointer-plus-index shape is the one they'll
// reuse (node pointer, block plus offset, and so on).
//
// The iterator. One template for both constnesses: ArrayIterator<T, false> is Iterator, ArrayIterator<T, true> is
// ConstIterator. std::conditional_t on IsConst picks const T or T for reference and pointer (already in the stub);
// do the same for the element pointer you store. value_type stays T without const, even for the const iterator.
// Don't copy-paste a second class.
// - The six member types. Both tags are std::random_access_iterator_tag. Not contiguous: you're practicing for
//   containers whose elements aren't next to each other.
// - A default constructor, and a constructor that begin and end can use to make an iterator at a position.
// - operator*: returns reference. Make it a const member function. Like span, constness is shallow: a const iterator
//   OBJECT still reads and writes the element. Constness of the ELEMENT comes from the iterator type.
// - operator->: returns pointer, the address of the element, so it->Member works.
// - Pre-increment ++it: moves forward and returns a reference to the iterator itself (*this).
// - Post-increment it++: takes an unused int parameter (that's how C++ tells the two apart), saves a copy, moves
//   forward, and returns the saved copy BY VALUE. Write it by calling your pre-increment.
// - Pre-decrement and post-decrement, the mirror images of the increments.
// - operator==: two iterators are equal when they point at the same position. Defaulting it (= default) compares every
//   member, which is exactly right here. C++20 writes != for you.
// - it += n and it -= n: move n steps (n can be negative) and return *this by reference.
// - it + n, n + it, and it - n: return a new iterator. n + it has the int on the left, so it can't be a member
//   function. Write it as a friend function defined inside the class.
// - it2 - it1: returns difference_type, how many steps from it1 to it2.
// - it[n]: returns reference, the element n steps away. Same as *(it + n).
// - Ordering: <, <=, >, >=. Defaulting operator<=> gives you all four at once.
// - A converting constructor so an Iterator turns into a ConstIterator, but never the other way. Going from mutable
//   to const is safe; the reverse would let you write through a read-only view. Same rule as span's
//   Span<int> -> Span<const int>. The const version needs to read the mutable version's private members, so either
//   befriend ArrayIterator<T, !IsConst> or add a small public accessor.
// - Trap: the obvious converting constructor, one that takes a const ArrayIterator<T, false>& with a requires clause
//   on IsConst, breaks the normal iterator. In ArrayIterator<T, false> that parameter type is the class itself, so
//   the constructor IS the copy constructor. A requires clause doesn't change that: the compiler stops generating
//   the real copy constructor, the requires clause turns yours off, and the iterator can no longer be copied. Fix it
//   by making the converting constructor a template over the other iterator's IsConst (like span's converting
//   constructor is a template), constrained so it only accepts the mutable iterator and only exists in the const one.
//   The stub already has that shape.
// - With the converting constructor in place, comparing an Iterator with a ConstIterator also works.
// - Array's begin() and end(), both overloads. end() is the position one past the last element. The const overloads
//   return ConstIterator, so range-for over a const Array works.
//
// Advance and Distance. std::advance and std::distance are how algorithms move and measure without caring which
// container they're on. The versions here take any std::random_access_iterator:
// - Advance(it, n): it += n in one step. n can be negative.
// - Distance(first, last): last - first in one step.
// The tests use an iterator that counts how often each operator is called, so they can tell a jump from a loop: if
// you loop with ++, the test fails. The general versions, which fall back to a ++ loop for weaker categories, come
// with the first container whose iterator can't jump.
//
// Left out on purpose, each taught where it's needed:
// - Forward and bidirectional iterators, built where a structure can't do better (linked lists). Advance and
//   Distance grow an if constexpr per category there.
// - Proxy iterators, where *it returns an object instead of a real reference (vector<bool>, challenge 242).
// - Sentinels, where end() returns a different type than begin() (C++20 ranges, a later section).
// - Iterator invalidation, when a container change makes existing iterators unsafe to use (vector, challenge 210).
// - std::reverse_iterator, which wraps any bidirectional iterator and walks it backward. You get it for free; the
//   tests use it on yours.
// - next and prev, which are just Advance on a copy.
//
// Resources:
// - https://en.cppreference.com/w/cpp/iterator  (the category ladder and the concepts)
// - https://en.cppreference.com/w/cpp/iterator/iterator_traits
// - https://en.cppreference.com/w/cpp/iterator/random_access_iterator  (the full list of what it asks for)
// - https://en.cppreference.com/w/cpp/language/range-for  (what range-for turns into)
// - https://en.cppreference.com/w/cpp/language/operator_incdec  (pre vs. post increment)
// - https://en.cppreference.com/w/cpp/iterator/advance
// - https://en.cppreference.com/w/cpp/iterator/distance
// - https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp

#pragma once

#include <compare>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include "test.hpp"

namespace challenges::iterators {

template <typename T, bool IsConst>
class ArrayIterator {
public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<IsConst, const T, T>&;
    using pointer = std::conditional_t<IsConst, const T, T>*;

    ArrayIterator() {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator(pointer first, difference_type index) {
        // Your code here.
        Test::Todo();
    }

    // Converting constructor, Iterator -> ConstIterator only (see the trap in the description).
    template <bool OtherIsConst>
        requires (IsConst && !OtherIsConst)
    ArrayIterator(const ArrayIterator<T, OtherIsConst>& other) {
        // Your code here.
        Test::Todo();
    }

    reference operator*() const {
        // Your code here.
        Test::Todo();
    }

    pointer operator->() const {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator& operator++() {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator operator++(int) {
        // Your code here.
        Test::Todo();
    }

    friend bool operator==(const ArrayIterator& a, const ArrayIterator& b) = default;

    ArrayIterator& operator--() {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator operator--(int) {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator& operator+=(difference_type n) {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator& operator-=(difference_type n) {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator operator+(difference_type n) const {
        // Your code here.
        Test::Todo();
    }

    friend ArrayIterator operator+(difference_type n, const ArrayIterator& it) {
        // Your code here.
        Test::Todo();
    }

    ArrayIterator operator-(difference_type n) const {
        // Your code here.
        Test::Todo();
    }

    friend difference_type operator-(const ArrayIterator& a, const ArrayIterator& b) {
        // Your code here.
        Test::Todo();
    }

    reference operator[](difference_type n) const {
        // Your code here.
        Test::Todo();
    }

    friend auto operator<=>(const ArrayIterator& a, const ArrayIterator& b) = default;

private:
    // Your code here: the pointer to the first element and the index. Give both default values, so the default
    // constructor can be "= default" and two default-constructed iterators compare equal (the tests check that).
};

template <typename T, size_t N>
struct Array {
    using Iterator = ArrayIterator<T, false>;
    using ConstIterator = ArrayIterator<T, true>;

    Iterator begin() {
        // Your code here.
        Test::Todo();
    }

    Iterator end() {
        // Your code here.
        Test::Todo();
    }

    ConstIterator begin() const {
        // Your code here.
        Test::Todo();
    }

    ConstIterator end() const {
        // Your code here.
        Test::Todo();
    }

    T Elements[N];
};

template <std::random_access_iterator It>
constexpr void Advance(It& it, std::iter_difference_t<It> n) {
    // Your code here.
    Test::Todo();
}

template <std::random_access_iterator It>
constexpr std::iter_difference_t<It> Distance(It first, It last) {
    // Your code here.
    Test::Todo();
}

}  // namespace challenges::iterators
