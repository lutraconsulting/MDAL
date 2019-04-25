/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_ugrid.hpp"
#include <netcdf.h>
#include <assert.h>

MDAL::DriverUgrid::DriverUgrid()
  : DriverCF(
      "Ugrid",
      "UGRID Results",
      "*.nc" )
{
}

MDAL::DriverUgrid *MDAL::DriverUgrid::create()
{
  return new DriverUgrid();
}

MDAL::CFDimensions MDAL::DriverUgrid::populateDimensions( const NetCDFFile &ncFile )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  if ( ncFile.hasDimension( "nmesh2d_node" ) )
    mMesh2dName = "mesh2d"; // DFlow 1.1.x
  else if ( ncFile.hasDimension( "nMesh2D_node" ) )
    mMesh2dName = "Mesh2D"; // D-Flow 1.2.x
  else
    throw MDAL_Status::Err_UnknownFormat;

  // 2D Mesh
  ncFile.getDimension( "n" + mMesh2dName + "_node", &count, &ncid );
  dims.setDimension( CFDimensions::Vertex2D, count, ncid );

  ncFile.getDimension( "n" + mMesh2dName + "_face", &count, &ncid );
  dims.setDimension( CFDimensions::Face2D, count, ncid );

  ncFile.getDimension( "n" + mMesh2dName + "_edge", &count, &ncid );
  dims.setDimension( CFDimensions::Face2DEdge, count, ncid );

  ncFile.getDimension( "max_n" + mMesh2dName + "_face_nodes", &count, &ncid );
  dims.setDimension( CFDimensions::MaxVerticesInFace, count, ncid );

  // Time
  ncFile.getDimension( "time", &count, &ncid );
  dims.setDimension( CFDimensions::Time, count, ncid );

  return dims;
}

void MDAL::DriverUgrid::populateFacesAndVertices( Vertices &vertices, Faces &faces )
{
  populateVertices( vertices );
  populateFaces( faces );
}

void MDAL::DriverUgrid::populateVertices( MDAL::Vertices &vertices )
{
  assert( vertices.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex2D );
  vertices.resize( vertexCount );
  Vertex *vertexPtr = vertices.data();

  // 2D
  std::vector<double> vertices2D_x = mNcFile.readDoubleArr( mMesh2dName + "_node_x", vertexCount );
  std::vector<double> vertices2D_y = mNcFile.readDoubleArr( mMesh2dName + "_node_y", vertexCount );
  for ( size_t i = 0; i < vertexCount; ++i, ++vertexPtr )
  {
    vertexPtr->x = vertices2D_x[i];
    vertexPtr->y = vertices2D_y[i];
  }
}

void MDAL::DriverUgrid::populateFaces( MDAL::Faces &faces )
{
  assert( faces.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face2D );
  faces.resize( faceCount );

  // 2D
  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  int fill_val = mNcFile.getAttrInt( mMesh2dName + "_face_nodes", "_FillValue" );
  int start_index = mNcFile.getAttrInt( mMesh2dName + "_face_nodes", "start_index" );
  std::vector<int> face_nodes_conn = mNcFile.readIntArr( mMesh2dName + "_face_nodes", faceCount * verticesInFace );

  for ( size_t i = 0; i < faceCount; ++i )
  {
    size_t nVertices = verticesInFace;
    std::vector<size_t> idxs;

    for ( size_t j = 0; j < verticesInFace; ++j )
    {
      size_t idx = verticesInFace * i + j;
      int val = face_nodes_conn[idx];

      if ( fill_val == val )
      {
        // found fill val
        nVertices = j;
        assert( nVertices > 1 );
        break;
      }
      else
      {
        idxs.push_back( static_cast<size_t>( val - start_index ) );
      }
    }
    faces[i] = idxs;
  }

}

void MDAL::DriverUgrid::addBedElevation( MDAL::Mesh *mesh )
{
  MDAL_UNUSED( mesh );
}

std::string MDAL::DriverUgrid::getCoordinateSystemVariableName()
{
  std::string coordinate_system_variable;

  // first try to get the coordinate system variable from grid definition
  if ( mNcFile.hasArr( mMesh2dName + "_node_z" ) )
  {
    coordinate_system_variable = mNcFile.getAttrStr( mMesh2dName + "_node_z", "grid_mapping" );
  }

  // if automatic discovery fails, try to check some hardcoded common variables that store projection
  if ( coordinate_system_variable.empty() )
  {
    if ( mNcFile.hasArr( "projected_coordinate_system" ) )
      coordinate_system_variable = "projected_coordinate_system";
    else if ( mNcFile.hasArr( "wgs84" ) )
      coordinate_system_variable = "wgs84";
  }

  // return, may be empty
  return coordinate_system_variable;
}

std::set<std::string> MDAL::DriverUgrid::ignoreNetCDFVariables()
{
  std::set<std::string> ignore_variables;

  ignore_variables.insert( "projected_coordinate_system" );
  ignore_variables.insert( "time" );
  ignore_variables.insert( "timestep" );

  std::vector<std::string> meshes;
  meshes.push_back( "mesh1d" );
  meshes.push_back( "mesh2d" );
  meshes.push_back( "Mesh2D" );
  meshes.push_back( "Mesh1D" );

  for ( const std::string &mesh : meshes )
  {
    ignore_variables.insert( mesh );
    ignore_variables.insert( mesh + "_face_nodes" );
    ignore_variables.insert( mesh + "_face_x" );
    ignore_variables.insert( mesh + "_face_y" );
    ignore_variables.insert( mesh + "_face_x_bnd" );
    ignore_variables.insert( mesh + "_face_y_bnd" );
    ignore_variables.insert( mesh + "_flowelem_ba" );
    ignore_variables.insert( mesh + "_node_x" );
    ignore_variables.insert( mesh + "_node_y" );
    ignore_variables.insert( mesh + "_edge_nodes" );
    ignore_variables.insert( mesh + "_edge_x" );
    ignore_variables.insert( mesh + "_edge_y" );
    ignore_variables.insert( mesh + "_edge_x_bnd" );
    ignore_variables.insert( mesh + "_edge_y_bnd" );
    ignore_variables.insert( mesh + "_edge_type" );
  }

  return ignore_variables;
}

std::string MDAL::DriverUgrid::nameSuffix( MDAL::CFDimensions::Type type )
{
  MDAL_UNUSED( type );
  return "";
}

void MDAL::DriverUgrid::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
{
  *is_vector = false;
  *is_x = true;

  std::string long_name = mNcFile.getAttrStr( "long_name", varid );
  if ( long_name.empty() )
  {
    std::string standard_name = mNcFile.getAttrStr( "standard_name", varid );
    if ( standard_name.empty() )
    {
      name = variableName;
    }
    else
    {
      if ( MDAL::contains( standard_name, "_x_" ) )
      {
        *is_vector = true;
        name = MDAL::replace( standard_name, "_x_", "" );
      }
      else if ( MDAL::contains( standard_name, "_y_" ) )
      {
        *is_vector = true;
        *is_x = false;
        name = MDAL::replace( standard_name, "_y_", "" );
      }
      else
      {
        name = standard_name;
      }
    }
  }
  else
  {
    if ( MDAL::contains( long_name, ", x-component" ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, ", x-component", "" );
    }
    else if ( MDAL::contains( long_name, ", y-component" ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, ", y-component", "" );
    }
    else
    {
      name = long_name;
    }
  }
}
