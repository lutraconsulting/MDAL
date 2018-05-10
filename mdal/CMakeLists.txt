SET(MDAL_SOURCES 
  mdal.cpp
  mdal_utils.cpp
  mdal_loader.cpp
  frmts/mdal_2dm.cpp
  frmts/mdal_ascii_dat.cpp
)

SET(MDAL_HEADERS
  api/mdal.h
  mdal_utils.hpp
  mdal_loader.hpp
  mdal_defines.hpp
  frmts/mdal_2dm.hpp
  frmts/mdal_ascii_dat.hpp
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

IF(WIN32)
  INSTALL(TARGETS mdal RUNTIME DESTINATION lib)
ELSE(WIN32)
  INSTALL(TARGETS mdal LIBRARY DESTINATION lib)
ENDIF(WIN32)
INSTALL(FILES api/mdal.h DESTINATION include)
