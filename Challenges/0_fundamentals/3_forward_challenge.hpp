// REQUIRED
//
// Challenge 3: std::forward
//
// A function template often takes an argument and passes it on to another function (a wrapper, a factory,
// emplace_back). The problem: once the argument has a name, it's an lvalue, even if the caller passed an rvalue. Pass
// it on as-is and every rvalue turns into an lvalue, so moves become copies.
//
//   template <typename T>
//   void Wrapper(T&& arg) {
//       Target(arg);                   // always an lvalue, even for Wrapper(Widget{})
//       Target(std::forward<T>(arg));  // an lvalue if the caller passed an lvalue, an rvalue if they passed an rvalue
//   }
//
// std::forward is a conditional std::move. T&& is a forwarding reference, so T remembers what the caller passed:
//
//   Wrapper(x);         // lvalue: T = int&  -> std::forward<int&>(arg)  returns int&   (stays an lvalue)
//   Wrapper(1);         // rvalue: T = int   -> std::forward<int>(arg)   returns int&&  (back to an rvalue)
//
// Like std::move, it's just a cast. Unlike std::move, you always write T yourself (std::forward<T>(arg)), because
// the whole point is to use the T the wrapper deduced, not a new one. That's why the parameters below use
// std::remove_reference_t<T>: it stops T from being deduced from the argument.
//
// Parameter packs: a template can take any number of arguments with `typename... Args` and `Args&&... args`, and
// forward all of them at once with `std::forward<Args>(args)...`. This is exactly how emplace_back works.
//
// Implement both Forward overloads so they do exactly what std::forward does. The first handles lvalues (the normal
// case); the second handles rvalues, like Forward<int>(42).
//
// Resources:
// - https://en.cppreference.com/w/cpp/utility/forward
// - https://stackoverflow.com/questions/3582001/what-are-the-main-purposes-of-stdforward-and-which-problems-does-it-solve

#pragma once

#include <type_traits>

#include "test.hpp"

namespace challenges::forward {

template <typename T>
constexpr decltype(auto) Forward(std::remove_reference_t<T>& value) noexcept {
    // Your code here.
    Test::Todo();
    return static_cast<T&>(value);
}

template <typename T>
constexpr decltype(auto) Forward(std::remove_reference_t<T>&& value) noexcept {
    // Your code here.
    Test::Todo();
    return static_cast<T&>(value);
}

}  // namespace challenges::forward
