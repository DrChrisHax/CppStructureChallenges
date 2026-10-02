// REQUIRED
//
// Challenge 201: std::span
//
// std::span is a pointer and a length. It points at elements that live somewhere else (a C array, a std::array, a
// std::vector, etc.) and never owns, allocates, or frees them. Copying a span copies the pointer and the length, not
// the elements. If the elements are destroyed, the span dangles.
//
// The interesting part is how the span knows its length. There are two kinds. (std::span calls this the extent, and
// DynamicLength is std::dynamic_extent.)
// - Static length: Span<int, 4>. The length is part of the type, like std::array<int, 4>. It's known at compile time,
//   so it isn't stored anywhere: sizeof(Span<int, 4>) is exactly sizeof(int*).
// - Dynamic length: Span<int>, which means Span<int, DynamicLength>. The length is only known at runtime, so the span
//   has to store it next to the pointer: sizeof(Span<int>) is two words.
// Size() works on both. Your job is to make the length cost zero bytes when it's static. Decide how to store it: one
// way is a small helper struct whose primary template holds nothing and just returns Length, with one specialization
// for DynamicLength that holds the length. An empty member still takes at least one byte, which padding turns into a
// whole word, so mark it [[no_unique_address]] (C++20) to let it take none. Put your members where the stub says. The
// tests check sizeof.
//
// DynamicLength is the largest std::size_t, used as a "no fixed length" marker. No real span can be that long, so the
// value is free to mean something else.
//
// Constness is shallow. A span is like a pointer: a const Span<int> can't be pointed somewhere else, but it can still
// write the ints it points at. std::array has a const and a non-const version of operator[], begin, and end. Span
// doesn't: every member function is const and still returns T& and T*. To get a read-only view, use Span<const int>.
//
// What to implement:
// - Constructors:
//   - (pointer, length): views length elements starting at the pointer.
//   - From a C array: views the whole array. The array's size N becomes the length. The requires clause only allows
//     this when the span is dynamic or Length == N, so Span<int, 4> can't view an int[3].
//   - From a std::array: the same idea, using data() and size().
//   - From another Span: the converting constructor. Span<int> becomes Span<const int> (adding const is safe, and
//     the requires clause allows it), and Span<int, 4> becomes Span<int> (forgetting the length is safe). The other
//     direction, Span<const int> to Span<int>, must not compile: it would let you write through a read-only view.
// - explicit: you write it, the stub leaves it off. std::span makes a constructor explicit when it can't check the
//   length at compile time and the result has a static length. Building a Span<int, 4> from (pointer, length) or from
//   a dynamic Span<int> just trusts that the length really is 4, so you have to ask for it by name:
//   Span<int, 4>(ptr, 4) is fine, Span<int, 4> s = someDynamicSpan; should not compile. Use explicit(condition),
//   a C++20 feature that turns explicit on or off with a compile-time bool. The C array and std::array constructors
//   are never explicit: their length is checked by the requires clause. The tests check which conversions compile.
// - operator[]: no bounds check, same as std::array.
// - Data, Size, SizeBytes: SizeBytes is Size() * sizeof(T), the number of bytes viewed.
// - begin and end: T* again, because the elements are contiguous. Only const versions (see constness above).
// - First(length), Last(length), Subspan(offset, length): smaller views of the same elements. No copying, just a new
//   pointer and length. They return a dynamic span because length is only known at runtime. Subspan's length defaults
//   to DynamicLength, meaning "everything from offset to the end".
// - First<N>(): the same as First, but N is a template argument, so the result has a static length:
//   Span<T, N>. This is how a dynamic view turns into a static one.
// - AsBytes and AsWritableBytes (free functions): view the same memory as bytes. A Span<int, 4> becomes a
//   Span<const std::byte, 16> (on a 4-byte int). A dynamic span stays dynamic. Reading any object's bytes through
//   std::byte is allowed by the strict aliasing rules. AsWritableBytes isn't allowed for Span<const T>.
// - Deduction guides: write them where the stub says, after the struct. They tell the compiler what template
//   arguments to pick when you write Span s = something; with no <...>. The compiler makes some guides for you from
//   the constructors: Span(T*, SizeType) already deduces Span<T> from (pointer, length). It can't for the C array and
//   std::array constructors, so given int arr[3], the line Span s = arr; doesn't compile until you write guides that
//   turn an int[3] or a std::array<int, 3> into Span<int, 3>.
//
// Left out on purpose: front, back, empty, at, reverse and c iterators, Last<N>() and Subspan<Offset, N>()
// (same idea as First<N>()), the (first, last) iterator-pair constructor, the generic contiguous-range
// constructor (it needs the contiguous-range concepts), a const std::array constructor, and initializer_list (C++26).
// A span has no comparison operators and no member swap.
//
// Resources:
// - https://en.cppreference.com/w/cpp/container/span
// - https://en.cppreference.com/w/cpp/container/span/span  (the table of which constructors are explicit)
// - https://en.cppreference.com/w/cpp/language/explicit
// - https://en.cppreference.com/w/cpp/language/class_template_argument_deduction
// - https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/include/std/span  (the real thing)

