// Created by Chris Manlove

#include <cstddef>
#include <utility>

#include "0_fundamentals/4_swap_tests.hpp"
#include "0_fundamentals/4_swap_challenge.hpp"
#include "0_fundamentals/4_swap_solution.hpp"

namespace tests::swap {

struct Counter {
    Counter(int value)
        : Value(value)
    {}

    Counter(const Counter& other)
        : Value(other.Value)
    {
        ++Copies;
    }

    Counter(Counter&& other) noexcept = default;

    Counter& operator=(const Counter& other) {
        Value = other.Value;
        ++Copies;
        return *this;
    }

    Counter& operator=(Counter&& other) noexcept = default;

    inline static int Copies = 0;
    int Value = 0;
};

struct MoveOnly {
    MoveOnly(int value)
        : Value(value)
    {}

    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;

    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&) = default;

    int Value = 0;
};

struct ThrowingMove {
    ThrowingMove() = default;
    ThrowingMove(ThrowingMove&&) {}

    ThrowingMove& operator=(ThrowingMove&&) {
        return *this;
    }
};

struct User {
    template <typename T>
    static void Swap(T& a, T& b) noexcept(noexcept(challenges::swap::Swap(a, b))) {
        challenges::swap::Swap(a, b);
    }

    template <typename T, std::size_t N>
    static void Swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(challenges::swap::Swap(a, b))) {
        challenges::swap::Swap(a, b);
    }

    template <typename T, typename U>
    static T Exchange(T& obj, U&& newValue)
        noexcept(noexcept(challenges::swap::Exchange(obj, std::forward<U>(newValue)))) {
        return challenges::swap::Exchange(obj, std::forward<U>(newValue));
    }
};

struct Solution {
    template <typename T>
    static void Swap(T& a, T& b) noexcept(noexcept(solutions::swap::Swap(a, b))) {
        solutions::swap::Swap(a, b);
    }

    template <typename T, std::size_t N>
    static void Swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(solutions::swap::Swap(a, b))) {
        solutions::swap::Swap(a, b);
    }

    template <typename T, typename U>
    static T Exchange(T& obj, U&& newValue)
        noexcept(noexcept(solutions::swap::Exchange(obj, std::forward<U>(newValue)))) {
        return solutions::swap::Exchange(obj, std::forward<U>(newValue));
    }
};

struct Stl {
    template <typename T>
    static void Swap(T& a, T& b) noexcept(noexcept(std::swap(a, b))) {
        std::swap(a, b);
    }

    template <typename T, std::size_t N>
    static void Swap(T (&a)[N], T (&b)[N]) noexcept(noexcept(std::swap(a, b))) {
        std::swap(a, b);
    }

    template <typename T, typename U>
    static T Exchange(T& obj, U&& newValue) noexcept(noexcept(std::exchange(obj, std::forward<U>(newValue)))) {
        return std::exchange(obj, std::forward<U>(newValue));
    }
};

template <typename Impl>
struct Owner {
    Owner(int* ptr)
        : Ptr(ptr)
    {}

    Owner(Owner&& other) noexcept
        : Ptr(Impl::Exchange(other.Ptr, nullptr))
    {}

    int* Ptr = nullptr;
};

template <typename Impl>
void Ints() {
    int x = 1;
    int y = 2;
    Impl::Swap(x, y);
    Test::Check(x == 2 && y == 1, "Swap(x, y) should leave x == 2 and y == 1");
}

template <typename Impl>
void NoCopies() {
    Counter a = 1;
    Counter b = 2;
    Counter::Copies = 0;
    Impl::Swap(a, b);
    Test::Check(a.Value == 2 && b.Value == 1, "Swap(a, b) should leave a == 2 and b == 1");
    Test::Check(Counter::Copies == 0, "Swap(a, b) should not make any copies");
}

template <typename Impl>
void MoveOnlyType() {
    MoveOnly a = 1;
    MoveOnly b = 2;
    Impl::Swap(a, b);
    Test::Check(a.Value == 2 && b.Value == 1, "Swap(a, b) should leave a == 2 and b == 1");
}

template <typename Impl>
void SwapNoexcept() {
    int x = 1;
    int y = 2;
    ThrowingMove a;
    ThrowingMove b;
    Impl::Swap(x, y);
    Test::Check(noexcept(Impl::Swap(x, y)), "Swap on ints should be noexcept");
    Test::Check(!noexcept(Impl::Swap(a, b)), "Swap on a type whose move can throw should not be noexcept");
}

template <typename Impl>
void Arrays() {
    int a[3] = {1, 2, 3};
    int b[3] = {4, 5, 6};
    Impl::Swap(a, b);
    Test::Check(a[0] == 4 && a[1] == 5 && a[2] == 6, "Swap(a, b) should leave a == {4, 5, 6}");
    Test::Check(b[0] == 1 && b[1] == 2 && b[2] == 3, "Swap(a, b) should leave b == {1, 2, 3}");
}

template <typename Impl>
void MultidimensionalArrays() {
    int a[2][2] = {{1, 2}, {3, 4}};
    int b[2][2] = {{5, 6}, {7, 8}};
    Impl::Swap(a, b);
    Test::Check(a[0][0] == 5 && a[0][1] == 6 && a[1][0] == 7 && a[1][1] == 8, "Swap(a, b) should leave a == {{5, 6}, {7, 8}}");
    Test::Check(b[0][0] == 1 && b[0][1] == 2 && b[1][0] == 3 && b[1][1] == 4, "Swap(a, b) should leave b == {{1, 2}, {3, 4}}");
}

