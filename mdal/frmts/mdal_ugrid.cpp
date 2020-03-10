/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_ugrid.hpp"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#include <netcdf.h>
#include <assert.h>
#include <algorithm>
#include <cmath>

MDAL::DriverUgrid::DriverUgrid()
  : DriverCF(
      "Ugrid",
      "UGRID Results",
      "*.nc",
      Capability::ReadMesh | Capability::SaveMesh )
{

}

MDAL::DriverUgrid *MDAL::DriverUgrid::create()
{
  return new DriverUgrid();
}

std::vector<std::string> MDAL::DriverUgrid::findMeshesNames() const
{
  std::vector<std::string> meshes_in_file;

  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const auto &var : variables )
  {
    bool is_mesh_topology = mNcFile->getAttrStr( var, "cf_role" ) == "mesh_topology";
    if ( is_mesh_topology )
    {
      // file can include more meshes
      meshes_in_file.push_back( var );
    }
  }

  return meshes_in_file;
}

std::string MDAL::DriverUgrid::nodeZVariableName() const
{
  const std::vector<std::string> variables = mNcFile->readArrNames();
  for ( const std::string &varName : variables )
  {
    const std::string stdName = mNcFile->getAttrStr( varName, "standard_name" );
    const std::string meshName = mNcFile->getAttrStr( varName, "mesh" );
    const std::string location = mNcFile->getAttrStr( varName, "location" );

    if ( stdName == "altitude" && meshName == mMeshName && location == "node" )
    {
      return varName;
    }
  }

  // not found, the file in non UGRID standard conforming,
  // but lets try the common name
  return mMeshName + "_node_z";
}

