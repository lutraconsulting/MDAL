CMAKE_MINIMUM_REQUIRED(VERSION 3.9)
PROJECT(MDAL)

#############################################################
# deps
INCLUDE(GenerateExportHeader)
FIND_PACKAGE(Qt5 COMPONENTS Core Test REQUIRED)

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
# library
ADD_SUBDIRECTORY(mdal)

#############################################################
# testing
SET (ENABLE_TESTS TRUE CACHE BOOL "Build tests?")
IF (ENABLE_TESTS)
  SET( QT_USE_QTTEST  TRUE )
  ADD_DEFINITIONS(-DENABLE_TESTS)
  ENABLE_TESTING()
  ADD_SUBDIRECTORY(tests)
ENDIF(ENABLE_TESTS)
