// RECOMMENDED
//
// Challenge 5: std::addressof
//
// &obj usually gives you obj's address. But operator& can be overloaded, and then &obj calls that function and
// returns whatever it wants:
//
//   struct Handle {
//       Handle* operator&() { return nullptr; }   // some COM smart pointers and proxy types do this
//   };
//
//   Handle h;
//   Handle* p = &h;              // nullptr, not h's address
//   Handle* q = AddressOf(h);    // h's real address
//
// Containers can't trust &obj. vector<T> has to find where each element lives to construct it, destroy it, or hand
// out a pointer from operator->, and T is whatever the user gave it. From here on, every container you write uses
// AddressOf wherever it needs an element's address.
//
// How it works: nobody can overload operator& for char. So view obj's memory as a char (reinterpret_cast), take the
// address of that char with the built-in &, and cast the resulting pointer back to T* (reinterpret_cast again).
// Along the way, const and volatile get in the way: reinterpret_cast can add them but never remove them, and
// const_cast is the only cast that can. The cppreference page linked below has a possible implementation that shows
// the exact cast chain. Read it and make sure you understand why each cast is there. (Its second overload, for
// function types, isn't needed here.)
//
// Arrays need nothing extra. For int arr[3], T is int[3] and T* is int(*)[3], the same type &arr gives you.
//
// Rvalues: taking the address of a temporary is almost always a bug, since it's gone at the end of the line. T&
// already refuses AddressOf(42), but not a const rvalue: for AddressOf(std::move(constObj)), T is deduced as a
// const type and T& binds to it. std::addressof blocks this with a deleted overload that takes const T&&. A deleted
// function still takes part in overload resolution, and if it wins, the call refuses to compile. cppreference shows
// it too. The second declaration below is that overload, but right now it doesn't block anything.
//
// AddressOf can never throw, so it should be noexcept. std::addressof is also constexpr because the standard library
// uses a compiler builtin (__builtin_addressof). reinterpret_cast isn't allowed in constexpr, so yours won't be, and
// that's fine. Don't use the builtin or std::addressof.
//
// Implement AddressOf so it does what std::addressof does.
//
// Resources:
// - https://en.cppreference.com/w/cpp/memory/addressof  (see "Possible implementation")
// - https://en.cppreference.com/w/cpp/language/reinterpret_cast
// - https://en.cppreference.com/w/cpp/language/const_cast

#pragma once

#include "test.hpp"

namespace challenges::addressof {

template <typename T>
T* AddressOf(T& obj) {
    // Your code here.
    Test::Todo();
}

template <typename T>
const T* AddressOf(const T&&);

}  // namespace challenges::addressof
