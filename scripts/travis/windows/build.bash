#!/usr/bin/env bash
set -e

QGIS_VER=3.2
QGIS_DIR="C:/Program Files/QGIS ${QGIS_VER}"
echo "Using libraries from QGIS ${QGIS_VER}"
ls -la ${QGIS_DIR}/lib


echo "Windows native build"
mkdir -p build_win
cd build_win
C:/Program\ Files/CMake/bin/cmake -G "Visual Studio 15" ${CMAKE_OPTIONS} \
   -DCMAKE_BUILD_TYPE=Rel \
   -DENABLE_TESTS=ON \
   -DNETCDF_PREFIX=${QGIS_DIR} \
   -DHDF5_ROOT=${QGIS_DIR} \
   -DGDAL_ROOT=${QGIS_DIR} \
   ..

C:/Program\ Files/CMake/bin/cmake --build .
C:/Program\ Files/CMake/bin/ctest -VV

cd ..
