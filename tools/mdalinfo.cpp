/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <iostream>
#include <algorithm>
#include <limits>
#include <string>

#include "mdal.h"

int main( int argc, char *argv[] )
{
  std::cout << "mdalinfo " << MDAL_Version() << std::endl;

  // PARSE ARGS
  if ( std::find( argv, argv + argc, "-h" ) != argv + argc )
  {
    std::cout << "mdalinfo mesh_file [-h]" << std::endl;
    return EXIT_SUCCESS;
  }

  if ( argc < 2 ) // no mesh argument
  {
    std::cout << "Missing mesh file argument" << std::endl;
    return EXIT_FAILURE;
  }

  std::string mesh_file = argv[1];


  // MESH
  std::cout << "Mesh File: " << mesh_file << std::endl;
  MeshH m = MDAL_LoadMesh( mesh_file.c_str() );
  if ( m )
  {
    std::cout << "Mesh loaded: OK" << std::endl;
    std::cout << "  Vertex count: " << MDAL_M_vertexCount( m ) <<  std::endl;
    std::cout << "  Face count: " << MDAL_M_faceCount( m ) << std::endl;
  }
  else
  {
    std::cout << "Mesh loaded: ERR" << std::endl;
    std::cout << "Status:" << MDAL_LastStatus() <<  std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