template <typename Impl>
void ArraySwapNoexcept() {
    int x[3] = {1, 2, 3};
    int y[3] = {4, 5, 6};
    ThrowingMove a[3];
    ThrowingMove b[3];
    Impl::Swap(x, y);
    Test::Check(noexcept(Impl::Swap(x, y)), "Swap on int arrays should be noexcept");
    Test::Check(!noexcept(Impl::Swap(a, b)), "Swap on arrays of a type whose move can throw should not be noexcept");
}

template <typename Impl>
void MultidimensionalArraySwapNoexcept() {
    int x[2][2] = {{1, 2}, {3, 4}};
    int y[2][2] = {{5, 6}, {7, 8}};
    ThrowingMove a[2][2];
    ThrowingMove b[2][2];
    Impl::Swap(x, y);
    Test::Check(noexcept(Impl::Swap(x, y)), "Swap on 2D int arrays should be noexcept");
    Test::Check(!noexcept(Impl::Swap(a, b)), "Swap on 2D arrays of a type whose move can throw should not be noexcept");
}

template <typename Impl>
void ExchangeBasic() {
    int x = 1;
    int old = Impl::Exchange(x, 2);
    Test::Check(old == 1, "Exchange(x, 2) should return the old value 1");
    Test::Check(x == 2, "Exchange(x, 2) should leave x == 2");
}

template <typename Impl>
void ExchangeMovesNewValue() {
    Counter x = 1;
    Counter::Copies = 0;
    Counter old = Impl::Exchange(x, Counter(2));
    Test::Check(old.Value == 1 && x.Value == 2, "Exchange(x, Counter(2)) should return 1 and leave x == 2");
    Test::Check(Counter::Copies == 0, "Exchange(x, Counter(2)) should not make any copies");
}

template <typename Impl>
void ExchangeCopiesLValue() {
    Counter x = 1;
    Counter replacement = 2;
    Counter::Copies = 0;
    Counter old = Impl::Exchange(x, replacement);
    Test::Check(old.Value == 1 && x.Value == 2, "Exchange(x, replacement) should return 1 and leave x == 2");
    Test::Check(Counter::Copies == 1, "Exchange(x, replacement) should copy replacement, not move from it");
}

template <typename Impl>
void MoveConstructorIdiom() {
    int value = 1;
    Owner<Impl> source(&value);
    Owner<Impl> destination(std::move(source));
    Test::Check(destination.Ptr == &value, "The new Owner should own the pointer");
    Test::Check(source.Ptr == nullptr, "The moved-from Owner should be left with nullptr");
}

template <typename Impl>
void ExchangeNoexcept() {
    int x = 1;
    ThrowingMove a;
    Impl::Exchange(x, 2);
    Test::Check(noexcept(Impl::Exchange(x, 2)), "Exchange on ints should be noexcept");
    Test::Check(
        !noexcept(Impl::Exchange(a, ThrowingMove())),
        "Exchange on a type whose move can throw should not be noexcept");
}

}  // namespace tests::swap

Test4::Test4()
    : Test(4, "swap")
{}

void Test4::RunTests() {
    using namespace tests::swap;

    Run("Ints: Swap(x, y)", 1, Ints<User>, Ints<Solution>, Ints<Stl>);
    Run("No copies: Swap(a, b) on Counter", 1, NoCopies<User>, NoCopies<Solution>, NoCopies<Stl>);
    Run("Move-only type: Swap(a, b) on MoveOnly", 1, MoveOnlyType<User>, MoveOnlyType<Solution>, MoveOnlyType<Stl>);
    Run("Swap noexcept: int vs ThrowingMove", 1, SwapNoexcept<User>, SwapNoexcept<Solution>, SwapNoexcept<Stl>);
    Run("Arrays: int a[3], b[3]", 1, Arrays<User>, Arrays<Solution>, Arrays<Stl>);
    Run("Multidimensional arrays: int a[2][2], b[2][2]", 1, MultidimensionalArrays<User>, MultidimensionalArrays<Solution>, MultidimensionalArrays<Stl>);
    Run("Array swap noexcept: int[3] vs ThrowingMove[3]", 1, ArraySwapNoexcept<User>, ArraySwapNoexcept<Solution>, ArraySwapNoexcept<Stl>);
    Run(
        "Multidimensional array swap noexcept: int[2][2] vs ThrowingMove[2][2]",
        1,
        MultidimensionalArraySwapNoexcept<User>,
        MultidimensionalArraySwapNoexcept<Solution>, MultidimensionalArraySwapNoexcept<Stl>);
    Run("Exchange: Exchange(x, 2)", 1, ExchangeBasic<User>, ExchangeBasic<Solution>, ExchangeBasic<Stl>);
    Run("Exchange moves the new value: Exchange(x, Counter(2))", 1, ExchangeMovesNewValue<User>, ExchangeMovesNewValue<Solution>, ExchangeMovesNewValue<Stl>);
    Run("Exchange copies an lvalue: Exchange(x, replacement)", 1, ExchangeCopiesLValue<User>, ExchangeCopiesLValue<Solution>, ExchangeCopiesLValue<Stl>);
    Run("Move constructor idiom: Exchange(other.Ptr, nullptr)", 1, MoveConstructorIdiom<User>, MoveConstructorIdiom<Solution>, MoveConstructorIdiom<Stl>);
    Run("Exchange noexcept: int vs ThrowingMove", 1, ExchangeNoexcept<User>, ExchangeNoexcept<Solution>, ExchangeNoexcept<Stl>);
}
