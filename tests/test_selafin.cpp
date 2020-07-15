/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 ARTELIA - Christophe Coulet
 (christophe dot coulet at arteliagroup dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshSLFTest, MalpassetGeometry )
{
  std::string path = test_file( "/slf/example.slf" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 13541 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );
  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 26000, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!

  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "bottom" ), std::string( name ) );

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
  ASSERT_EQ( 13541, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 70.0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 94.5398330688477, value );
  value = getValue( ds, 1000 );
  EXPECT_DOUBLE_EQ( 1.73051724061679e-008, value );
  value = getValue( ds, 9571 );
  EXPECT_DOUBLE_EQ( 7.5623664855957, value );

  MDAL_CloseMesh( m );
}

TEST( MeshSLFTest, MalpassetResultFrench )
{
  std::string path = test_file( "/slf/example_res_fr.slf" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 13541 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );
  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 26000, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!

  int var_count = MDAL_M_datasetGroupCount( m );
  ASSERT_EQ( 4, var_count ); // 4 variables (Velocity, Water Depth, Free Surface and Bottom)

  // ///////////
  // Scalar Dataset
  // ///////////
  DatasetGroupH r = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( r, nullptr );

  int meta_count = MDAL_G_metadataCount( r );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( r );
  EXPECT_EQ( std::string( "surface libre   m" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( r );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( r );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( r ) );
  DatasetH ds = MDAL_G_dataset( r, 1 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 4000, time );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 1 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 13541, count );

  double value = getValue( ds, 8667 );
  EXPECT_DOUBLE_EQ( 31.965662002563477, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -0.00673320097848773, min );
  EXPECT_DOUBLE_EQ( 100.00228118896484, max );

  MDAL_G_minimumMaximum( r, &min, &max );
  EXPECT_DOUBLE_EQ( -0.00673320097848773, min );
  EXPECT_DOUBLE_EQ( 100.00228118896484, max );

  // ///////////
  // Vector Dataset
  // ///////////
  r = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( r, nullptr );

  meta_count = MDAL_G_metadataCount( r );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( r );
  EXPECT_EQ( std::string( "vitesse       ms" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( r );
  EXPECT_EQ( false, scalar );

  onVertices = MDAL_G_isOnVertices( r );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( r ) );
  ds = MDAL_G_dataset( r, 1 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 1 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 13541, count );

  value = getValueX( ds, 8667 );
  EXPECT_DOUBLE_EQ( 6.2320127487182617, value );
  value = getValueY( ds, 8667 );
  EXPECT_DOUBLE_EQ( -0.97271907329559326, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 2.3694833011052991e-12, min );
  EXPECT_DOUBLE_EQ( 7.5673562379016834, max );

  MDAL_CloseMesh( m );
}


TEST( MeshSLFTest, DoublePrecision )
{
  std::string path = test_file( "/slf/test_sd_7.slf" );

  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 17830 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 440745.06147386681, x );
  EXPECT_DOUBLE_EQ( 5420249.8978509316, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  x = getVertexXCoordinatesAt( m, 1000 );
  y = getVertexYCoordinatesAt( m, 1000 );
  z = getVertexZCoordinatesAt( m, 1000 );
  EXPECT_DOUBLE_EQ( 440750.06147266628, x );
  EXPECT_DOUBLE_EQ( 5420258.4996587345, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 35093, f_count );

  // ///////////
  // Extent
  // ///////////
  double xmin, xmax, ymin, ymax;
  MDAL_M_extent( m, &xmin, &xmax, &ymin, &ymax );
  EXPECT_EQ( xmin, 440745.0614738668 );
  EXPECT_EQ( xmax, 440755.0614738668 );
  EXPECT_EQ( ymin, 5420249.897850932 );
  EXPECT_EQ( ymax, 5420349.908870826 );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!
  int f_v = getFaceVerticesIndexAt( m, 100, 0 );
  EXPECT_EQ( 2133, f_v );
  f_v = getFaceVerticesIndexAt( m, 100, 1 );
  EXPECT_EQ( 2011, f_v ); \
  f_v = getFaceVerticesIndexAt( m, 100, 2 );
  EXPECT_EQ( 2012, f_v );

  // Datasets
  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "velocity      ms" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  ASSERT_EQ( 11, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 17830, count );

  double valueY = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 0.027486738969071053, valueY );
  valueY = getValueY( ds, 20 );
  EXPECT_DOUBLE_EQ( 0.33878578833223305, valueY );
  valueY = getValueY( ds, 1000 );
  EXPECT_DOUBLE_EQ( 0.37488353797245938, valueY );

  MDAL_CloseMesh( m );
}


TEST( MeshSLFTest, JanetFile )
{
  std::string path = test_file( "/slf/test_sd_6.slf" );

  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 17830 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 440745.06147386681, x );
  EXPECT_DOUBLE_EQ( 5420249.8978509316, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  x = getVertexXCoordinatesAt( m, 1000 );
  y = getVertexYCoordinatesAt( m, 1000 );
  z = getVertexZCoordinatesAt( m, 1000 );
  EXPECT_DOUBLE_EQ( 440750.06147266628, x );
  EXPECT_DOUBLE_EQ( 5420258.4996587345, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 35093, f_count );

  // ///////////
  // Extent
  // ///////////
  double xmin, xmax, ymin, ymax;
  MDAL_M_extent( m, &xmin, &xmax, &ymin, &ymax );
  EXPECT_EQ( xmin, 440745.0614738668 );
  EXPECT_EQ( xmax, 440755.0614738668 );
  EXPECT_EQ( ymin, 5420249.897850932 );
  EXPECT_EQ( ymax, 5420349.908870826 );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!
  int f_v = getFaceVerticesIndexAt( m, 100, 0 );
  EXPECT_EQ( 2133, f_v );
  f_v = getFaceVerticesIndexAt( m, 100, 1 );
  EXPECT_EQ( 2011, f_v ); \
  f_v = getFaceVerticesIndexAt( m, 100, 2 );
  EXPECT_EQ( 2012, f_v );

  // Datasets
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "bottom          m" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 17830, count );

  double value = getValue( ds, 0 );
  EXPECT_EQ( 101.1, value );
  value = getValue( ds, 20 );
  EXPECT_EQ( 99.1, value );
  value = getValue( ds, 1000 );
  EXPECT_EQ( 99.09139914, value );
  value = getValue( ds, 10000 );
  EXPECT_EQ( 100.50871584346136, value );

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
