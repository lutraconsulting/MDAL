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

TEST( MeshUgridTest, DFlow11Manzese )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 3042 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 1824, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 25, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 50, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 51, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 26, f_v );

  ASSERT_EQ( 10, MDAL_M_datasetGroupCount( m ) );

  // ///////////
  // Dataset
  // ///////////
  DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Total bed shear stress" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1824, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 0.0, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 15.907056943512494, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0.0, time );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow11ManzeseNodeZValue )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  double z = getVertexZCoordinatesAt( m, 0 );
  ASSERT_EQ( z, 42.8397 );
}

TEST( MeshUgridTest, DFlow11Simplebox )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/simplebox_hex7_map.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 720 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 810, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 479, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 524, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 480, f_v );

  // ///////////
  // dataset
  // ///////////
  ASSERT_EQ( 10, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 7 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "water depth at pressure points" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 13, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 810, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 3.1245244868590514, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 2.1346136585097848, min );
  EXPECT_DOUBLE_EQ( 5.8788940865351824, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 2.1346136585097848, min );
  EXPECT_DOUBLE_EQ( 6.3681219945588952, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0.0097222222222222224, time );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow12RivierGridClm )
{
  std::string path = test_file( "/ugrid/D-Flow1.2/bw_11_zonder_riviergrid_met_1dwtg_clm.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12310 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 11987, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // dataset
  // ///////////
  ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water level" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 45, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 6, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 2, min );
  EXPECT_DOUBLE_EQ( 12, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -2.0422003887246905e-301, min );
  EXPECT_DOUBLE_EQ( 560.18823529411759, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 183.5, time );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow12RivierGridMap )
{
  std::string path = test_file( "/ugrid/D-Flow1.2/bw_11_zonder_riviergrid_met_1dwtg_map.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12310 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 11987, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // scalar dataset
  // ///////////
  ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water level" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 23, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 3.8664888639861923, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -0.3397185302284324, min );
  EXPECT_DOUBLE_EQ( 12.898470433826372, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -0.3397185302284324, min );
  EXPECT_DOUBLE_EQ( 12.898470433826372, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 184, time );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Flow element center velocity vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 23, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 20 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  value = getValueX( ds, 82 );
  EXPECT_DOUBLE_EQ( 0, value );

  value = getValueX( ds, 6082 );
  EXPECT_DOUBLE_EQ( 0.042315615419157751, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0.66413616798770714, max );

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

