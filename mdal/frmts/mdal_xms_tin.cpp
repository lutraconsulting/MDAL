/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Lutra Consulting Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <limits>
#include <algorithm>

#include "mdal_xms_tin.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"

#define DRIVER_NAME "XMS_TIN"

MDAL::DriverXmsTin::DriverXmsTin():
  Driver( DRIVER_NAME,
          "XMS Tin Mesh File",
          "*.tin",
          Capability::ReadMesh
        )
{
}

MDAL::DriverXmsTin *MDAL::DriverXmsTin::create()
{
  return new DriverXmsTin();
}

MDAL::DriverXmsTin::~DriverXmsTin() = default;

bool MDAL::DriverXmsTin::canReadMesh( const std::string &uri )
{
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) || !startsWith( line, "TIN" ) )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverXmsTin::load( const std::string &meshFile, MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;

  std::ifstream in( meshFile, std::ifstream::in );
  std::string line;
  if ( !std::getline( in, line ) || !startsWith( line, "TIN" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }

  // Read vertices
  if ( !std::getline( in, line ) || !startsWith( line, "BEGT" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }
  if ( !std::getline( in, line ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }
  std::vector<std::string> chunks = split( line,  ' ' );
  if ( ( chunks.size() != 2 ) || ( chunks[0] != "VERT" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }
  size_t vertexCount = MDAL::toSizeT( chunks[1] );
  Vertices vertices( vertexCount );
  for ( size_t i = 0; i < vertexCount; ++i )
  {
    if ( !std::getline( in, line ) )
    {
      if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
      return nullptr;
    }
    chunks = split( line,  ' ' );
    if ( chunks.size() != 4 )
    {
      if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
      return nullptr;
    }

    Vertex &vertex = vertices[i];
    vertex.x = MDAL::toDouble( chunks[0] );
    vertex.y = MDAL::toDouble( chunks[1] );
    vertex.z = MDAL::toDouble( chunks[2] );
  }

  // Read triangles
  if ( !std::getline( in, line ) )
  {
    if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
    return nullptr;
  }
  chunks = split( line,  ' ' );
  if ( ( chunks.size() != 2 ) || ( chunks[0] != "TRI" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }
  size_t faceCount = MDAL::toSizeT( chunks[1] );
  Faces faces( faceCount );
  for ( size_t i = 0; i < faceCount; ++i )
  {
    if ( !std::getline( in, line ) )
    {
      if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
      return nullptr;
    }
    chunks = split( line,  ' ' );
    if ( chunks.size() != 3 )
    {
      // should have 3 indexes
      if ( status ) *status = MDAL_Status::Err_IncompatibleMesh;
      return nullptr;
    }

    Face &face = faces[i];
    face.resize( MAX_VERTICES_PER_FACE_TIN );
    face[0] = MDAL::toSizeT( chunks[0] ) - 1;
    face[1] = MDAL::toSizeT( chunks[1] ) - 1;
    face[2] = MDAL::toSizeT( chunks[2] ) - 1;
  }

  // Final keyword
  if ( !std::getline( in, line ) || !startsWith( line, "ENDT" ) )
  {
    if ( status ) *status = MDAL_Status::Err_UnknownFormat;
    return nullptr;
  }

  std::unique_ptr< MemoryMesh > mesh(
    new MemoryMesh(
      DRIVER_NAME,
      vertices.size(),
      0,
      faces.size(),
      MAX_VERTICES_PER_FACE_TIN,
      computeExtent( vertices ),
      meshFile
    )
  );
  mesh->faces = faces;
  mesh->vertices = vertices;

  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), vertices );

  return std::unique_ptr<Mesh>( mesh.release() );
}
