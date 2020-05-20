#!/usr/bin/env bash

set -e

echo "OSX native build"
mkdir -p build_osx
cd build_osx

export PATH=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/bin:$PATH

cmake ${CMAKE_OPTIONS} \
      -DCMAKE_BUILD_TYPE=Rel \
      -DENABLE_TESTS=ON \
      -DGDAL_LIBRARY=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/lib/libgdal.dylib \
      -DGDAL_INCLUDE_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/include \
      -DGDAL_CONFIG=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/bin/gdal-config \
      -DNETCDF_PREFIX=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage \
      -DSQLITE3_INCLUDE_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/include \
      -DSQLITE3_LIBRARY=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/lib/libsqlite3.dylib \
      -DHDF5_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage \
      -DLIBXML2_INCLUDE_DIR=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/include \
      -DLIBXML2_LIBRARY=/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/lib/libxml2.dylib \
      ..

make

ctest -VV
