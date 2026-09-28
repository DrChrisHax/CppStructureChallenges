// Created by Chris Manlove

#pragma once

#include <type_traits>

namespace solutions::addressof {

template <typename T>
T* AddressOf(T& obj) noexcept {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(obj)));
}

template <typename T>
const T* AddressOf(const T&&) = delete;

}  // namespace solutions::addressof
