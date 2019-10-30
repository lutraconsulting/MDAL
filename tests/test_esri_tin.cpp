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

  MeshH m = MDAL_LoadMesh( path.c_str() );

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

  MeshH m = MDAL_LoadMesh( path.c_str() );

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

  MeshH m = MDAL_LoadMesh( path.c_str() );

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

  MeshH m = MDAL_LoadMesh( path.c_str() );

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
