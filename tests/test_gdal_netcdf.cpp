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

TEST( MeshGdalNetCDFTest, OceanCurrents )
{
  std::string path = test_file( std::string( "/netcdf/Copernicus Ocean Currents Forecast Model/cmems_global-analysis-forecast-phy-001-024.nc" ) );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "NETCDF:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( -51, minX );
  EXPECT_DOUBLE_EQ( -31, maxX );
  EXPECT_DOUBLE_EQ( -30, minY );
  EXPECT_DOUBLE_EQ( -15, maxY );

  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Eulerian velocity (Navier-Stokes current)_depth:0.49402499" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  int active = getActive( ds, 144 );
  EXPECT_EQ( 1, active );

  active = getActive( ds, 143 );
  EXPECT_EQ( 0, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 43621, count );

  double value = getValueY( ds, 144 );
  EXPECT_DOUBLE_EQ( -0.4306640625, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0009765625, min );
  EXPECT_DOUBLE_EQ( 0.86543495488269506, max );

  EXPECT_TRUE( compareReferenceTime( g, "1950-01-01T00:00:00" ) );

  ds = MDAL_G_dataset( g, 0 );
  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 613752.5, time ) );

  MDAL_CloseMesh( m );
}

TEST( MeshGdalNetCDFTest, Indonesia )
{
  std::vector<std::string> files;
  files.push_back( "indonesia_nc3.nc" );
  files.push_back( "indonesia_nc4.nc" );
  for ( const std::string &file : files )
  {
    std::string path = test_file( std::string( "/netcdf/" ) + file );
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "NETCDF:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );

    double minX, maxX, minY, maxY;
    MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
    EXPECT_DOUBLE_EQ( 100, minX );
    EXPECT_DOUBLE_EQ( 150, maxX );
    EXPECT_DOUBLE_EQ( -10, minY );
    EXPECT_DOUBLE_EQ( 6, maxY );

    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Total cloud cover" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 31, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
    int active = getActive( ds, 50 );
    EXPECT_EQ( 1, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 234, count );

    double value = getValue( ds, 50 );
    EXPECT_DOUBLE_EQ( 0.99988487698798889, value );

    EXPECT_TRUE( compareReferenceTime( g, "1900-01-01T00:00:00" ) );

    ds = MDAL_G_dataset( g, 0 );
    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 1008072, time ) );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshGdalNetCDFTest, InconsistentDatasets )
{
  std::string path = test_file( std::string( "/netcdf/nonConstitentDataset.nc" ) );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "NETCDF:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 351958.45, minX );
  EXPECT_DOUBLE_EQ( 351970.45, maxX );
  EXPECT_DOUBLE_EQ( 6690971.55, minY );
  EXPECT_DOUBLE_EQ( 6690978.55, maxY );

  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "gti" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 58, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 50 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  int active = getActive( ds, 260 );
  EXPECT_EQ( 1, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 375, count );

  double value = getValue( ds, 260 );
  EXPECT_DOUBLE_EQ( 31.05268039902328, value );

  EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );

  ds = MDAL_G_dataset( g, 0 );
  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 429528.25, time ) );

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

