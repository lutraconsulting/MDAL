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

  bool saveMeshCapability = MDAL_DR_saveMeshCapability( driver );
  bool writeFaceDatasetsCapability = MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnFaces2D );
  bool writeVerticesDatasetsCapability = MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices2D );
  bool writeVolumesDatasetsCapability = MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVolumes3D );
  std::string writeFlag;
  if ( saveMeshCapability )
    writeFlag += " -Wmesh-";
  if ( writeFaceDatasetsCapability )
    writeFlag += " -Wface-";
  if ( writeVerticesDatasetsCapability )
    writeFlag += " -Wvertex-";
  if ( writeVolumesDatasetsCapability )
    writeFlag += " -Wvolume-";

  std::cout << name << " "
            << meshFlag << ""
            << writeFlag << ": "
            << longName << " "
            << "(" << filters << ")"
            << std::endl;
}

void printFormats()
{
  int driverCount = MDAL_driverCount();

  std::cout << std::endl;
  std::cout << "-mesh-" << " can read mesh frame and datasets " << std::endl
            << "-data-" << " can read only datasets " << std::endl
            << "-Wmesh-" << " can write mesh frame" << std::endl
            << "-Wface-" << " can write datasets defined on faces " << std::endl
            << "-Wvertex-" << " can write datasets defined on vertices " << std::endl
            << "-Wvolume-" << " can write datasets defined on volumes " << std::endl
            << std::endl;

  for ( int i = 0; i < driverCount; ++i )
  {
    printDriverInfo( i );
  }
}


int main( int argc, char *argv[] )
{
  std::cout << "mdalinfo " << MDAL_Version() << std::endl;

  std::vector<std::string> args( static_cast<size_t>( argc ) );
  for ( int i = 0; i < argc; ++i )
    args[static_cast<size_t>( i )] = argv[i];

  // PARSE ARGS
  if ( std::find( args.begin(), args.end(), "-h" ) != args.end() )
  {
    std::cout << "mdalinfo mesh_file [dataset_file ...] [-h] [--formats] [--stats]" << std::endl;
    return EXIT_SUCCESS;
  }

  if ( std::find( args.begin(), args.end(),  "--formats" ) != args.end() )
  {
    printFormats();
    return EXIT_SUCCESS;
  }

  bool stats = false;
  auto it = std::find( args.begin(), args.end(),  "--stats" );
  if ( it != args.end() )
  {
    stats = true;
    args.erase( it );
    --argc;
  }

  if ( argc < 2 ) // no mesh argument
  {
    std::cout << "Missing mesh file argument" << std::endl;
    return EXIT_FAILURE;
  }

  std::string mesh_file = args[1];

  std::vector<std::string> extraDatasets;
  if ( argc > 2 ) // additional dataset arguments
  {
    for ( int i = 2; i < argc; ++i )
      extraDatasets.push_back( args[static_cast<size_t>( i )] );
  }

  // MESH
  std::cout << "Mesh File: " << mesh_file << std::endl;
  MeshH m = MDAL_LoadMesh( mesh_file.c_str() );
  if ( m )
  {
    std::cout << "Mesh loaded: OK" << std::endl;
    std::cout << "  Driver: " << MDAL_M_driverName( m ) <<  std::endl;
    std::cout << "  Vertex count: " << MDAL_M_vertexCount( m ) <<  std::endl;
    std::cout << "  Edge count: " << MDAL_M_edgeCount( m ) <<  std::endl;
    std::cout << "  Face count: " << MDAL_M_faceCount( m ) << std::endl;
    std::string projection = MDAL_M_projection( m );
    if ( projection.empty() )
      projection = "undefined";
    std::cout << "  Projection: " << projection << std::endl;
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
    std::cout << "  " << MDAL_G_name( group );
    if ( !MDAL_G_hasScalarData( group ) )
      std::cout << " ( Vector ) ";

    if ( stats )
    {
      double min, max;
      MDAL_G_minimumMaximum( group, &min, &max );
      std::string definedOn;
      switch ( MDAL_G_dataLocation( group ) )
      {
        case MDAL_DataLocation::DataOnFaces2D:
          definedOn = "faces";
          break;
        case MDAL_DataLocation::DataOnVertices2D:
          definedOn = "vertices";
          break;
        case MDAL_DataLocation::DataOnVolumes3D:
          definedOn = "volumes";
          break;
        default:
          definedOn = "unknown";
          break;
      }
      std::cout << std::endl << "    driver:        " << MDAL_G_driverName( group );
      std::cout << std::endl << "    dataset count: " << MDAL_G_datasetCount( group );
      std::cout << std::endl << "    defined on:    " << definedOn;
      std::cout << std::endl << "    min:           " << min;
      std::cout << std::endl << "    max:           " << max;

    }
    std::cout << std::endl;
  }

  MDAL_CloseMesh( m );
  return EXIT_SUCCESS;
}
