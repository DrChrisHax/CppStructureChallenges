// REQUIRED
//
// Challenge 2: std::move and std::move_if_noexcept
//
// std::move doesn't actually move anything. It's a cast: it takes any value, lvalue or rvalue, and turns it into an
// rvalue (an xvalue, to be exact). That rvalue is what tells the compiler "it's OK to steal from this object", so
// a move constructor or move assignment operator gets picked instead of the copy version. The stealing happens
// there, not in std::move.
//
// std::move_if_noexcept is a more careful std::move: it only says "OK to steal" when moving can't throw. Why that
// matters: a vector that runs out of room allocates a bigger buffer and moves every element across. If the 3rd move
// out of 5 throws, the first 2 originals have already been gutted, and the vector can't go back to how it was.
// Copying leaves the originals untouched, so if a copy throws, the vector just throws away the new buffer and nothing
// is lost. So:
//
//   - moving can't throw            -> return T&&       (move, it's safe)
//   - moving can throw, T copyable  -> return const T&  (copy instead, so a throw can be undone)
//   - moving can throw, no copying  -> return T&&       (no other option, move anyway)
//
// You'll need a few standard type traits to ask these questions at compile time. Look up:
// std::is_nothrow_move_constructible_v, std::is_copy_constructible_v, and std::conditional_t.
//
// Implement Move so it does exactly what std::move does, and MoveIfNoexcept so it does exactly what
// std::move_if_noexcept does.
//
// Resources:
// - https://en.cppreference.com/w/cpp/utility/move
// - https://stackoverflow.com/questions/7510182/how-does-stdmove-transfer-values-into-rvalues
// - https://en.cppreference.com/w/cpp/utility/move_if_noexcept

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

template <typename T>
constexpr decltype(auto) MoveIfNoexcept(T& value) noexcept {
    // Your code here.
    Test::Todo();
    return static_cast<T&>(value);
}

}  // namespace challenges::move
