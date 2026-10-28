// Created by Chris Manlove

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>

#include "0_fundamentals/8_aligned_new_tests.hpp"
#include "0_fundamentals/8_aligned_new_challenge.hpp"
#include "0_fundamentals/8_aligned_new_solution.hpp"

namespace tests::aligned_new {

// Bigger than PTRDIFF_MAX, so aligned_alloc always refuses it. volatile so GCC can't see the constant and warn.
volatile size_t impossibleSize = std::numeric_limits<size_t>::max() / 2 + 1;

// Frees memory that CacheLine's operator new got from aligned_alloc, without going through your operator delete.
[[gnu::noinline]] void FreeRaw(void* ptr) {
    std::free(ptr);
}

bool IsAligned(const void* ptr, size_t alignment) {
    return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
}

// Over-aligned enough that malloc's 16-byte alignment is almost never good enough by luck.
struct alignas(4096) Page {
    std::byte Bytes[4096];
};

struct alignas(64) Line {
    int32_t Value;
};

struct Counted {
    Counted() {
        ++Alive;
    }

    ~Counted() {
        --Alive;
    }

    inline static int32_t Alive = 0;
};

struct User {
    using CacheLine = challenges::aligned_new::CacheLine;
    using Stats = challenges::aligned_new::AlignedStats;

    static size_t AlignUp(size_t value, size_t alignment) {
        return challenges::aligned_new::AlignUp(value, alignment);
    }

    template <typename T>
    static T* AllocateFor(size_t count) {
        return challenges::aligned_new::AllocateFor<T>(count);
    }

    template <typename T>
    static void DeallocateFor(T* ptr, size_t count) {
        challenges::aligned_new::DeallocateFor<T>(ptr, count);
    }
};

struct Solution {
    using CacheLine = solutions::aligned_new::CacheLine;
    using Stats = solutions::aligned_new::AlignedStats;

    static size_t AlignUp(size_t value, size_t alignment) {
        return solutions::aligned_new::AlignUp(value, alignment);
    }

    template <typename T>
    static T* AllocateFor(size_t count) {
        return solutions::aligned_new::AllocateFor<T>(count);
    }

