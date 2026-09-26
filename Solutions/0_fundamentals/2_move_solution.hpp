// Created by Chris Manlove

#pragma once

#include <type_traits>

namespace solutions::move {

template <typename T>
constexpr std::remove_reference_t<T>&& Move(T&& value) noexcept {
    return static_cast<std::remove_reference_t<T>&&>(value);
}

}  // namespace solutions::move
