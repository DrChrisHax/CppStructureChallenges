// Created by Chris Manlove

#include <type_traits>
#include <utility>

#include "0_fundamentals/2_move_tests.hpp"
#include "0_fundamentals/2_move_challenge.hpp"
#include "0_fundamentals/2_move_solution.hpp"

namespace {

struct Widget {
    int Value = 0;
};

class MoveOnly {
public:
    MoveOnly() = default;
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&&) = default;

    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly& operator=(MoveOnly&&) = default;
};

struct ThrowingMove {
    ThrowingMove() = default;

    ThrowingMove(const ThrowingMove&) {
        ++Copies;
    }

    ThrowingMove(ThrowingMove&&) {
        ++Moves;
    }

    inline static int Copies = 0;
    inline static int Moves = 0;
};

struct ThrowingMoveOnly {
    ThrowingMoveOnly() = default;
    ThrowingMoveOnly(const ThrowingMoveOnly&) = delete;
    ThrowingMoveOnly(ThrowingMoveOnly&&) {}
};

}  // namespace

namespace tests::move {

struct User {
    template <typename T>
    static decltype(challenges::move::Move(std::declval<T>())) Move(T&& value) {
        return challenges::move::Move(std::forward<T>(value));
    }

    template <typename T>
    static decltype(challenges::move::MoveIfNoexcept(std::declval<T&>())) MoveIfNoexcept(T& value) {
        return challenges::move::MoveIfNoexcept(value);
    }
};

struct Solution {
    template <typename T>
    static decltype(solutions::move::Move(std::declval<T>())) Move(T&& value) {
        return solutions::move::Move(std::forward<T>(value));
    }

    template <typename T>
    static decltype(solutions::move::MoveIfNoexcept(std::declval<T&>())) MoveIfNoexcept(T& value) {
        return solutions::move::MoveIfNoexcept(value);
    }
};

template <typename Impl>
void LValue() {
    int x = 1;
    Impl::Move(x);
    Test::Check(std::is_same_v<decltype(Impl::Move(x)), int&&>, "Move(x) should return int&&");
}

template <typename Impl>
void RValue() {
    Impl::Move(Widget{});
    Test::Check(std::is_same_v<decltype(Impl::Move(Widget{})), Widget&&>, "Move(Widget{}) should return Widget&&");
}

template <typename Impl>
void LValueReference() {
    const int x = 1;
    const int& y = x;
    Impl::Move(y);
    Test::Check(std::is_same_v<decltype(Impl::Move(y)), const int&&>, "Move(y) should return const int&&");
}

template <typename Impl>
void NonCopyableType() {
    MoveOnly original;
    Impl::Move(original);
    Test::Check(std::is_same_v<decltype(Impl::Move(original)), MoveOnly&&>, "Move(original) should return MoveOnly&&");
    Test::Check(
        std::is_constructible_v<MoveOnly, decltype(Impl::Move(original))>,
        "A new MoveOnly should be constructible from Move(original)");
}

template <typename Impl>
void CStyleArray() {
    int arr[3] = {1, 2, 3};
    Impl::Move(arr);
    Test::Check(std::is_same_v<decltype(Impl::Move(arr)), int(&&)[3]>, "Move(arr) should return int(&&)[3]");
}

template <typename Impl>
void IfNoexceptInt() {
    int x = 1;
    Impl::MoveIfNoexcept(x);
    Test::Check(std::is_same_v<decltype(Impl::MoveIfNoexcept(x)), int&&>, "MoveIfNoexcept(x) should return int&&");
}

template <typename Impl>
void IfNoexceptNothrowMove() {
    Widget w;
    Impl::MoveIfNoexcept(w);
    Test::Check(
        std::is_same_v<decltype(Impl::MoveIfNoexcept(w)), Widget&&>,
        "MoveIfNoexcept(w) should return Widget&& because Widget's move can't throw");
}

template <typename Impl>
void IfNoexceptThrowingMove() {
    ThrowingMove t;
    Impl::MoveIfNoexcept(t);
    Test::Check(
        std::is_same_v<decltype(Impl::MoveIfNoexcept(t)), const ThrowingMove&>,
        "MoveIfNoexcept(t) should return const ThrowingMove& because its move can throw and it can be copied");
}

template <typename Impl>
void IfNoexceptThrowingMoveOnly() {
    ThrowingMoveOnly t;
    Impl::MoveIfNoexcept(t);
    Test::Check(
        std::is_same_v<decltype(Impl::MoveIfNoexcept(t)), ThrowingMoveOnly&&>,
        "MoveIfNoexcept(t) should return ThrowingMoveOnly&& because it can't be copied");
}

template <typename Impl>
void IfNoexceptPicksCopy() {
    ThrowingMove original;
    ThrowingMove::Copies = 0;
    ThrowingMove::Moves = 0;
    ThrowingMove made(Impl::MoveIfNoexcept(original));
    Test::Check(ThrowingMove::Copies == 1, "Building from MoveIfNoexcept(original) should copy");
    Test::Check(ThrowingMove::Moves == 0, "Building from MoveIfNoexcept(original) should not move");
}

}  // namespace tests::move

Test2::Test2()
    : Test(2, "move")
{}

void Test2::RunTests() {
    using namespace tests::move;

    Run("L value: int x = 1", 1, LValue<User>, LValue<Solution>);
    Run("R value: Move(Widget{})", 1, RValue<User>, RValue<Solution>);
    Run("L value reference: const int x = 1; const int& y = x", 1, LValueReference<User>, LValueReference<Solution>);
    Run("Non-copyable type: MoveOnly", 1, NonCopyableType<User>, NonCopyableType<Solution>);
    Run("C-style array: int arr[3] = {1, 2, 3}", 1, CStyleArray<User>, CStyleArray<Solution>);
    Run("MoveIfNoexcept int: int x = 1", 1, IfNoexceptInt<User>, IfNoexceptInt<Solution>);
    Run("MoveIfNoexcept nothrow move: Widget", 1, IfNoexceptNothrowMove<User>, IfNoexceptNothrowMove<Solution>);
    Run("MoveIfNoexcept throwing move, copyable: ThrowingMove", 1, IfNoexceptThrowingMove<User>, IfNoexceptThrowingMove<Solution>);
    Run("MoveIfNoexcept throwing move, move-only: ThrowingMoveOnly", 1, IfNoexceptThrowingMoveOnly<User>, IfNoexceptThrowingMoveOnly<Solution>);
    Run("MoveIfNoexcept picks the copy: ThrowingMove made(MoveIfNoexcept(original))", 1, IfNoexceptPicksCopy<User>, IfNoexceptPicksCopy<Solution>);
}
