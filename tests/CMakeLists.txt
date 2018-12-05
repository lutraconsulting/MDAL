# MDAL - Mesh Data Abstraction Library (MIT License)
# Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)

FIND_PACKAGE(Threads REQUIRED)

#########################################################################
#########################################################################
#########################################################################
#########################################################################
# https://crascit.com/2015/07/25/cmake-gtest/

# Download and unpack googletest at configure time
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/GTestCMakeLists.in" "${CMAKE_BINARY_DIR}/googletest-download/CMakeLists.txt")
EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
EXECUTE_PROCESS(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
SET(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
ADD_SUBDIRECTORY("${CMAKE_BINARY_DIR}/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build"
                 EXCLUDE_FROM_ALL)

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()

#########################################################################
#########################################################################
SET(TESTDATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
ADD_DEFINITIONS(-DTESTDATA="${TESTDATA_DIR}")
ADD_LIBRARY(mdaltestutils OBJECT mdal_testutils.hpp mdal_testutils.cpp )
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/mdal/api)

#########################################################################
#########################################################################

INCLUDE_DIRECTORIES(  
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/mdal
  ${CMAKE_SOURCE_DIR}/mdal
)

MACRO (ADD_MDAL_TEST TESTSRC)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE "test_" "" TESTNAME ${TESTNAME})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "mdal_${TESTNAME}_test")
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC} $<TARGET_OBJECTS:mdaltestutils>)
  TARGET_LINK_LIBRARIES(${TESTNAME} gtest gmock ${CMAKE_THREAD_LIBS_INIT} mdal)
  ADD_TEST(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME})
ENDMACRO (ADD_MDAL_TEST)

SET(TESTS
    test_2dm.cpp
    test_mesh_read.cpp
    test_ascii_dat.cpp
    test_binary_dat.cpp
)

IF(HDF5_FOUND)
  SET(TESTS ${TESTS}
    test_xmdf.cpp
  )
ENDIF(HDF5_FOUND)

IF(GDAL_FOUND)
  SET(TESTS ${TESTS}
    test_gdal_grib.cpp
  )
ENDIF(GDAL_FOUND)

IF(NETCDF_FOUND)
SET(TESTS ${TESTS}
  test_3di.cpp
)
ENDIF(NETCDF_FOUND)

IF(GDAL_FOUND AND NETCDF_FOUND)
  SET(TESTS ${TESTS}
    test_gdal_netcdf.cpp
  )
ENDIF(GDAL_FOUND AND NETCDF_FOUND)

FOREACH(TESTSRC ${TESTS})
    ADD_MDAL_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)

# mdalinfo utility test
ADD_TEST(NAME mdalinfo_test COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/regular_grid.2dm" "${TESTDATA_DIR}/xmdf/regular_grid.xmdf" )