#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "test.hpp"

namespace challenges::span {

inline constexpr std::size_t DynamicLength = static_cast<std::size_t>(-1);

// The length of a Span<T, Length> viewed as bytes, for AsBytes and AsWritableBytes. A dynamic length stays dynamic.
template <typename T, std::size_t Length>
inline constexpr std::size_t BytesLength = Length == DynamicLength ? DynamicLength : Length * sizeof(T);

// Your length storage helper (if you use one) goes here.

template <typename T, std::size_t Length = DynamicLength>
struct Span {
    using ElementType = T;
    using ValueType = std::remove_cv_t<T>;
    using SizeType = std::size_t;
    using Iterator = T*;

    constexpr Span(T* ptr, SizeType length)
    {
        // Your code here.
        Test::Todo();
    }

    template <std::size_t N>
        requires(Length == DynamicLength || Length == N)
    constexpr Span(std::type_identity_t<T> (&arr)[N]) noexcept
    {
        // Your code here.
        Test::Todo();
    }

    template <typename U, std::size_t N>
        requires(Length == DynamicLength || Length == N) && std::is_convertible_v<U (*)[], T (*)[]>
    constexpr Span(std::array<U, N>& arr) noexcept
    {
        // Your code here.
        Test::Todo();
    }

    template <typename U, std::size_t OtherLength>
        requires(Length == DynamicLength || OtherLength == DynamicLength || Length == OtherLength) &&
                std::is_convertible_v<U (*)[], T (*)[]>
    constexpr Span(const Span<U, OtherLength>& other) noexcept
    {
        // Your code here.
        Test::Todo();
    }

    constexpr T& operator[](SizeType index) const {
        // Your code here.
        Test::Todo();
    }

    constexpr T* Data() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr SizeType Size() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr SizeType SizeBytes() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator begin() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Iterator end() const noexcept {
        // Your code here.
        Test::Todo();
    }

    constexpr Span<T> First(SizeType length) const {
        // Your code here.
        Test::Todo();
    }

    template <std::size_t N>
    constexpr Span<T, N> First() const {
        // Your code here.
        Test::Todo();
    }

    constexpr Span<T> Last(SizeType length) const {
        // Your code here.
        Test::Todo();
    }

    constexpr Span<T> Subspan(SizeType offset, SizeType length = DynamicLength) const {
        // Your code here.
        Test::Todo();
    }

    // Your members go here: the pointer, and the length only when Length is DynamicLength.
};

// Your deduction guides go here: one for a C array, one for a std::array.

template <typename T, std::size_t Length>
Span<const std::byte, BytesLength<T, Length>> AsBytes(Span<T, Length> s) noexcept {
    // Your code here.
    Test::Todo();
}

template <typename T, std::size_t Length>
    requires(!std::is_const_v<T>)
Span<std::byte, BytesLength<T, Length>> AsWritableBytes(Span<T, Length> s) noexcept {
    // Your code here.
    Test::Todo();
}

}  // namespace challenges::span
