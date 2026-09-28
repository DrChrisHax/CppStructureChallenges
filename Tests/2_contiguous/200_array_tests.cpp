// Created by Chris Manlove

#include <cstddef>

#include <algorithm>
#include <array>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "2_contiguous/200_array_tests.hpp"
#include "2_contiguous/200_array_challenge.hpp"
#include "2_contiguous/200_array_solution.hpp"

namespace tests::array {

template <typename T, std::size_t N>
struct StlArray : std::array<T, N> {
    using ValueType = T;
    using SizeType = std::size_t;
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
    template <typename T, std::size_t N>
    using Array = challenges::array::Array<T, N>;
};

struct Solution {
    template <typename T, std::size_t N>
    using Array = solutions::array::Array<T, N>;
};

struct Stl {
    template <typename T, std::size_t N>
    using Array = StlArray<T, N>;
};

template <typename Impl, typename T, std::size_t N>
using ArrayOf = typename Impl::template Array<T, N>;

template <typename Impl>
constexpr int ConstexprSum() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    a[0] = 4;
    return a[0] + a[1] + a[2] + static_cast<int>(a.Size());
}

template <int Value>
struct ConstantProbe {};

template <typename Impl>
concept WorksAtCompileTime = requires {
    typename ConstantProbe<ConstexprSum<Impl>()>;
};

template <typename Impl>
void AddOne(ArrayOf<Impl, int, 3> copy) {
    copy[0] += 1;
}

template <typename Impl>
void NoExtraStorage() {
    Test::Check(std::is_aggregate_v<ArrayOf<Impl, int, 3>>, "Array must stay an aggregate");
    Test::Check(sizeof(ArrayOf<Impl, int, 3>) == sizeof(int[3]), "sizeof(Array<int, 3>) should be sizeof(int[3])");
    Test::Check(
        sizeof(ArrayOf<Impl, std::string, 2>) == sizeof(std::string[2]),
        "sizeof(Array<std::string, 2>) should be sizeof(std::string[2])");
}

template <typename Impl>
void ElementsInsideObject() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    const char* start = reinterpret_cast<const char*>(&a);
    const char* first = reinterpret_cast<const char*>(a.Data());
    const char* last = reinterpret_cast<const char*>(a.Data() + a.Size());
    Test::Check(first >= start && last <= start + sizeof(a), "The elements should live inside the Array object itself");
}

template <typename Impl>
void Subscript() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    Test::Check(a[0] == 1 && a[1] == 2 && a[2] == 3, "a[i] should read the elements {1, 2, 3}");
    a[1] = 20;
    Test::Check(a[1] == 20, "a[1] = 20 should write the element");
}

template <typename Impl>
void ConstSubscript() {
    const ArrayOf<Impl, int, 3> a = {1, 2, 3};
    Test::Check(a[2] == 3, "a[2] on a const Array should read 3");
    Test::Check(std::is_same_v<decltype(a[0]), const int&>, "a[0] on a const Array should return const int&");
}

template <typename Impl>
void At() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    Test::Check(a.At(0) == 1 && a.At(2) == 3, "At(i) should read the elements {1, 2, 3}");
    a.At(1) = 20;
    Test::Check(a.At(1) == 20, "At(1) = 20 should write the element");

    bool threw = false;
    try {
        a.At(3);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    Test::Check(threw, "At(3) on an Array of size 3 should throw std::out_of_range");
}

template <typename Impl>
void ConstAt() {
    const ArrayOf<Impl, int, 3> a = {1, 2, 3};
    Test::Check(a.At(1) == 2, "At(1) on a const Array should read 2");
    Test::Check(std::is_same_v<decltype(a.At(0)), const int&>, "At(0) on a const Array should return const int&");

    bool threw = false;
    try {
        a.At(100);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    Test::Check(threw, "At(100) on a const Array of size 3 should throw std::out_of_range");
}

template <typename Impl>
void Data() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    int* p = a.Data();
    Test::Check(p == &a[0], "Data() should point at the first element");
    Test::Check(p[1] == 2 && p[2] == 3, "Data() should point at all three elements, stored next to each other");

    const ArrayOf<Impl, int, 3>& c = a;
    Test::Check(std::is_same_v<decltype(c.Data()), const int*>, "Data() on a const Array should return const int*");
}

template <typename Impl>
void Size() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    ArrayOf<Impl, int, 5> b = {};
    Test::Check(a.Size() == 3, "Size() of Array<int, 3> should be 3");
    Test::Check(b.Size() == 5, "Size() of Array<int, 5> should be 5");
}

template <typename Impl>
void CompileTime() {
    ConstexprSum<Impl>();
    Test::Check(
        WorksAtCompileTime<Impl>,
        "Brace init, operator[], and Size() should all work in a constant expression (mark them constexpr)");
}

