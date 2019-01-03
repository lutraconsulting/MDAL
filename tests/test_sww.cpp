/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshSWWTest, Cairns )
{
  std::string path = test_file( "/sww/cairns.sww" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 2579 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 3.1099998950958252, z );

  std::vector<double> expectedCoords =
  {
    0.0, 0.0, 0.0,
    12, 0, 0.0,
    12, 12, 0.0,
    0, 12, 0.0,
    12, 24, 0.0,
    0, 24, 0.0,
    24, 0, 0.0,
    24, 12, 0.0,
    24, 24, 0.0
  };
  EXPECT_EQ( expectedCoords.size(), 9 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 9 );

  compareVectors( expectedCoords, coordinates );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4962, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!
  int f_v = getFaceVerticesIndexAt( m, 10, 0 );
  EXPECT_EQ( 2531, f_v );
  f_v = getFaceVerticesIndexAt( m, 10, 1 );
  EXPECT_EQ( 2532, f_v );
  f_v = getFaceVerticesIndexAt( m, 10, 2 );
  EXPECT_EQ( 2481, f_v );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 3, MDAL_M_datasetGroupCount( m ) );

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
  ASSERT_EQ( 2579, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 3.1099998950958252, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 3.0109999179840088, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 2.9200000762939453, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 2.7599999904632568, value );

  // ///////////
  // "Stage"
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Stage" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 51, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 30 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( false, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2579, count );

  value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 3.9895098209381104, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0.57015693187713623, min );
  EXPECT_DOUBLE_EQ( 6.7160892486572266, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 6.7305092811584473, max );

  // ///////////
  // "Depth"
  // ///////////
  g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 51, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 29 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 50 );
  EXPECT_EQ( false, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2579, count );

  value = getValue( ds, 1000 );
  EXPECT_DOUBLE_EQ( 1.34523606300354, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 6.7142167091369629, max );

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

