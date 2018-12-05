#!/usr/bin/env bash

echo "OSX native build"
mkdir -p build_osx
cd build_osx
export CMAKE_PREFIX_PATH=/usr/local/opt/gdal2

cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make

/usr/local/opt/gdal2/bin/gdalinfo --formats

# https://github.com/lutraconsulting/MDAL/issues/35
ctest -VV --exclude-regex "mdal_gdal_netcdf_test"