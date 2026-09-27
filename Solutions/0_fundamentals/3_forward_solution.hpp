// Created by Chris Manlove

#pragma once

#include <type_traits>

namespace solutions::forward {

template <typename T>
constexpr decltype(auto) Forward(std::remove_reference_t<T>& value) noexcept {
    return static_cast<T&&>(value);
}

template <typename T>
constexpr decltype(auto) Forward(std::remove_reference_t<T>&& value) noexcept {
    static_assert(!std::is_lvalue_reference_v<T>, "Can't forward an rvalue as an lvalue");
    return static_cast<T&&>(value);
}

}  // namespace solutions::forward
