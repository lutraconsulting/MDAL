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
which valgrind
valgrind --version

mkdir -p build_db_lnx
cd build_db_lnx
cmake ${CMAKE_OPTIONS} \
      -DCMAKE_BUILD_TYPE=Debug \
      -DMEMORYCHECK_COMMAND:FILEPATH=`which valgrind` \
      -DMEMORYCHECK_COMMAND_OPTIONS="--leak-check=full --show-leak-kinds=all --track-origins=yes" \
      -DENABLE_TESTS=ON ..
make
ctest -D NightlyMemoryCheck

cd ..