    template <typename T>
    static void DeallocateFor(T* ptr, size_t count) {
        solutions::aligned_new::DeallocateFor<T>(ptr, count);
    }
};

// Part 1: AlignUp

template <typename Impl>
void AlignUpValues() {
    Test::Check(Impl::AlignUp(0, 8) == 0, "AlignUp(0, 8) should be 0");
    Test::Check(Impl::AlignUp(1, 8) == 8, "AlignUp(1, 8) should be 8");
    Test::Check(Impl::AlignUp(8, 8) == 8, "AlignUp(8, 8) should be 8: a multiple stays the same");
    Test::Check(Impl::AlignUp(9, 8) == 16, "AlignUp(9, 8) should be 16");
    Test::Check(Impl::AlignUp(13, 1) == 13, "AlignUp(13, 1) should be 13: everything is 1-aligned");
    Test::Check(Impl::AlignUp(100, 64) == 128, "AlignUp(100, 64) should be 128");
    Test::Check(Impl::AlignUp(4097, 4096) == 8192, "AlignUp(4097, 4096) should be 8192");
}

// Part 2: AllocateFor and DeallocateFor

template <typename Impl>
void AllocateForInts() {
    int32_t* p = Impl::template AllocateFor<int32_t>(10);
    Test::Check(p != nullptr, "AllocateFor<int32_t>(10) should return memory");
    Test::Check(IsAligned(p, alignof(int32_t)), "AllocateFor<int32_t> should be aligned for int32_t");
    for (int32_t i = 0; i < 10; ++i) {
        p[i] = i;
    }
    for (int32_t i = 0; i < 10; ++i) {
        Test::Check(p[i] == i, "All 10 ints should be usable");
    }
    Impl::DeallocateFor(p, 10);
}

template <typename Impl>
void AllocateForZero() {
    int32_t* p = Impl::template AllocateFor<int32_t>(0);
    Test::Check(p != nullptr, "AllocateFor(0) should still return a non-null pointer");
    Impl::DeallocateFor(p, 0);
}

template <typename Impl>
void AllocateForNoObjects() {
    Counted::Alive = 0;
    Counted* p = Impl::template AllocateFor<Counted>(4);
    Test::Check(Counted::Alive == 0, "AllocateFor should only allocate; it must not construct anything");
    Impl::DeallocateFor(p, 4);
    Test::Check(Counted::Alive == 0, "DeallocateFor should only free; it must not destroy anything");
}

template <typename Impl>
void AllocateForLine() {
    for (int32_t i = 0; i < 8; ++i) {
        Line* p = Impl::template AllocateFor<Line>(5);
        Test::Check(IsAligned(p, 64), "AllocateFor<Line> should be 64-aligned: pass std::align_val_t");
        for (int32_t j = 0; j < 5; ++j) {
            p[j].Value = j;
        }
        Impl::DeallocateFor(p, 5);
    }
}

template <typename Impl>
void AllocateForPage() {
    for (int32_t i = 0; i < 8; ++i) {
        Page* p = Impl::template AllocateFor<Page>(3);
        Test::Check(IsAligned(p, 4096), "AllocateFor<Page> should be 4096-aligned: pass std::align_val_t");
        p[2].Bytes[4095] = std::byte{1};
        Impl::DeallocateFor(p, 3);
    }
}

// Part 3: CacheLine's aligned allocation functions

template <typename Impl>
void CacheLineNew() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    CacheLine* p = new CacheLine(5);
    Test::Check(p->Value == 5, "new CacheLine(5) should construct a CacheLine with value 5");
    Test::Check(IsAligned(p, 64), "new CacheLine should return 64-aligned memory");
    Test::Check(Stats::Allocations == 1, "new CacheLine should call your aligned operator new once");
    Test::Check(Stats::BytesAllocated == sizeof(CacheLine), "BytesAllocated should be the size you were given");
    Test::Check(Stats::LastAlignment == 64, "LastAlignment should be the alignment you were given");
    p->~CacheLine();
    FreeRaw(p);
}

template <typename Impl>
void CacheLineDelete() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    CacheLine* p = new CacheLine(5);
    Stats::Reset();
    delete p;
    Test::Check(Stats::Deallocations == 1, "delete should call your aligned operator delete once");
    Test::Check(Stats::BytesFreed == sizeof(CacheLine), "BytesFreed should be the size operator delete was given");
    Test::Check(Stats::LastAlignment == 64, "operator delete should record the alignment it was given");
}

template <typename Impl>
void CacheLineDeleteNull() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    CacheLine::operator delete(nullptr, sizeof(CacheLine), std::align_val_t{64});
    Test::Check(Stats::Deallocations == 0, "operator delete(nullptr) should do nothing and not be counted");
}

template <typename Impl>
void CacheLineNewZero() {
    using CacheLine = typename Impl::CacheLine;
    void* a = CacheLine::operator new(0, std::align_val_t{64});
    void* b = CacheLine::operator new(0, std::align_val_t{64});
    Test::Check(a != nullptr && b != nullptr, "operator new(0, 64) should return a non-null pointer");
    Test::Check(a != b, "Two calls to operator new(0, 64) should return different pointers");
    Test::Check(IsAligned(a, 64) && IsAligned(b, 64), "operator new(0, 64) should still be 64-aligned");
    FreeRaw(a);
    FreeRaw(b);
}

template <typename Impl>
void CacheLineNewOddSize() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    void* p = CacheLine::operator new(100, std::align_val_t{256});
    Test::Check(IsAligned(p, 256), "operator new(100, 256) should be 256-aligned");
    Test::Check(Stats::BytesAllocated == 100, "BytesAllocated should be the size you were given, not the rounded size");
    Test::Check(Stats::LastAlignment == 256, "LastAlignment should be 256");
    static_cast<unsigned char*>(p)[99] = 1;
    FreeRaw(p);
}

