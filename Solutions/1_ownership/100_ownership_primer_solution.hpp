// Created by Chris Manlove

#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "test.hpp"

namespace solutions::ownership_primer {

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
    explicit WidgetHolder(Widget* widget)
        : widget_(widget)
    {}

    ~WidgetHolder() {
        delete widget_;
    }

    WidgetHolder(const WidgetHolder&) = delete;
    WidgetHolder& operator=(const WidgetHolder&) = delete;

    Widget* Get() const {
        return widget_;
    }

private:
    Widget* widget_;
};

// Part 2
// All of these functions have bugs so
// find the memory bug and fix it
// Using the WidgetHolder from Part 1 is recommended wherever a function owns a Widget.

// Returns value, or 0 if value is negative.
inline int32_t ValueOrZero(int32_t value) {
    WidgetHolder holder(new Widget(value));
    if (holder.Get()->Value < 0) {
        return 0;
    }

    return holder.Get()->Value;
}

// Returns value. Throws std::invalid_argument (from Validate) if value is negative.
inline int32_t CheckedValue(int32_t value) {
    WidgetHolder holder(new Widget(value));
    Validate(*holder.Get());
    return holder.Get()->Value;
}

// Returns twice the widget's value.
// This function should only observe the widget
inline int32_t Doubled(Widget* widget) {
    return widget->Value * 2;
}

// Makes a Widget with the given value and returns twice its value.
inline int32_t MakeDoubled(int32_t value) {
    WidgetHolder holder(new Widget(value));
    return Doubled(holder.Get());
}

// Makes one Widget per value and returns the largest value.
inline int32_t Largest(const std::vector<int32_t>& values) {
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

    int32_t largestValue = largest->Value;

    for (Widget* widget : widgets) {
        delete widget;
    }
    return largestValue;
}

}  // namespace solutions::ownership_primer
