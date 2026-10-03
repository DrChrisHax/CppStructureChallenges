// Created by Chris Manlove

#include <cstdint>
#include <iostream>

#include "test.hpp"

#include "0_fundamentals/2_move_tests.hpp"
#include "0_fundamentals/3_forward_tests.hpp"
#include "0_fundamentals/4_swap_tests.hpp"
#include "0_fundamentals/5_addressof_tests.hpp"
#include "1_ownership/100_ownership_primer_tests.hpp"
#include "2_contiguous/200_array_tests.hpp"
#include "2_contiguous/201_span_tests.hpp"

int main() {
    int32_t number = -1;
    std::cout << "Enter a challenge number: ";
    std::cin >> number;
    std::cout << '\n';

    const char TEXT_ONLY[] = "The challenge you selected is reading only.\nThere is no code to implement nor tests to be run.\n\n";

    switch (number) {
        case 0: {
            std::cout << TEXT_ONLY;
            break;
        }
        case 1: {
            std::cout << TEXT_ONLY;
            break;
        }
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
        case 5: {
            Test5 test;
            test.RunAll();
            break;
        }
        case 100: {
            Test100 test;
            test.RunAll();
            break;
        }
        case 200: {
            Test200 test;
            test.RunAll();
            break;
        }
        case 201: {
            Test201 test;
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
