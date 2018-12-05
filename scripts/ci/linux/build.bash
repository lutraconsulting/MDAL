#!/usr/bin/env bash

set -e

echo "Linux Release build"
mkdir -p build_rel_lnx
cd build_rel_lnx
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
CTEST_TARGET_SYSTEM=Linux-gcc; ctest -VV
cd ..

echo "MinGW Cross-compile Windows build"
mkdir -p build_mingw
cd build_mingw
cmake .. -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32.cmake \
         -DWITH_GDAL=OFF -DENABLE_TESTS=OFF -DWITH_HDF5=OFF \
         -DWITH_NETCDF=OFF ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel
make
cd ..

echo "Linux Valgrind"
mkdir -p build_db_lnx
cd build_db_lnx
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON ..
make
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ctest -VV
cd ..
