// Created by Chris Manlove

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

#include "0_fundamentals/7_operator_new_tests.hpp"
#include "0_fundamentals/7_operator_new_challenge.hpp"
#include "0_fundamentals/7_operator_new_solution.hpp"

// AddressSanitizer aborts on an impossible malloc by default. This makes malloc return nullptr instead, as it does
// without ASan, so the out-of-memory tests behave the same in both builds.
extern "C" const char* __asan_default_options() {
    return "allocator_may_return_null=1";
}

namespace tests::operator_new {

// Bigger than PTRDIFF_MAX, so malloc always refuses it. volatile so GCC can't see the constant reach malloc and warn.
volatile size_t impossibleSize = std::numeric_limits<size_t>::max() / 2 + 1;
constexpr int32_t DEFAULT_VALUE = 42;

// Frees memory that Widget's operator new got from malloc, without going through your operator delete. noinline so
// GCC can't see the new-expression on the other side and warn about pairing new with free.
[[gnu::noinline]] void FreeRaw(void* ptr) {
    std::free(ptr);
}

struct Counted {
    Counted()
        : Value(DEFAULT_VALUE)
    {
        ++Alive;
    }

    explicit Counted(int32_t value)
        : Value(value)
    {
        ++Alive;
    }

    Counted(const Counted&) = delete;
    Counted& operator=(const Counted&) = delete;

    ~Counted() {
        --Alive;
    }

    inline static int32_t Alive = 0;

    int32_t Value = 0;
};

struct MoveOnly {
    explicit MoveOnly(int32_t value)
        : Value(value)
    {}

    MoveOnly(MoveOnly&&) = default;
    MoveOnly(const MoveOnly&) = delete;

    int32_t Value = 0;
};

// Takes a move-only argument, so it only compiles if the arguments are forwarded.
struct Holder {
    Holder(MoveOnly moveOnly, int32_t extra)
        : Value(moveOnly.Value + extra)
    {}

    int32_t Value = 0;
};

// A type with its own allocation functions. Its nothrow operator new always fails, and declaring any operator new
// hides the global placement form.
struct NoMemory {
    static void* operator new(size_t) {
        throw std::bad_alloc();
    }

    static void* operator new(size_t, const std::nothrow_t&) noexcept {
        return nullptr;
    }

    static void operator delete(void* ptr) noexcept {
        ::operator delete(ptr);
    }

    int32_t Value = 3;
};

struct User {
    using Widget = challenges::operator_new::Widget;
    using Stats = challenges::operator_new::WidgetStats;

