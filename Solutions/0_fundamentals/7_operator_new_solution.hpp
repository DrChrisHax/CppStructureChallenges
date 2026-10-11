// Created by Chris Manlove

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

namespace solutions::operator_new {

// ---------------------------------------------------------------------------------------------------------------------
// DO NOT CHANGE THIS SECTION
// ---------------------------------------------------------------------------------------------------------------------

// Every call to Widget's allocation functions, recorded by the code you write in Part 2. Alive counts live Widgets.
struct WidgetStats {
    static void Reset() {
        Allocations = 0;
        Deallocations = 0;
        BytesAllocated = 0;
        BytesFreed = 0;
        Alive = 0;
    }

    inline static int32_t Allocations = 0;
    inline static int32_t Deallocations = 0;
    inline static size_t BytesAllocated = 0;
    inline static size_t BytesFreed = 0;
    inline static int32_t Alive = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// MAKE YOUR CHANGES BELOW
// ---------------------------------------------------------------------------------------------------------------------

// Part 1

// One object on the heap. The caller frees it with delete.
template <typename T, typename... Args>
T* MakeOne(Args&&... args) {
    return new T(std::forward<Args>(args)...);
}

// count default-constructed objects in one allocation. The caller frees them with delete[].
template <typename T>
T* MakeMany(size_t count) {
    return new T[count];
}

// One object on the heap, or nullptr if the allocation fails. The caller frees it with delete.
template <typename T, typename... Args>
T* TryMake(Args&&... args) {
    // Your code here.
    return new(std::nothrow) T(std::forward<Args>(args)...);
}

// One object built in buffer, which is big enough and aligned for T. The caller calls the destructor and owns buffer.
template <typename T, typename... Args>
T* ConstructIn(void* buffer, Args&&... args) {
    return ::new(buffer) T(std::forward<Args>(args)...);
}

// Part 2

struct Widget {
    // Given: the constructor, the destructor, and the nothrow pair at the bottom. Don't change them.
    explicit Widget(int32_t value = 0)
        : Value(value)
    {
        if (value < 0) {
            throw std::invalid_argument("negative widget value");
        }
        ++WidgetStats::Alive;
    }

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    ~Widget() {
        --WidgetStats::Alive;
    }

    static void* operator new(size_t size) {
        void* ptr = malloc((size == 0)? 1 : size);
        if (ptr == nullptr) { throw std::bad_alloc(); }
        ++WidgetStats::Allocations;
        WidgetStats::BytesAllocated += size;
        return ptr;
    }

    static void operator delete(void* ptr, size_t size) noexcept {
        if (ptr == nullptr) { return; }
        ++WidgetStats::Deallocations;
        WidgetStats::BytesFreed += size;
        std::free(ptr);
    }

    static void* operator new[](size_t size) {
        return operator new(size);
    }

    static void operator delete[](void* ptr, size_t size) noexcept {
        operator delete(ptr, size);
    }

    // Given: the nothrow pair, built on your operator new and operator delete.
    static void* operator new(size_t size, const std::nothrow_t&) noexcept {
        try {
            return operator new(size);
        } catch (const std::bad_alloc&) {
            return nullptr;
        }
    }

    static void operator delete(void* ptr, const std::nothrow_t&) noexcept {
        operator delete(ptr, sizeof(Widget));
    }

    int32_t Value = 0;
};


}  // namespace solutions::operator_new
