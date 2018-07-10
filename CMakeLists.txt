CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(MDAL)
SET (CMAKE_CXX_STANDARD 11)

#############################################################
# global vars
SET (MDAL_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output CACHE PATH "Output base directory")
SET (ENABLE_TESTS TRUE CACHE BOOL "Build tests?")
SET (BUILD_TOOLS TRUE CACHE BOOL "Build tool executables?")
SET (WITH_HDF5 TRUE CACHE BOOL "Build providers that require HDF5")
SET (WITH_GDAL TRUE CACHE BOOL "Build providers that require GDAL (e.g. GRIB)")

#############################################################
# warnings https://stackoverflow.com/a/3818084/2838364
IF(MSVC)
  # Force to always compile with W4
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  ENDIF()
ELSEIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
ENDIF()

#############################################################
# optional libraries
IF (WITH_HDF5)
  FIND_PACKAGE(HDF5 REQUIRED)
  IF (HDF5_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_HDF5 TRUE)
  ENDIF (HDF5_FOUND)
ENDIF()

IF (WITH_GDAL)
  FIND_PACKAGE(GDAL REQUIRED)
  IF (GDAL_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_GDAL TRUE)
  ENDIF (GDAL_FOUND)
ENDIF()

#############################################################
# create mdal_config.h
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/cmake_templates/mdal_config.hpp.in ${CMAKE_BINARY_DIR}/mdal_config.hpp)
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR})

#############################################################
# library
ADD_SUBDIRECTORY(mdal)

#############################################################
# tools
IF(BUILD_TOOLS)
  ADD_SUBDIRECTORY(tools)
ENDIF(BUILD_TOOLS)

#############################################################
# testing
IF (ENABLE_TESTS)
  ADD_DEFINITIONS(-DENABLE_TESTS)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF(ENABLE_TESTS)
