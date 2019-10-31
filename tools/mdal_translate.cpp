/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Marek Urban (marek dot urban at lutraconsulting dot co dot uk)
*/

#include <iostream>
#include <algorithm>
#include <vector>
#include <exception>

#include "mdal_config.hpp"
#include "mdal.h"


void printFormats()
{
  int dc = MDAL_driverCount();
  std::vector<std::string> driverNames;

  for (int i = 0; i < dc; i++)
  {
    DriverH d = MDAL_driverFromIndex(i);
    if (MDAL_DR_SaveMeshCapability(d))
        std::cout << MDAL_DR_name(d) << std::endl;
  }
}

void printHelp()
{
  std::cout << "mdal_translate [-h] [-of format] src_mesh dst_mesh" << std::endl;
  printFormats();
}

MeshH loadMeshFile( const std::string &meshPath )
{
  MeshH mesh = MDAL_LoadMesh( meshPath.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  if ( s != MDAL_Status::None )
    throw std::runtime_error("Loading mesh file failed");

  return mesh;
}

void saveMeshAs( MeshH mesh, const std::string &format, const std::string &outputFilePath )
{
  MDAL_SaveMesh( mesh, outputFilePath.c_str(), format.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  if ( s != MDAL_Status::None )
    throw std::runtime_error("Saving mesh file failed");
}

int main( int argc, char *argv[] )
{
  std::vector<std::string> args( static_cast<size_t>( argc ) );

  for ( int i = 0; i < argc; ++i )
    args[static_cast<size_t>( i )] = argv[i];

  // Parse arguments
  if ( std::find( args.begin(), args.end(), "-h" ) != args.end() )
  {
    printHelp();
    return EXIT_SUCCESS;
  }

  if ( args.size() < 4 )
  {
    printHelp();
    return EXIT_FAILURE;
  }

  try
  {
    // Load Mesh
    MeshH mesh = loadMeshFile( args[1] );

    // Save Mesh
    saveMeshAs( mesh, args[2], args[3] );
  }
  catch ( std::exception &e )
  {
    std::cout << "Error has accoured: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
