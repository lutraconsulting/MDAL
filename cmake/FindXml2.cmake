# Find XML2
# ~~~~~~~~~~~~
# Copyright (c) 2020, Peter Petrik <zilolv at gmail.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# CMake module to search for LibXML2
# It looks for custom build and
# fallbacks to the default CMake implementation if not found

FIND_PATH(LIBXML2_INCLUDE_DIR libxml/xpath.h "$ENV{LIB_DIR}/include" NO_DEFAULT_PATH)
FIND_LIBRARY(LIBXML2_LIBRARY NAMES xml2 PATHS "$ENV{LIB_DIR}/lib" NO_DEFAULT_PATH)
FIND_PROGRAM(LIBXML2_XMLLINT_EXECUTABLE xmllint PATHS "$ENV{LIB_DIR}/bin" NO_DEFAULT_PATH)

FIND_PACKAGE(LibXml2)
