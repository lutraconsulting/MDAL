/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"



TEST( MeshPlyTest, WrongFiles )
{
  for ( int i = 0; i < 4; ++i )
  {
    std::string fileName = test_file( "/ply/invalid/invalid" + std::to_string( i ) + ".ply" );

    if ( i == 0 )
      EXPECT_TRUE( std::string( MDAL_MeshNames( fileName.c_str() ) ).empty() );
    else
      EXPECT_EQ( MDAL_MeshNames( fileName.c_str() ), "PLY:\"" + fileName + "\"" );

    MDAL_MeshH m = MDAL_LoadMesh( fileName.c_str() );
    EXPECT_EQ( m, nullptr );
  }
}

TEST( MeshPlyTest, all_features )
{
  std::string path = test_file( "/ply/all_features.ply" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "PLY:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "PLY" );

  std::string proj = MDAL_M_projection( m );
  EXPECT_EQ( proj, "+proj=tmerc" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  double x = getVertexXCoordinatesAt( m, 2 );
  double y = getVertexYCoordinatesAt( m, 2 );
  double z = getVertexZCoordinatesAt( m, 2 );
  EXPECT_DOUBLE_EQ( 10, x );
  EXPECT_DOUBLE_EQ( 10, y );
  EXPECT_DOUBLE_EQ( 0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count );
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 3, f_v );

  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 6, e_count );

  std::vector<int> start;
  start.resize( e_count );
  std::vector<int> end;
  end.resize( e_count );

  getEdgeVertexIndices( m, e_count, start, end );
  EXPECT_EQ( start[0], 0 );
  EXPECT_EQ( end[0], 1 );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 10, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 20, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 11, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 10, value );

  //test vertex data

  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "red" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );

  //test vertex vector data

  g = MDAL_M_datasetGroup( m, 4 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "vertex_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 102, value );


  //test face data

  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "m" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2000, value );

  //test face vector data

  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "face_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  //test edge data

  g = MDAL_M_datasetGroup( m, 9 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "blue_edge" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );


  //test edge vector data

  g = MDAL_M_datasetGroup( m, 10 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "edge_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 102, value );
  MDAL_CloseMesh( m );
}


// test the alternative element order
TEST( MeshPlyInvTest, all_features_inv )
{
  std::string path = test_file( "/ply/all_features_inv.ply" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "PLY:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "PLY" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  double x = getVertexXCoordinatesAt( m, 2 );
  double y = getVertexYCoordinatesAt( m, 2 );
  double z = getVertexZCoordinatesAt( m, 2 );
  EXPECT_DOUBLE_EQ( 10, x );
  EXPECT_DOUBLE_EQ( 10, y );
  EXPECT_DOUBLE_EQ( 0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count );
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 3, f_v );

  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 6, e_count );

  std::vector<int> start;
  start.resize( e_count );
  std::vector<int> end;
  end.resize( e_count );

  getEdgeVertexIndices( m, e_count, start, end );

  EXPECT_EQ( start[0], 0 );
  EXPECT_EQ( end[0], 1 );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 10, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 20, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 11, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 10, value );

  //test vertex data

  g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "red" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );

//test vertex vector data

  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "vertex_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 201, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 202, value );

  //test face data

  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "m" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2000, value );

  //test face vector data

  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "face_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 200, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 201, value );


  //test edge data

  g = MDAL_M_datasetGroup( m, 10 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "blue_edge" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );

  //test edge vector data

  g = MDAL_M_datasetGroup( m, 7 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "edge_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 200, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 201, value );
  MDAL_CloseMesh( m );
}

TEST( MeshPlyTest, all_features_binary )
{
  std::string path = test_file( "/ply/all_features_binary.ply" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "PLY:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "PLY" );

  std::string proj = MDAL_M_projection( m );
  EXPECT_EQ( proj, "+proj=tmerc" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  double x = getVertexXCoordinatesAt( m, 2 );
  double y = getVertexYCoordinatesAt( m, 2 );
  double z = getVertexZCoordinatesAt( m, 2 );
  EXPECT_DOUBLE_EQ( 10, x );
  EXPECT_DOUBLE_EQ( 10, y );
  EXPECT_DOUBLE_EQ( 0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count );
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 3, f_v );

  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 6, e_count );

  std::vector<int> start;
  start.resize( e_count );
  std::vector<int> end;
  end.resize( e_count );

  getEdgeVertexIndices( m, e_count, start, end );
  EXPECT_EQ( start[0], 0 );
  EXPECT_EQ( end[0], 1 );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 10, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 20, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 11, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 10, value );

  //test vertex data

  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "red" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );

  //test vertex vector data

  g = MDAL_M_datasetGroup( m, 4 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "vertex_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 102, value );


  //test face data

  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "m" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2000, value );

  //test face vector data

  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "face_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  //test edge data

  g = MDAL_M_datasetGroup( m, 9 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "blue_edge" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 255, value );


  //test edge vector data

  g = MDAL_M_datasetGroup( m, 10 );
  ASSERT_NE( g, nullptr );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "edge_vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 101, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 102, value );
  MDAL_CloseMesh( m );
}

