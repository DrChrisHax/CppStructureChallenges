// Created by Chris Manlove

#pragma once

#include <compare>
#include <cstddef>
#include <iterator>
#include <type_traits>

namespace solutions::iterators {

template <typename T, bool IsConst>
class ArrayIterator {
public:
    using iterator_concept = std::random_access_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using reference = std::conditional_t<IsConst, const T, T>&;
    using pointer = std::conditional_t<IsConst, const T, T>*;

    ArrayIterator() = default;

    ArrayIterator(pointer first, difference_type index)
        : first_(first)
        , index_(index)
    {}

    // Converting constructor, Iterator -> ConstIterator only. A template over the other iterator's IsConst so it
    // never collides with the copy constructor (see the trap in the challenge description).
    template <bool OtherIsConst>
        requires (IsConst && !OtherIsConst)
    ArrayIterator(const ArrayIterator<T, OtherIsConst>& other)
        : first_(other.first_)
        , index_(other.index_)
    {}

    reference operator*() const { return first_[index_]; }
    pointer operator->() const { return first_ + index_; }

    ArrayIterator& operator++() {
        ++index_;
        return *this;
    }

    ArrayIterator operator++(int) {
        ArrayIterator old = *this;
        ++(*this);
        return old;
    }

    friend bool operator==(const ArrayIterator& a, const ArrayIterator& b) = default;

    ArrayIterator& operator--() {
        --index_;
        return *this;
    }

    ArrayIterator operator--(int) {
        ArrayIterator old = *this;
        --(*this);
        return old;
    }

    ArrayIterator& operator+=(difference_type n) {
        index_ += n;
        return *this;
    }

    ArrayIterator& operator-=(difference_type n) {
        index_ -= n;
        return *this;
    }

    ArrayIterator operator+(difference_type n) const {
        ArrayIterator copy = *this;
        copy += n;
        return copy;
    }

    friend ArrayIterator operator+(difference_type n, const ArrayIterator& it) { return it + n; }

    ArrayIterator operator-(difference_type n) const {
        ArrayIterator copy = *this;
        copy -= n;
        return copy;
    }

    friend difference_type operator-(const ArrayIterator& a, const ArrayIterator& b) { return a.index_ - b.index_; }

    reference operator[](difference_type n) const { return *(*this + n); }

    friend auto operator<=>(const ArrayIterator& a, const ArrayIterator& b) = default;

private:
    // The const iterator reads the mutable iterator's members in its converting constructor.
    friend class ArrayIterator<T, !IsConst>;

    pointer first_ = nullptr;
    difference_type index_ = 0;
};

template <typename T, size_t N>
struct Array {
    using Iterator = ArrayIterator<T, false>;
    using ConstIterator = ArrayIterator<T, true>;

    Iterator begin() { return Iterator(Elements, 0); }
    Iterator end() { return Iterator(Elements, N); }
    ConstIterator begin() const { return ConstIterator(Elements, 0); }
    ConstIterator end() const { return ConstIterator(Elements, N); }

    T Elements[N];
};

template <std::random_access_iterator It>
constexpr void Advance(It& it, std::iter_difference_t<It> n) {
    it += n;
}

template <std::random_access_iterator It>
constexpr std::iter_difference_t<It> Distance(It first, It last) {
    return last - first;
}

}  // namespace solutions::iterators
