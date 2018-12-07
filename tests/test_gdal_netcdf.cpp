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

TEST( MeshGdalNetCDFTest, Indonesia )
{
  std::vector<std::string> files;
  files.push_back( "indonesia_nc3.nc" );
  files.push_back( "indonesia_nc4.nc" );
  for ( const std::string &file : files )
  {
    std::string path = test_file( std::string( "/netcdf/" ) + file );
    MeshH m = MDAL_LoadMesh( path.c_str() );
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

    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Total cloud cover" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( true, onVertices );

    ASSERT_EQ( 31, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 50 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 234, count );

    double value = getValue( ds, 50 );
    EXPECT_DOUBLE_EQ( 32759, value );

    MDAL_CloseMesh( m );
  }
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