    template <typename T, typename... Args>
    static T* MakeOne(Args&&... args) {
        return challenges::operator_new::MakeOne<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    static T* MakeMany(size_t count) {
        return challenges::operator_new::MakeMany<T>(count);
    }

    template <typename T, typename... Args>
    static T* TryMake(Args&&... args) {
        return challenges::operator_new::TryMake<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    static T* ConstructIn(void* buffer, Args&&... args) {
        return challenges::operator_new::ConstructIn<T>(buffer, std::forward<Args>(args)...);
    }
};

struct Solution {
    using Widget = solutions::operator_new::Widget;
    using Stats = solutions::operator_new::WidgetStats;

    template <typename T, typename... Args>
    static T* MakeOne(Args&&... args) {
        return solutions::operator_new::MakeOne<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    static T* MakeMany(size_t count) {
        return solutions::operator_new::MakeMany<T>(count);
    }

    template <typename T, typename... Args>
    static T* TryMake(Args&&... args) {
        return solutions::operator_new::TryMake<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename... Args>
    static T* ConstructIn(void* buffer, Args&&... args) {
        return solutions::operator_new::ConstructIn<T>(buffer, std::forward<Args>(args)...);
    }
};

// Part 1: the four new-expressions

template <typename Impl>
void MakeOneCounted() {
    Counted::Alive = 0;
    Counted* p = Impl::template MakeOne<Counted>(5);
    Test::Check(p != nullptr, "MakeOne should return the new object");
    Test::Check(p->Value == 5, "MakeOne<Counted>(5) should construct a Counted with value 5");
    Test::Check(Counted::Alive == 1, "MakeOne should construct exactly one object");
    delete p;
    Test::Check(Counted::Alive == 0, "delete should destroy the object from MakeOne");
}

template <typename Impl>
void MakeOneForwards() {
    Holder* p = Impl::template MakeOne<Holder>(MoveOnly(3), 4);
    Test::Check(p->Value == 7, "MakeOne<Holder>(MoveOnly(3), 4) should forward both arguments to the constructor");
    delete p;
}

template <typename Impl>
void MakeManyCounted() {
    Counted::Alive = 0;
    Counted* p = Impl::template MakeMany<Counted>(4);
    Test::Check(Counted::Alive == 4, "MakeMany<Counted>(4) should construct 4 objects");
    for (int32_t i = 0; i < 4; ++i) {
        Test::Check(p[i].Value == DEFAULT_VALUE, "Every element should be default-constructed");
    }
    delete[] p;
    Test::Check(Counted::Alive == 0, "delete[] should destroy all 4 objects from MakeMany");
}

template <typename Impl>
void MakeManyZero() {
    Counted::Alive = 0;
    Counted* p = Impl::template MakeMany<Counted>(0);
    Test::Check(p != nullptr, "new T[0] returns a valid, non-null pointer");
    Test::Check(Counted::Alive == 0, "MakeMany(0) should construct nothing");
    delete[] p;
}

template <typename Impl>
void TryMakeCounted() {
    Counted::Alive = 0;
    Counted* p = Impl::template TryMake<Counted>(9);
    Test::Check(p != nullptr, "TryMake should return the new object when memory is available");
    Test::Check(p->Value == 9, "TryMake<Counted>(9) should construct a Counted with value 9");
    delete p;
    Test::Check(Counted::Alive == 0, "delete should destroy the object from TryMake");
}

template <typename Impl>
void TryMakeForwards() {
    Holder* p = Impl::template TryMake<Holder>(MoveOnly(1), 2);
    Test::Check(p != nullptr && p->Value == 3, "TryMake<Holder>(MoveOnly(1), 2) should forward both arguments");
    delete p;
}

template <typename Impl>
void TryMakeOutOfMemory() {
    NoMemory* p = Impl::template TryMake<NoMemory>();
    Test::Check(
        p == nullptr,
        "TryMake<NoMemory>() should return nullptr: NoMemory's nothrow operator new fails (did you write ::new? that "
        "skips the class's operator new)");
    delete p;
}

template <typename Impl>
void ConstructInCounted() {
    Counted::Alive = 0;
    alignas(Counted) std::byte buffer[sizeof(Counted)];
    Counted* p = Impl::template ConstructIn<Counted>(buffer, 6);
    Test::Check(static_cast<void*>(p) == static_cast<void*>(buffer), "ConstructIn should build the object in buffer");
    Test::Check(p->Value == 6, "ConstructIn<Counted>(buffer, 6) should construct a Counted with value 6");
    Test::Check(Counted::Alive == 1, "ConstructIn should construct exactly one object");
    p->~Counted();
    Test::Check(Counted::Alive == 0, "Calling the destructor should end the object's life");
}

template <typename Impl>
void ConstructInForwards() {
    alignas(Holder) std::byte buffer[sizeof(Holder)];
    Holder* p = Impl::template ConstructIn<Holder>(buffer, MoveOnly(5), 5);
    Test::Check(p->Value == 10, "ConstructIn<Holder>(buffer, MoveOnly(5), 5) should forward both arguments");
    p->~Holder();
}

template <typename Impl>
void ConstructInOwnOperatorNew() {
    alignas(NoMemory) std::byte buffer[sizeof(NoMemory)];
    NoMemory* p = Impl::template ConstructIn<NoMemory>(buffer);
    Test::Check(
        static_cast<void*>(p) == static_cast<void*>(buffer) && p->Value == 3,
        "ConstructIn should build a NoMemory in buffer even though NoMemory has its own operator new");
    p->~NoMemory();
}

// Part 2: Widget's allocation functions

template <typename Impl>
void WidgetNew() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    Widget* w = new Widget(5);
    Test::Check(w->Value == 5, "new Widget(5) should construct a Widget with value 5");
    Test::Check(Stats::Allocations == 1, "new Widget should call operator new once and count it in Allocations");
    Test::Check(Stats::BytesAllocated == sizeof(Widget), "BytesAllocated should be the size operator new was given");
    w->~Widget();
    FreeRaw(w);
}

template <typename Impl>
void WidgetDelete() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    Widget* w = new Widget(5);
    delete w;
    Test::Check(Stats::Alive == 0, "delete should destroy the Widget");
    Test::Check(Stats::Deallocations == 1, "delete should call operator delete once and count it in Deallocations");
    Test::Check(Stats::BytesFreed == sizeof(Widget), "BytesFreed should be the size operator delete was given");
}

template <typename Impl>
void WidgetDeleteNull() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    Widget::operator delete(nullptr, sizeof(Widget));
    Test::Check(Stats::Deallocations == 0, "operator delete(nullptr) should do nothing and not be counted");
}

template <typename Impl>
void WidgetNewZero() {
    using Widget = typename Impl::Widget;
    void* a = Widget::operator new(0);
    void* b = Widget::operator new(0);
    Test::Check(a != nullptr && b != nullptr, "operator new(0) should return a non-null pointer");
    Test::Check(a != b, "Two calls to operator new(0) should return different pointers");
    FreeRaw(a);
    FreeRaw(b);
}

template <typename Impl>
void WidgetNewOutOfMemory() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    bool threw = false;
    try {
        void* p = Widget::operator new(impossibleSize);
        FreeRaw(p);
    } catch (const std::bad_alloc&) {
        threw = true;
    }
    Test::Check(threw, "operator new should throw std::bad_alloc when malloc returns nullptr");
    Test::Check(Stats::Allocations == 0, "A failed allocation should not be counted");
}

template <typename Impl>
void WidgetConstructorThrows() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    bool threw = false;
    try {
        Widget* w = new Widget(-1);
        delete w;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Test::Check(threw, "new Widget(-1) should let the constructor's exception through");
    Test::Check(Stats::Allocations == 1, "The memory is allocated before the constructor runs");
    Test::Check(Stats::Deallocations == 1, "When the constructor throws, the compiler calls your operator delete");
    Test::Check(Stats::BytesFreed == Stats::BytesAllocated, "Every allocated byte should be freed");
}

template <typename Impl>
void WidgetArray() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    Widget* widgets = new Widget[3];
    Test::Check(Stats::Alive == 3, "new Widget[3] should construct 3 Widgets");
    Test::Check(Stats::Allocations == 1, "new Widget[3] should call operator new[] once");
    Test::Check(
        Stats::BytesAllocated > 3 * sizeof(Widget),
        "operator new[] should record the size it was given, which includes the array cookie");
    delete[] widgets;
    Test::Check(Stats::Alive == 0, "delete[] should destroy all 3 Widgets");
    Test::Check(Stats::Deallocations == 1, "delete[] should call operator delete[] once");
    Test::Check(Stats::BytesFreed == Stats::BytesAllocated, "operator delete[] should get the same size operator new[] got");
}

