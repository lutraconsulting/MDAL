# MDAL - Mesh Data Abstraction Library (MIT License)
# Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)

set(CMAKE_CXX_STANDARD 14)

SET(MDAL_SOURCES 
  mdal.cpp
  mdal_utils.cpp
  mdal_driver_manager.cpp
  mdal_data_model.cpp
  mdal_datetime.cpp
  mdal_logger.cpp
  mdal_memory_data_model.cpp
  frmts/mdal_driver.cpp
  frmts/mdal_dynamic_driver.cpp
  frmts/mdal_2dm.cpp
  frmts/mdal_xms_tin.cpp
  frmts/mdal_ascii_dat.cpp
  frmts/mdal_binary_dat.cpp
  frmts/mdal_selafin.cpp
  frmts/mdal_esri_tin.cpp
  frmts/mdal_ply.cpp
  frmts/libplyxx/libplyxx.cpp
)

SET(MDAL_HEADERS
  api/mdal.h
  mdal_utils.hpp
  mdal_driver_manager.hpp
  mdal_data_model.hpp
  mdal_datetime.hpp
  mdal_logger.hpp
  mdal_memory_data_model.hpp
  frmts/mdal_driver.hpp
  frmts/mdal_dynamic_driver.hpp
  frmts/mdal_2dm.hpp
  frmts/mdal_xms_tin.hpp
  frmts/mdal_ascii_dat.hpp
  frmts/mdal_binary_dat.hpp
  frmts/mdal_selafin.hpp
  frmts/mdal_esri_tin.hpp
  frmts/mdal_ply.hpp
  frmts/libplyxx/libplyxx.h
  frmts/libplyxx/libplyxx_internal.h
  frmts/libplyxx/textio.h
)

IF(HDF5_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_xmdf.cpp
    frmts/mdal_hdf5.cpp
    frmts/mdal_flo2d.cpp
    frmts/mdal_hec2d.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_hdf5.hpp
    frmts/mdal_xmdf.hpp
    frmts/mdal_flo2d.hpp
    frmts/mdal_hec2d.hpp
  )
ENDIF(HDF5_FOUND)

IF(GDAL_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_gdal.cpp
    frmts/mdal_gdal_grib.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_gdal.hpp
    frmts/mdal_gdal_grib.hpp
  )
ENDIF(GDAL_FOUND)

IF(NETCDF_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_cf.cpp
    frmts/mdal_ugrid.cpp
    frmts/mdal_netcdf.cpp
    frmts/mdal_sww.cpp
    frmts/mdal_tuflowfv.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_cf.hpp
    frmts/mdal_ugrid.hpp
    frmts/mdal_netcdf.hpp
    frmts/mdal_sww.hpp
    frmts/mdal_tuflowfv.hpp
  )
ENDIF(NETCDF_FOUND)

IF(GDAL_FOUND AND NETCDF_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_gdal_netcdf.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_gdal_netcdf.hpp
  )
ENDIF(GDAL_FOUND AND NETCDF_FOUND)

IF(XML_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_xml.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_xml.hpp
  )
ENDIF(XML_FOUND)

IF(HDF5_FOUND AND XML_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_xdmf.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_xdmf.hpp
  )
ENDIF(HDF5_FOUND AND XML_FOUND)

IF(SQLITE3_FOUND AND NETCDF_FOUND)
  SET(MDAL_SOURCES ${MDAL_SOURCES}
    frmts/mdal_sqlite3.cpp
    frmts/mdal_3di.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_sqlite3.hpp
    frmts/mdal_3di.hpp
  )
ENDIF(SQLITE3_FOUND AND NETCDF_FOUND)

SET(MDAL_LIBS)

# STATIC LIBRARY
IF(BUILD_STATIC OR ENABLE_TESTS)
  SET(MDAL_LIBS mdal_a)
  ADD_LIBRARY(mdal_a STATIC
    ${MDAL_SOURCES}
    ${MDAL_HEADERS}
  )
  INSTALL(TARGETS mdal_a ARCHIVE DESTINATION lib)
  TARGET_COMPILE_DEFINITIONS(mdal_a PUBLIC MDAL_STATIC)
ENDIF(BUILD_STATIC OR ENABLE_TESTS)

# SHARED LIBRARY
IF (BUILD_SHARED OR ENABLE_TESTS)
  SET(MDAL_LIBS ${MDAL_LIBS} mdal)
  ADD_LIBRARY(mdal SHARED
    ${MDAL_SOURCES}
    ${MDAL_HEADERS}
  )

  TARGET_LINK_LIBRARIES(mdal PRIVATE ${CMAKE_DL_LIBS})

  IF(WIN32)
    INSTALL(TARGETS mdal RUNTIME DESTINATION lib)
  ELSE(WIN32)
    INSTALL(TARGETS mdal LIBRARY DESTINATION lib)
  ENDIF(WIN32)
ENDIF (BUILD_SHARED OR ENABLE_TESTS)

# LINK
FOREACH(LIB_NAME ${MDAL_LIBS})
  TARGET_INCLUDE_DIRECTORIES(${LIB_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/api>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  )

  IF(HDF5_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${HDF5_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${HDF5_C_LIBRARIES} )
    ADD_DEFINITIONS(${HDF5_DEFINITIONS})
  ENDIF(HDF5_FOUND)

  IF(GDAL_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${GDAL_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${GDAL_LIBRARIES} )
  ENDIF(GDAL_FOUND)

  IF(NETCDF_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${NETCDF_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${NETCDF_LIBRARY} )
  ENDIF(NETCDF_FOUND)

  IF(XML_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${LIBXML2_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${LIBXML2_LIBRARIES} )
    TARGET_COMPILE_DEFINITIONS(${LIB_NAME} PRIVATE ${LIBXML2_DEFINITIONS})
  ENDIF(XML_FOUND)

  IF(SQLITE3_FOUND)
    TARGET_INCLUDE_DIRECTORIES(${LIB_NAME} PRIVATE ${SQLITE3_INCLUDE_DIR})
    TARGET_LINK_LIBRARIES(${LIB_NAME} PUBLIC ${SQLITE3_LIBRARIES} )
  ENDIF(SQLITE3_FOUND)
ENDFOREACH(LIB_NAME ${MDAL_LIBS})

# INSTALL HEADER
INSTALL(FILES api/mdal.h DESTINATION include)
