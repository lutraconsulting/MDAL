#!/usr/bin/env bash

set -e

echo "Linux Release build"
mkdir -p build_rel_lnx
cd build_rel_lnx
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
CTEST_TARGET_SYSTEM=Linux-gcc; ctest -VV
# CTEST_TARGET_SYSTEM=Linux-gcc; ctest -D Continuous
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
valgrind --version

mkdir -p build_db_lnx
cd build_db_lnx
cmake ${CMAKE_OPTIONS} \
      -DCMAKE_BUILD_TYPE=Debug \
      -DMEMORYCHECK_COMMAND_OPTIONS="--leak-check=yes --show-leak-kinds=definite --gen-suppressions=all --track-origins=yes --num-callers=20 --leak-resolution=high --show-reachable=no" \
      -DMEMORYCHECK_SUPPRESSIONS_FILE=../scripts/ci/linux/valgrind.supp \
      -DENABLE_TESTS=ON ..

make # VERBOSE=1

# run and submit to cdash
# GLIBCPP_FORCE_NEW=1; \
# GLIBCXX_FORCE_NEW=1; \
# CTEST_TARGET_SYSTEM=Linux-gcc; \
# ctest -D ContinuousMemCheck 2>&1 | tee memcheck.log

#https://stackoverflow.com/a/30403709/2838364
GLIBCPP_FORCE_NEW=1; \
GLIBCXX_FORCE_NEW=1; \
CTEST_TARGET_SYSTEM=Linux-gcc; \
ctest -T memcheck 2>&1 | tee memcheck.log

if grep -q "Defects:" "memcheck.log"; then
  echo "Error: Show memcheck results"
  ls -la /home/travis/build/lutraconsulting/MDAL/build_db_lnx/Testing/Temporary/MemoryChecker.*.log
  cat /home/travis/build/lutraconsulting/MDAL/build_db_lnx/Testing/Temporary/MemoryChecker.*.log
  exit 1
fi
cd ..

echo "Linux code coverage"
mkdir -p build_coverage_lnx
cd build_coverage_lnx
cmake ${CMAKE_OPTIONS} -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON -DENABLE_COVERAGE=ON ..
make
CTEST_TARGET_SYSTEM=Linux-gcc; ctest -VV
coveralls #--exclude googletest-src --exclude tests --exclude CMakeFiles
cd ..
