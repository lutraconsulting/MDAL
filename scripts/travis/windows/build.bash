#!/usr/bin/env bash

echo "Windows native build"
mkdir -p build_win
cd build_win
cmake -G "Visual Studio 15" ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
cmake --build .
ctest -VV
cd ..

