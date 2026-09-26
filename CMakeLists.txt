cmake_minimum_required(VERSION 3.20)
project(CppStructuresChallenges CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(CSC_SANITIZE "Build with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

file(GLOB_RECURSE CSC_TEST_SOURCES CONFIGURE_DEPENDS Tests/*_tests.cpp)

add_executable(csc main.cpp Utils/test.cpp ${CSC_TEST_SOURCES})
target_include_directories(csc PRIVATE Utils Challenges Solutions Tests)
target_compile_options(csc PRIVATE -Wall -Wextra -Wno-unused-parameter)

if(CSC_SANITIZE)
    target_compile_options(csc PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(csc PRIVATE -fsanitize=address,undefined)
endif()