template <typename Impl>
void WidgetNothrow() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    Widget* w = new (std::nothrow) Widget(4);
    Test::Check(w != nullptr && w->Value == 4, "new (std::nothrow) Widget(4) should construct a Widget with value 4");
    Test::Check(Stats::Allocations == 1, "The given nothrow operator new should call your operator new");
    delete w;
    Test::Check(Stats::Deallocations == 1, "delete should call your operator delete");
}

template <typename Impl>
void WidgetNothrowOutOfMemory() {
    using Widget = typename Impl::Widget;
    void* p = Widget::operator new(impossibleSize, std::nothrow);
    Test::Check(
        p == nullptr,
        "The nothrow operator new should return nullptr, which needs your operator new to throw std::bad_alloc");
    FreeRaw(p);
}

template <typename Impl>
void WidgetNothrowConstructorThrows() {
    using Widget = typename Impl::Widget;
    using Stats = typename Impl::Stats;
    Stats::Reset();
    bool threw = false;
    try {
        Widget* w = new (std::nothrow) Widget(-1);
        delete w;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Test::Check(threw, "new (std::nothrow) only makes the allocation nothrow; the constructor's exception still escapes");
    Test::Check(
        Stats::Deallocations == 1,
        "When the constructor throws after a nothrow new, the nothrow operator delete frees the memory");
}

}  // namespace tests::operator_new

