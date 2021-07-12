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

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 10, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 20, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 8, MDAL_M_datasetGroupCount( m ) );

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

  //test face data

  g = MDAL_M_datasetGroup( m, 4 );
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

  //test edge data

  g = MDAL_M_datasetGroup( m, 7 );
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

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 10, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 20, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 8, MDAL_M_datasetGroupCount( m ) );

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

  //test face data

  g = MDAL_M_datasetGroup( m, 4 );
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

  //test edge data

  g = MDAL_M_datasetGroup( m, 7 );
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

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret = RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

