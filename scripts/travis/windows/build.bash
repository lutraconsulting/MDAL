#!/usr/bin/env bash
set -e

echo "Windows native build"
mkdir -p build_win
cd build_win
C:/Program\ Files/CMake/bin/cmake -G "Visual Studio 15" ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
C:/Program\ Files/CMake/bin/cmake --build .
C:/Program\ Files/CMake/bin/ctest -VV
cd ..