// test the alternative element order
TEST( MeshPlyFileTest, real_file )
{
  std::string path = test_file( "/ply/test_mesh.ply" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "PLY:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 3 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "PLY" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 38487 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 76294, f_count );

  MDAL_CloseMesh( m );
}


TEST( MeshPlyTest, Save2DMeshToFile )
{
  saveAndCompareMesh(
    test_file( "/ply/all_features.ply" ),
    tmp_file( "/all_features_saved.ply" ),
    "PLY"
  );
}

TEST( MeshPlyTest, Save2DMeshToFileBinary )
{
  saveAndCompareMesh(
    test_file( "/ply/all_features_binary.ply" ),
    tmp_file( "/all_features_binary_saved.ply" ),
    "PLY"
  );
}

TEST( MeshPlyTest, TestSavePrecsion )
{
  saveAndCompareMesh(
    test_file( "/ply/test_precision.ply" ),
    tmp_file( "/test_precision_saved.ply" ),
    "PLY"
  );
}

// test saving dataset
TEST( MeshPlyTest, Save2DDataset )
{
  std::string path = test_file( "/ply/all_features.ply" );
  MDAL_MeshH mesh = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( mesh, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( mesh );
  int f_count = MDAL_M_faceCount( mesh );

  MDAL_MeshH mesh2 = MDAL_CreateMesh( MDAL_driverFromName( "PLY" ) );

  std::vector<double> vertices( v_count * 3 );
  MDAL_MeshVertexIteratorH vi = MDAL_M_vertexIterator( mesh );
  MDAL_MeshFaceIteratorH fi = MDAL_M_faceIterator( mesh );
  MDAL_VI_next( vi, v_count, vertices.data() );
  MDAL_M_addVertices( mesh2, v_count, vertices.data() );
  std::vector<int> face( MDAL_M_faceVerticesMaximumCount( mesh ) );
  int offset;
  for ( int i = 0; i < f_count; i++ )
  {
    MDAL_FI_next( fi, 1, &offset, face.size(), face.data() );
    MDAL_M_addFaces( mesh2, 1, &offset, face.data() );
  }

  MDAL_SaveMesh( mesh2, tmp_file( "/dg2d.ply" ).c_str(), "PLY" );

  for ( int i = 0; i < MDAL_M_datasetGroupCount( mesh ); i++ )
  {
    MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mesh, i );
    if ( MDAL_G_dataLocation( group ) == MDAL_DataLocation::DataOnEdges ) break;

    // create a new 3D datasetGroup
    int index = MDAL_M_datasetGroupCount( mesh2 );
    MDAL_M_addDatasetGroup( mesh2,
                            MDAL_G_name( group ),
                            MDAL_G_dataLocation( group ),
                            MDAL_G_hasScalarData( group ),
                            MDAL_driverFromName( "PLY" ),
                            tmp_file( "/dg2d.ply" ).c_str() );
    ASSERT_TRUE( index <  MDAL_M_datasetGroupCount( mesh2 ) );
    MDAL_DatasetGroupH group2 = MDAL_M_datasetGroup( mesh2, index );

    MDAL_DatasetH dataset = MDAL_G_dataset( group, 0 );
    v_count = MDAL_D_valueCount( dataset );
    if ( MDAL_G_hasScalarData( group ) )
    {
      std::vector<double> values( v_count, 0 );
      MDAL_D_data( dataset, 0, v_count, MDAL_DataType::SCALAR_DOUBLE, values.data() );
      MDAL_DatasetH dataset2 = MDAL_G_addDataset( group2, 0, values.data(), nullptr );
      ASSERT_EQ( MDAL_D_valueCount( dataset2 ), v_count );
    }
    else
    {
      std::vector<double> values( v_count * 2, 0 );
      MDAL_D_data( dataset, 0, v_count, MDAL_DataType::VECTOR_2D_DOUBLE, values.data() );
      MDAL_DatasetH dataset2 = MDAL_G_addDataset( group2, 0, values.data(), nullptr );
      ASSERT_EQ( MDAL_D_valueCount( dataset2 ), v_count );
    }
    MDAL_G_closeEditMode( group2 );
    s = MDAL_LastStatus();
    ASSERT_EQ( MDAL_Status::None, s );
  }

  MDAL_CloseMesh( mesh );
  MDAL_CloseMesh( mesh2 );
  MDAL_FI_close( fi );
  MDAL_VI_close( vi );
}

// test the memorydataset3D
TEST( Memory3D, ScalarMesh )
{
  std::string path = test_file( "/tuflowfv/withMaxes/trap_steady_05_3D.nc" );
  MDAL_MeshH mesh = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( mesh, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mesh, 1 );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnVolumes );

  int v_count = MDAL_M_vertexCount( mesh );
  int f_count = MDAL_M_faceCount( mesh );

  MDAL_MeshH mesh2 = MDAL_CreateMesh( MDAL_driverFromName( "PLY" ) );

  std::vector<double> vertices( v_count * 3 );
  MDAL_MeshVertexIteratorH vi = MDAL_M_vertexIterator( mesh );
  MDAL_MeshFaceIteratorH fi = MDAL_M_faceIterator( mesh );
  MDAL_VI_next( vi, v_count, vertices.data() );
  MDAL_M_addVertices( mesh2, v_count, vertices.data() );
  std::vector<int> face( MDAL_M_faceVerticesMaximumCount( mesh ) );
  int offset;
  for ( int i = 0; i < f_count; i++ )
  {
    MDAL_FI_next( fi, 1, &offset, face.size(), face.data() );
    MDAL_M_addFaces( mesh2, 1, &offset, face.data() );
  }

  MDAL_SaveMesh( mesh2, tmp_file( "/volumetric.ply" ).c_str(), "PLY" );

  // create a new 3D datasetGroup
  int index = MDAL_M_datasetGroupCount( mesh2 );
  MDAL_M_addDatasetGroup( mesh2,
                          "test",
                          MDAL_DataLocation::DataOnVolumes,
                          true,
                          MDAL_driverFromName( "PLY" ),
                          tmp_file( "/volumetric.ply" ).c_str() );
  ASSERT_TRUE( index <  MDAL_M_datasetGroupCount( mesh2 ) );
  MDAL_DatasetGroupH group2 = MDAL_M_datasetGroup( mesh2, index );
  ASSERT_EQ( MDAL_G_dataLocation( group2 ), MDAL_DataLocation::DataOnVolumes );

// create a new 3D dataset

  MDAL_DatasetH dataset = MDAL_G_dataset( group, 0 );
  v_count = MDAL_D_valueCount( dataset );
  std::vector<int> lc( f_count, 0 );
  std::vector<int> f2V( f_count, 0 );
  std::vector<double> ve( f_count + v_count, 0 );
  std::vector<double> values( v_count, 0 );
  MDAL_D_data( dataset, 0, f_count, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, lc.data() );
  MDAL_D_data( dataset, 0, f_count, MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER, f2V.data() );
  MDAL_D_data( dataset, 0, f_count + v_count, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, ve.data() );
  MDAL_D_data( dataset, 0, v_count, MDAL_DataType::SCALAR_VOLUMES_DOUBLE, values.data() );
  MDAL_DatasetH dataset2 = MDAL_G_addDataset3D( group2, 0, values.data(), lc.data(), ve.data() );
  ASSERT_EQ( MDAL_D_valueCount( dataset2 ), v_count );

  MDAL_G_closeEditMode( group2 );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  MDAL_MeshH mesh3 = MDAL_LoadMesh( tmp_file( "/volumetric.ply" ).c_str() );
  EXPECT_NE( mesh3, nullptr );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  MDAL_DatasetGroupH group3 = MDAL_M_datasetGroup( mesh3, 1 );
  ASSERT_EQ( MDAL_G_dataLocation( group3 ), MDAL_DataLocation::DataOnVolumes );
  f_count = MDAL_M_faceCount( mesh3 );
  MDAL_DatasetH dataset3 = MDAL_G_dataset( group3, 0 );
  v_count = MDAL_D_valueCount( dataset3 );

  // test data equality
  std::vector<int> lc2( f_count, 0 );
  std::vector<int> f2V2( f_count, 0 );
  std::vector<double> ve2( f_count + v_count, 0 );
  std::vector<double> values2( v_count, 0 );
  MDAL_D_data( dataset3, 0, f_count, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, lc2.data() );
  MDAL_D_data( dataset3, 0, f_count, MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER, f2V2.data() );
  MDAL_D_data( dataset3, 0, f_count + v_count, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, ve2.data() );
  MDAL_D_data( dataset3, 0, v_count, MDAL_DataType::SCALAR_VOLUMES_DOUBLE, values2.data() );
  ASSERT_TRUE( compareVectors( lc, lc2 ) );
  ASSERT_TRUE( compareVectors( f2V, f2V2 ) );
  ASSERT_TRUE( compareVectors( ve, ve2 ) );
  ASSERT_TRUE( compareVectors( values, values2 ) );

  MDAL_CloseMesh( mesh );
  MDAL_CloseMesh( mesh2 );
  MDAL_CloseMesh( mesh3 );
  MDAL_FI_close( fi );
  MDAL_VI_close( vi );
}

