/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshXmdfTest, MissingMesh )
{
  MeshH m = nullptr;
  std::string path = test_file( "/xmdf/xmdf_format.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
}

TEST( MeshXmdfTest, QuadAndTriangleFile )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/regular_grid.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 186, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 0 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( true, scalar );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = MDAL_D_active( ds, 0 );
  EXPECT_EQ( false, active );

  bool onVertices = MDAL_D_isOnVertices( ds );
  EXPECT_EQ( true, onVertices );

  int meta_count = MDAL_D_metadataCount( ds );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_D_metadataKey( ds, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_D_metadataValue( ds, 0 );
  EXPECT_EQ( std::string( "Depth" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1976, count );

  double value = MDAL_D_value( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

