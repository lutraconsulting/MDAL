/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at reos dot site)
*/
#include "gtest/gtest.h"


#include <fstream>
#include <iostream>
#include <math.h>
#include <bitset>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

TEST( MeshEsriTinTest, load_mesh_simple )
{
  std::string path = test_file( "/esri_tin/mesh_simple/tnxy.adf" );

  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "ESRI_TIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );

  ASSERT_TRUE( nullptr != m );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 8 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 7 );

  std::vector<std::vector<size_t>> faceIndexes(
  {
    {2, 4, 1},
    {6, 7, 2},
    {5, 0, 4},
    {7, 4, 2},
    {4, 0, 1},
    {3, 0, 5},
    {5, 4, 7},
  } );

  for ( size_t i = 0; i < static_cast<size_t>( f_count ); ++i )
    for ( size_t j = 0; j < 3; j++ )
      EXPECT_EQ( faceIndexes[i][j], getFaceVerticesIndexAt( m, int( i ), int( j ) ) );

  for ( int i = 0; i < f_count; ++i )
  {
    int count = getFaceVerticesCountAt( m, i );
    EXPECT_EQ( 3, count );
  }

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( "", crs );

  MDAL_CloseMesh( m );
}

TEST( MeshEsriTinTest, test_dem_tintriangle )
{
  std::string path = test_file( "/esri_tin/dem/tnxy.adf" );

  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "ESRI_TIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );

  ASSERT_TRUE( nullptr != m );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 277 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 528 );

  double x = getVertexXCoordinatesAt( m, 10 );
  double y = getVertexYCoordinatesAt( m, 10 );
  double z = getVertexZCoordinatesAt( m, 10 );

  EXPECT_TRUE( fabs( x - 18.681404444 ) < 0.0000001 );
  EXPECT_TRUE( fabs( y - 45.795076438 ) < 0.0000001 );
  EXPECT_TRUE( fabs( z - 196.7906 ) < 0.001 );

  MDAL_CloseMesh( m );
}

TEST( MeshEsriTinTest, test_dem_with_holes_tintriangle )
{
  std::string path = test_file( "/esri_tin/dem_with_holes/tnxy.adf" );

  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "ESRI_TIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );

  ASSERT_TRUE( nullptr != m );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 518 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 773 );

  double x = getVertexXCoordinatesAt( m, 10 );
  double y = getVertexYCoordinatesAt( m, 10 );
  double z = getVertexZCoordinatesAt( m, 10 );

  EXPECT_TRUE( fabs( x - 18.6814065 ) < 0.0000001 );
  EXPECT_TRUE( fabs( y - 45.795075 ) < 0.0000001 );
  EXPECT_TRUE( fabs( z - 196.7906 ) < 0.001 );

  MDAL_CloseMesh( m );
}

TEST( MeshEsriTinTest, test_Islands_tintriangle )
{
  std::string path = test_file( "/esri_tin/islands/tnxy.adf" );

  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "ESRI_TIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );

  ASSERT_TRUE( nullptr != m );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 402 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 462 );

  double x = getVertexXCoordinatesAt( m, 10 );
  double y = getVertexYCoordinatesAt( m, 10 );
  double z = getVertexZCoordinatesAt( m, 10 );

  EXPECT_TRUE( fabs( x - 18.69513731324716 ) < 0.0000001 );
  EXPECT_TRUE( fabs( y - 45.7859876600764 ) < 0.0000001 );
  EXPECT_TRUE( fabs( z - 146.6611 ) < 0.001 );

  // Bed elevation dataset
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Altitude" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 402, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 200, value );
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
