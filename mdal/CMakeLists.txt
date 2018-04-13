SET(MDAL_SOURCES 
  mdal.cpp
  mdal_utils.cpp
  mdal_loader.cpp
  frmts/mdal_2dm.cpp
)

SET(MDAL_HEADERS
  api/mdal.h
  mdal_utils.hpp
  mdal_loader.hpp
  mdal_defines.hpp
  frmts/mdal_2dm.hpp
)

SET_PROPERTY(
   SOURCE mdal.cpp
   APPEND PROPERTY COMPILE_DEFINITIONS
   MDAL_VERSION=${MDAL_VERSION}
)

ADD_LIBRARY(mdal SHARED
	${MDAL_SOURCES}
	${MDAL_HEADERS}
)

TARGET_INCLUDE_DIRECTORIES(mdal
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    )


INSTALL(TARGETS mdal LIBRARY DESTINATION lib)
INSTALL(FILES mdal.h DESTINATION include)
