#!/usr/bin/env bash
set -e


OSGEO4W_DIR="C:/OSGeo4W64"
echo "Using libraries from ${OSGEO4W_DIR}"

echo "Windows Visual Studio 15 64b build"
mkdir -p build_win
cd build_win
/mnt/c/Program\ Files/CMake/bin/cmake.exe -G "Visual Studio 16 2019" ${CMAKE_OPTIONS} \
   -DCMAKE_BUILD_TYPE=Rel \
   -DENABLE_TESTS=ON \
   -DNETCDF_PREFIX="${OSGEO4W_DIR}" \
   -DHDF5_ROOT="${OSGEO4W_DIR}" \
   -DGDAL_DIR="${OSGEO4W_DIR}" \
   -DGDAL_LIBRARY="${OSGEO4W_DIR}/lib/gdal_i.lib" \
   -DGDAL_INCLUDE_DIR="${OSGEO4W_DIR}/include" \
   -DLIBXML2_LIBRARIES="${OSGEO4W_DIR}/lib/libxml2.lib" \
   -DLIBXML2_INCLUDE_DIR="${OSGEO4W_DIR}/include/libxml2" \
   ..

/mnt/c/Program\ Files/CMake/bin/cmake.exe --build .

export PATH="$PATH:/c/OSGeo4W64/bin:/c/Users/travis/build/lutraconsulting/MDAL/build_win/tools/Debug:/c/Users/travis/build/lutraconsulting/MDAL/build_win/mdal/Debug"
echo "PATH used: $PATH"
/mnt/c/Program\ Files/CMake/bin/ctest.exe -VV --exclude-regex "mdalinfo_test"
cd ..
