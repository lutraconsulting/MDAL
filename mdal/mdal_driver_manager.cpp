/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_config.hpp"
#include "mdal_driver_manager.hpp"
#include "frmts/mdal_2dm.hpp"
#include "frmts/mdal_ascii_dat.hpp"
#include "frmts/mdal_binary_dat.hpp"
#include "mdal_utils.hpp"

#ifdef HAVE_HDF5
#include "frmts/mdal_xmdf.hpp"
#include "frmts/mdal_flo2d.hpp"
#include "frmts/mdal_hec2d.hpp"
#endif

#ifdef HAVE_GDAL
#include "frmts/mdal_gdal_grib.hpp"
#endif

#ifdef HAVE_NETCDF
#include "frmts/mdal_3di.hpp"
#include "frmts/mdal_sww.hpp"
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
#include "frmts/mdal_gdal_netcdf.hpp"
#endif

static std::vector<std::shared_ptr<MDAL::Driver>> meshDrivers()
{
  std::vector<std::shared_ptr<MDAL::Driver>> ret;

  ret.push_back( std::make_shared<MDAL::Driver2dm>() );

#ifdef HAVE_HDF5
  ret.push_back( std::make_shared<MDAL::DriverFlo2D>() );
  ret.push_back( std::make_shared<MDAL::DriverHec2D>() );
#endif

#ifdef HAVE_NETCDF
  ret.push_back( std::make_shared<MDAL::Driver3Di>() );
  ret.push_back( std::make_shared<MDAL::DriverSWW>() );
  ret.push_back( std::make_shared<MDAL::DriverGdalNetCDF>() );
#endif

#if defined HAVE_GDAL && defined HAVE_NETCDF
  ret.push_back( std::make_shared<MDAL::DriverGdalGrib>() );
#endif // HAVE_GDAL && HAVE_NETCDF

  return ret;
}

static std::vector<std::shared_ptr<MDAL::Driver>> datasetDrivers()
{
  std::vector<std::shared_ptr<MDAL::Driver>> ret;
  ret.push_back( std::make_shared<MDAL::DriverAsciiDat>() );
  ret.push_back( std::make_shared<MDAL::DriverBinaryDat>() );
#ifdef HAVE_HDF5
  ret.push_back( std::make_shared<MDAL::DriverXmdf>() );
#endif
  return ret;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverManager::load( const std::string &meshFile, MDAL_Status *status )
{
  std::unique_ptr<MDAL::Mesh> mesh;

  if ( !MDAL::fileExists( meshFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return std::unique_ptr<MDAL::Mesh>();
  }

  std::vector<std::shared_ptr<MDAL::Driver>> drivers = meshDrivers();
  for ( auto driver : drivers )
  {
    if ( driver->canRead( meshFile ) )
    {
      mesh = driver->load( meshFile, status );
      if ( mesh ) // stop if he have the mesh
        break;
    }
  }

  if ( status && !mesh )
    *status = MDAL_Status::Err_UnknownFormat;

  return mesh;
}

void MDAL::DriverManager::loadDatasets( Mesh *mesh, const std::string &datasetFile, MDAL_Status *status )
{
  if ( !MDAL::fileExists( datasetFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return;
  }

  if ( !mesh )
  {
    if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
    return;
  }

  std::vector<std::shared_ptr<MDAL::Driver>> drivers = datasetDrivers();
  for ( auto driver : drivers )
  {
    if ( driver->canRead( datasetFile ) )
    {
      driver->load( datasetFile, mesh, status );
      return;
    }
  }

  if ( status )
    *status = MDAL_Status::Err_UnknownFormat;
}

std::vector<std::shared_ptr<MDAL::Driver> > MDAL::DriverManager::drivers()
{
  auto meshDrs = meshDrivers();
  const auto datasetDrs = datasetDrivers();
  meshDrs.insert( meshDrs.end(), datasetDrs.begin(), datasetDrs.end() );
  return meshDrs;
}

std::shared_ptr<MDAL::Driver> MDAL::DriverManager::driver( const std::string &driverName )
{
  const auto drvs = drivers();
  for ( const auto &dr : drvs )
  {
    if ( dr->name() == driverName )
      return dr;
  }
  return std::shared_ptr<MDAL::Driver>();
}
