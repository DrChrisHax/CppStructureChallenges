// Created by Chris Manlove

#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include "1_ownership/100_ownership_primer_tests.hpp"
#include "1_ownership/100_ownership_primer_challenge.hpp"
#include "1_ownership/100_ownership_primer_solution.hpp"

namespace tests::ownership_primer {

struct User {
    using Widget = challenges::ownership_primer::Widget;
    using WidgetHolder = challenges::ownership_primer::WidgetHolder;

    static constexpr int32_t (*ValueOrZero)(int32_t) = challenges::ownership_primer::ValueOrZero;
    static constexpr int32_t (*CheckedValue)(int32_t) = challenges::ownership_primer::CheckedValue;
    static constexpr int32_t (*Doubled)(Widget*) = challenges::ownership_primer::Doubled;
    static constexpr int32_t (*MakeDoubled)(int32_t) = challenges::ownership_primer::MakeDoubled;
    static constexpr int32_t (*Largest)(const std::vector<int32_t>&) = challenges::ownership_primer::Largest;
};

struct Solution {
    using Widget = solutions::ownership_primer::Widget;
    using WidgetHolder = solutions::ownership_primer::WidgetHolder;

    static constexpr int32_t (*ValueOrZero)(int32_t) = solutions::ownership_primer::ValueOrZero;
    static constexpr int32_t (*CheckedValue)(int32_t) = solutions::ownership_primer::CheckedValue;
    static constexpr int32_t (*Doubled)(Widget*) = solutions::ownership_primer::Doubled;
    static constexpr int32_t (*MakeDoubled)(int32_t) = solutions::ownership_primer::MakeDoubled;
    static constexpr int32_t (*Largest)(const std::vector<int32_t>&) = solutions::ownership_primer::Largest;
};

template <typename Impl>
void HolderDeletesAtScopeEnd() {
    using Widget = typename Impl::Widget;
    int32_t before = Widget::Alive;
    {
        Widget* widget = new Widget(5);
        const typename Impl::WidgetHolder holder(widget);
        Test::Check(Widget::Alive == before + 1, "The holder should not delete its Widget while it is alive");
        Test::Check(holder.Get() == widget, "Get should return the pointer the holder was given");
        Test::Check(holder.Get()->Value == 5, "Get()->Value should be 5");
    }
    Test::Check(Widget::Alive == before, "The holder should delete its Widget when it goes out of scope");
}

template <typename Impl>
void HolderDeletesOnException() {
    using Widget = typename Impl::Widget;
    int32_t before = Widget::Alive;
    bool caught = false;
    try {
        typename Impl::WidgetHolder holder(new Widget(5));
        throw std::runtime_error("test");
    } catch (const std::runtime_error&) {
        caught = true;
    }
    Test::Check(caught, "The exception should reach the catch block");
    Test::Check(Widget::Alive == before, "The holder should delete its Widget when an exception leaves its scope");
}

template <typename Impl>
void HolderNotCopyable() {
    using WidgetHolder = typename Impl::WidgetHolder;
    WidgetHolder holder(new typename Impl::Widget(1));
    Test::Check(!std::is_copy_constructible_v<WidgetHolder>, "WidgetHolder should not be copy constructible");
    Test::Check(!std::is_copy_assignable_v<WidgetHolder>, "WidgetHolder should not be copy assignable");
}

template <typename Impl>
void ValueOrZeroNoLeak() {
    int32_t before = Impl::Widget::Alive;
    Test::Check(Impl::ValueOrZero(7) == 7, "ValueOrZero(7) should return 7");
    Test::Check(Impl::Widget::Alive == before, "ValueOrZero(7) leaked a Widget");
    Test::Check(Impl::ValueOrZero(-3) == 0, "ValueOrZero(-3) should return 0");
    Test::Check(Impl::Widget::Alive == before, "ValueOrZero(-3) leaked a Widget");
}

template <typename Impl>
void CheckedValueNoLeak() {
    int32_t before = Impl::Widget::Alive;
    Test::Check(Impl::CheckedValue(7) == 7, "CheckedValue(7) should return 7");
    Test::Check(Impl::Widget::Alive == before, "CheckedValue(7) leaked a Widget");

    bool caught = false;
    try {
        Impl::CheckedValue(-3);
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    Test::Check(caught, "CheckedValue(-3) should throw std::invalid_argument");
    Test::Check(Impl::Widget::Alive == before, "CheckedValue(-3) leaked a Widget when Validate threw");
}

template <typename Impl>
void DoubledOnlyObserves() {
    using Widget = typename Impl::Widget;
    int32_t before = Widget::Alive;
    Widget* widget = new Widget(4);
    Test::Check(Impl::Doubled(widget) == 8, "Doubled on a Widget with value 4 should return 8");
    Test::Check(Widget::Alive == before + 1, "Doubled should not delete a Widget it only observes");
    delete widget;
}

template <typename Impl>
void MakeDoubledNoDoubleDelete() {
    int32_t before = Impl::Widget::Alive;
    Test::Check(Impl::MakeDoubled(4) == 8, "MakeDoubled(4) should return 8");
    Test::Check(Impl::Widget::Alive == before, "MakeDoubled(4) should delete its Widget exactly once");
}

template <typename Impl>
void LargestNoDangling() {
    int32_t before = Impl::Widget::Alive;
    Test::Check(Impl::Largest({3, 9, 4}) == 9, "Largest({3, 9, 4}) should return 9");
    Test::Check(Impl::Largest({-5}) == -5, "Largest({-5}) should return -5");
    Test::Check(Impl::Widget::Alive == before, "Largest leaked a Widget");
}

}  // namespace tests::ownership_primer

Test100::Test100()
    : Test(100, "ownership primer")
{}

void Test100::RunTests() {
    using namespace tests::ownership_primer;

    Run("WidgetHolder: deletes its Widget at scope end", 1, HolderDeletesAtScopeEnd<User>, HolderDeletesAtScopeEnd<Solution>);
    Run("WidgetHolder: deletes its Widget when an exception passes", 1, HolderDeletesOnException<User>, HolderDeletesOnException<Solution>);
    Run("WidgetHolder: can't be copied", 1, HolderNotCopyable<User>, HolderNotCopyable<Solution>);
    Run("ValueOrZero: no leak on the early return", 1, ValueOrZeroNoLeak<User>, ValueOrZeroNoLeak<Solution>);
    Run("CheckedValue: no leak when Validate throws", 1, CheckedValueNoLeak<User>, CheckedValueNoLeak<Solution>);
    Run("Doubled: only observes the Widget it's given", 1, DoubledOnlyObserves<User>, DoubledOnlyObserves<Solution>);
    Run("MakeDoubled: deletes its Widget exactly once", 1, MakeDoubledNoDoubleDelete<User>, MakeDoubledNoDoubleDelete<Solution>);
    Run("Largest: no use after delete", 1, LargestNoDangling<User>, LargestNoDangling<Solution>);
}
