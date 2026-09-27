// REQUIRED
//
// Challenge 2: std::move
//
// std::move doesn't actually move anything. It's a cast: it takes any value, lvalue or rvalue, and turns it into an
// rvalue (an xvalue, to be exact). That rvalue is what tells the compiler "it's OK to steal from this object", so
// a move constructor or move assignment operator gets picked instead of the copy version. The stealing happens
// there, not in std::move.
//
// Implement Move so it does exactly what std::move does.
//
// Resources:
// - https://en.cppreference.com/w/cpp/utility/move
// - https://stackoverflow.com/questions/7510182/how-does-stdmove-transfer-values-into-rvalues

#pragma once

#include <type_traits>

#include "test.hpp"

namespace challenges::move {

template <typename T>
constexpr decltype(auto) Move(T&& value) noexcept {
    // Your code here.
    Test::Todo();
    return static_cast<T&>(value);
}

}  // namespace challenges::move
