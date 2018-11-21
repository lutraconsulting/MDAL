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

TEST( Mesh3DiTest, Mesh2D4cells301steps )
{
  std::string path = test_file( "/3di/2d_4cells301steps/results_3di.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 9 );
  double z = MDAL_M_vertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );

  std::vector<std::pair<double, double>> expectedCoords;
  expectedCoords.push_back( std::make_pair( 0, 0 ) );
  expectedCoords.push_back( std::make_pair( 12, 0 ) );
  expectedCoords.push_back( std::make_pair( 12, 12 ) );
  expectedCoords.push_back( std::make_pair( 0, 12 ) );
  expectedCoords.push_back( std::make_pair( 12, 24 ) );
  expectedCoords.push_back( std::make_pair( 0, 24 ) );
  expectedCoords.push_back( std::make_pair( 24, 0 ) );
  expectedCoords.push_back( std::make_pair( 24, 12 ) );
  expectedCoords.push_back( std::make_pair( 24, 24 ) );

  int i = 0;
  for ( auto coors : expectedCoords )
  {
    double x = MDAL_M_vertexXCoordinatesAt( m, i );
    double y = MDAL_M_vertexYCoordinatesAt( m, i );
    EXPECT_DOUBLE_EQ( coors.first, x );
    EXPECT_DOUBLE_EQ( coors.second, y );
    ++i;
  }

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4, f_count );

  // test face 1
  int f_v_count = MDAL_M_faceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = MDAL_M_faceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 3, f_v );
  f_v = MDAL_M_faceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 2, f_v );
  f_v = MDAL_M_faceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 4, f_v );
  f_v = MDAL_M_faceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 5, f_v );

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
  ASSERT_EQ( 4, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 0, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "waterlevel" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 301, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "flow velocity in cell centre" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 301, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  MDAL_CloseMesh( m );
}

TEST( Mesh3DiTest, Mesh2D16cells7steps )
{
  std::string path = test_file( "/3di/2d_16cells7steps/results_3di.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 25 );
  double z = MDAL_M_vertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 16, f_count );

  ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );
  DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( 7, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

