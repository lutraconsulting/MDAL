/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2022 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <math.h>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"


TEST( MeshH2iTest, Driver )
{
  MDAL_DriverH driver = MDAL_driverFromName( "H2i" );
  EXPECT_EQ( strcmp( MDAL_DR_filters( driver ), "*.json" ), 0 );
  EXPECT_TRUE( MDAL_DR_meshLoadCapability( driver ) );
  EXPECT_FALSE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( MDAL_DR_faceVerticesMaximumCount( driver ), 4 );
}

TEST( MeshH2iTest, LoadMesh )
{
  std::string path = test_file( "/h2i/de_tol_small/metadata.json" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "H2i:\"" + path + "\":de tol small" );

  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );

}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

