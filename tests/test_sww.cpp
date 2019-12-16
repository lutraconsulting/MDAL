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
  std::string path = test_file( "/sww/anuga-viewer/cairns.sww" );
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
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices2D );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

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
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "stage" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices2D );

  ASSERT_EQ( 51, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 30 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

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

  EXPECT_FALSE( hasReferenceTime( g ) );

  EXPECT_EQ( std::string( "seconds" ), std::string( MDAL_G_TimeUnit( g ) ) ) ;

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.083333333333 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshSWWTest, Flat )
{
  std::string path = test_file( "/sww/anuga-viewer/flat.sww" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( 2579, v_count );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4962, f_count );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_CloseMesh( m );
}


TEST( MeshSWWTest, Catchment )
{
  std::string path = test_file( "/sww/anuga-viewer/Small_catchment_testcase.sww" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( 19164, v_count );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 6388, f_count );

  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( false, MDAL_G_hasScalarData( g ) );
  }
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum/Maximums" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( false, MDAL_G_hasScalarData( g ) );
  }
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "elevation" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( true, MDAL_G_hasScalarData( g ) );
  }
  MDAL_CloseMesh( m );
}

TEST( MeshSWWTest, Laminar )
{
  std::string path = test_file( "/sww/anuga-viewer/laminar.sww" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( 3721, v_count );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 7200, f_count );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_CloseMesh( m );
}

TEST( MeshSWWTest, Wave )
{
  std::string path = test_file( "/sww/anuga-viewer/holl_bch_wave_mesh_elevation_smooth_ys10.0_ft500.0_size4802.sww" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( 2500, v_count );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4802, f_count );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

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

