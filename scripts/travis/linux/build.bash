#!/usr/bin/env bash

echo "Linux native build"
mkdir -p build_lnx
cd build_lnx
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

echo "Check code syntax"
cd scripts; ./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
cd ..