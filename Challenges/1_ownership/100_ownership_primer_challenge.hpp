// REQUIRED
//
// Challenge 100: Ownership primer
//
// Every object made with new has to be deleted exactly once. Not zero times (a leak), not twice (a double delete),
// and nobody may use it after it's deleted (a dangling pointer). A raw pointer can't help you with any of that. A
// Widget* looks the same whether it's the one responsible for deleting the Widget or something that only looks at it.
//
// Two words used for the rest of this section:
// - Owner: the one responsible for deleting the object. There is exactly one.
// - Observer: anything that only looks at the object. It never deletes it, and it must not be used after the owner
//   has deleted it.
// Every bug below comes from mixing those two up, or from an owner that forgets its job. A common convention, and the
// one this project follows: a function that takes a raw pointer is an observer. It never deletes what it was given.
//
// RAII (Resource Acquisition Is Initialization) is the fix. Put the owning pointer inside an object whose destructor
// does the delete. C++ always runs a local object's destructor when its scope ends, whether by reaching the closing
// brace, an early return, or an exception passing through. So the delete can't be forgotten. std::vector,
// std::string, and std::unique_ptr all work this way.
//
// Part 1: WidgetHolder
// Write a small RAII owner for one Widget. The constructor takes a pointer from new and becomes its owner. The
// destructor deletes it. Get returns the raw pointer so others can observe the Widget. Then think about what copying
// a WidgetHolder would do: two holders with the same pointer, and both would delete it. Make copying (both the copy
// constructor and copy assignment) refuse to compile. The tests check this. Moving isn't needed yet. We will add it
// later when we implement unique_ptr.
//
// Part 2: Fix the program
// Below WidgetHolder are four functions. Each has one ownership bug: a leak, a leak when an exception is thrown, a
// double delete, or a dangling pointer. Fix each one so it does what its comment says. Keep the signatures. For some,
// WidgetHolder is the fix. For others, the fix is deciding who owns the Widget and making everything else an observer.
// Widget counts how many are alive, and the tests use that count to catch leaks.
// Each function starts with Test::Todo() so its test shows TODO instead of FAIL until you start. Delete that line
// when you start fixing the function.
//
// Finding the bugs with AddressSanitizer (ASan): a double delete or a use after delete is undefined behavior. It
// might crash, or it might look fine and return the right number by luck. ASan adds checks to every memory access so
// these bugs always stop the program, with a report naming the bug (attempting double-free, heap-use-after-free)
// and the line it happened on. Read the report from the top. The first stack frame in this file is your bug. Run
// ./run_with_ASan.sh instead of ./run.sh to turn it on. It's slower, so ./run.sh stays without it. ASan's leak
// checker doesn't run here (the test runner skips the normal program exit), which is why Widget counts itself instead.
//
// Resources:
// - https://www.learncpp.com/cpp-tutorial/dynamic-memory-allocation-with-new-and-delete/
// - https://www.learncpp.com/cpp-tutorial/introduction-to-smart-pointers-move-semantics/
// - https://en.cppreference.com/w/cpp/language/raii
// - https://en.cppreference.com/w/cpp/language/rule_of_three
// - https://github.com/google/sanitizers/wiki/AddressSanitizer

#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "test.hpp"

namespace challenges::ownership_primer {

// ---------------------------------------------------------------------------------------------------------------------
// DO NOT CHANGE THIS SECTION
// ---------------------------------------------------------------------------------------------------------------------

// Alive is how many Widgets currently exist.
struct Widget {
    explicit Widget(int32_t value)
        : Value(value)
    {
        ++Alive;
    }

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    ~Widget() {
        --Alive;
    }

    inline static int32_t Alive = 0;

    int32_t Value = 0;
};

// Throws std::invalid_argument if the widget's value is negative.
inline void Validate(const Widget& widget) {
    if (widget.Value < 0) {
        throw std::invalid_argument("negative widget value");
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// MAKE YOUR CHANGES BELOW
// ---------------------------------------------------------------------------------------------------------------------

// Part 1
class WidgetHolder {
public:
    // This pointer was created with the new operator
    explicit WidgetHolder(Widget* widget) {
        // Your code here.
        Test::Todo();
    }

    ~WidgetHolder() {
        // Your code here.
        Test::Todo();
    }

    Widget* Get() const {
        // Your code here.
        Test::Todo();
    }
};

// Part 2
// All of these functions have bugs so
// find the memory bug and fix it
// Using the WidgetHolder from Part 1 is recommended wherever a function owns a Widget.

// Returns value, or 0 if value is negative.
inline int32_t ValueOrZero(int32_t value) {
    Test::Todo();  // Delete this line when you start fixing this function.

    Widget* widget = new Widget(value);
    if (widget->Value < 0) {
        return 0;
    }

    int32_t result = widget->Value;
    delete widget;
    return result;
}

// Returns value. Throws std::invalid_argument (from Validate) if value is negative.
inline int32_t CheckedValue(int32_t value) {
    Test::Todo();  // Delete this line when you start fixing this function.

    Widget* widget = new Widget(value);
    Validate(*widget);

    int32_t result = widget->Value;
    delete widget;
    return result;
}

// Returns twice the widget's value.
// This function should only observe the widget
inline int32_t Doubled(Widget* widget) {
    Test::Todo();  // Delete this line when you start fixing this function.

    int32_t result = widget->Value * 2;
    delete widget;
    return result;
}

// Makes a Widget with the given value and returns twice its value.
inline int32_t MakeDoubled(int32_t value) {
    Test::Todo();  // Delete this line when you start fixing this function.

    Widget* widget = new Widget(value);
    int32_t result = Doubled(widget);
    delete widget;
    return result;
}

// Makes one Widget per value and returns the largest value.
inline int32_t Largest(const std::vector<int32_t>& values) {
    Test::Todo();  // Delete this line when you start fixing this function.

    if (values.empty()) { return 0; }

    std::vector<Widget*> widgets;
    for (int32_t value : values) {
        widgets.push_back(new Widget(value));
    }

    Widget* largest = widgets[0];
    for (Widget* widget : widgets) {
        if (widget->Value > largest->Value) {
            largest = widget;
        }
    }

    for (Widget* widget : widgets) {
        delete widget;
    }
    return largest->Value;
}

}  // namespace challenges::ownership_primer
