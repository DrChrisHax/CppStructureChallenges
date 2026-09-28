// Created by Chris Manlove

#include <string>
#include <type_traits>
#include <utility>

#include "0_fundamentals/3_forward_tests.hpp"
#include "0_fundamentals/3_forward_challenge.hpp"
#include "0_fundamentals/3_forward_solution.hpp"

namespace tests::forward {

struct User {
    template <typename T, typename U>
    static decltype(auto) Forward(U&& value) {
        return challenges::forward::Forward<T>(std::forward<U>(value));
    }
};

struct Solution {
    template <typename T, typename U>
    static decltype(auto) Forward(U&& value) {
        return solutions::forward::Forward<T>(std::forward<U>(value));
    }
};

struct Stl {
    template <typename T, typename U>
    static decltype(auto) Forward(U&& value) {
        return std::forward<T>(std::forward<U>(value));
    }
};

char Which(int&) {
    return 'L';
}

char Which(int&&) {
    return 'R';
}

template <typename Impl, typename T>
char Wrapper(T&& arg) {
    return Which(Impl::template Forward<T>(arg));
}

template <typename Impl, typename... Args>
std::string WrapperPack(Args&&... args) {
    std::string result;
    ((result += Which(Impl::template Forward<Args>(args))), ...);
    return result;
}

template <typename Impl>
void LValueAsLValue() {
    int x = 1;
    Impl::template Forward<int&>(x);
    Test::Check(std::is_same_v<decltype(Impl::template Forward<int&>(x)), int&>, "Forward<int&>(x) should return int&");
}

template <typename Impl>
void LValueAsRValue() {
    int x = 1;
    Impl::template Forward<int>(x);
    Test::Check(std::is_same_v<decltype(Impl::template Forward<int>(x)), int&&>, "Forward<int>(x) should return int&&");
}

template <typename Impl>
void RValue() {
    Impl::template Forward<int>(1);
    Test::Check(std::is_same_v<decltype(Impl::template Forward<int>(1)), int&&>, "Forward<int>(1) should return int&&");
}

template <typename Impl>
void Const() {
    const int c = 1;
    Impl::template Forward<const int&>(c);
    Test::Check(
        std::is_same_v<decltype(Impl::template Forward<const int&>(c)), const int&>,
        "Forward<const int&>(c) should return const int&");
    Test::Check(
        std::is_same_v<decltype(Impl::template Forward<const int>(c)), const int&&>,
        "Forward<const int>(c) should return const int&&");
}

template <typename Impl>
void ThroughWrapper() {
    int x = 1;
    Test::Check(Wrapper<Impl>(x) == 'L', "Wrapper(x) should pass x on as an lvalue");
    Test::Check(Wrapper<Impl>(1) == 'R', "Wrapper(1) should pass 1 on as an rvalue");
}

template <typename Impl>
void ParameterPack() {
    int x = 1;
    int y = 2;
    Test::Check(WrapperPack<Impl>(x, 1, y, 2) == "LRLR", "WrapperPack(x, 1, y, 2) should pass on L, R, L, R");
}

}  // namespace tests::forward

Test3::Test3()
    : Test(3, "forward")
{}

void Test3::RunTests() {
    using namespace tests::forward;

    Run("L value as L value: Forward<int&>(x)", 1, LValueAsLValue<User>, LValueAsLValue<Solution>, LValueAsLValue<Stl>);
    Run("L value as R value: Forward<int>(x)", 1, LValueAsRValue<User>, LValueAsRValue<Solution>, LValueAsRValue<Stl>);
    Run("R value: Forward<int>(1)", 1, RValue<User>, RValue<Solution>, RValue<Stl>);
    Run("Const: Forward<const int&>(c), Forward<const int>(c)", 1, Const<User>, Const<Solution>, Const<Stl>);
    Run("Through a wrapper: Wrapper(x), Wrapper(1)", 1, ThroughWrapper<User>, ThroughWrapper<Solution>, ThroughWrapper<Stl>);
    Run("Parameter pack: WrapperPack(x, 1, y, 2)", 1, ParameterPack<User>, ParameterPack<Solution>, ParameterPack<Stl>);
}
