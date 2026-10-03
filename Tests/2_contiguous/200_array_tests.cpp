// Created by Chris Manlove

#include <cstddef>
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "2_contiguous/200_array_tests.hpp"
#include "2_contiguous/200_array_challenge.hpp"
#include "2_contiguous/200_array_solution.hpp"

namespace tests::array {

template <typename T, size_t N>
struct StlArray : std::array<T, N> {
    using ValueType = T;
    using SizeType = size_t;
    using Iterator = typename std::array<T, N>::iterator;
    using ConstIterator = typename std::array<T, N>::const_iterator;

    constexpr T& At(SizeType index) { return this->at(index); }
    constexpr const T& At(SizeType index) const { return this->at(index); }
    constexpr T* Data() noexcept { return this->data(); }
    constexpr const T* Data() const noexcept { return this->data(); }
    constexpr SizeType Size() const noexcept { return this->size(); }
    constexpr void Swap(StlArray& other) noexcept(std::is_nothrow_swappable_v<T>) { this->swap(other); }
};

struct ThrowingMove {
    ThrowingMove() = default;
    ThrowingMove(ThrowingMove&&) {}

    ThrowingMove& operator=(ThrowingMove&&) {
        return *this;
    }
};

struct User {
    template <typename T, size_t N>
    using Array = challenges::array::Array<T, N>;
};

struct Solution {
    template <typename T, size_t N>
    using Array = solutions::array::Array<T, N>;
};

struct Stl {
    template <typename T, size_t N>
    using Array = StlArray<T, N>;
};

template <typename Impl, typename T, size_t N>
using ArrayOf = typename Impl::template Array<T, N>;

template <typename Impl>
void Subscript() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    const ArrayOf<Impl, int32_t, 3>& c = a;
    Test::Check(a[0] == 1 && a[1] == 2 && a[2] == 3, "a[i] should read back the values written");
    Test::Check(c[2] == 3, "a[i] on a const Array should read the element");
    Test::Check(std::is_same_v<decltype(c[0]), const int32_t&>, "a[i] on a const Array should return const int32_t&");
}

template <typename Impl>
void At() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    const ArrayOf<Impl, int32_t, 3>& c = a;
    a.At(1) = 20;
    Test::Check(a.At(0) == 1 && a.At(1) == 20 && a.At(2) == 3, "At(i) should read and write the elements");
    Test::Check(c.At(2) == 3, "At(i) on a const Array should read the element");
    Test::Check(std::is_same_v<decltype(c.At(0)), const int32_t&>, "At(i) on a const Array should return const int32_t&");

    bool threw = false;
    try {
        a.At(3);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    Test::Check(threw, "At(3) on an Array of size 3 should throw std::out_of_range");
}

template <typename Impl>
void Data() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    const ArrayOf<Impl, int32_t, 3>& c = a;
    Test::Check(a.Data() == &a[0], "Data() should point at the first element");
    Test::Check(a.Data()[2] == 3, "Data() should point at the elements, stored next to each other");
    Test::Check(std::is_same_v<decltype(c.Data()), const int32_t*>, "Data() on a const Array should return const int32_t*");
}

template <typename Impl>
void Size() {
    ArrayOf<Impl, int32_t, 3> a = {};
    ArrayOf<Impl, int32_t, 5> b = {};
    Test::Check(a.Size() == 3, "Size() of Array<int32_t, 3> should be 3");
    Test::Check(b.Size() == 5, "Size() of Array<int32_t, 5> should be 5");
}

template <typename Impl>
void Begin() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    const ArrayOf<Impl, int32_t, 3>& c = a;
    using ConstIterator = typename ArrayOf<Impl, int32_t, 3>::ConstIterator;
    Test::Check(&*a.begin() == &a[0], "begin() should point at the first element");
    Test::Check(&*c.begin() == &a[0], "begin() on a const Array should point at the first element");
    Test::Check(std::is_same_v<decltype(c.begin()), ConstIterator>, "begin() on a const Array should return ConstIterator");
}

template <typename Impl>
void End() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    const ArrayOf<Impl, int32_t, 3>& c = a;
    using ConstIterator = typename ArrayOf<Impl, int32_t, 3>::ConstIterator;
    Test::Check(&*(a.end() - 1) == &a[2], "end() should point one past the last element");
    Test::Check(a.end() - a.begin() == 3, "end() - begin() should be 3");
    Test::Check(c.end() - c.begin() == 3, "end() - begin() on a const Array should be 3");
    Test::Check(std::is_same_v<decltype(c.end()), ConstIterator>, "end() on a const Array should return ConstIterator");
}

template <typename Impl>
void Swap() {
    ArrayOf<Impl, int32_t, 3> a = {1, 2, 3};
    ArrayOf<Impl, int32_t, 3> b = {4, 5, 6};
    ArrayOf<Impl, ThrowingMove, 2> x;
    ArrayOf<Impl, ThrowingMove, 2> y;
    a.Swap(b);
    Test::Check(a[0] == 4 && a[1] == 5 && a[2] == 6, "a.Swap(b) should leave a == {4, 5, 6}");
    Test::Check(b[0] == 1 && b[1] == 2 && b[2] == 3, "a.Swap(b) should leave b == {1, 2, 3}");
    Test::Check(noexcept(a.Swap(b)), "Swap on an Array of ints should be noexcept");
    Test::Check(!noexcept(x.Swap(y)), "Swap on an Array of a type whose move can throw should not be noexcept");
}

template <typename Impl>
void UsingTheArray() {
    ArrayOf<Impl, int32_t, 3> a = {3, 1, 2};
    Test::Check(std::is_aggregate_v<ArrayOf<Impl, int32_t, 3>>, "Array must be an aggregate so brace init works");
    const char* start = reinterpret_cast<const char*>(&a);
    const char* first = reinterpret_cast<const char*>(a.Data());
    Test::Check(sizeof(a) == sizeof(int32_t[3]), "sizeof(Array<int32_t, 3>) should be sizeof(int32_t[3]), no extra storage");
    Test::Check(first >= start && first + sizeof(int32_t[3]) <= start + sizeof(a), "The elements should live inside the object");

    int32_t sum = 0;
    for (int32_t value : a) {
        sum += value;
    }
    Test::Check(sum == 6, "range-for should visit every element");

    std::sort(a.begin(), a.end());
    Test::Check(a[0] == 1 && a[1] == 2 && a[2] == 3, "std::sort(begin(), end()) should sort to {1, 2, 3}");

    ArrayOf<Impl, int32_t, 3> copy = a;
    copy[0] = 10;
    Test::Check(a[0] == 1 && copy[0] == 10, "A copy should be a separate array");
}

}  // namespace tests::array

Test200::Test200()
    : Test(200, "array")
{}

void Test200::RunTests() {
    using namespace tests::array;

    Run("operator[]", 1, Subscript<User>, Subscript<Solution>, Subscript<Stl>);
    Run("At", 1, At<User>, At<Solution>, At<Stl>);
    Run("Data", 1, Data<User>, Data<Solution>, Data<Stl>);
    Run("Size", 1, Size<User>, Size<Solution>, Size<Stl>);
    Run("begin", 1, Begin<User>, Begin<Solution>, Begin<Stl>);
    Run("end", 1, End<User>, End<Solution>, End<Stl>);
    Run("Swap", 1, Swap<User>, Swap<Solution>, Swap<Stl>);
    Run("Using the array: brace init, inline storage, range-for, std::sort, copy", 1, UsingTheArray<User>, UsingTheArray<Solution>, UsingTheArray<Stl>);
}
