/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

TEST( MeshTinTest, WrongFiles )
{
  for ( int i = 0; i < 9; ++i )
  {
    std::string fileName = test_file( "/xms_tin/invalid/invalid" + std::to_string( i ) + ".tin" );
    MDAL_MeshH m = MDAL_LoadMesh( fileName.c_str() );
    EXPECT_EQ( m, nullptr );
  }
}

TEST( MeshTinTest, ParaboloidFile )
{
  std::string path = test_file( "/xms_tin/paraboloid.m.tin" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 3 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "XMS_TIN" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 239 );
  double x = getVertexXCoordinatesAt( m, 2 );
  double y = getVertexYCoordinatesAt( m, 2 );
  double z = getVertexZCoordinatesAt( m, 2 );
  EXPECT_DOUBLE_EQ( 43.741403313474954, x );
  EXPECT_DOUBLE_EQ( 2.8822251839036763, y );
  EXPECT_DOUBLE_EQ( 37.262135141179115, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 347, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 3 );
  EXPECT_EQ( 3, f_v_count );
  int f_v = getFaceVerticesIndexAt( m, 3, 0 );
  EXPECT_EQ( 36, f_v );
  f_v = getFaceVerticesIndexAt( m, 3, 1 );
  EXPECT_EQ( 68, f_v );
  f_v = getFaceVerticesIndexAt( m, 3, 2 );
  EXPECT_EQ( 231, f_v );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( -43.822593540032521, minX );
  EXPECT_DOUBLE_EQ( 44.986275235744273, maxX );
  EXPECT_DOUBLE_EQ( -45.012860971759714, minY );
  EXPECT_DOUBLE_EQ( 43.78966755326303, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

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
  ASSERT_EQ( 239, count );

  double value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 37.13464052406677, value );

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

