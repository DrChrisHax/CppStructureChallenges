// Created by Chris Manlove

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

#include "2_contiguous/201_span_tests.hpp"
#include "2_contiguous/201_span_challenge.hpp"
#include "2_contiguous/201_span_solution.hpp"

namespace tests::span {

template <typename S>
void TakeSpan(S) {}

template <typename S, typename... Args>
concept ImplicitlyConstructs = requires(Args&&... args) { TakeSpan<S>({std::forward<Args>(args)...}); };

// GCC drops std::span's explicit(...) when its constructors are inherited with using, so StlSpan forwards to
// std::span by hand and copies its explicitness.
template <typename T, std::size_t Length = std::dynamic_extent>
struct StlSpan : std::span<T, Length> {
    using Base = std::span<T, Length>;

    template <typename... Args>
        requires std::is_constructible_v<Base, Args...>
    constexpr explicit(!ImplicitlyConstructs<Base, Args...>) StlSpan(Args&&... args)
        : Base(std::forward<Args>(args)...)
    {}

    using ElementType = T;
    using ValueType = std::remove_cv_t<T>;
    using SizeType = std::size_t;
    using Iterator = typename Base::iterator;

    constexpr T* Data() const noexcept { return this->data(); }
    constexpr SizeType Size() const noexcept { return this->size(); }
    constexpr SizeType SizeBytes() const noexcept { return this->size_bytes(); }
    constexpr StlSpan<T> First(SizeType length) const { return StlSpan<T>(this->first(length)); }
    template <std::size_t N>
    constexpr StlSpan<T, N> First() const { return StlSpan<T, N>(this->template first<N>()); }
    constexpr StlSpan<T> Last(SizeType length) const { return StlSpan<T>(this->last(length)); }
    constexpr StlSpan<T> Subspan(SizeType offset, SizeType length = std::dynamic_extent) const {
        return StlSpan<T>(this->subspan(offset, length));
    }
};

template <typename T, std::size_t N>
StlSpan(T (&)[N]) -> StlSpan<T, N>;

template <typename T, std::size_t N>
StlSpan(std::array<T, N>&) -> StlSpan<T, N>;

template <typename T>
StlSpan(T*, std::size_t) -> StlSpan<T>;

// Each impl wraps its own AsBytes and AsWritableBytes, and checks deduction guides with requires expressions so a
// missing guide is a failed check instead of a compile error.
struct User {
    template <typename T, std::size_t Length = challenges::span::DynamicLength>
    using Span = challenges::span::Span<T, Length>;

    template <typename T, std::size_t Length>
    static auto AsBytes(Span<T, Length> s) { return challenges::span::AsBytes(s); }

    template <typename T, std::size_t Length>
    static auto AsWritableBytes(Span<T, Length> s) { return challenges::span::AsWritableBytes(s); }

    template <typename S>
    static constexpr bool CanAsWritableBytes = requires(S s) { challenges::span::AsWritableBytes(s); };

    template <typename Arg, typename Expected>
    static constexpr bool DeducesFrom = requires(Arg& arg) {
        { challenges::span::Span{arg} } -> std::same_as<Expected>;
    };

    template <typename Pointer, typename Expected>
    static constexpr bool DeducesFromPointerAndLength = requires(Pointer p, std::size_t n) {
        { challenges::span::Span(p, n) } -> std::same_as<Expected>;
    };
};

struct Solution {
    template <typename T, std::size_t Length = solutions::span::DynamicLength>
    using Span = solutions::span::Span<T, Length>;

    template <typename T, std::size_t Length>
    static auto AsBytes(Span<T, Length> s) { return solutions::span::AsBytes(s); }

    template <typename T, std::size_t Length>
    static auto AsWritableBytes(Span<T, Length> s) { return solutions::span::AsWritableBytes(s); }

    template <typename S>
    static constexpr bool CanAsWritableBytes = requires(S s) { solutions::span::AsWritableBytes(s); };

    template <typename Arg, typename Expected>
    static constexpr bool DeducesFrom = requires(Arg& arg) {
        { solutions::span::Span{arg} } -> std::same_as<Expected>;
    };

