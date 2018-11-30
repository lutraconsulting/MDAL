/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

static MeshH mesh()
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  return m;
}

TEST( MeshAsciiDatTest, MissingMesh )
{
  MeshH m = nullptr;
  std::string path = test_file( "ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
}

TEST( MeshAsciiDatTest, MissingFile )
{
  MeshH m = mesh();
  std::string path = test_file( "non/existent/path.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_FileNotFound, s );
  EXPECT_EQ( 1, MDAL_M_datasetGroupCount( m ) );
}

TEST( MeshAsciiDatTest, WrongFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/not_a_data_file.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
  EXPECT_EQ( 1, MDAL_M_datasetGroupCount( m ) );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceScalarFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "FaceScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 2, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 2, max );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceVectorFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "FaceVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

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
  ASSERT_EQ( 2, count );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 2.8284271247461903, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 2.8284271247461903, max );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

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
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarFileWithTabs )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar_tabs.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

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
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexVectorFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

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
  ASSERT_EQ( 5, count );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0, time );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 1, value );

  MDAL_CloseMesh( m );
}


int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