Test7::Test7()
    : Test(7, "operator new")
{}

void Test7::RunTests() {
    using namespace tests::operator_new;

    Run("MakeOne: new T(args)", 1, MakeOneCounted<User>, MakeOneCounted<Solution>);
    Run("MakeOne: forwards a move-only argument", 1, MakeOneForwards<User>, MakeOneForwards<Solution>);
    Run("MakeMany: new T[4]", 1, MakeManyCounted<User>, MakeManyCounted<Solution>);
    Run("MakeMany: new T[0]", 1, MakeManyZero<User>, MakeManyZero<Solution>);
    Run("TryMake: new (std::nothrow) T(args)", 1, TryMakeCounted<User>, TryMakeCounted<Solution>);
    Run("TryMake: forwards a move-only argument", 1, TryMakeForwards<User>, TryMakeForwards<Solution>);
    Run("TryMake: returns nullptr when allocation fails", 1, TryMakeOutOfMemory<User>, TryMakeOutOfMemory<Solution>);
    Run("ConstructIn: new (buffer) T(args)", 1, ConstructInCounted<User>, ConstructInCounted<Solution>);
    Run("ConstructIn: forwards a move-only argument", 1, ConstructInForwards<User>, ConstructInForwards<Solution>);
    Run(
        "ConstructIn: type with its own operator new",
        1,
        ConstructInOwnOperatorNew<User>,
        ConstructInOwnOperatorNew<Solution>);
    Run("Widget: operator new", 1, WidgetNew<User>, WidgetNew<Solution>);
    Run("Widget: operator delete", 1, WidgetDelete<User>, WidgetDelete<Solution>);
    Run("Widget: operator delete(nullptr)", 1, WidgetDeleteNull<User>, WidgetDeleteNull<Solution>);
    Run("Widget: operator new(0)", 1, WidgetNewZero<User>, WidgetNewZero<Solution>);
    Run("Widget: operator new throws std::bad_alloc", 1, WidgetNewOutOfMemory<User>, WidgetNewOutOfMemory<Solution>);
    Run("Widget: constructor throws, memory freed", 1, WidgetConstructorThrows<User>, WidgetConstructorThrows<Solution>);
    Run("Widget: operator new[] and operator delete[]", 1, WidgetArray<User>, WidgetArray<Solution>);
    Run("Widget: new (std::nothrow)", 1, WidgetNothrow<User>, WidgetNothrow<Solution>);
    Run("Widget: nothrow new returns nullptr", 1, WidgetNothrowOutOfMemory<User>, WidgetNothrowOutOfMemory<Solution>);
    Run(
        "Widget: nothrow new, constructor throws",
        1,
        WidgetNothrowConstructorThrows<User>,
        WidgetNothrowConstructorThrows<Solution>);
}
