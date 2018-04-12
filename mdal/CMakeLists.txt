SET(MDAL_SOURCES 
  mdal.cpp
  mdal_utils.cpp
  mdal_loader.cpp
  frmts/mdal_2dm.cpp
)

SET(MDAL_HEADERS
  mdal.h
  mdal_utils.hpp
  mdal_loader.hpp
  mdal_defines.hpp
  frmts/mdal_2dm.hpp
)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}/mdal
)

ADD_LIBRARY(mdal SHARED
	${MDAL_SOURCES}
	${MDAL_HEADERS}
)

INSTALL(TARGETS mdal LIBRARY DESTINATION lib)
INSTALL(FILES mdal.h DESTINATION include)
