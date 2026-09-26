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
