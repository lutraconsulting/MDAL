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

int main( int argc, char *argv[] )
{
  std::cout << "mdalinfo " << MDAL_Version() << std::endl;

  // PARSE ARGS
  if ( std::find( argv, argv + argc, "-h" ) != argv + argc )
  {
    std::cout << "mdalinfo mesh_file [dataset_file ...] [-h]" << std::endl;
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
