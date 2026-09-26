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

}  // namespace

namespace tests::move {

struct User {
    template <typename T>
    static decltype(challenges::move::Move(std::declval<T>())) Move(T&& value) {
        return challenges::move::Move(std::forward<T>(value));
    }
};

struct Solution {
    template <typename T>
    static decltype(solutions::move::Move(std::declval<T>())) Move(T&& value) {
        return solutions::move::Move(std::forward<T>(value));
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
}
