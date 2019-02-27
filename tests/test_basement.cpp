/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( BasementTest, SimpleChannel )
{
  std::string path = test_file( "/basement/basement3/SimpleChannel/SimpleChannel.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "2DM" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 54 );
  double x = getVertexXCoordinatesAt( m, 4 );
  double y = getVertexYCoordinatesAt( m, 4 );
  double z = getVertexZCoordinatesAt( m, 4 );
  EXPECT_DOUBLE_EQ( 8.0, x );
  EXPECT_DOUBLE_EQ( 0.0, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 77, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 3, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 29, f_v );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 20, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 5, maxY );

  f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( f_v_count, 3 ); //triangle
  f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // Bed elevation dataset
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( true, onVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 54, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0, value );
  }

  // Bed elevation dataset and face elevation dataset
  {

    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation (Face)" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 77, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0.19500000000000001, value );
  }

  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

