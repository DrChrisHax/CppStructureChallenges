// REQUIRED
//
// Challenge 7: operator new vs. the new-expression
//
// "new" is two different things that share a name:
// - The new-EXPRESSION, new Widget(5), is the language feature you've always used. It does two jobs in order: get raw
//   memory big enough for a Widget, then run Widget's constructor in that memory.
// - The allocation FUNCTION, operator new, is only the first job. It's an ordinary function that takes a byte count
//   and returns a void* to that many raw bytes, with no object in them yet. The new-expression calls it for you.
//
// delete mirrors it: the delete-expression runs the destructor, then calls the deallocation function, operator delete,
// to give the bytes back.
//
//   Widget* w = new Widget(5);    // 1. void* raw = Widget::operator new(sizeof(Widget))
//                                 // 2. construct a Widget(5) in raw
//   delete w;                     // 1. w->~Widget()
//                                 // 2. Widget::operator delete(w, sizeof(Widget))
//
// You can't change what the new-expression does, but you CAN replace operator new and operator delete, and you can
// use the two halves separately. Every container from here on depends on that split. A vector with capacity 8 and
// size 3 has memory for 8 elements but only 3 objects in it: it allocated once, then constructs elements one at a
// time as you push_back. new T[8] can't do that, because it would construct all 8 up front.
//
// The four new-expressions. Each one exists for a reason, and you'll see all four again:
// - new T(args): one object on the heap. If the constructor throws, the memory is already allocated, so the compiler
//   calls the matching operator delete for you before the exception leaves. No leak, and you didn't write any code
//   for it. Free it with delete.
// - new T[n]: n objects in one allocation. delete[] has to run n destructors, but it's only given a pointer, so the
//   compiler hides the count in a few extra bytes (the "array cookie") just before the first element. That's why
//   operator new[] gets asked for MORE than n * sizeof(T), and why delete and delete[] can't be mixed: delete on an
//   array runs one destructor and frees from the wrong address, delete[] on a single object reads a cookie that
//   isn't there. Both are undefined behavior. Free it with delete[].
// - new (std::nothrow) T(args): when memory runs out, returns nullptr instead of throwing std::bad_alloc. For code
//   built without exceptions (games, embedded, kernels) and code that would rather check a pointer than catch. Only
//   the allocation is nothrow: if the constructor throws, the exception still escapes. Free it with delete.
// - Placement new, new (buffer) T(args): constructs a T in memory you already have, and allocates nothing. This is
//   the second half of the new-expression on its own, and it's how vector, optional, variant, and every container
//   that keeps spare room put objects into raw memory. Since nothing was allocated, never delete it: call the
//   destructor yourself (p->~T()) and let whoever owns the memory free it.
//
// Who gets called. A new-expression looks for operator new in the class first, and only if the class has none does it
// use the global one. That's how a class takes over its own allocation: a pool of same-sized blocks for a type you
// make millions of, a counter, a debug guard. Writing ::new instead of new skips the class and goes straight to the
// global functions.
//
// Trap: a class that declares ANY operator new hides EVERY global form, including the placement one. If T has its
// own operator new(std::size_t), then new (buffer) T fails to compile:
//
//   error: no matching function for call to 'T::operator new(sizetype, void*&)'
//
// Placement new doesn't allocate, so it should never go through a class's allocation functions anyway. Write
// ::new (buffer) T(args) and it always finds the global placement form. Containers do exactly that. Don't do it for
// the other three: ::new T skips the class's operator new, which is the opposite of what the class asked for.
//
// Part 1: the four new-expressions. MakeOne, MakeMany, TryMake, and ConstructIn each use one of them. They're
// templates so the tests can hand them types with their own operator new; forward the arguments the way you did in
// challenge 3. MakeMany default-constructs every element. Each comment above a function says how the caller frees
// the result.
//
// Part 2: write Widget's allocation functions with std::malloc and std::free. malloc is C's allocator: it returns raw
// bytes or nullptr, never throws, and knows nothing about constructors. It's what operator new sits on in most
// standard libraries. Widget also records every call in WidgetStats so the tests can see them, the same way the
// next challenge counts every allocation in a whole program.
// - operator new(size): malloc the bytes. If malloc returns nullptr, throw std::bad_alloc; operator new never returns
//   null. A size of 0 must still give back a unique, non-null pointer, but malloc(0) is allowed to return nullptr, so
//   ask malloc for at least one byte. Then add 1 to Allocations and size to BytesAllocated.
// - operator delete(ptr, size): free the bytes. A null ptr does nothing and isn't counted. Otherwise add 1 to
//   Deallocations and size to BytesFreed. size is the same number operator new was given: C++14 "sized deallocation"
//   passes it back so an allocator doesn't have to look it up.
// - operator new[] and operator delete[]: the same pair for arrays. The size includes the array cookie, so for
//   new Widget[3] it's bigger than 3 * sizeof(Widget). The tests check that you recorded the real size.
// The nothrow pair is given. Read it: it's the throwing operator new wrapped in a try/catch. Its operator delete
// only runs in one case, when the constructor throws after a nothrow new, so the compiler has a matching function to
// free the memory with. Widget has to declare it because of the trap above: once Widget has an operator new,
// new (std::nothrow) Widget can't see the global nothrow form either.
//
// Left out on purpose:
// - The new-handler loop: the real operator new calls a user-installed function and retries before giving up and
//   throwing. std::set_new_handler is rarely used.
// - Replacing the GLOBAL operator new, which changes every allocation in the program (challenge 8).
// - The aligned forms that take std::align_val_t, for over-aligned types (challenge 9).
// - std::construct_at and std::destroy, the library versions of placement new and p->~T() (challenge 10).
//
// Toolbox: std::nothrow, std::nothrow_t, and std::bad_alloc come from <new>. std::malloc and std::free come from
// <cstdlib>.
//
// Resources:
// - https://en.cppreference.com/w/cpp/language/new  (the new-expression, placement new, the array cookie)
// - https://en.cppreference.com/w/cpp/memory/new/operator_new  (all the forms, and class-specific overloads)
// - https://en.cppreference.com/w/cpp/memory/new/operator_delete
// - https://en.cppreference.com/w/cpp/memory/c/malloc
// - https://www.learncpp.com/cpp-tutorial/dynamic-memory-allocation-with-new-and-delete/

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <utility>

#include "test.hpp"

namespace challenges::operator_new {

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
    // Your code here.
    Test::Todo();
}

// count default-constructed objects in one allocation. The caller frees them with delete[].
template <typename T>
T* MakeMany(size_t count) {
    // Your code here.
    Test::Todo();
}

// One object on the heap, or nullptr if the allocation fails. The caller frees it with delete.
template <typename T, typename... Args>
T* TryMake(Args&&... args) {
    // Your code here.
    Test::Todo();
}

// One object built in buffer, which is big enough and aligned for T. The caller calls the destructor and owns buffer.
template <typename T, typename... Args>
T* ConstructIn(void* buffer, Args&&... args) {
    // Your code here.
    Test::Todo();
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
        // Your code here.
        Test::Todo();
    }

    static void operator delete(void* ptr, size_t size) noexcept {
        // Your code here.
        Test::Todo();
    }

    static void* operator new[](size_t size) {
        // Your code here.
        Test::Todo();
    }

    static void operator delete[](void* ptr, size_t size) noexcept {
        // Your code here.
        Test::Todo();
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

}  // namespace challenges::operator_new
