// Created by Chris Manlove

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <stdexcept>

namespace solutions::aligned_new {

// ---------------------------------------------------------------------------------------------------------------------
// DO NOT CHANGE THIS SECTION
// ---------------------------------------------------------------------------------------------------------------------

// Every call to CacheLine's allocation functions, recorded by the code you write in Part 3.
struct AlignedStats {
    static void Reset() {
        Allocations = 0;
        Deallocations = 0;
        BytesAllocated = 0;
        BytesFreed = 0;
        LastAlignment = 0;
    }

    inline static int32_t Allocations = 0;
    inline static int32_t Deallocations = 0;
    inline static size_t BytesAllocated = 0;
    inline static size_t BytesFreed = 0;
    inline static size_t LastAlignment = 0;
};

// ---------------------------------------------------------------------------------------------------------------------
// MAKE YOUR CHANGES BELOW
// ---------------------------------------------------------------------------------------------------------------------

// Part 1

// value rounded up to a multiple of alignment, which is a power of two.
inline size_t AlignUp(size_t value, size_t alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

// Part 2

// Raw memory for count Ts, aligned for T, with no objects in it. The caller frees it with DeallocateFor.
template <typename T>
T* AllocateFor(size_t count) {
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        return static_cast<T*>(::operator new(count * sizeof(T), std::align_val_t{alignof(T)}));
    } else {
        return static_cast<T*>(::operator new(count * sizeof(T)));
    }
}

// Frees memory from AllocateFor<T>(count). count is the same number AllocateFor was given.
template <typename T>
void DeallocateFor(T* ptr, size_t count) noexcept {
    if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
        ::operator delete(ptr, count * sizeof(T), std::align_val_t{alignof(T)});
    } else {
        ::operator delete(ptr, count * sizeof(T));
    }
}

// Part 3

struct alignas(64) CacheLine {
    // Given: the constructor and the unsized operator delete at the bottom. Don't change them.
    explicit CacheLine(int32_t value = 0)
        : Value(value)
    {
        if (value < 0) {
            throw std::invalid_argument("negative cache line value");
        }
    }

    static void* operator new(size_t size, std::align_val_t alignment) {
        const size_t align = static_cast<size_t>(alignment);
        void* ptr = std::aligned_alloc(align, AlignUp(size == 0 ? 1 : size, align));
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        ++AlignedStats::Allocations;
        AlignedStats::BytesAllocated += size;
        AlignedStats::LastAlignment = align;
        return ptr;
    }

    static void operator delete(void* ptr, size_t size, std::align_val_t alignment) noexcept {
        if (ptr == nullptr) {
            return;
        }
        std::free(ptr);
        ++AlignedStats::Deallocations;
        AlignedStats::BytesFreed += size;
        AlignedStats::LastAlignment = static_cast<size_t>(alignment);
    }

    // Given: the unsized aligned operator delete, built on yours.
    static void operator delete(void* ptr, std::align_val_t alignment) noexcept {
        operator delete(ptr, sizeof(CacheLine), alignment);
    }

    int32_t Value = 0;
};

}  // namespace solutions::aligned_new
