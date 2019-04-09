#!/usr/bin/env bash

set -e

/usr/local/opt/osgeo-gdal/bin/gdalinfo --formats

echo "OSX native build"
mkdir -p build_osx
cd build_osx

cmake ${CMAKE_OPTIONS} \
      -DCMAKE_BUILD_TYPE=Rel \
      -DENABLE_TESTS=ON \
      -DGDAL_LIBRARY=/usr/local/opt/osgeo-gdal/lib/libgdal.dylib \
      -DGDAL_INCLUDE_DIR=/usr/local/opt/osgeo-gdal/include/ \
      -DGDAL_CONFIG=/usr/local/opt/osgeo-gdal/bin/gdal-config \
      -DNETCDF_PREFIX=/usr/local/opt/osgeo-netcdf \
      ..
make


# https://github.com/lutraconsulting/MDAL/issues/35
# ctest -VV --exclude-regex "mdal_gdal_netcdf_test"