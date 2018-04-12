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
                 "${CMAKE_BINARY_DIR}/googletest-build")

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
# if(CMAKE_VERSION VERSION_LESS 2.8.11)
#    include_directories("${gtest_SOURCE_DIR}/include"
#                        "${gmock_SOURCE_DIR}/include")
# endif()
INCLUDE_DIRECTORIES(
    "${gtest_SOURCE_DIR}/include"
    "${gmock_SOURCE_DIR}/include"
)

#########################################################################
#########################################################################
#########################################################################
#########################################################################

INCLUDE_DIRECTORIES(  
  # ${GTEST_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/mdal
  ${CMAKE_SOURCE_DIR}/mdal
)



MACRO (ADD_MDAL_TEST TESTSRC)
  SET (TESTNAME  ${TESTSRC})
  STRING(REPLACE "test" "" TESTNAME ${TESTNAME})
  STRING(REPLACE "mdal" "" TESTNAME ${TESTNAME})
  STRING(REPLACE ".cpp" "" TESTNAME ${TESTNAME})
  SET (TESTNAME  "mdal_${TESTNAME}test")
  ADD_EXECUTABLE(${TESTNAME} ${TESTSRC})
  TARGET_LINK_LIBRARIES(${TESTNAME} gtest gmock ${CMAKE_THREAD_LIBS_INIT} mdal)
  ADD_TEST(${TESTNAME} ${CMAKE_CURRENT_BINARY_DIR}/${TESTNAME})
ENDMACRO (ADD_MDAL_TEST)

SET(TESTS
    testmdalcore.cpp
)

FOREACH(TESTSRC ${TESTS})
    ADD_MDAL_TEST(${TESTSRC})
ENDFOREACH(TESTSRC)
