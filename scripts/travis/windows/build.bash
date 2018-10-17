#!/usr/bin/env bash
set -e


OSGEO4W_DIR="C:/OSGeo4W64"
echo "Using libraries from ${OSGEO4W_DIR}"
ls -la "${OSGEO4W_DIR}/lib"


echo "Windows native build"
mkdir -p build_win
cd build_win
C:/Program\ Files/CMake/bin/cmake -G "Visual Studio 15 Win64" ${CMAKE_OPTIONS} \
   -DCMAKE_BUILD_TYPE=Rel \
   -DENABLE_TESTS=ON \
   -DNETCDF_PREFIX="${OSGEO4W_DIR}" \
   -DHDF5_ROOT="${OSGEO4W_DIR}" \
   -DGDAL_DIR="${OSGEO4W_DIR}" \
   -DGDAL_LIBRARY="${OSGEO4W_DIR}/lib/gdal_i.lib" \
   -DGDAL_INCLUDE_DIR="${OSGEO4W_DIR}/include" \
   ..

C:/Program\ Files/CMake/bin/cmake --build .
C:/Program\ Files/CMake/bin/ctest -VV

cd ..
