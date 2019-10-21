/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_tuflowfv.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_netcdf.hpp"
#include <math.h>

MDAL::DriverTuflowFV::DriverTuflowFV():
  DriverCF( "TUFLOWFV",
            "TUFLOW FV",
            "*.nc"
          )
{
}

MDAL::DriverTuflowFV::~DriverTuflowFV() = default;

MDAL::DriverTuflowFV *MDAL::DriverTuflowFV::create()
{
  return new DriverTuflowFV();
}

MDAL::CFDimensions MDAL::DriverTuflowFV::populateDimensions( )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  // 2D Mesh
  mNcFile.getDimension( "NumCells2D", &count, &ncid );
  dims.setDimension( CFDimensions::Face2D, count, ncid );

  mNcFile.getDimension( "MaxNumCellVert", &count, &ncid );
  dims.setDimension( CFDimensions::MaxVerticesInFace, count, ncid );

  mNcFile.getDimension( "NumVert2D", &count, &ncid );
  dims.setDimension( CFDimensions::Vertex2D, count, ncid );

  mNcFile.getDimension( "NumCells3D", &count, &ncid );
  dims.setDimension( CFDimensions::Volume3D, count, ncid );

  size_t levels = 1;
  if ( dims.size( CFDimensions::Face2D ) > 0 )
    levels = ( dims.size( CFDimensions::Volume3D ) / dims.size( CFDimensions::Face2D ) ) + 1;
  dims.setDimension( CFDimensions::Levels3D, levels, ncid );

  // Time
  mNcFile.getDimension( "Time", &count, &ncid );
  dims.setDimension( CFDimensions::Time, count, ncid );

  return dims;
}

void MDAL::DriverTuflowFV::populateFacesAndVertices( Vertices &vertices, Faces &faces )
{
  populateVertices( vertices );
  populateFaces( faces );
}

void MDAL::DriverTuflowFV::populateVertices( MDAL::Vertices &vertices )
{
  assert( vertices.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex2D );
  vertices.resize( vertexCount );
  Vertex *vertexPtr = vertices.data();

  // Parse 2D Mesh
  const std::vector<double> vertices2D_x = mNcFile.readDoubleArr( "node_X", vertexCount );
  const std::vector<double> vertices2D_y = mNcFile.readDoubleArr( "node_Y", vertexCount );
  const std::vector<double> vertices2D_z = mNcFile.readDoubleArr( "node_Zb", vertexCount );

  for ( size_t i = 0; i < vertexCount; ++i, ++vertexPtr )
  {
    vertexPtr->x = vertices2D_x[i];
    vertexPtr->y = vertices2D_y[i];
    vertexPtr->z = vertices2D_z[i];
  }
}

void MDAL::DriverTuflowFV::populateFaces( MDAL::Faces &faces )
{
  assert( faces.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face2D );
  faces.resize( faceCount );

  // Parse 2D Mesh
  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  std::vector<int> face_nodes_conn = mNcFile.readIntArr( "cell_node", faceCount * verticesInFace );
  std::vector<int> face_vertex_counts = mNcFile.readIntArr( "cell_Nvert", faceCount );

  for ( size_t i = 0; i < faceCount; ++i )
  {
    size_t nVertices = static_cast<size_t>( face_vertex_counts[i] );
    std::vector<size_t> idxs;

    for ( size_t j = 0; j < nVertices; ++j )
    {
      size_t idx = verticesInFace * i + j;
      int val = face_nodes_conn[idx];
      idxs.push_back( static_cast<size_t>( val ) );
    }
    faces[i] = idxs;
  }
}

void MDAL::DriverTuflowFV::addBedElevation( MDAL::MemoryMesh *mesh )
{
  MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices );
}

std::string MDAL::DriverTuflowFV::getCoordinateSystemVariableName()
{
  return "";
}

std::set<std::string> MDAL::DriverTuflowFV::ignoreNetCDFVariables()
{
  std::set<std::string> ignore_variables;

  ignore_variables.insert( getTimeVariableName() );
  ignore_variables.insert( "cell_Nvert" );
  ignore_variables.insert( "cell_node" );
  ignore_variables.insert( "idx2" );
  ignore_variables.insert( "idx3" );
  ignore_variables.insert( "cell_X" );
  ignore_variables.insert( "cell_Y" );
  ignore_variables.insert( "cell_Zb" );
  ignore_variables.insert( "cell_A" );
  ignore_variables.insert( "node_X" );
  ignore_variables.insert( "node_Y" );
  ignore_variables.insert( "node_Zb" );
  ignore_variables.insert( "layerface_Z" );
  ignore_variables.insert( "stat" );

  return ignore_variables;
}

void MDAL::DriverTuflowFV::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
{
  *is_vector = false;
  *is_x = true;

  std::string long_name = mNcFile.getAttrStr( "long_name", varid );
  if ( long_name.empty() )
  {
    if ( MDAL::endsWith( variableName, "_x" ) )
    {
      *is_vector = true;
      name = MDAL::replace( variableName, "_x", "" );
    }
    else if ( MDAL::endsWith( variableName, "_y" ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( variableName, "_y", "" );
    }
    else
    {
      name = variableName;
    }
  }
  else
  {
    if ( MDAL::startsWith( long_name, "x_" ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, "x_", "" );
    }
    else if ( MDAL::startsWith( long_name, "y_" ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, "y_", "" );
    }
    else
    {
      name = long_name;
    }
  }
}

std::string MDAL::DriverTuflowFV::getTimeVariableName() const
{
  return "ResTime";
}
