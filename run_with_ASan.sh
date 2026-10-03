#!/bin/bash

cmake -S . -B build-asan -DCSC_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug && cmake --build build-asan -j && ./build-asan/csc