template <typename Impl>
void CacheLineOutOfMemory() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    bool threw = false;
    try {
        void* p = CacheLine::operator new(impossibleSize, std::align_val_t{64});
        FreeRaw(p);
    } catch (const std::bad_alloc&) {
        threw = true;
    }
    Test::Check(threw, "operator new should throw std::bad_alloc when aligned_alloc returns nullptr");
    Test::Check(Stats::Allocations == 0, "A failed allocation should not be counted");
}

template <typename Impl>
void CacheLineConstructorThrows() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    bool threw = false;
    try {
        CacheLine* p = new CacheLine(-1);
        delete p;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Test::Check(threw, "new CacheLine(-1) should let the constructor's exception through");
    Test::Check(Stats::Allocations == 1, "The memory is allocated before the constructor runs");
    Test::Check(Stats::Deallocations == 1, "When the constructor throws, the compiler calls your aligned operator delete");
}

template <typename Impl>
void CacheLineArray() {
    using CacheLine = typename Impl::CacheLine;
    using Stats = typename Impl::Stats;
    // A scalar new goes through yours. Unimplemented, this hits Todo and the test shows TODO instead of PASS.
    delete new CacheLine(1);
    Stats::Reset();
    CacheLine* lines = new CacheLine[3];
    Test::Check(IsAligned(lines, 64), "new CacheLine[3] should be 64-aligned, from the global aligned operator new[]");
    Test::Check(Stats::Allocations == 0, "CacheLine has no operator new[], so arrays shouldn't call your operator new");
    delete[] lines;
    Test::Check(Stats::Deallocations == 0, "CacheLine has no operator delete[], so delete[] shouldn't call yours");
}

}  // namespace tests::aligned_new

Test8::Test8()
    : Test(8, "Alignment and std::align_val_t")
{}

void Test8::RunTests() {
    using namespace tests::aligned_new;

    Run("AlignUp: rounds up to a power of two", 1, AlignUpValues<User>, AlignUpValues<Solution>);
    Run("AllocateFor: ints", 1, AllocateForInts<User>, AllocateForInts<Solution>);
    Run("AllocateFor: zero elements", 1, AllocateForZero<User>, AllocateForZero<Solution>);
    Run("AllocateFor: constructs nothing", 1, AllocateForNoObjects<User>, AllocateForNoObjects<Solution>);
    Run("AllocateFor: 64-aligned type", 1, AllocateForLine<User>, AllocateForLine<Solution>);
    Run("AllocateFor: 4096-aligned type", 1, AllocateForPage<User>, AllocateForPage<Solution>);
    Run("CacheLine: aligned operator new", 1, CacheLineNew<User>, CacheLineNew<Solution>);
    Run("CacheLine: aligned operator delete", 1, CacheLineDelete<User>, CacheLineDelete<Solution>);
    Run("CacheLine: operator delete(nullptr)", 1, CacheLineDeleteNull<User>, CacheLineDeleteNull<Solution>);
    Run("CacheLine: operator new(0)", 1, CacheLineNewZero<User>, CacheLineNewZero<Solution>);
    Run("CacheLine: size not a multiple of alignment", 1, CacheLineNewOddSize<User>, CacheLineNewOddSize<Solution>);
    Run("CacheLine: throws std::bad_alloc", 1, CacheLineOutOfMemory<User>, CacheLineOutOfMemory<Solution>);
    Run(
        "CacheLine: constructor throws, memory freed",
        1,
        CacheLineConstructorThrows<User>,
        CacheLineConstructorThrows<Solution>);
    Run("CacheLine: arrays use the global aligned forms", 1, CacheLineArray<User>, CacheLineArray<Solution>);
}
