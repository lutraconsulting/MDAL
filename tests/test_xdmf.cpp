/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

/*
TEST( XdmfTest, Basement3SimpleChannel )
{
  std::string path = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.xdmf" );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  // ///////////
  // Scalar Dataset
  // ///////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "h" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 21, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5790, count );

    double value = getValue( ds, 210 );
    EXPECT_DOUBLE_EQ( 0.0, value );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.97462528944015503, value);

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 1.0916943550109863, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( -0.15686002373695374, min );
    EXPECT_DOUBLE_EQ( 1.0916943550109863, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 100.895, time );

    // lets try another timestep too
    ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.19425453245639801, value );
  }

  MDAL_CloseMesh( m );
}

*/
TEST( XdmfTest, Simple )
{
  std::string path = test_file( "/xdmf/simple/simpleXFMD.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/simple/simpleXFMD.xmf" );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  // ///////////
  // Scalar Dataset
  // ///////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "h" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 21, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5790, count );

    double value = getValue( ds, 210 );
    EXPECT_DOUBLE_EQ( 0.0, value );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.97462530517332602, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 1.0916943102403027, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( -0.1568600200000731, min );
    EXPECT_DOUBLE_EQ( 1.0916943102403027, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 100.895, time );

    // lets try another timestep too
    ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.19425453944902599, value );
  }

  // //////////////
  // Vector Dataset
  // //////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "v" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    bool onVertices = MDAL_G_isOnVertices( g );
    EXPECT_EQ( false, onVertices );

    ASSERT_EQ( 21, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    bool active = getActive( ds, 0 );
    EXPECT_EQ( true, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5790, count );

    double valueX = getValueX( ds, 145 );
    EXPECT_DOUBLE_EQ( 3.0896201122474545, valueX );

    double valueY = getValueY( ds, 145 );
    EXPECT_DOUBLE_EQ( -0.02535337911766803, valueY );

    valueX = getValueX( ds, 196 );
    EXPECT_DOUBLE_EQ( 2.3246286772752853, valueX );

    valueY = getValueY( ds, 196 );
    EXPECT_DOUBLE_EQ( -0.27034955553581924, valueY );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 3.5179872000733399, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 3.5179872000733399, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 100.895, time );

    // lets try another timestep too
    ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    valueX = getValueX( ds, 196 );
    EXPECT_DOUBLE_EQ( 0, valueX );

    valueY = getValueY( ds, 196 );
    EXPECT_DOUBLE_EQ( 0, valueY );
  }

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
