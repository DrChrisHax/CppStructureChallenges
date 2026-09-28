// Created by Chris Manlove

#include <memory>
#include <type_traits>
#include <utility>

#include "0_fundamentals/5_addressof_tests.hpp"
#include "0_fundamentals/5_addressof_challenge.hpp"
#include "0_fundamentals/5_addressof_solution.hpp"

namespace tests::addressof {

struct Handle {
    Handle* operator&() { return nullptr; }
    const Handle* operator&() const { return nullptr; }

    int Value = 0;
};

struct FreeHandle {
    int Value = 0;
};

FreeHandle* operator&(FreeHandle&) {
    return nullptr;
}

struct Left {
    int L = 1;
};

struct Right {
    int R = 2;
};

struct Both : Left, Right {};

struct User {
    template <typename T>
    static decltype(challenges::addressof::AddressOf(std::declval<T>())) AddressOf(T&& value)
        noexcept(noexcept(challenges::addressof::AddressOf(std::declval<T>()))) {
        return challenges::addressof::AddressOf(std::forward<T>(value));
    }
};

struct Solution {
    template <typename T>
    static decltype(solutions::addressof::AddressOf(std::declval<T>())) AddressOf(T&& value)
        noexcept(noexcept(solutions::addressof::AddressOf(std::declval<T>()))) {
        return solutions::addressof::AddressOf(std::forward<T>(value));
    }
};

struct Stl {
    template <typename T>
    static decltype(std::addressof(std::declval<T>())) AddressOf(T&& value)
        noexcept(noexcept(std::addressof(std::declval<T>()))) {
        return std::addressof(std::forward<T>(value));
    }
};

template <typename Impl, typename T>
concept CanTakeAddress = requires(T&& value) {
    Impl::AddressOf(std::forward<T>(value));
};

template <typename Impl>
void Ints() {
    int x = 1;
    int* p = Impl::AddressOf(x);
    Test::Check(p == &x, "AddressOf(x) should equal &x");
    Test::Check(std::is_same_v<decltype(Impl::AddressOf(x)), int*>, "AddressOf(x) should return int*");
}

template <typename Impl>
void Pointer() {
    int x = 1;
    int* p = &x;
    int** pp = Impl::AddressOf(p);
    Test::Check(pp == &p, "AddressOf(p) should equal &p, the address of the pointer itself");
    Test::Check(std::is_same_v<decltype(Impl::AddressOf(p)), int**>, "AddressOf(p) should return int**");
}

template <typename Impl>
void MemberOperator() {
    Handle handles[1];
    Handle* real = handles;
    Test::Check(Impl::AddressOf(handles[0]) == real, "AddressOf(h) should be h's real address, not operator&'s nullptr");
}

template <typename Impl>
void FreeOperator() {
    FreeHandle handles[1];
    FreeHandle* real = handles;
    Test::Check(Impl::AddressOf(handles[0]) == real, "AddressOf(h) should be h's real address, not operator&'s nullptr");
}

template <typename Impl>
void Const() {
    const int c = 1;
    const int* p = Impl::AddressOf(c);
    Test::Check(p == &c, "AddressOf(c) should equal &c");
    Test::Check(std::is_same_v<decltype(Impl::AddressOf(c)), const int*>, "AddressOf(c) should return const int*");
}

template <typename Impl>
void Volatile() {
    volatile int v = 1;
    volatile int* p = Impl::AddressOf(v);
    Test::Check(p == &v, "AddressOf(v) should equal &v");
    Test::Check(
        std::is_same_v<decltype(Impl::AddressOf(v)), volatile int*>,
        "AddressOf(v) should return volatile int*");
}

template <typename Impl>
void ConstVolatile() {
    const volatile int cv = 1;
    const volatile int* p = Impl::AddressOf(cv);
    Test::Check(p == &cv, "AddressOf(cv) should equal &cv");
    Test::Check(
        std::is_same_v<decltype(Impl::AddressOf(cv)), const volatile int*>,
        "AddressOf(cv) should return const volatile int*");
}

template <typename Impl>
void ConstMemberOperator() {
    const Handle handles[1] = {};
    const Handle* real = handles;
    Test::Check(Impl::AddressOf(handles[0]) == real, "AddressOf(h) should be h's real address, not operator&'s nullptr");
    Test::Check(
        std::is_same_v<decltype(Impl::AddressOf(handles[0])), const Handle*>,
        "AddressOf(h) on a const Handle should return const Handle*");
}

template <typename Impl>
void Arrays() {
    int arr[3] = {1, 2, 3};
    int (*p)[3] = Impl::AddressOf(arr);
    Test::Check(p == &arr, "AddressOf(arr) should equal &arr");
    Test::Check(std::is_same_v<decltype(Impl::AddressOf(arr)), int(*)[3]>, "AddressOf(arr) should return int(*)[3]");
}

template <typename Impl>
void MultidimensionalArrays() {
    int grid[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int (*p)[2][3] = Impl::AddressOf(grid);
    Test::Check(p == &grid, "AddressOf(grid) should equal &grid");
    Test::Check(
        std::is_same_v<decltype(Impl::AddressOf(grid)), int(*)[2][3]>,
        "AddressOf(grid) should return int(*)[2][3]");
}

template <typename Impl>
void BaseSubobject() {
    Both both;
    Right& right = both;
    Right* p = Impl::AddressOf(right);
    Test::Check(p == static_cast<Right*>(&both), "AddressOf(right) should point at the Right part of both");
    Test::Check(p->R == 2, "AddressOf(right)->R should be 2");
}

template <typename Impl>
void RValuesRejected() {
    int x = 1;
    Impl::AddressOf(x);
    Test::Check(CanTakeAddress<Impl, int&>, "AddressOf(x) should compile for an lvalue");
    Test::Check(!CanTakeAddress<Impl, int>, "AddressOf(1) should not compile");
    Test::Check(!CanTakeAddress<Impl, const int>, "AddressOf(std::move(c)) on a const int should not compile");
}

template <typename Impl>
void AddressOfNoexcept() {
    int x = 1;
    Handle handles[1];
    Impl::AddressOf(x);
    Test::Check(noexcept(Impl::AddressOf(x)), "AddressOf(x) should be noexcept");
    Test::Check(noexcept(Impl::AddressOf(handles[0])), "AddressOf(h) should be noexcept");
}

}  // namespace tests::addressof

Test5::Test5()
    : Test(5, "addressof")
{}

void Test5::RunTests() {
    using namespace tests::addressof;

    Run("Ints: AddressOf(x)", 1, Ints<User>, Ints<Solution>, Ints<Stl>);
    Run("Pointer: AddressOf(p) on int* p", 1, Pointer<User>, Pointer<Solution>, Pointer<Stl>);
    Run("Member operator&: Handle returns nullptr", 1, MemberOperator<User>, MemberOperator<Solution>, MemberOperator<Stl>);
    Run("Free operator&: operator&(FreeHandle&) returns nullptr", 1, FreeOperator<User>, FreeOperator<Solution>, FreeOperator<Stl>);
    Run("Const: const int c", 1, Const<User>, Const<Solution>, Const<Stl>);
    Run("Volatile: volatile int v", 1, Volatile<User>, Volatile<Solution>, Volatile<Stl>);
    Run("Const volatile: const volatile int cv", 1, ConstVolatile<User>, ConstVolatile<Solution>, ConstVolatile<Stl>);
    Run(
        "Const member operator&: const Handle",
        1,
        ConstMemberOperator<User>,
        ConstMemberOperator<Solution>,
        ConstMemberOperator<Stl>);
    Run("Arrays: int arr[3]", 1, Arrays<User>, Arrays<Solution>, Arrays<Stl>);
    Run(
        "Multidimensional arrays: int grid[2][3]",
        1,
        MultidimensionalArrays<User>,
        MultidimensionalArrays<Solution>,
        MultidimensionalArrays<Stl>);
    Run("Base subobject: Right& right = both", 1, BaseSubobject<User>, BaseSubobject<Solution>, BaseSubobject<Stl>);
    Run("Rvalues rejected: AddressOf(1), AddressOf(std::move(c))", 1, RValuesRejected<User>, RValuesRejected<Solution>, RValuesRejected<Stl>);
    Run("AddressOf noexcept: int and Handle", 1, AddressOfNoexcept<User>, AddressOfNoexcept<Solution>, AddressOfNoexcept<Stl>);
}