    template <typename Pointer, typename Expected>
    static constexpr bool DeducesFromPointerAndLength = requires(Pointer p, std::size_t n) {
        { solutions::span::Span(p, n) } -> std::same_as<Expected>;
    };
};

struct Stl {
    template <typename T, std::size_t Length = std::dynamic_extent>
    using Span = StlSpan<T, Length>;

    template <typename T, std::size_t Length>
    static auto AsBytes(Span<T, Length> s) {
        auto bytes = std::as_bytes(std::span<T, Length>(s));
        return StlSpan<const std::byte, decltype(bytes)::extent>(bytes);
    }

    template <typename T, std::size_t Length>
    static auto AsWritableBytes(Span<T, Length> s) {
        auto bytes = std::as_writable_bytes(std::span<T, Length>(s));
        return StlSpan<std::byte, decltype(bytes)::extent>(bytes);
    }

    template <typename S>
    static constexpr bool CanAsWritableBytes = requires(S s) { std::as_writable_bytes(s); };

    template <typename Arg, typename Expected>
    static constexpr bool DeducesFrom = requires(Arg& arg) {
        { StlSpan{arg} } -> std::same_as<Expected>;
    };

    template <typename Pointer, typename Expected>
    static constexpr bool DeducesFromPointerAndLength = requires(Pointer p, std::size_t n) {
        { StlSpan(p, n) } -> std::same_as<Expected>;
    };
};

template <typename Impl, typename T, std::size_t Length = std::dynamic_extent>
using SpanOf = typename Impl::template Span<T, Length>;

template <typename S>
concept ImplicitFromPointerAndLength = requires(int* p, std::size_t n) { TakeSpan<S>({p, n}); };

template <typename Impl>
void PointerAndLengthConstructor() {
    int a[4] = {1, 2, 3, 4};
    SpanOf<Impl, int> s(a, 3);
    SpanOf<Impl, int, 4> f(a, 4);
    Test::Check(s.Data() == a, "Span(ptr, 3) should point at ptr");
    Test::Check(s.Size() == 3, "Span(ptr, 3) should have Size() 3");
    Test::Check(s[0] == 1 && s[2] == 3, "Span(ptr, 3) should view the first 3 elements");
    Test::Check(f.Data() == a && f.Size() == 4, "Span<int, 4>(ptr, 4) should point at ptr and have Size() 4");
}

template <typename Impl>
void CArrayConstructor() {
    int a[3] = {1, 2, 3};
    SpanOf<Impl, int> s(a);
    SpanOf<Impl, int, 3> f(a);
    Test::Check(s.Data() == a && s.Size() == 3, "Span<int> from int[3] should view all 3 elements");
    Test::Check(f.Data() == a && f.Size() == 3, "Span<int, 3> from int[3] should view all 3 elements");
    Test::Check(!std::is_constructible_v<SpanOf<Impl, int, 4>, int (&)[3]>, "Span<int, 4> from int[3] should not compile");
    Test::Check(std::is_constructible_v<SpanOf<Impl, const int>, int (&)[3]>, "Span<const int> from int[3] should compile");
}

template <typename Impl>
void StdArrayConstructor() {
    std::array<int, 3> a = {1, 2, 3};
    SpanOf<Impl, int> s(a);
    SpanOf<Impl, int, 3> f(a);
    Test::Check(s.Data() == a.data() && s.Size() == 3, "Span<int> from std::array<int, 3> should view all 3 elements");
    Test::Check(f.Data() == a.data() && f.Size() == 3, "Span<int, 3> from std::array<int, 3> should view all 3 elements");
    Test::Check(
        !std::is_constructible_v<SpanOf<Impl, int, 4>, std::array<int, 3>&>,
        "Span<int, 4> from std::array<int, 3> should not compile");
    Test::Check(
        std::is_constructible_v<SpanOf<Impl, const int>, std::array<int, 3>&>,
        "Span<const int> from std::array<int, 3> should compile");
}

template <typename Impl>
void ConvertingConstructor() {
    int a[4] = {1, 2, 3, 4};
    SpanOf<Impl, int, 4> f(a);
    SpanOf<Impl, int> s(a, 4);
    SpanOf<Impl, const int> fromStatic(f);
    SpanOf<Impl, const int> fromDynamic(s);
    Test::Check(fromStatic.Data() == a && fromStatic.Size() == 4, "Span<const int> from Span<int, 4> should view the same 4 elements");
    Test::Check(fromDynamic.Data() == a && fromDynamic.Size() == 4, "Span<const int> from Span<int> should view the same 4 elements");
    Test::Check(
        std::is_constructible_v<SpanOf<Impl, const int>, SpanOf<Impl, int>>,
        "Span<int> to Span<const int> should compile");
    Test::Check(
        !std::is_constructible_v<SpanOf<Impl, int>, SpanOf<Impl, const int>>,
        "Span<const int> to Span<int> should not compile");
    Test::Check(
        std::is_constructible_v<SpanOf<Impl, int>, SpanOf<Impl, int, 4>>,
        "Span<int, 4> to Span<int> should compile");
    Test::Check(
        !std::is_constructible_v<SpanOf<Impl, int, 3>, SpanOf<Impl, int, 4>>,
        "Span<int, 4> to Span<int, 3> should not compile");
}

template <typename Impl>
void Explicit() {
    Test::Check(
        ImplicitFromPointerAndLength<SpanOf<Impl, int>>,
        "Span<int> from (ptr, n) should not be explicit");
    Test::Check(
        !ImplicitFromPointerAndLength<SpanOf<Impl, int, 4>>,
        "Span<int, 4> from (ptr, n) should be explicit");
    Test::Check(
        std::is_constructible_v<SpanOf<Impl, int, 4>, SpanOf<Impl, int>>,
        "Span<int, 4>(someDynamicSpan) should compile");
    Test::Check(
        !std::is_convertible_v<SpanOf<Impl, int>, SpanOf<Impl, int, 4>>,
        "Span<int, 4> s = someDynamicSpan; should not compile (explicit)");
    Test::Check(
        std::is_convertible_v<SpanOf<Impl, int, 4>, SpanOf<Impl, int>>,
        "Span<int> s = someSpanOf4; should compile (not explicit)");
    Test::Check(std::is_convertible_v<int (&)[4], SpanOf<Impl, int, 4>>, "Span<int, 4> s = intArray4; should compile (not explicit)");
    Test::Check(
        std::is_convertible_v<std::array<int, 4>&, SpanOf<Impl, int, 4>>,
        "Span<int, 4> s = stdArray4; should compile (not explicit)");
}

template <typename Impl>
void Subscript() {
    int a[3] = {1, 2, 3};
    SpanOf<Impl, int> s(a);
    const SpanOf<Impl, int> c(a);
    Test::Check(s[0] == 1 && s[1] == 2 && s[2] == 3, "s[i] should read the viewed elements");
    s[1] = 20;
    Test::Check(a[1] == 20, "Writing s[i] should change the original array");
    c[2] = 30;
    Test::Check(a[2] == 30, "Writing through a const Span<int> should change the original array (shallow const)");
    Test::Check(std::is_same_v<decltype(c[0]), int&>, "s[i] on a const Span<int> should return int&");

    SpanOf<Impl, int> copy = s;
    copy[0] = 10;
    Test::Check(a[0] == 10 && s[0] == 10, "A copy of a span should view the same elements");
}

template <typename Impl>
void DataSizeSizeBytes() {
    int a[4] = {1, 2, 3, 4};
    SpanOf<Impl, int> s(a, 3);
    SpanOf<Impl, int, 4> f(a);
    Test::Check(s.Data() == a && f.Data() == a, "Data() should point at the first element");
    Test::Check(s.Size() == 3 && f.Size() == 4, "Size() should be 3 for Span<int>(a, 3) and 4 for Span<int, 4>");
    Test::Check(s.SizeBytes() == 3 * sizeof(int), "SizeBytes() of Span<int>(a, 3) should be 3 * sizeof(int)");
    Test::Check(f.SizeBytes() == 4 * sizeof(int), "SizeBytes() of Span<int, 4> should be 4 * sizeof(int)");
}

template <typename Impl>
void BeginEnd() {
    int a[3] = {3, 1, 2};
    SpanOf<Impl, int> s(a);
    const SpanOf<Impl, int, 3> f(a);
    Test::Check(&*s.begin() == a, "begin() should point at the first element");
    Test::Check(s.end() - s.begin() == 3, "end() - begin() should be Size()");
    Test::Check(f.end() - f.begin() == 3, "end() - begin() on a const Span<int, 3> should be 3");

    int sum = 0;
    for (int value : s) {
        sum += value;
    }
    Test::Check(sum == 6, "range-for should visit every element");

    std::sort(f.begin(), f.end());
    Test::Check(a[0] == 1 && a[1] == 2 && a[2] == 3, "std::sort(begin(), end()) on a const span should sort the original array");
}

template <typename Impl>
void FirstLastSubspan() {
    int a[5] = {1, 2, 3, 4, 5};
    SpanOf<Impl, int, 5> s(a);
    auto first = s.First(2);
    auto last = s.Last(2);
    auto middle = s.Subspan(1, 3);
    auto rest = s.Subspan(2);
    Test::Check(std::is_same_v<decltype(first), SpanOf<Impl, int>>, "First(length) should return a dynamic Span<int>");
    Test::Check(first.Data() == a && first.Size() == 2, "First(2) should view {1, 2} in place");
    Test::Check(last.Data() == a + 3 && last.Size() == 2, "Last(2) should view {4, 5} in place");
    Test::Check(middle.Data() == a + 1 && middle.Size() == 3, "Subspan(1, 3) should view {2, 3, 4} in place");
    Test::Check(rest.Data() == a + 2 && rest.Size() == 3, "Subspan(2) should view everything from index 2 to the end");
    first[0] = 10;
    Test::Check(a[0] == 10, "Writing through a subview should change the original array");
}

template <typename Impl>
void FirstN() {
    int a[5] = {1, 2, 3, 4, 5};
    SpanOf<Impl, int> s(a, 5);
    auto first = s.template First<2>();
    Test::Check(std::is_same_v<decltype(first), SpanOf<Impl, int, 2>>, "First<2>() should return Span<int, 2>");
    Test::Check(first.Data() == a && first.Size() == 2, "First<2>() should view {1, 2} in place");
}

template <typename Impl>
void StaticLengthIsNotStored() {
    int a[4] = {1, 2, 3, 4};
    char c[1] = {'x'};
    SpanOf<Impl, int, 4> f(a);
    SpanOf<Impl, char, 1> one(c);
    SpanOf<Impl, int> s(a, 3);
    Test::Check(sizeof(SpanOf<Impl, int, 4>) == sizeof(int*), "sizeof(Span<int, 4>) should be sizeof(int*), no stored size");
    Test::Check(sizeof(SpanOf<Impl, char, 1>) == sizeof(char*), "sizeof(Span<char, 1>) should be sizeof(char*), no stored size");
    Test::Check(
        sizeof(SpanOf<Impl, int>) == sizeof(int*) + sizeof(std::size_t),
        "sizeof(Span<int>) should be sizeof(int*) + sizeof(std::size_t)");
    Test::Check(f.Size() == 4, "Size() of a static Span<int, 4> should be 4");
    Test::Check(one.Size() == 1, "Size() of a static Span<char, 1> should be 1");
    Test::Check(s.Size() == 3, "Size() of a dynamic Span<int>(a, 3) should be 3");
}

template <typename Impl>
void DeductionGuides() {
    Test::Check(
        Impl::template DeducesFrom<int[3], SpanOf<Impl, int, 3>>,
        "Span s = intArray3; should deduce Span<int, 3>");
    Test::Check(
        Impl::template DeducesFrom<std::array<int, 3>, SpanOf<Impl, int, 3>>,
        "Span s = stdArray3; should deduce Span<int, 3>");
    Test::Check(
        Impl::template DeducesFromPointerAndLength<int*, SpanOf<Impl, int>>,
        "Span s(ptr, n); should deduce Span<int>");
}

template <typename Impl>
void AsBytesAsWritableBytes() {
    int a[2] = {1, 2};
    SpanOf<Impl, int, 2> f(a);
    SpanOf<Impl, int> s(a, 2);
    auto bytes = Impl::AsBytes(f);
    auto dynamicBytes = Impl::AsBytes(s);
    Test::Check(
        std::is_same_v<decltype(bytes), SpanOf<Impl, const std::byte, 2 * sizeof(int)>>,
        "AsBytes(Span<int, 2>) should return Span<const std::byte, 2 * sizeof(int)>");
    Test::Check(
        std::is_same_v<decltype(dynamicBytes), SpanOf<Impl, const std::byte>>,
        "AsBytes(Span<int>) should return a dynamic Span<const std::byte>");
    Test::Check(
        bytes.Data() == reinterpret_cast<const std::byte*>(a) && bytes.Size() == sizeof(a),
        "AsBytes should view the same memory, sizeof(a) bytes long");
    Test::Check(dynamicBytes.Size() == sizeof(a), "AsBytes on a dynamic span should be sizeof(a) bytes long");

    auto writable = Impl::AsWritableBytes(f);
    Test::Check(
        std::is_same_v<decltype(writable), SpanOf<Impl, std::byte, 2 * sizeof(int)>>,
        "AsWritableBytes(Span<int, 2>) should return Span<std::byte, 2 * sizeof(int)>");
    for (std::size_t i = 0; i < sizeof(int); ++i) {
        writable[i] = std::byte{0};
    }
    Test::Check(a[0] == 0 && a[1] == 2, "Zeroing the first int's bytes through AsWritableBytes should set a[0] to 0");
    Test::Check(Impl::template CanAsWritableBytes<SpanOf<Impl, int>>, "AsWritableBytes(Span<int>) should compile");
    Test::Check(
        !Impl::template CanAsWritableBytes<SpanOf<Impl, const int>>,
        "AsWritableBytes(Span<const int>) should not compile");
}

}  // namespace tests::span

