// Created by Chris Manlove

#include <iostream>

#include "test.hpp"

#include "0_fundamentals/2_move_tests.hpp"
#include "0_fundamentals/3_forward_tests.hpp"
#include "0_fundamentals/4_swap_tests.hpp"

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
        case 3: {
            Test3 test;
            test.RunAll();
            break;
        }
        case 4: {
            Test4 test;
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
