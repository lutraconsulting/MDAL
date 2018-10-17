#!/usr/bin/env bash
set -e

QGIS_VER=3.2
echo "Using libraries from QGIS ${QGIS_VER}"
ls -la C:/Program\ Files/QGIS/ ${QGIS_VER}/lib


echo "Windows native build"
mkdir -p build_win
cd build_win
C:/Program\ Files/CMake/bin/cmake -G "Visual Studio 15" ${CMAKE_OPTIONS} \
   -DCMAKE_BUILD_TYPE=Rel \
   -DENABLE_TESTS=ON \
   -DNETCDF_PREFIX=C:/Program\ Files/QGIS\ ${QGIS_VER} \
   -DHDF5_ROOT=C:/Program\ Files/QGIS\ ${QGIS_VER} \
   -DGDAL_ROOT=C:/Program\ Files/QGIS\ ${QGIS_VER} \
   ..

C:/Program\ Files/CMake/bin/cmake --build .
C:/Program\ Files/CMake/bin/ctest -VV

cd ..
