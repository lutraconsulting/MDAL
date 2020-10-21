# MDAL - Mesh Data Abstraction Library (MIT License)
# Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)

CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(MDAL)

IF(MSVC)
  # FindHDF5 still uses incompatible HDF5_ROOT
  CMAKE_POLICY(SET CMP0074 OLD)
ENDIF()

SET (CMAKE_CXX_STANDARD 11)
# set path to additional CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

#############################################################
# global vars
SET (MDAL_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output CACHE PATH "Output base directory")
SET (ENABLE_TESTS TRUE CACHE BOOL "Build tests?")
SET (ENABLE_COVERAGE FALSE CACHE BOOL "Enable GCOV code coverage?")
SET (WITH_HDF5 TRUE CACHE BOOL "Build providers that require HDF5 (e.g. XMDF, XDMF)")
SET (WITH_GDAL TRUE CACHE BOOL "Build providers that require GDAL (e.g. GRIB)")
SET (WITH_NETCDF TRUE CACHE BOOL "Build providers that require NETCDF (e.g. 3Di)")
SET (WITH_XML TRUE CACHE BOOL "Build providers that require LIBXML2 (e.g. XDMF)")
SET (WITH_SQLITE3 TRUE CACHE BOOL "Build providers that require SQLITE3 (e.g. 3Di 1D)")
SET (BUILD_STATIC FALSE CACHE BOOL "Build static mdal library" )
SET (BUILD_SHARED TRUE CACHE BOOL "Build shared mdal library" )
SET (BUILD_TOOLS TRUE CACHE BOOL "Build tool executables")

#############################################################
# Setup code coverage
IF(ENABLE_COVERAGE)
  IF (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(FATAL_ERROR "gcov coverage must be run in debug build type")
  ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")

  IF(MSVC)
    MESSAGE(FATAL_ERROR "gcov coverage is not implemented for Windows")
  ENDIF(MSVC)

  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
ENDIF(ENABLE_COVERAGE)

#############################################################
# warnings https://stackoverflow.com/a/3818084/2838364
IF(MSVC)
  # Force to always compile with W4
  IF(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    STRING(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  ELSE()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  ENDIF()
ELSEIF(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR APPLE )
  # Update if necessary
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
ENDIF(MSVC)

#############################################################
# optional libraries
IF (WITH_HDF5)
  FIND_PACKAGE(HDF5 REQUIRED)
  IF (HDF5_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_HDF5 TRUE)
    MESSAGE( STATUS "HDF5 VERSION ${HDF5_VERSION}")
    MESSAGE( STATUS "HDF5 ADD_DEFINITIONS ${HDF5_DEFINITIONS}")
  ENDIF (HDF5_FOUND)
ENDIF(WITH_HDF5)

IF (WITH_GDAL)
  FIND_PACKAGE(GDAL REQUIRED)
  IF (GDAL_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_GDAL TRUE)
  ENDIF (GDAL_FOUND)
ENDIF(WITH_GDAL)

IF (WITH_NETCDF)
  FIND_PACKAGE(NetCDF REQUIRED)
  IF (NETCDF_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_NETCDF TRUE)
  ENDIF (NETCDF_FOUND)
ENDIF(WITH_NETCDF)

IF (WITH_XML)
  FIND_PACKAGE(LibXml2 REQUIRED)
  IF (LIBXML2_FOUND)
    # following variable is used in mdal_config.h
    SET (XML_FOUND TRUE)
    SET (HAVE_XML TRUE)
  ENDIF (LIBXML2_FOUND)
ENDIF(WITH_XML)

IF (WITH_SQLITE3)
  FIND_PACKAGE(SQLite3 REQUIRED)
  IF (SQLITE3_FOUND)
    # following variable is used in mdal_config.h
    SET (HAVE_SQLITE3 TRUE)
  ENDIF (SQLITE3_FOUND)
ENDIF(WITH_SQLITE3)

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
  INCLUDE (CTest)
  ADD_DEFINITIONS(-DENABLE_TESTS)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF(ENABLE_TESTS)
