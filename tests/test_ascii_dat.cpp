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
  EXPECT_EQ( 0, MDAL_M_datasetCount( m ) );
}

TEST( MeshAsciiDatTest, WrongFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/not_a_data_file.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
  EXPECT_EQ( 0, MDAL_M_datasetCount( m ) );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceScalarFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 0 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( true, scalar );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = MDAL_D_active( ds, 0 );
  EXPECT_EQ( true, active );

  bool onVertices = MDAL_D_isOnVertices( ds );
  EXPECT_EQ( false, onVertices );

  int meta_count = MDAL_D_metadataCount( ds );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_D_metadataKey( ds, 1 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_D_metadataValue( ds, 1 );
  EXPECT_EQ( std::string( "FaceScalarDataset" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  double value = MDAL_D_value( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_value( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceVectorFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 0 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( false, scalar );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = MDAL_D_active( ds, 0 );
  EXPECT_EQ( true, active );

  bool onVertices = MDAL_D_isOnVertices( ds );
  EXPECT_EQ( false, onVertices );

  int meta_count = MDAL_D_metadataCount( ds );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_D_metadataKey( ds, 1 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_D_metadataValue( ds, 1 );
  EXPECT_EQ( std::string( "FaceVectorDataset" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  double value = MDAL_D_value( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = MDAL_D_valueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 0 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( true, scalar );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = MDAL_D_active( ds, 0 );
  EXPECT_EQ( true, active );

  bool onVertices = MDAL_D_isOnVertices( ds );
  EXPECT_EQ( true, onVertices );

  int meta_count = MDAL_D_metadataCount( ds );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_D_metadataKey( ds, 1 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_D_metadataValue( ds, 1 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = MDAL_D_value( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_value( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexVectorFile )
{
  MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 0 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( false, scalar );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = MDAL_D_active( ds, 0 );
  EXPECT_EQ( true, active );

  bool onVertices = MDAL_D_isOnVertices( ds );
  EXPECT_EQ( true, onVertices );

  int meta_count = MDAL_D_metadataCount( ds );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_D_metadataKey( ds, 1 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_D_metadataValue( ds, 1 );
  EXPECT_EQ( std::string( "VertexVectorDataset" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = MDAL_D_value( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = MDAL_D_valueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = MDAL_D_valueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 1, value );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}


int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

