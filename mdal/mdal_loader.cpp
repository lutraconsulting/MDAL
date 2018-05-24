/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_config.hpp"
#include "mdal_loader.hpp"
#include "frmts/mdal_2dm.hpp"
#include "frmts/mdal_ascii_dat.hpp"
#include "frmts/mdal_binary_dat.hpp"

#ifdef HAVE_HDF5
#include "frmts/mdal_xmdf.hpp"
#endif

std::unique_ptr<MDAL::Mesh> MDAL::Loader::load( const std::string &meshFile, MDAL_Status *status )
{
  MDAL::Loader2dm loader( meshFile );
  std::unique_ptr<MDAL::Mesh> mesh = loader.load( status );
  return mesh;
}

void MDAL::Loader::loadDatasets( Mesh *mesh, const std::string &datasetFile, MDAL_Status *status )
{
  MDAL::LoaderAsciiDat loader( datasetFile );
  loader.load( mesh, status );

  if ( status && *status == MDAL_Status::Err_UnknownFormat )
  {
    MDAL::LoaderBinaryDat loader( datasetFile );
    loader.load( mesh, status );
  }

#ifdef HAVE_HDF5
  if ( status && *status == MDAL_Status::Err_UnknownFormat )
  {
    MDAL::LoaderXmdf loader( datasetFile );
    loader.load( mesh, status );
  }
#endif
}
