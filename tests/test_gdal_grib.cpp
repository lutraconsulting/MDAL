/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "gdal.h"

TEST( MeshGdalGribTest, ScalarFile )
{
  std::string path = test_file( "/grib/Madagascar.wave.7days.grb" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 3, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Primary wave mean period [s]" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 27, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( false, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1683, count );

  double value = getValue( ds, 1600 );
  EXPECT_DOUBLE_EQ( 15.34, value );

  MDAL_CloseMesh( m );
}

TEST( MeshGdalGribTest, VectorFile )
{
  std::string path = test_file( "/grib/Madagascar.wind.7days.grb" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "wind [m/s]" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 27, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 15 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( false, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1683, count );

  double valueX = getValueX( ds, 1600 );
  EXPECT_DOUBLE_EQ( -5.9000000000000004, valueX );

  double valueY = getValueY( ds, 1600 );
  EXPECT_DOUBLE_EQ( 2.8200097656250001, valueY );

  MDAL_CloseMesh( m );
}

// with older GDAL versions it raises getgridtemplate: GDT Template 3.12 not defined.
#if GDAL_VERSION_MAJOR >=2 && GDAL_VERSION_MINOR >=3
TEST( MeshGdalGribTest, WithoutNODATA )
{
  // see https://github.com/lutraconsulting/MDAL/issues/104
  std::string path = test_file( "/grib/saga_flow_without_nodata.grb" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "flow" ), std::string( name ) );

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
  ASSERT_EQ( 191178, count );

  double valueX = getValueX( ds, 1600 );
  EXPECT_DOUBLE_EQ( 0, valueX );

  double valueY = getValueY( ds, 1600 );
  EXPECT_DOUBLE_EQ( 1, valueY );
  MDAL_CloseMesh( m );
}
#endif

TEST( MeshGdalGribTest, ScalarFileWithUComponent )
{
  // https://github.com/lutraconsulting/MDAL/issues/79
  std::string path = test_file( "/grib/wind_only_u_component.grib" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "10 metre wind [m/s]" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( true, onVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 115680, count );

  double value = getValue( ds, 1600 );
  EXPECT_DOUBLE_EQ( -0.818756103515625, value );

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

