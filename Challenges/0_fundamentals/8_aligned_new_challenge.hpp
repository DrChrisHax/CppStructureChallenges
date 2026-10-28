// REQUIRED
//
// Challenge 8: Alignment and std::align_val_t
//
// Every type has an alignment: the address of every object of that type must be a multiple of it. alignof(T) tells
// you what it is, and it's always a power of two. An int32_t is 4-aligned, a double is 8-aligned, and a struct is as
// aligned as its most aligned member. The CPU wants this: an aligned load reads the value in one go, a misaligned one
// can be slower, can tear when another thread writes it, and for some instructions crashes outright.
//
// sizeof(T) is always a multiple of alignof(T). That's why arrays work: if element 0 is aligned, element i at
// base + i * sizeof(T) is aligned too. The struct gets padding at the end to make it so.
//
// Most types never need more than 16. alignas lets you ask for more, and there are real reasons to:
// - SIMD: AVX instructions load 32 bytes at a time, and the fast aligned versions need 32-byte aligned addresses.
// - Cache lines: the CPU moves memory in 64-byte lines. If two threads write two different variables that share a
//   line, each write steals the line from the other core ("false sharing") and both slow to a crawl. A lock-free queue
//   puts its head and tail in alignas(64) members so they never share.
// - Pages: hardware and the OS hand out memory in 4096-byte pages, and some buffers (DMA, memory mapping) must start
//   on one.
// A type whose alignment is bigger than alignof(std::max_align_t) is called OVER-ALIGNED.
//
// Here's the problem. operator new(size) only gets a byte count, so it has no idea what the bytes are for. It
// promises one alignment for everything, __STDCPP_DEFAULT_NEW_ALIGNMENT__, which is 16 on x86-64 Linux. That's
// enough for every ordinary type, which is all malloc was built for. Before C++17, new on an alignas(64) type called
// that same function and got 16-aligned memory back. The compiler assumed 64 and generated code that relied on it.
// Undefined behavior, and the crash showed up in the SIMD code, nowhere near the new.
//
// C++17 fixed it by passing the alignment in. std::align_val_t is an enum class wrapping a size_t, and it's a
// separate type so it can't be mistaken for the size. When alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__, the
// new-expression calls a different overload:
//
//   struct alignas(64) Line { ... };
//   Line* p = new Line;      // 1. void* raw = operator new(sizeof(Line), std::align_val_t{64})
//                            // 2. construct a Line in raw
//   delete p;                // 1. p->~Line()
//                            // 2. operator delete(p, sizeof(Line), std::align_val_t{64})
//
// Every form you met last challenge has an aligned twin: new[], delete[], nothrow, all of them, each with an extra
// std::align_val_t after the size. Memory from an aligned operator new must go back through an aligned operator
// delete with the same alignment. Many allocators keep aligned blocks somewhere else entirely, so freeing one with the
// wrong function corrupts the heap.
//
// Trap: if a class declares its own operator new but not the aligned one, an over-aligned class silently skips the
// global aligned form. Lookup finds the class's functions first, tries them with the alignment, finds no match, and
// then tries them again WITHOUT the alignment. Last challenge's Widget with alignas(64) on it would quietly get
// 16-aligned bytes from malloc. A class that takes over allocation for an over-aligned type has to declare the aligned
// forms itself, which is Part 3.
//
// How it shows up in the structures you're about to build:
// - The new-expression picks the aligned form for you. Containers don't use the new-expression: they get raw memory
//   and construct elements one at a time (challenge 7). Calling ::operator new yourself means choosing the form
//   yourself, and forgetting gives every vector<OverAligned> misaligned elements. That's Part 2, and it's exactly what
//   std::allocator<T>::allocate does. Every heap-allocating container you write from here on goes through it.
// - Node-based containers (list, map, unordered_map) allocate nodes, not Ts. A node holds a T, so alignof(Node) is at
//   least alignof(T), and the same choice applies to the node type.
// - Containers that keep elements inline instead of on the heap (array, optional, variant, the small buffer in a
//   small vector) use alignas(T) std::byte storage[sizeof(T)]. You've already seen this in the tests for challenge 7.
//   Without the alignas, a std::byte array is only 1-aligned.
// - Arenas and pools carve many objects out of one big block. Each object starts at the next address that's a
//   multiple of its alignment, which is Part 1.
//
// Part 1: AlignUp(value, alignment) rounds value up to the next multiple of alignment, which is a power of two. A
// value that's already a multiple stays the same. Do it with bit operations, not a loop or a division: a power of two
// has one bit set, so alignment - 1 is a mask of every bit below it.
//
// Part 2: AllocateFor<T>(count) returns raw memory for count Ts, with no objects in it, and DeallocateFor<T> gives it
// back. Call ::operator new and ::operator delete directly. If alignof(T) is more than
// __STDCPP_DEFAULT_NEW_ALIGNMENT__, pass std::align_val_t{alignof(T)} to both; otherwise use the plain forms. The
// choice is made at compile time, so use if constexpr. Pass the size to ::operator delete as well, so the sized forms
// get used.
//
// Part 3: write CacheLine's aligned allocation functions with std::aligned_alloc and std::free.
// std::aligned_alloc(alignment, size) is malloc with an alignment, and it's what aligned operator new sits on in most
// standard libraries. It returns nullptr on failure, and C requires size to be a multiple of alignment, so round the
// size up with your AlignUp. Like last time, a size of 0 must still give back a unique, non-null pointer.
// - operator new(size, alignment): allocate, throw std::bad_alloc if it fails, then record the call in AlignedStats:
//   add 1 to Allocations, add size (the size you were given, not the rounded one) to BytesAllocated, and set
//   LastAlignment.
// - operator delete(ptr, size, alignment): a null ptr does nothing and isn't counted. Otherwise free it, add 1 to
//   Deallocations, add size to BytesFreed, and set LastAlignment.
// The unsized operator delete(ptr, alignment) is given. Read it: it forwards to yours with sizeof(CacheLine). Compilers
// don't agree on which of the two to call. GCC calls the unsized one for delete p, and when the constructor throws it
// ONLY looks for the unsized one. Leave it out and GCC finds nothing, calls nothing, and leaks the memory without a
// warning. Declaring both keeps every compiler happy.
// CacheLine declares no plain operator new, so the trap above can't fire, and no operator new[], so arrays of CacheLine
// fall back to the global aligned forms.
//
// Left out on purpose:
// - std::allocator throws std::bad_array_new_length when count * sizeof(T) would overflow. A real container checks
//   that before it ever calls AllocateFor.
// - std::align, which does Part 1 on a pointer and a remaining-space count at once. You'll write an arena with your
//   own AlignUp, which is easier to read.
// - Over-aligned types on the stack and in globals: the compiler aligns those for you.
// - std::construct_at and std::destroy, the library versions of placement new and p->~T() (challenge 9).
//
// Toolbox: std::align_val_t comes from <new>. std::aligned_alloc and std::free come from <cstdlib>.
// static_cast<size_t>(alignment) gets the number back out of a std::align_val_t.
//
// Resources:
// - https://en.cppreference.com/w/cpp/language/object#Alignment
// - https://en.cppreference.com/w/cpp/memory/new/align_val_t
// - https://en.cppreference.com/w/cpp/memory/new/operator_new  (the aligned forms and the class-scope lookup rules)
// - https://en.cppreference.com/w/cpp/memory/c/aligned_alloc
// - https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0035r4.html  (the C++17 paper, with the motivation)

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <stdexcept>

#include "test.hpp"

namespace challenges::aligned_new {

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
    // Your code here.
    Test::Todo();
}

// Part 2

// Raw memory for count Ts, aligned for T, with no objects in it. The caller frees it with DeallocateFor.
template <typename T>
T* AllocateFor(size_t count) {
    // Your code here.
    Test::Todo();
}

// Frees memory from AllocateFor<T>(count). count is the same number AllocateFor was given.
template <typename T>
void DeallocateFor(T* ptr, size_t count) noexcept {
    // Your code here.
    Test::Todo();
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
        // Your code here.
        Test::Todo();
    }

    static void operator delete(void* ptr, size_t size, std::align_val_t alignment) noexcept {
        // Your code here.
        Test::Todo();
    }

    // Given: the unsized aligned operator delete, built on yours.
    static void operator delete(void* ptr, std::align_val_t alignment) noexcept {
        operator delete(ptr, sizeof(CacheLine), alignment);
    }

    int32_t Value = 0;
};

}  // namespace challenges::aligned_new
