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
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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
    368725.71875, 8129547, 3.109999895095825,
    368745.71875, 8129547, 3.010999917984009,
    368765.71875, 8129547, 2.920000076293945
  };
  EXPECT_EQ( expectedCoords.size(), 3 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 3 );

  EXPECT_TRUE( compareVectors( expectedCoords, coordinates ) );

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

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

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
  EXPECT_EQ( std::string( "stage" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

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

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.083333333333 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshSWWTest, Flat )
{
  std::string path = test_file( "/sww/anuga-viewer/flat.sww" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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

  // friction_c shifts momentum and elevation groups by one compared to the
  // old code that skipped all _c variables.  The file also gains momentum_c,
  // elevation_c and stage_c face groups.
  ASSERT_EQ( 13, MDAL_M_datasetGroupCount( m ) );

  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( false, MDAL_G_hasScalarData( g ) );
  }
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum/Maximums" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( false, MDAL_G_hasScalarData( g ) );
  }
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 6 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "elevation" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( true, MDAL_G_hasScalarData( g ) );
  }
  // New face groups
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "friction_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( true, MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
    EXPECT_EQ( 6388, MDAL_D_valueCount( MDAL_G_dataset( g, 0 ) ) );
  }
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 10 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( false, MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
  }
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 11 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "elevation_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( true, MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
  }
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 12 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "stage_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_EQ( true, MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
  }
  MDAL_CloseMesh( m );
}

TEST( MeshSWWTest, Laminar )
{
  std::string path = test_file( "/sww/anuga-viewer/laminar.sww" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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

TEST( MeshSWWTest, Merimbula )
{
  std::string path = test_file( "/sww/merimbula_basic_mesh.sww" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SWW:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // CRS
  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:32755" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SWW" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5719 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 10785 );

  // ///////////
  // Dataset groups
  // ///////////
  // Expected groups (in file variable order):
  //  0  Bed Elevation       - vertex scalar  (static, from "elevation")
  //  1  friction            - vertex scalar  (static)
  //  2  friction/Maximums   - vertex scalar  (static)
  //  3  elevation           - vertex scalar  (static, same data as Bed Elevation)
  //  4  elevation/Maximums  - vertex scalar  (static)
  //  5  friction_c          - face scalar    (static)
  //  6  elevation_c         - face scalar    (static)
  //  7  momentum            - vertex vector  (time-dependent, xmomentum+ymomentum)
  //  8  momentum/Maximums   - vertex vector  (static range)
  //  9  stage               - vertex scalar  (time-dependent)
  // 10  stage/Maximums      - vertex scalar  (static range)
  // 11  momentum_c          - face vector    (time-dependent, xmomentum_c+ymomentum_c)
  // 12  stage_c             - face scalar    (time-dependent)
  ASSERT_EQ( 13, MDAL_M_datasetGroupCount( m ) );

  // --- Bed Elevation (group 0) ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnVertices, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    EXPECT_EQ( 5719, MDAL_D_valueCount( ds ) );
    EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );
  }

  // --- friction_c (group 5) — static scalar on faces ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "friction_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    EXPECT_EQ( 10785, MDAL_D_valueCount( ds ) );
    EXPECT_DOUBLE_EQ( 0.0, getValue( ds, 0 ) );
  }

  // --- elevation_c (group 6) — static scalar on faces ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 6 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "elevation_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    EXPECT_EQ( 10785, MDAL_D_valueCount( ds ) );
    EXPECT_NEAR( -0.04933963, getValue( ds, 0 ), 1e-5 );
    EXPECT_NEAR( -0.04984199, getValue( ds, 1 ), 1e-5 );
  }

  // --- stage (group 9) — time-dependent scalar on vertices ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 9 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "stage" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnVertices, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
    EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );
    // times are 0, 10, 20, 30, 40, 50 seconds
    EXPECT_TRUE( compareDurationInHours( MDAL_D_time( MDAL_G_dataset( g, 1 ) ),
                                         10.0 / 3600.0 ) );
  }

  // --- momentum_c (group 11) — time-dependent vector on faces ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 11 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "momentum_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_FALSE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    EXPECT_EQ( 10785, MDAL_D_valueCount( ds ) );
    EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );
  }

  // --- stage_c (group 12) — time-dependent scalar on faces ---
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 12 );
    ASSERT_NE( g, nullptr );
    EXPECT_EQ( std::string( "stage_c" ), std::string( MDAL_G_name( g ) ) );
    EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
    EXPECT_EQ( MDAL_DataLocation::DataOnFaces, MDAL_G_dataLocation( g ) );
    ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    EXPECT_EQ( 10785, MDAL_D_valueCount( ds ) );
    EXPECT_NEAR( 2.0, getValue( ds, 0 ), 1e-5 );
    EXPECT_NEAR( 1.0, getValue( ds, 2 ), 1e-5 );
    EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );
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