template <typename Impl>
void Iterators() {
    ArrayOf<Impl, int, 3> a = {3, 1, 2};
    Test::Check(a.end() - a.begin() == 3, "end() - begin() should be 3");
    Test::Check(&*a.begin() == &a[0], "begin() should point at the first element");

    int sum = 0;
    for (int value : a) {
        sum += value;
    }
    Test::Check(sum == 6, "range-for over {3, 1, 2} should visit every element");

    std::sort(a.begin(), a.end());
    Test::Check(a[0] == 1 && a[1] == 2 && a[2] == 3, "std::sort(begin(), end()) should sort to {1, 2, 3}");
    Test::Check(
        std::contiguous_iterator<typename ArrayOf<Impl, int, 3>::Iterator>,
        "Iterator should be a contiguous iterator");
}

template <typename Impl>
void ConstIterators() {
    const ArrayOf<Impl, int, 3> a = {1, 2, 3};
    using ConstIterator = typename ArrayOf<Impl, int, 3>::ConstIterator;
    int sum = 0;
    for (int value : a) {
        sum += value;
    }
    Test::Check(sum == 6, "range-for over a const Array should visit every element");
    Test::Check(a.end() - a.begin() == 3, "end() - begin() on a const Array should be 3");
    Test::Check(std::is_same_v<decltype(a.begin()), ConstIterator>, "begin() on a const Array should return ConstIterator");
}

template <typename Impl>
void Copy() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    ArrayOf<Impl, int, 3> b = a;
    b[0] = 10;
    Test::Check(a[0] == 1, "Changing a copy should not change the original");
    Test::Check(b[0] == 10 && b[1] == 2 && b[2] == 3, "The copy should hold {10, 2, 3}");

    ArrayOf<Impl, int, 3> c = {7, 8, 9};
    c = a;
    Test::Check(c[0] == 1 && c[1] == 2 && c[2] == 3, "c = a should copy every element");
}

template <typename Impl>
void PassByValue() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    AddOne<Impl>(a);
    Test::Check(a[0] == 1, "Passing an Array by value should copy it, not decay to a pointer");
}

template <typename Impl>
void Swap() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    ArrayOf<Impl, int, 3> b = {4, 5, 6};
    a.Swap(b);
    Test::Check(a[0] == 4 && a[1] == 5 && a[2] == 6, "a.Swap(b) should leave a == {4, 5, 6}");
    Test::Check(b[0] == 1 && b[1] == 2 && b[2] == 3, "a.Swap(b) should leave b == {1, 2, 3}");
}

template <typename Impl>
void SwapNoexcept() {
    ArrayOf<Impl, int, 3> a = {1, 2, 3};
    ArrayOf<Impl, int, 3> b = {4, 5, 6};
    ArrayOf<Impl, ThrowingMove, 2> x = {};
    ArrayOf<Impl, ThrowingMove, 2> y = {};
    a.Swap(b);
    Test::Check(noexcept(a.Swap(b)), "Swap on an Array of ints should be noexcept");
    Test::Check(!noexcept(x.Swap(y)), "Swap on an Array of a type whose move can throw should not be noexcept");
}

}  // namespace tests::array

Test200::Test200()
    : Test(200, "array")
{}

void Test200::RunTests() {
    using namespace tests::array;

    Run("No extra storage: sizeof(Array<int, 3>) == sizeof(int[3])", 1, NoExtraStorage<User>, NoExtraStorage<Solution>, NoExtraStorage<Stl>);
    Run("Inline storage: the elements live inside the object", 1, ElementsInsideObject<User>, ElementsInsideObject<Solution>, ElementsInsideObject<Stl>);
    Run("operator[]: read and write", 1, Subscript<User>, Subscript<Solution>, Subscript<Stl>);
    Run("operator[]: const Array", 1, ConstSubscript<User>, ConstSubscript<Solution>, ConstSubscript<Stl>);
    Run("At: read, write, throws when out of range", 1, At<User>, At<Solution>, At<Stl>);
    Run("At: const Array", 1, ConstAt<User>, ConstAt<Solution>, ConstAt<Stl>);
    Run("Data: points at the elements", 1, Data<User>, Data<Solution>, Data<Stl>);
    Run("Size: Array<int, 3> and Array<int, 5>", 1, Size<User>, Size<Solution>, Size<Stl>);
    Run("Compile time: usable in a constant expression", 1, CompileTime<User>, CompileTime<Solution>, CompileTime<Stl>);
    Run("Iterators: begin(), end(), range-for, std::sort", 1, Iterators<User>, Iterators<Solution>, Iterators<Stl>);
    Run("Iterators: const Array", 1, ConstIterators<User>, ConstIterators<Solution>, ConstIterators<Stl>);
    Run("Value semantics: copy and assign", 1, Copy<User>, Copy<Solution>, Copy<Stl>);
    Run("Value semantics: pass by value", 1, PassByValue<User>, PassByValue<Solution>, PassByValue<Stl>);
    Run("Swap: a.Swap(b)", 1, Swap<User>, Swap<Solution>, Swap<Stl>);
    Run("Swap: noexcept for int vs ThrowingMove", 1, SwapNoexcept<User>, SwapNoexcept<Solution>, SwapNoexcept<Stl>);
}