TEST( Memory3D, VectorMesh )
{
  std::string path = test_file( "/tuflowfv/withMaxes/trap_steady_05_3D.nc" );
  MDAL_MeshH mesh = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( mesh, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mesh, 6 );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnVolumes );

  int v_count = MDAL_M_vertexCount( mesh );
  int f_count = MDAL_M_faceCount( mesh );

  MDAL_MeshH mesh2 = MDAL_CreateMesh( MDAL_driverFromName( "PLY" ) );

  std::vector<double> vertices( v_count * 3 );
  MDAL_MeshVertexIteratorH vi = MDAL_M_vertexIterator( mesh );
  MDAL_MeshFaceIteratorH fi = MDAL_M_faceIterator( mesh );
  MDAL_VI_next( vi, v_count, vertices.data() );
  MDAL_M_addVertices( mesh2, v_count, vertices.data() );
  std::vector<int> face( MDAL_M_faceVerticesMaximumCount( mesh ) );
  int offset;
  for ( int i = 0; i < f_count; i++ )
  {
    MDAL_FI_next( fi, 1, &offset, face.size(), face.data() );
    MDAL_M_addFaces( mesh2, 1, &offset, face.data() );
  }

  // create a new 3D datasetGroup
  int index = MDAL_M_datasetGroupCount( mesh2 );
  MDAL_M_addDatasetGroup( mesh2,
                          "test",
                          MDAL_DataLocation::DataOnVolumes,
                          false,
                          MDAL_driverFromName( "PLY" ),
                          tmp_file( "/volumetric_vecttor.ply" ).c_str() );
  ASSERT_TRUE( index <  MDAL_M_datasetGroupCount( mesh2 ) );
  MDAL_DatasetGroupH group2 = MDAL_M_datasetGroup( mesh2, index );
  ASSERT_EQ( MDAL_G_dataLocation( group2 ), MDAL_DataLocation::DataOnVolumes );

