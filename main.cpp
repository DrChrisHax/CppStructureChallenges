// Created by Chris Manlove

#include <iostream>

#include "test.hpp"

#include "0_fundamentals/2_move_tests.hpp"

int main() {
    int number = -1;
    std::cout << "Enter a challenge number: ";
    std::cin >> number;
    std::cout << '\n';

    switch (number) {
        case 2: {
            Test2 test;
            test.RunAll();
            break;
        }
        default: {
            std::cout << "No tests found for challenge " << number << '\n';
            return 1;
        }
    }

    return 0;
}
