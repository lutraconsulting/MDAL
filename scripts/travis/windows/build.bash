#!/usr/bin/env bash

echo "Windows native build"
mkdir -p build_win
cd build_win
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
ctest -VV
cd ..

