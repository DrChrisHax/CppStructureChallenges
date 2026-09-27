// Created by Chris Manlove

#pragma once

#include <type_traits>

namespace solutions::move {

template <typename T>
constexpr std::remove_reference_t<T>&& Move(T&& value) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(value);
}

template <typename T>
constexpr std::conditional_t<
    std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>,
    T&&,
    const T&> MoveIfNoexcept(T& value) noexcept {
    return static_cast<T&&>(value);
}

}  // namespace solutions::move