Test201::Test201()
    : Test(201, "span")
{}

void Test201::RunTests() {
    using namespace tests::span;

    Run("Pointer and length constructor", 1, PointerAndLengthConstructor<User>, PointerAndLengthConstructor<Solution>, PointerAndLengthConstructor<Stl>);
    Run("C array constructor", 1, CArrayConstructor<User>, CArrayConstructor<Solution>, CArrayConstructor<Stl>);
    Run("std::array constructor", 1, StdArrayConstructor<User>, StdArrayConstructor<Solution>, StdArrayConstructor<Stl>);
    Run("Converting constructor", 1, ConvertingConstructor<User>, ConvertingConstructor<Solution>, ConvertingConstructor<Stl>);
    Run("explicit", 1, Explicit<User>, Explicit<Solution>, Explicit<Stl>);
    Run("operator[]", 1, Subscript<User>, Subscript<Solution>, Subscript<Stl>);
    Run("Data, Size, SizeBytes", 1, DataSizeSizeBytes<User>, DataSizeSizeBytes<Solution>, DataSizeSizeBytes<Stl>);
    Run("begin and end", 1, BeginEnd<User>, BeginEnd<Solution>, BeginEnd<Stl>);
    Run("First, Last, Subspan", 1, FirstLastSubspan<User>, FirstLastSubspan<Solution>, FirstLastSubspan<Stl>);
    Run("First<N>", 1, FirstN<User>, FirstN<Solution>, FirstN<Stl>);
    Run("Static length is not stored", 1, StaticLengthIsNotStored<User>, StaticLengthIsNotStored<Solution>, StaticLengthIsNotStored<Stl>);
    Run("Deduction guides", 1, DeductionGuides<User>, DeductionGuides<Solution>, DeductionGuides<Stl>);
    Run("AsBytes and AsWritableBytes", 1, AsBytesAsWritableBytes<User>, AsBytesAsWritableBytes<Solution>, AsBytesAsWritableBytes<Stl>);
}
