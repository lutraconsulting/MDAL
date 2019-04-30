# MDAL - Mesh Data Abstraction Library (MIT License)
# Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)

SET(MDAL_SOURCES 
  mdal.cpp
  mdal_utils.cpp
  mdal_driver_manager.cpp
  mdal_data_model.cpp
  mdal_memory_data_model.cpp
  frmts/mdal_driver.cpp
  frmts/mdal_2dm.cpp
  frmts/mdal_ascii_dat.cpp
  frmts/mdal_binary_dat.cpp
)

SET(MDAL_HEADERS
  api/mdal.h
  mdal_utils.hpp
  mdal_driver_manager.hpp
  mdal_data_model.hpp
  mdal_memory_data_model.hpp
  frmts/mdal_driver.hpp
  frmts/mdal_2dm.hpp
  frmts/mdal_ascii_dat.hpp
  frmts/mdal_binary_dat.hpp
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
    frmts/mdal_3di.cpp
    frmts/mdal_ugrid.cpp
    frmts/mdal_netcdf.cpp
    frmts/mdal_sww.cpp
  )
  SET(MDAL_HEADERS ${MDAL_HEADERS}
    frmts/mdal_cf.hpp
    frmts/mdal_3di.hpp
    frmts/mdal_ugrid.hpp
    frmts/mdal_netcdf.hpp
    frmts/mdal_sww.hpp
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
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
)

IF(HDF5_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdal PRIVATE ${HDF5_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(mdal PUBLIC ${HDF5_C_LIBRARIES} )
ENDIF(HDF5_FOUND)

IF(GDAL_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdal PRIVATE ${GDAL_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(mdal PUBLIC ${GDAL_LIBRARIES} )
ENDIF(GDAL_FOUND)

IF(NETCDF_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdal PRIVATE ${NETCDF_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(mdal PUBLIC ${NETCDF_LIBRARY} )
ENDIF(NETCDF_FOUND)

IF(XML_FOUND)
  TARGET_INCLUDE_DIRECTORIES(mdal PRIVATE ${LIBXML2_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(mdal PUBLIC ${LIBXML2_LIBRARIES} )
  TARGET_COMPILE_DEFINITIONS(mdal PRIVATE ${LIBXML2_DEFINITIONS})
ENDIF(XML_FOUND)

IF(WIN32)
  INSTALL(TARGETS mdal RUNTIME DESTINATION lib)
ELSE(WIN32)
  INSTALL(TARGETS mdal LIBRARY DESTINATION lib)
ENDIF(WIN32)
INSTALL(FILES api/mdal.h DESTINATION include)