// create a new 3D dataset
  MDAL_DatasetH dataset = MDAL_G_dataset( group, 0 );
  v_count = MDAL_D_valueCount( dataset );
  std::vector<int> lc( f_count, 0 );
  std::vector<double> ve( f_count + v_count, 0 );
  std::vector<double> values( 2 * v_count, 0 );
  MDAL_D_data( dataset, 0, f_count, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, lc.data() );
  MDAL_D_data( dataset, 0, f_count + v_count, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, ve.data() );
  MDAL_D_data( dataset, 0, v_count, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, values.data() );
  MDAL_DatasetH dataset2 = MDAL_G_addDataset3D( group2, 0, values.data(), lc.data(), ve.data() );
  ASSERT_EQ( MDAL_D_valueCount( dataset2 ), v_count );

  MDAL_G_closeEditMode( group2 );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::Err_IncompatibleDatasetGroup, s );

  // test data equality
  std::vector<int> lc2( f_count, 0 );
  std::vector<double> ve2( f_count + v_count, 0 );
  std::vector<double> values2( 2 * v_count, 0 );
  MDAL_D_data( dataset2, 0, f_count, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, lc2.data() );
  MDAL_D_data( dataset2, 0, f_count + v_count, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, ve2.data() );
  MDAL_D_data( dataset2, 0, v_count, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, values2.data() );
  ASSERT_TRUE( compareVectors( lc, lc2 ) );
  ASSERT_TRUE( compareVectors( ve, ve2 ) );
  ASSERT_TRUE( compareVectors( values, values2 ) );

  MDAL_CloseMesh( mesh );
  MDAL_CloseMesh( mesh2 );
  MDAL_FI_close( fi );
  MDAL_VI_close( vi );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret = RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
