/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshGribTest, ScalarFile )
{
  std::string path = test_file( "/grib/Madagascar.wave.7days.grb" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 81, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 50 );
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
  EXPECT_EQ( std::string( "Primary wave mean period [s]" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1683, count );

  double value = MDAL_D_value( ds, 1600 );
  EXPECT_DOUBLE_EQ( 7, value );

  MDAL_CloseMesh( m );
}

TEST( MeshGribTest, VectorFile )
{
  std::string path = test_file( "/grib/Madagascar.wind.7days.grb" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 27, MDAL_M_datasetCount( m ) );
  DatasetH ds = MDAL_M_dataset( m, 15 );
  ASSERT_NE( ds, nullptr );

  bool scalar = MDAL_D_hasScalarData( ds );
  EXPECT_EQ( false, scalar );

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
  EXPECT_EQ( std::string( "wind [m/s]" ), std::string( val ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1683, count );

  double valueX = MDAL_D_valueX( ds, 1600 );
  EXPECT_DOUBLE_EQ( -5.9000000000000004, valueX );

  double valueY = MDAL_D_valueY( ds, 1600 );
  EXPECT_DOUBLE_EQ( 2.8200097656250001, valueY );

  MDAL_M_CloseDataset( ds );
  MDAL_CloseMesh( m );
}


int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

