/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <iostream>
#include <algorithm>
#include <limits>
#include <string>
#include <vector>

#include "mdal_config.hpp"
#include "mdal.h"

void printDriverInfo( int index )
{
  DriverH driver = MDAL_driverFromIndex( index );
  std::string name = MDAL_DR_name( driver );
  bool onlyMesh = MDAL_DR_meshLoadCapability( driver );
  std::string meshFlag = onlyMesh ? "-mesh-" : "-data-";
  std::string longName = MDAL_DR_longName( driver );
  std::string filters = MDAL_DR_filters( driver );

  std::cout << name << " "
            << meshFlag << " (r): "
            << longName << " "
            << "(" << filters << ")"
            << std::endl;
}

void printFormats()
{
  int driverCount = MDAL_driverCount();
  for ( int i = 0; i < driverCount; ++i )
  {
    printDriverInfo( i );
  }
}


int main( int argc, char *argv[] )
{
  std::cout << "mdalinfo " << MDAL_Version() << std::endl;

  std::vector<std::string> args( argc );
  for ( int i = 0; i < argc; ++i )
    args[static_cast<size_t>( i )] = argv[i];

  // PARSE ARGS
  if ( std::find( args.begin(), args.end(), "-h" ) != args.end() )
  {
    std::cout << "mdalinfo mesh_file [dataset_file ...] [-h] [--formats]" << std::endl;
    return EXIT_SUCCESS;
  }

  if ( std::find( args.begin(), args.end(),  "--formats" ) != args.end() )
  {
    printFormats();
    return EXIT_SUCCESS;
  }

  if ( argc < 2 ) // no mesh argument
  {
    std::cout << "Missing mesh file argument" << std::endl;
    return EXIT_FAILURE;
  }

  std::string mesh_file = argv[1];

  std::vector<std::string> extraDatasets;
  if ( argc > 2 ) // additional dataset arguments
  {
    for ( int i = 2; i < argc; ++i )
      extraDatasets.push_back( argv[i] );
  }


  // MESH
  std::cout << "Mesh File: " << mesh_file << std::endl;
  MeshH m = MDAL_LoadMesh( mesh_file.c_str() );
  if ( m )
  {
    std::cout << "Mesh loaded: OK" << std::endl;
    std::cout << "  Vertex count: " << MDAL_M_vertexCount( m ) <<  std::endl;
    std::cout << "  Face count: " << MDAL_M_faceCount( m ) << std::endl;
    std::cout << "  Projection: " << MDAL_M_projection( m ) << std::endl;
  }
  else
  {
    std::cout << "Mesh loaded: ERR" << std::endl;
    std::cout << "Status:" << MDAL_LastStatus() <<  std::endl;
    return EXIT_FAILURE;
  }

  // EXTRA DATASETS
  for ( const std::string &dataset : extraDatasets )
  {
    std::cout << "Dataset File: " << dataset << std::endl;
    MDAL_M_LoadDatasets( m, dataset.c_str() );
    if ( MDAL_LastStatus() != MDAL_Status::None )
    {
      std::cout << "Dataset loaded: ERR" << std::endl;
      std::cout << "Status:" << MDAL_LastStatus() <<  std::endl;
      return EXIT_FAILURE;
    }
  }

  std::cout << "Datasets loaded: OK" << std::endl;
  std::cout << "  Groups count: " << MDAL_M_datasetGroupCount( m ) <<  std::endl;
  for ( int i = 0; i < MDAL_M_datasetGroupCount( m ); ++i )
  {
    auto group = MDAL_M_datasetGroup( m, i );
    std::cout << "  " << MDAL_G_name( group )
              << " " << MDAL_G_datasetCount( group );
    if ( !MDAL_G_hasScalarData( group ) )
      std::cout << " ( Vector ) ";
    std::cout << std::endl;
  }

  MDAL_CloseMesh( m );
  return EXIT_SUCCESS;
}
