# - Find TinyXML2
# Find the native TinyXML2 includes and library
# Copyright (c) 2018, Peter Petrik <zilolv at gmail dot com>
#
# This module returns these variables for the rest of the project to use.
#
#  TINYXML2_FOUND          - True if TinyXML2 found including required interfaces (see below)
#  TINYXML2_LIBRARY        - All tinyxml2 related libraries
#  TINYXML2_INCLUDE_DIR    - All directories to include

IF (TINYXML2_INCLUDE_DIR AND TINYXML2_LIBRARY)
  # Already in cache, be silent
  SET (TINYXML2_FIND_QUIETLY TRUE)
ENDIF ()

FIND_PACKAGE(PkgConfig)
PKG_CHECK_MODULES(PC_TINYXML2 QUIET tinyxml2)
SET(TINYXML2_DEFINITIONS ${PC_TINYXML2_CFLAGS_OTHER})

FIND_PATH (TINYXML2_INCLUDE_DIR tinyxml2.h 
           HINTS ${PC_TINYXML2_INCLUDEDIR} ${PC_TINYXML2_INCLUDE_DIRS} ${TINYXML2_PREFIX}/include
           PATH_SUFFIXES libtinyxml2 )
           
FIND_LIBRARY (TINYXML2_LIBRARY 
              NAMES tinyxml2 libtinyxml2 
              HINTS HINTS ${PC_TINYXML2_LIBDIR} ${PC_TINYXML2_LIBRARY_DIRS} ${TINYXML2_PREFIX}/lib)

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS (TinyXML2
  DEFAULT_MSG TINYXML2_LIBRARY TINYXML2_INCLUDE_DIR)