MDAL::CFDimensions MDAL::DriverUgrid::populateDimensions( )
{
  CFDimensions dims;
  size_t count;
  int ncid;

  /* getting mesh name */
  std::vector<std::string> mAllMeshesNames = findMeshesNames();

  if ( mAllMeshesNames.empty() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, name(), "Did not find any mesh name in file" );
  if ( mAllMeshesNames.size() == 1 )
    mMeshName = mAllMeshesNames.at( 0 );
  else // there are more meshes in file
  {
    if ( MDAL::contains( mAllMeshesNames.at(0), "network" ) ) // ignore the network variable for a moment
      mMeshName = mAllMeshesNames.at( 1 );
    else
      mMeshName = mAllMeshesNames.at( 0 );

    MDAL::Log::warning( MDAL_Status::Warn_MultipleMeshesInFile, name(), "Found multiple meshes in file, working with: " + mMeshName );
  }

  if ( mMeshName.empty() ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Unable to parse mesh name from file" );

  /* set the corresponding topology dimension */
  mMeshDimension = mNcFile->getAttrInt( mMeshName, "topology_dimension" );

  if ( ( mMeshDimension < 1 ) || ( mMeshDimension > 2 ) )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, name(), "Unable to parse topology dimension from mesh or mesh is 3D" );

  MDAL::Log::info( "Parsing " + std::to_string( mMeshDimension ) + "D mesh" );

  /* node dimensions - learn many nodes are in mesh */
  std::string nodeXVariable, nodeYVariable;

  if ( mMeshDimension == 1)
    parseNodeCoordinatesFrom1dMesh( mMeshName, nodeXVariable, nodeYVariable);
  else
    parse2VariablesFromAttribute( mMeshName, "node_coordinates", nodeXVariable, nodeYVariable, false );

  std::vector<size_t> nodeDimension;
  std::vector<int> nodeDimensionId;
  mNcFile->getDimensions( nodeXVariable, nodeDimension, nodeDimensionId );

  if ( nodeDimension.size() != 1 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while parsing dimensions" );

  dims.setDimension( CFDimensions::Vertex, nodeDimension.at( 0 ), nodeDimensionId.at( 0 ) );

  /* continue parsing dimension dependent variables */
  if ( mMeshDimension == 1 )
    populate1DMeshDimensions( dims );
  else
    populate2DMeshDimensions( dims, ncid );

  /* Time variable - not required for UGRID format */
  if ( mNcFile->hasDimension( "time" ) )
  {
    mNcFile->getDimension( "time", &count, &ncid );
    dims.setDimension( CFDimensions::Time, count, ncid );
  }
  else
  {
    dims.setDimension( CFDimensions::Time, 0, -1 );
  }

  return dims;
}

void MDAL::DriverUgrid::populate1DMeshDimensions( MDAL::CFDimensions &dims )
{
  /* Parse number of edges ( dimension ) from mesh */
  std::string edgeConnectivityVariableName = mNcFile->getAttrStr( mMeshName, "edge_node_connectivity" );
  if ( edgeConnectivityVariableName.empty() )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Did not find edge node connectivity attribute" );

  std::vector<size_t> edgeDimension;
  std::vector<int> edgeDimensionId;
  mNcFile->getDimensions( edgeConnectivityVariableName, edgeDimension, edgeDimensionId );
  if ( edgeDimension.size() != 2 )
    throw MDAL::Error( MDAL_Status::Err_InvalidData, name(), "Unable to parse dimensions for edge_nodes_connectivity variable" );

  size_t edgesCount = edgeDimension.at(0); // Only interested in first value, edge will always have only 2 nodes
  int edgesCountId = edgeDimensionId.at(0);

  dims.setDimension( CFDimensions::Edge, edgesCount, edgesCountId );
}

void MDAL::DriverUgrid::populate2DMeshDimensions( MDAL::CFDimensions &dims, int &ncid )
{
  // face dimension location is retrieved from the face_node_connectivity variable
  // if face_dimension is defined as attribute, the dimension at this location help to desambiguate vertex per faces and number of faces
  std::string faceConnectivityVariablesName = mNcFile->getAttrStr( mMeshName, "face_node_connectivity" );
  std::string faceDimensionLocation = mNcFile->getAttrStr( mMeshName, "face_dimension" );
  if ( faceConnectivityVariablesName == "" )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Did not find face connectivity attribute" );

  size_t facesCount;
  size_t maxVerticesPerFace;
  size_t count;

  std::vector<size_t> faceDimension;
  std::vector<int> faceDimensionId;
  int facesIndexDimensionId;
  int maxVerticesPerFaceDimensionId;
  mNcFile->getDimensions( faceConnectivityVariablesName, faceDimension, faceDimensionId );
  if ( faceDimension.size() != 2 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Face dimension is 2D" );

  // if face_dimension is not present in file, get it from dimension element
  if ( faceDimensionLocation != "" )
  {
    MDAL::Log::info( "face_dimension attribute not present in mesh, trying to parse it from dimensions" );
    mNcFile->getDimension( faceDimensionLocation, &facesCount, &ncid );
    if ( facesCount == faceDimension.at( 0 ) )
    {
      facesIndexDimensionId = faceDimensionId.at( 0 );
      maxVerticesPerFaceDimensionId = faceDimensionId.at( 1 );
      maxVerticesPerFace = faceDimension.at( 1 );
    }
    else
    {
      facesIndexDimensionId = faceDimensionId.at( 1 );
      maxVerticesPerFaceDimensionId = faceDimensionId.at( 0 );
      maxVerticesPerFace = faceDimension.at( 0 );
    }
  }
  else
  {
    facesIndexDimensionId = faceDimensionId.at( 0 );
    facesCount = faceDimension.at( 0 );
    maxVerticesPerFaceDimensionId = faceDimensionId.at( 1 );
    maxVerticesPerFace = faceDimension.at( 1 );
  }

  dims.setDimension( CFDimensions::Face, facesCount, facesIndexDimensionId );
  dims.setDimension( CFDimensions::MaxVerticesInFace, maxVerticesPerFace, maxVerticesPerFaceDimensionId );

  // number of edges in the mesh, not required for UGRID format
  const std::string mesh2dEdge = mNcFile->getAttrStr( mMeshName, "edge_dimension" );
  if ( mNcFile->hasDimension( mesh2dEdge ) )
  {
    mNcFile->getDimension( mesh2dEdge, &count, &ncid );
    dims.setDimension( CFDimensions::Face2DEdge, count, ncid );
  }
  else
  {
    dims.setDimension( CFDimensions::Face2DEdge, 0, -1 );
  }
}

void MDAL::DriverUgrid::populateElements( Vertices &vertices, Edges& edges, Faces &faces )
{
  populateVertices( vertices );

  if ( mMeshDimension == 1 )
    populateEdges( edges ); // 1D mesh
  else
    populateFaces( faces ); // 2D mesh
}

void MDAL::DriverUgrid::populateVertices( MDAL::Vertices &vertices )
{
  assert( vertices.empty() );
  size_t vertexCount = mDimensions.size( CFDimensions::Vertex );
  vertices.resize( vertexCount );
  Vertex *vertexPtr = vertices.data();

  // node_coordinates should be something like Mesh2D_node_x Mesh2D_node_y
  std::string verticesXName, verticesYName;
  if ( mMeshDimension == 1 )
    parseNodeCoordinatesFrom1dMesh( mMeshName, verticesXName, verticesYName );
  else
    parse2VariablesFromAttribute( mMeshName, "node_coordinates", verticesXName, verticesYName, false );

  const std::vector<double> vertices_x = mNcFile->readDoubleArr( verticesXName, vertexCount );
  const std::vector<double> vertices_y = mNcFile->readDoubleArr( verticesYName, vertexCount );

  std::vector<double> vertices_z;
  if ( mNcFile->hasArr( nodeZVariableName() ) )
  {
    vertices_z = mNcFile->readDoubleArr( nodeZVariableName(), vertexCount );
  }

  for ( size_t i = 0; i < vertexCount; ++i, ++vertexPtr )
  {
    vertexPtr->x = vertices_x[i];
    vertexPtr->y = vertices_y[i];
    if ( !vertices_z.empty() )
      vertexPtr->z = vertices_z[i];
  }
}

void MDAL::DriverUgrid::populateEdges( MDAL::Edges &edges )
{
  assert( edges.empty() );

  // number of edges
  size_t edgesCount = mDimensions.size( CFDimensions::Edge );
  edges.resize( edgesCount );

  const std::string edgeNodeConnectivityVar = mNcFile->getAttrStr( mMeshName, "edge_node_connectivity" );
  if ( edgeNodeConnectivityVar.empty() )
    MDAL::Log::error(MDAL_Status::Err_MissingDriver, "Unable to find edge_node_connectivity attribute of " + mMeshName);

  // load edges
  std::vector<int> edge_nodes_idxs = mNcFile->readIntArr( edgeNodeConnectivityVar, edgesCount * 2); // two nodes per edge
  int start_index = mNcFile->getAttrInt( edgeNodeConnectivityVar, "start_index" );

  // iterate over all edge_nodes coordinates - those are indexes for nodes
  for (size_t edge_i = 0; edge_i < edgesCount; ++edge_i ) {
    std::vector<size_t> edge_i_inxs;

    int start_edge_ix = edge_i * 2;
    int end_edge_ix = edge_i * 2 + 1;

    edges[edge_i].startVertex = edge_nodes_idxs[start_edge_ix] - start_index;
    edges[edge_i].endVertex = edge_nodes_idxs[end_edge_ix] - start_index;
  }
}

void MDAL::DriverUgrid::populateFaces( MDAL::Faces &faces )
{
  assert( faces.empty() );
  size_t faceCount = mDimensions.size( CFDimensions::Face );
  faces.resize( faceCount );

  // Parse 2D Mesh
  // face_node_connectivity is usually something like Mesh2D_face_nodes
  const std::string mesh2dFaceNodeConnectivity = mNcFile->getAttrStr( mMeshName, "face_node_connectivity" );

  size_t verticesInFace = mDimensions.size( CFDimensions::MaxVerticesInFace );
  int fill_val = -1;
  if ( mNcFile->hasAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" ) )
    fill_val = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "_FillValue" );
  int start_index = mNcFile->getAttrInt( mesh2dFaceNodeConnectivity, "start_index" );
  std::vector<int> face_nodes_conn = mNcFile->readIntArr( mesh2dFaceNodeConnectivity, faceCount * verticesInFace );

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

void MDAL::DriverUgrid::addBedElevation( MDAL::MemoryMesh *mesh )
{
  if ( mNcFile->hasArr( nodeZVariableName() ) ) MDAL::addBedElevationDatasetGroup( mesh, mesh->vertices );
}

std::string MDAL::DriverUgrid::getCoordinateSystemVariableName()
{
  std::string coordinate_system_variable;

  // first try to get the coordinate system variable from grid definition
  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( mMeshName, "node_coordinates" ), ' ' );
  if ( nodeVariablesName.size() > 1 )
  {
    if ( mNcFile->hasArr( nodeVariablesName[0] ) )
    {
      coordinate_system_variable = mNcFile->getAttrStr( nodeVariablesName[0], "grid_mapping" );
    }
  }


  // if automatic discovery fails, try to check some hardcoded common variables that store projection
  if ( coordinate_system_variable.empty() )
  {
    if ( mNcFile->hasArr( "projected_coordinate_system" ) )
      coordinate_system_variable = "projected_coordinate_system";
    else if ( mNcFile->hasArr( "wgs84" ) )
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
  if ( mNcFile->hasArr( mMesh1dName ) )
    meshes.push_back( mMesh1dName );
  meshes.push_back( mMesh2dName );

  for ( const std::string &mesh : meshes )
  {
    std::string xName, yName;
    ignore_variables.insert( mesh );
    parse2VariablesFromAttribute( mesh, "node_coordinates", xName, yName, true );
    ignore_variables.insert( xName );
    ignore_variables.insert( yName );
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "edge_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "edge_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "face_node_connectivity" ) );
    parse2VariablesFromAttribute( mesh, "face_coordinates", xName, yName, true );
    if ( !xName.empty() )
    {
      ignore_variables.insert( xName );
      ignore_variables.insert( mNcFile->getAttrStr( xName, "bounds" ) );
    }
    if ( !yName.empty() )
    {
      ignore_variables.insert( yName );
      ignore_variables.insert( mNcFile->getAttrStr( yName, "bounds" ) );
    }
    ignore_variables.insert( mNcFile->getAttrStr( mesh, "edge_face_connectivity" ) );
  }

  return ignore_variables;
}

void MDAL::DriverUgrid::parseNetCDFVariableMetadata( int varid, const std::string &variableName, std::string &name, bool *is_vector, bool *is_x )
{
  *is_vector = false;
  *is_x = true;

  std::string long_name = mNcFile->getAttrStr( "long_name", varid );
  if ( long_name.empty() )
  {
    std::string standard_name = mNcFile->getAttrStr( "standard_name", varid );
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
    if ( MDAL::contains( long_name, ", x-component" ) || MDAL::contains( long_name, "u component of " ) )
    {
      *is_vector = true;
      name = MDAL::replace( long_name, ", x-component", "" );
      name = MDAL::replace( name, "u component of ", "" );
    }
    else if ( MDAL::contains( long_name, ", y-component" ) || MDAL::contains( long_name, "v component of " ) )
    {
      *is_vector = true;
      *is_x = false;
      name = MDAL::replace( long_name, ", y-component", "" );
      name = MDAL::replace( name, "v component of ", "" );
    }
    else
    {
      name = long_name;
    }
  }
}

std::string MDAL::DriverUgrid::getTimeVariableName() const
{
  return "time";
}

void MDAL::DriverUgrid::parseNodeCoordinatesFrom1dMesh( const std::string &meshName, std::string &var1, std::string &var2 )
{
  std::vector<std::string> nodeVariablesName = MDAL::split( mNcFile->getAttrStr( meshName, "node_coordinates" ), ' ' );

  if ( nodeVariablesName.size() < 2 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error while parsing node coordinates" );
  else if ( nodeVariablesName.size() > 3 )
  {
    MDAL::Log::warning( MDAL_Status::Warn_InvalidElements, name(),
                        "Node coordinates consists of more than 3 variables, taking variable with _x in name by default" );

    for ( const auto &nodeVar : nodeVariablesName )
    {
      if ( MDAL::contains( nodeVar, "_x" ) )
      {
        var1 = nodeVar;
      } else if ( MDAL::contains( nodeVar, "_y" ) )
      {
        var2 = nodeVar;
      }
    }
    if ( var1.empty() || var2.empty() )
      throw MDAL::Error( MDAL_Status::Err_InvalidData, name(), "Could not parse node coordinates from mesh" );
  }
  else // 2 variables as node coordinates
  {
    var1 = nodeVariablesName.at( 0 );
    var2 = nodeVariablesName.at( 1 );
  }
}

void MDAL::DriverUgrid::parse2VariablesFromAttribute( const std::string &name, const std::string &attr_name,
    std::string &var1, std::string &var2, bool optional ) const
{
  const std::string mesh2dNodeCoordinates = mNcFile->getAttrStr( name, attr_name );
  const std::vector<std::string> chunks = MDAL::split( mesh2dNodeCoordinates, ' ' );

  if ( chunks.size() != 2 )
  {
    if ( optional )
    {
      var1 = "";
      var2 = "";
    }
    else
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to parse variables from attribute" );
  }
  else
  {
    var1 = chunks[0];
    var2 = chunks[1];
  }
}

void MDAL::DriverUgrid::save( const std::string &uri, MDAL::Mesh *mesh )
{
  mFileName = uri;

  try
  {
    // Create file
    mNcFile.reset( new NetCDFFile );
    mNcFile->createFile( mFileName );

    // Write globals
    writeGlobals( );

    // Write variables
    writeVariables( mesh );
  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "could not save file " + uri );
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
  }
}


void MDAL::DriverUgrid::writeVariables( MDAL::Mesh *mesh )
{
  // Global dimensions
  int dimNodeCountId = mNcFile->defineDimension( "nmesh2d_node", mesh->verticesCount() );
  int dimFaceCountId = mNcFile->defineDimension( "nmesh2d_face", mesh->facesCount() );
  mNcFile->defineDimension( "nmesh2d_edge", 1 ); // no data on edges, cannot be 0, since 0==NC_UNLIMITED
  int dimTimeId = mNcFile->defineDimension( "time", NC_UNLIMITED );
  int dimMaxNodesPerFaceId = mNcFile->defineDimension( "max_nmesh2d_face_nodes",
                             mesh->faceVerticesMaximumCount() );

  // Mesh 2D Definition
  int mesh2dId = mNcFile->defineVar( "mesh2d", NC_INT, 0, nullptr );
  mNcFile->putAttrStr( mesh2dId, "cf_role", "mesh_topology" );
  mNcFile->putAttrStr( mesh2dId, "long_name", "Topology data of 2D network" );
  mNcFile->putAttrInt( mesh2dId, "topology_dimension", 2 );
  mNcFile->putAttrStr( mesh2dId, "node_coordinates", "mesh2d_node_x mesh2d_node_y" );
  mNcFile->putAttrStr( mesh2dId, "node_dimension", "nmesh2d_node" );
  mNcFile->putAttrStr( mesh2dId, "edge_dimension", "nmesh2d_edge" );
  mNcFile->putAttrStr( mesh2dId, "max_face_nodes_dimension", "max_nmesh2d_face_nodes" );
  mNcFile->putAttrStr( mesh2dId, "face_node_connectivity", "mesh2d_face_nodes" );
  mNcFile->putAttrStr( mesh2dId, "face_dimension", "nmesh2d_face" );

  // Nodes X coordinate
  int mesh2dNodeXId = mNcFile->defineVar( "mesh2d_node_x", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeXId, "standard_name", "projection_x_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeXId, "long_name", "x-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeXId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeXId, "location", "node" );

  // Nodes Y coordinate
  int mesh2dNodeYId = mNcFile->defineVar( "mesh2d_node_y", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeYId, "standard_name", "projection_y_coordinate" );
  mNcFile->putAttrStr( mesh2dNodeYId, "long_name", "y-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeYId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeYId, "location", "node" );

  // Nodes Z coordinate
  int mesh2dNodeZId = mNcFile->defineVar( "mesh2d_node_z", NC_DOUBLE, 1, &dimNodeCountId );
  mNcFile->putAttrStr( mesh2dNodeZId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2dNodeZId, "location", "node" );
  mNcFile->putAttrStr( mesh2dNodeZId, "coordinates", "mesh2d_node_x mesh2d_node_y" );
  mNcFile->putAttrStr( mesh2dNodeZId, "standard_name", "altitude" );
  mNcFile->putAttrStr( mesh2dNodeZId, "long_name", "z-coordinate of mesh nodes" );
  mNcFile->putAttrStr( mesh2dNodeZId, "grid_mapping", "projected_coordinate_system" );
  double fillNodeZCoodVal = -999.0;
  mNcFile->putAttrDouble( mesh2dNodeZId, "_FillValue", fillNodeZCoodVal );

  // Faces 2D Variable
  int mesh2FaceNodesId_dimIds [] { dimFaceCountId, dimMaxNodesPerFaceId };
  int mesh2FaceNodesId = mNcFile->defineVar( "mesh2d_face_nodes", NC_INT, 2, mesh2FaceNodesId_dimIds );
  mNcFile->putAttrStr( mesh2FaceNodesId, "cf_role", "face_node_connectivity" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "mesh", "mesh2d" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "location", "face" );
  mNcFile->putAttrStr( mesh2FaceNodesId, "long_name", "Mapping from every face to its corner nodes (counterclockwise)" );
  mNcFile->putAttrInt( mesh2FaceNodesId, "start_index", 0 );
  int fillFace2DVertexValue = -999;
  mNcFile->putAttrInt( mesh2FaceNodesId, "_FillValue", fillFace2DVertexValue );

  // Projected Coordinate System
  int pcsId = mNcFile->defineVar( "projected_coordinate_system", NC_INT, 0, nullptr );

  if ( mesh->crs() == "" )
  {
    mNcFile->putAttrInt( pcsId, "epsg", 0 );
    mNcFile->putAttrStr( pcsId, "EPSG_code", "epgs:0" );
  }
  else
  {
    std::vector<std::string> words = MDAL::split( mesh->crs(), ":" );

    if ( words[0] == "EPSG" && words.size() > 1 )
    {
      mNcFile->putAttrInt( pcsId, "epsg", std::stoi( words[1] ) );
      mNcFile->putAttrStr( pcsId, "EPSG_code", mesh->crs() );
    }
    else
    {
      mNcFile->putAttrStr( pcsId, "wkt", mesh->crs() );
    }
  }

  // Time array
  int timeId = mNcFile->defineVar( "time", NC_DOUBLE, 1, &dimTimeId );
  mNcFile->putAttrStr( timeId, "units", "hours since 2000-01-01 00:00:00" );

  // Turning off define mode - allows data write
  nc_enddef( mNcFile->handle() );

  // Write vertices

  const size_t maxBufferSize = 1000;
  const size_t bufferSize = std::min( mesh->verticesCount(), maxBufferSize );
  const size_t verticesCoordCount = bufferSize * 3;

  std::vector<double> verticesCoordinates( verticesCoordCount );
  std::unique_ptr<MDAL::MeshVertexIterator> vertexIterator = mesh->readVertices();

  {
    size_t vertexIndex = 0;
    size_t vertexFileIndex = 0;
    while ( vertexIndex < mesh->verticesCount() )
    {
      size_t verticesRead = vertexIterator->next( bufferSize, verticesCoordinates.data() );
      if ( verticesRead == 0 )
        break;

      for ( size_t i = 0; i < verticesRead; i++ )
      {
        mNcFile->putDataDouble( mesh2dNodeXId, vertexFileIndex, verticesCoordinates[3 * i] );
        mNcFile->putDataDouble( mesh2dNodeYId, vertexFileIndex, verticesCoordinates[3 * i + 1] );
        if ( std::isnan( verticesCoordinates[3 * i + 2] ) )
          mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, fillNodeZCoodVal );
        else
          mNcFile->putDataDouble( mesh2dNodeZId, vertexFileIndex, verticesCoordinates[3 * i + 2] );
        vertexFileIndex++;
      }
      vertexIndex += verticesRead;
    }
  }

  // Write faces
  std::unique_ptr<MDAL::MeshFaceIterator> faceIterator = mesh->readFaces();
  const size_t faceVerticesMax = mesh->faceVerticesMaximumCount();
  const size_t facesCount = mesh->facesCount();
  const size_t faceOffsetsBufferLen = std::min( facesCount, maxBufferSize );
  const size_t vertexIndicesBufferLen = faceOffsetsBufferLen * faceVerticesMax;

  std::vector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  std::vector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  size_t faceIndex = 0;
  while ( faceIndex < facesCount )
  {
    size_t facesRead = faceIterator->next(
                         faceOffsetsBufferLen,
                         faceOffsetsBuffer.data(),
                         vertexIndicesBufferLen,
                         vertexIndicesBuffer.data() );
    if ( facesRead == 0 )
      break;

    for ( size_t i = 0; i < facesRead; i++ )
    {
      std::vector<int> verticesFaceData( faceVerticesMax, fillFace2DVertexValue );
      int startIndex = 0;
      if ( i > 0 )
        startIndex = faceOffsetsBuffer[ i - 1 ];
      int endIndex = faceOffsetsBuffer[ i ];

      size_t k = 0;
      for ( int j = startIndex; j < endIndex; ++j )
      {
        int vertexIndex = vertexIndicesBuffer[ static_cast<size_t>( j ) ];
        verticesFaceData[k++] = vertexIndex;
      }
      mNcFile->putDataArrayInt( mesh2FaceNodesId, faceIndex + i, faceVerticesMax, verticesFaceData.data() );
    }
    faceIndex += facesRead;
  }

  // Time values (not implemented)
  mNcFile->putDataDouble( timeId, 0, 0.0 );

  // Turning on define mode
  nc_redef( mNcFile->handle() );
}

void MDAL::DriverUgrid::writeGlobals()
{
  mNcFile->putAttrStr( NC_GLOBAL, "source", "MDAL " + std::string( MDAL_Version() ) );
  mNcFile->putAttrStr( NC_GLOBAL, "date_created", MDAL::getCurrentTimeStamp() );
  mNcFile->putAttrStr( NC_GLOBAL, "Conventions", "CF-1.6 UGRID-1.0" );
}
