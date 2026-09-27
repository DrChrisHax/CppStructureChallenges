// Created by Chris Manlove

#pragma once

#include <cstddef>

#include <type_traits>
#include <utility>

namespace solutions::swap {

template <typename T>
constexpr void Swap(T& a, T& b) noexcept(
    std::is_nothrow_move_constructible_v<T> &&
    std::is_nothrow_move_assignable_v<T>) {
    T tmp = std::move(a);
    a = std::move(b);
    b = std::move(tmp);
}

template <typename T, std::size_t N>
constexpr void Swap(T (&a)[N], T (&b)[N]) noexcept(
    std::is_nothrow_swappable_v<T>) {
    for (std::size_t i = 0; i < N; ++i) {
        Swap(a[i], b[i]);
    }
}

template <typename T, typename U = T>
constexpr T Exchange(T& obj, U&& newValue) noexcept(
    std::is_nothrow_move_constructible_v<T> &&
    std::is_nothrow_assignable_v<T&, U>) {
    T tmp = std::move(obj);
    obj = std::forward<U>(newValue);
    return tmp;
}

}  // namespace solutions::swap
