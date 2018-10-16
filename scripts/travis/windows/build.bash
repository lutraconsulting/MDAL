#!/usr/bin/env bash

echo "Windows native build"
mkdir -p build_win
cd build_win
cmake -G "NMake Makefiles" ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
nmake
ctest -VV
cd ..

