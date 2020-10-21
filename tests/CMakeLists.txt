# MDAL - Mesh Data Abstraction Library (MIT License)
# Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(GTest)

IF (NOT GTEST_FOUND)
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

    INCLUDE_DIRECTORIES("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
ELSE(NOT GTEST_FOUND)
    INCLUDE_DIRECTORIES(${GTEST_INCLUDE_DIR})
ENDIF(NOT GTEST_FOUND)


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
  IF(GDAL_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${TESTNAME} PRIVATE ${GDAL_INCLUDE_DIRS})
  ENDIF(GDAL_FOUND)
  ADD_TEST(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME})
ENDMACRO (ADD_MDAL_TEST)

SET(TESTS
    test_2dm.cpp
    test_xms_tin.cpp
    test_api.cpp
    test_ascii_dat.cpp
    test_binary_dat.cpp
    test_selafin.cpp
    test_esri_tin.cpp
    test_ply.cpp
    )

IF(HDF5_FOUND)
  SET(TESTS ${TESTS}
    test_xmdf.cpp
    test_flo2d.cpp
    test_hec2d.cpp
  )
ENDIF(HDF5_FOUND)

IF(GDAL_FOUND)
  SET(TESTS ${TESTS}
    test_gdal_grib.cpp
  )
ENDIF(GDAL_FOUND)

IF(NETCDF_FOUND)
  SET(TESTS ${TESTS}
    test_sww.cpp
    test_ugrid.cpp
    test_tuflowfv.cpp
  )
ENDIF(NETCDF_FOUND)

IF(SQLITE3_FOUND AND NETCDF_FOUND)
  SET(TESTS ${TESTS}
    test_3di.cpp
  )
ENDIF(SQLITE3_FOUND AND NETCDF_FOUND)

IF(GDAL_FOUND AND NETCDF_FOUND)
  SET(TESTS ${TESTS}
    test_gdal_netcdf.cpp
  )
ENDIF(GDAL_FOUND AND NETCDF_FOUND)

IF(HDF5_FOUND AND XML_FOUND)
  SET(TESTS ${TESTS}
    test_xdmf.cpp
  )
ENDIF(HDF5_FOUND AND XML_FOUND)

FOREACH(TESTSRC ${TESTS})
    ADD_MDAL_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)

# mdalinfo utility test
ADD_TEST(NAME mdalinfo_test COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/regular_grid.2dm" "${TESTDATA_DIR}/xmdf/regular_grid.xmdf" )
SET_TESTS_PROPERTIES(mdalinfo_test PROPERTIES PASS_REGULAR_EXPRESSION "Groups count: 9")

ADD_TEST(NAME mdalinfo_test1 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/lines.2dm" "${TESTDATA_DIR}/ascii_dat/lines_vertex_scalar.dat" )
SET_TESTS_PROPERTIES(mdalinfo_test1 PROPERTIES PASS_REGULAR_EXPRESSION "Groups count: 2")

ADD_TEST(NAME mdalinfo_test2 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/quad_and_triangle.2dm" "${TESTDATA_DIR}/ascii_dat/quad_and_triangle_vertex_scalar.dat" )
SET_TESTS_PROPERTIES(mdalinfo_test2 PROPERTIES PASS_REGULAR_EXPRESSION "Groups count: 2")

ADD_TEST(NAME mdalinfo_test3 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/tuflowfv/withMaxes/trap_steady_05_3D.nc" )
SET_TESTS_PROPERTIES(mdalinfo_test3 PROPERTIES PASS_REGULAR_EXPRESSION "Groups count: 21")

ADD_TEST(NAME mdalinfo_test4 COMMAND $<TARGET_FILE:mdalinfo> "--formats" )
SET_TESTS_PROPERTIES(mdalinfo_test4 PROPERTIES PASS_REGULAR_EXPRESSION "2DM Mesh File")

ADD_TEST(NAME mdalinfo_test5 COMMAND $<TARGET_FILE:mdalinfo> "-h" )
SET_TESTS_PROPERTIES(mdalinfo_test5 PROPERTIES PASS_REGULAR_EXPRESSION "mdalinfo mesh_file")

ADD_TEST(NAME mdalinfo_test6 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/flo2d/BarnHDF5/BASE.OUT" "--stats" )
SET_TESTS_PROPERTIES(mdalinfo_test6 PROPERTIES PASS_REGULAR_EXPRESSION "defined on:    faces")

ADD_TEST(NAME mdalinfo_test_err1 COMMAND $<TARGET_FILE:mdalinfo> )
SET_TESTS_PROPERTIES(mdalinfo_test_err1 PROPERTIES WILL_FAIL TRUE)

ADD_TEST(NAME mdalinfo_test_err2 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/not_a_mesh_file.2dm")
SET_TESTS_PROPERTIES(mdalinfo_test_err2 PROPERTIES WILL_FAIL TRUE)

ADD_TEST(NAME mdalinfo_test_err3 COMMAND $<TARGET_FILE:mdalinfo> "${TESTDATA_DIR}/2dm/regular_grid.2dm" "${TESTDATA_DIR}/ascii_dat/not_a_data_file.dat")
SET_TESTS_PROPERTIES(mdalinfo_test_err3 PROPERTIES WILL_FAIL TRUE)

#########################################################################
# UNITTESTING
#########################################################################

SET(UNITTESTS_SRC
    unittests/mdal_unittests.cpp
    unittests/test_mdal_utils.cpp
    unittests/test_mdal_datetime.cpp
    mdal_testutils.hpp
    mdal_testutils.cpp
)

ADD_EXECUTABLE(mdal_unittests ${UNITTESTS_SRC})
TARGET_LINK_LIBRARIES(mdal_unittests gtest gmock ${CMAKE_THREAD_LIBS_INIT} mdal_a)
ADD_TEST(mdal_unittests ${CMAKE_CURRENT_BINARY_DIR}/mdal_unittests)
