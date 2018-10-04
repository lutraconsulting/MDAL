#!/usr/bin/env bash

echo "OSX native build"
mkdir -p build_osx
cd build_osx
export CMAKE_PREFIX_PATH=/usr/local/opt/gdal2

# disable netcdf since the timeout of travis for building gdal with drivers
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel \
      CMAKE_DISABLE_FIND_PACKAGE_NetCDF=TRUE \
      -DENABLE_TESTS=ON ..
make
ctest -VV