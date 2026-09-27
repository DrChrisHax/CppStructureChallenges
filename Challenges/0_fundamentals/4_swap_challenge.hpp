// REQUIRED
//
// Challenge 4: std::swap and std::exchange
//
// Swap trades the values of two objects. Sorting, heaps, and copy-and-swap all lean on it, and they swap a lot, so it
// has to be cheap. That means moving, never copying: a swap that copies turns every swap of a big vector into two
// full copies, and it won't compile at all for a move-only type like unique_ptr.
//
//   int x = 1;
//   int y = 2;
//   Swap(x, y);        // x == 2, y == 1
//
// std::swap also has an overload for C-style arrays of the same size, which swaps them element by element:
//
//   int a[3] = {1, 2, 3};
//   int b[3] = {4, 5, 6};
//   Swap(a, b);        // a == {4, 5, 6}, b == {1, 2, 3}
//
// Hint for the array overload's noexcept (this is template trivia, not the point of the challenge, so here's the
// answer): use noexcept(std::is_nothrow_swappable_v<T>).
//
//   template <typename T, std::size_t N>
//   constexpr void Swap(T (&a)[N], T (&b)[N]) noexcept(std::is_nothrow_swappable_v<T>)
//
// Why not the obvious options? For a 2D array like int[2][2], T is int[2], an array itself:
// - std::is_nothrow_move_constructible_v<T> is false for any array, because arrays can't be move-constructed.
// - noexcept(noexcept(Swap(a[0], b[0]))) looks right but isn't: inside its own noexcept, this overload can't see
//   itself yet, so for int[2] it asks the non-array Swap and gets false.
// std::is_nothrow_swappable_v<T> already handles arrays of any depth, and it's what the standard library itself uses.
//
// Exchange replaces an object's value and hands you back the old one, in one step:
//
//   int x = 1;
//   int old = Exchange(x, 2);    // old == 1, x == 2
//
// Why Exchange has two template parameters, <typename T, typename U = T>:
// - U lets the new value be a different type than obj. Exchange(ptr, nullptr) gives T = int*, U = std::nullptr_t.
//   With only T, the compiler would deduce T from both arguments, get two different types, and fail to compile.
// - The default U = T is for braced init. In Exchange(vec, {}), {} has no type, so U can't be deduced and falls back
//   to T.
//
// You'll use Exchange in almost every move constructor you write from here on. A move constructor has to take the
// other object's resource AND leave the other object empty, so its destructor doesn't free what you just took:
//
//   Owner(Owner&& other) noexcept
//       : ptr_(Exchange(other.ptr_, nullptr))   // take other's pointer, leave nullptr behind
//   {}
//
// Conditional noexcept: noexcept can take a compile-time bool. The function promises not to throw only when the
// bool is true:
//
//   template <typename T>
//   T Copy(const T& value) noexcept(std::is_nothrow_copy_constructible_v<T>);
//
// This matters because containers check noexcept before they move (remember move_if_noexcept from challenge 2). Swap
// and Exchange should be noexcept exactly when the operations they use can't throw. Work out which operations those
// are, and write the noexcept yourself; the stubs below don't have one.
//
// Giving your own type a swap: every container you build later gets a member Swap plus a friend swap that calls it,
// so swapping two containers just trades their internal pointers instead of moving every element:
//
//   class Vector {
//   public:
//       void Swap(Vector& other) noexcept;
//       friend void swap(Vector& a, Vector& b) noexcept { a.Swap(b); }
//   };
//
// In generic code that swaps elements (sorting, heaps), call std::ranges::swap(a, b). It picks up a type's own swap
// if it has one and falls back to the move-based swap otherwise.
//
// Implement both Swap overloads and Exchange so they do what std::swap and std::exchange do, including the noexcept.
//
// Resources:
// - https://en.cppreference.com/w/cpp/algorithm/swap
// - https://en.cppreference.com/w/cpp/utility/exchange
// - https://en.cppreference.com/w/cpp/language/noexcept_spec

#pragma once

#include <cstddef>

#include <type_traits>
#include <utility>

#include "test.hpp"

namespace challenges::swap {

template <typename T>
constexpr void Swap(T& a, T& b) {
    // Your code here.
    Test::Todo();
}

template <typename T, std::size_t N>
constexpr void Swap(T (&a)[N], T (&b)[N]) {
    // Your code here.
    Test::Todo();
}

template <typename T, typename U = T>
constexpr T Exchange(T& obj, U&& newValue) {
    // Your code here.
    Test::Todo();
}

}  // namespace challenges::swap
