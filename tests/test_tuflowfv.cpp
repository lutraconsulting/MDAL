/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshTuflowFVTest, TrapSteady053D )
{
  std::string path = test_file( "/tuflowfv/withoutMaxes/trap_steady_05_3D.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 369 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 320, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 1, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 3, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 7, f_v );

  ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  // /////////////////////////////////
  // Dataset: Bed Elevation
  // /////////////////////////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices2D );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    // check that 3D api returns zeroes
    EXPECT_DOUBLE_EQ( 0, getValue3D( ds, 0 ) );
    EXPECT_DOUBLE_EQ( 0, getValue3DX( ds, 0 ) );
    EXPECT_DOUBLE_EQ( 0, getValue3DY( ds, 0 ) );
    EXPECT_DOUBLE_EQ( -1, getLevelZ3D( ds, 0 ) );
    EXPECT_EQ( -1, getLevelsCount3D( ds, 0 ) );
    EXPECT_EQ( -1, get3DFrom2D( ds, 0 ) );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 369, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( -1.2562500238418579, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( -5.9875001907348633, min );
    EXPECT_DOUBLE_EQ( -1.25, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( -5.9875001907348633, min );
    EXPECT_DOUBLE_EQ( -1.25, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 0.0, time );
  }

  // /////////////////////////////////
  // Dataset: scalar on volumes
  // /////////////////////////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "temperature" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVolumes3D );

    ASSERT_EQ( 37, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 3 );
    ASSERT_NE( ds, nullptr );

    int maxLevels = MDAL_D_maximumVerticalLevelCount( ds );
    EXPECT_EQ( maxLevels, 10 );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    // check that 2D api returns zeroes
    EXPECT_DOUBLE_EQ( 0, getValue( ds, 0 ) );
    EXPECT_DOUBLE_EQ( 0, getValueX( ds, 0 ) );
    EXPECT_DOUBLE_EQ( 0, getValueY( ds, 0 ) );

    int index3d = get3DFrom2D( ds, 155 );
    EXPECT_EQ( 1550, index3d );

    int levelsCount = getLevelsCount3D( ds, 0 );
    EXPECT_EQ( 10, levelsCount );

    double levelZ1 = getLevelZ3D( ds, 0 );
    EXPECT_DOUBLE_EQ( -1.2562493085861206, levelZ1 );

    double levelZ2 = getLevelZ3D( ds, 11 );
    EXPECT_DOUBLE_EQ( -3.333423376083374, levelZ2 );

    double levelZ3 = getLevelZ3D( ds, 21 );
    EXPECT_DOUBLE_EQ( -3.762500524520874, levelZ3 );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 3200, count );

    double value = getValue3D( ds, 222 );
    EXPECT_DOUBLE_EQ( 0.49942651391029358, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 33.165805816650391, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 34.741443634033203, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 0.502121734619141, time );
  }

  // /////////////////////////////////
  // Dataset: vector on volumes
  // /////////////////////////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "velocity" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVolumes3D );

    ASSERT_EQ( 37, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 4 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    int active = getActive( ds, 0 );
    EXPECT_EQ( 0, active ); // edge values is inactive

    active = getActive( ds, 200 );
    EXPECT_EQ( 1, active ); // middle values is active

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 3200, count );

    int volumes = MDAL_D_volumesCount( ds );
    ASSERT_EQ( 3200, volumes );

    double valueX = getValue3DX( ds, 444 );
    EXPECT_DOUBLE_EQ( 0.0, valueX );

    double valueY = getValue3DY( ds, 444 );
    EXPECT_DOUBLE_EQ( 0.0, valueY );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 1.2225217354813651, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 1.7670363355554111, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 0.667265041139391, time );
  }

  // /////////////////////////////////
  // Dataset: scalar on faces
  // /////////////////////////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "water depth" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

    ASSERT_EQ( 37, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 7 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    int active = getActive( ds, 0 );
    EXPECT_EQ( 0, active );

    active = getActive( ds, 200 );
    EXPECT_EQ( 1, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 320, count );

    double value = getValue( ds, 789 );
    EXPECT_DOUBLE_EQ( 0.0, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1.7076177982744412e-07, min );
    EXPECT_DOUBLE_EQ( 2.4886724948883057, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1.1920928955078125e-07, min );
    EXPECT_DOUBLE_EQ( 2.488835334777832, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 1.16755709277259, time );
  }

  // Close mesh
  MDAL_CloseMesh( m );
}

TEST( MeshTuflowFVTest, TrapSteady053DWithMaxes )
{
  std::string path = test_file( "/tuflowfv/withMaxes/trap_steady_05_3D.nc" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 369 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 320, f_count );

  ASSERT_EQ( 21, MDAL_M_datasetGroupCount( m ) );

  // /////////////////////////////////
  // Dataset: maximums dataset
  // /////////////////////////////////
  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "temperature/Maximums" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVolumes3D );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 0, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 0, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 0, time );
  }

  // Close mesh
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
