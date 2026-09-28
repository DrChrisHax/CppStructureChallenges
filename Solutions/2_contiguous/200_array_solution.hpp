// Created by Chris Manlove

#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace solutions::array {

template <typename T, std::size_t N>
struct Array {
    using ValueType = T;
    using SizeType = std::size_t;
    using Iterator = T*;
    using ConstIterator = const T*;

    constexpr T& operator[](SizeType index) {
        return Elements[index];
    }

    constexpr const T& operator[](SizeType index) const {
        return Elements[index];
    }

    constexpr T& At(SizeType index) {
        if (index >= N) {
            throw std::out_of_range(
                "Array::At: index (which is " + std::to_string(index) + ") >= N (which is " + std::to_string(N) + ")");
        }
        return Elements[index];
    }

    constexpr const T& At(SizeType index) const {
        if (index >= N) {
            throw std::out_of_range(
                "Array::At: index (which is " + std::to_string(index) + ") >= N (which is " + std::to_string(N) + ")");
        }
        return Elements[index];
    }

    constexpr T* Data() noexcept {
        return Elements;
    }

    constexpr const T* Data() const noexcept {
        return Elements;
    }

    constexpr SizeType Size() const noexcept {
        return N;
    }

    constexpr Iterator begin() noexcept {
        return Elements;
    }

    constexpr ConstIterator begin() const noexcept {
        return Elements;
    }

    constexpr Iterator end() noexcept {
        return Elements + N;
    }

    constexpr ConstIterator end() const noexcept {
        return Elements + N;
    }

    constexpr void Swap(Array& other) noexcept(std::is_nothrow_swappable_v<T>) {
        std::swap(Elements, other.Elements);
    }

    T Elements[N];
};

}  // namespace solutions::array
