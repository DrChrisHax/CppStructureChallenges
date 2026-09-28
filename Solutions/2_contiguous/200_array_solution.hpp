// Created by Chris Manlove

#pragma once

#include <cstddef>

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "test.hpp"

namespace solutions::array {

template <typename T, std::size_t N>
struct Array {
    using ValueType = T;
    using SizeType = std::size_t;
    using Iterator = T*;
    using ConstIterator = const T*;

    constexpr T& operator[](SizeType index) {
        // Your code here.
        Test::Todo();
    }

    constexpr const T& operator[](SizeType index) const {
        // Your code here.
        Test::Todo();
    }

    constexpr T& At(SizeType index) {
        // Your code here.
        Test::Todo();
    }

    constexpr const T& At(SizeType index) const {
        // Your code here.
        Test::Todo();
    }

    constexpr T* Data() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr const T* Data() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr SizeType Size() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator begin() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr ConstIterator begin() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator end() noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr ConstIterator end() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr void Swap(Array& other) {
        // Your code here.
        Test::Todo();
    }

    T Elements[N];
};

}  // namespace solutions::array
