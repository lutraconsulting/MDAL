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

TEST( MeshFlo2dTest, BarnHDF5 )
{
  std::string path = test_file( "/flo2d/BarnHDF5/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 571 );

  std::vector<double> expectedCoords =
  {
    0.000,        0.000, 0.000,
    100.000,       0.000, 0.000,
    200.000,        0.000, 0.000,
    300.000,      0.000, 0.000
  };
  EXPECT_EQ( expectedCoords.size(), 4 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 4 );

  compareVectors( expectedCoords, coordinates );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 521, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 5, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 1, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

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
  ASSERT_EQ( 521, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 4261.2799999999997, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 4262.8800000000001, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 4262.8299999999999, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4262.7700000000004, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "FLOW DEPTH" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 20, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0.10124753560882101, time );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 1 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 521, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 4262.8798828125, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 4259.18017578125, min );
  EXPECT_DOUBLE_EQ( 4520, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 4259.18017578125, min );
  EXPECT_DOUBLE_EQ( 4520, max );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Velocity" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 20, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 521, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0, max );

  MDAL_CloseMesh( m );
}

TEST( MeshFlo2dTest, basic )
{
  std::vector<std::string> files;
  files.push_back( "basic" );
  files.push_back( "basic_with_dos_eol" );
  for ( const std::string &file : files )
  {
    std::string path = test_file( "/flo2d/" + file + "/BASE.OUT" );
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );

    // ///////////
    // Vertices
    // ///////////
    int v_count = MDAL_M_vertexCount( m );
    EXPECT_EQ( v_count, 16 );

    std::vector<double> expectedCoords =
    {
      1.59, 3.00, 0.00,
      2.59,  3.00, 0.00,
      3.59,  3.00, 0.00,
      1.59,  2.00, 0.00,
      2.59,  2.00, 0.00,
      3.59,  2.00, 0.00,
      1.59, 1.00, 0.00,
      2.59,  1.00, 0.00,
      3.59, 1.00, 0.00
    };
    EXPECT_EQ( expectedCoords.size(), 9 * 3 );

    std::vector<double> coordinates = getCoordinates( m, 9 );

    compareVectors( expectedCoords, coordinates );

    // ///////////
    // Faces
    // ///////////
    int f_count = MDAL_M_faceCount( m );
    EXPECT_EQ( 9, f_count );

    // test face 1
    int f_v_count = getFaceVerticesCountAt( m, 1 );
    EXPECT_EQ( 4, f_v_count ); //quad
    int f_v = getFaceVerticesIndexAt( m, 1, 0 );
    EXPECT_EQ( 4, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 1 );
    EXPECT_EQ( 5, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 2 );
    EXPECT_EQ( 1, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 3 );
    EXPECT_EQ( 0, f_v );

    // ///////////
    // Bed elevation dataset
    // ///////////
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

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
    ASSERT_EQ( 9, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1.48, value );

    // ///////////
    // Scalar Dataset
    // ///////////
    g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

    scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
    ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 9, count );

    value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 1, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1, min );
    EXPECT_DOUBLE_EQ( 1, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1, min );
    EXPECT_DOUBLE_EQ( 3, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 0.5, time );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshFlo2dTest, basic_required_files_only )
{
  std::string path = test_file( "/flo2d/basic_required_files_only/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 16 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 9, f_count );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

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
  ASSERT_EQ( 9, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.48, value );

  MDAL_CloseMesh( m );
}

TEST( MeshFlo2dTest, pro_16_02_14 )
{
  std::string path = test_file( "/flo2d/pro_16_02_14/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5443 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 5101, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

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
  ASSERT_EQ( 5101, count );

  double value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4904.2299999999996, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 4, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 150.0, time );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5101, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 0.098000000000000004, value );

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

