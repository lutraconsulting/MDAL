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

TEST( MeshEsriTinTest, loadTopMesh )
{
  std::string path = test_file( "/esri_tin/top/tnxy.adf" );
  //credits for TIN files: http://lab.usgin.org/

  MeshH m = MDAL_LoadMesh( path.c_str() );

  ASSERT_TRUE( nullptr != m );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 13 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 16 );

  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );

  EXPECT_TRUE( fabs( x + 87.779 ) < 0.0001 );
  EXPECT_TRUE( fabs( y - 34.076 ) < 0001 );
  EXPECT_TRUE( fabs( z - 1100 ) < 0.0001 );

  x = getVertexXCoordinatesAt( m, v_count - 1 );
  y = getVertexYCoordinatesAt( m, v_count - 1 );
  z = getVertexZCoordinatesAt( m, v_count - 1 );

  EXPECT_TRUE( fabs( x + 87.6369 ) < 0.0001 );
  EXPECT_TRUE( fabs( y - 33.7108 ) < 0.0001 );
  EXPECT_TRUE( fabs( z - 0 ) < 0.0001 );

  for ( int i = 0; i < f_count; ++i )
  {
    int count = getFaceVerticesCountAt( m, i );
    EXPECT_EQ( 3, count );
  }

  EXPECT_EQ( 10, getFaceVerticesIndexAt( m, 1, 0 ) );
  EXPECT_EQ( 6, getFaceVerticesIndexAt( m, 1, 1 ) );
  EXPECT_EQ( 3, getFaceVerticesIndexAt( m, 1, 2 ) );
  EXPECT_EQ( 4, getFaceVerticesIndexAt( m, 4, 0 ) );
  EXPECT_EQ( 3, getFaceVerticesIndexAt( m, 4, 1 ) );
  EXPECT_EQ( 7, getFaceVerticesIndexAt( m, 4, 2 ) );

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( "", crs );

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
