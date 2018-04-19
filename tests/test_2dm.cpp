/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( Mesh2DMTest, MissingFile )
{
  MeshH m = MDAL_LoadMesh( "non/existent/path.2dm" );
  EXPECT_EQ( nullptr, m );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_FileNotFound, s );
}

TEST( Mesh2DMTest, WrongFile )
{
  std::string path( data_path() );
  path += "/2dm/not_a_mesh_file.2dm";
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
}

TEST( Mesh2DMTest, QuadAndTriangleFile )
{
  std::string path( data_path() );
  path += "/2dm/quad_and_triangle.2dm";
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );
  double x = MDAL_M_vertexXCoordinatesAt( m, 0 );
  double y = MDAL_M_vertexYCoordinatesAt( m, 0 );
  EXPECT_EQ( 1000.0, x );
  EXPECT_EQ( 2000.0, y );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 2, f_count );

  int f_v_count = MDAL_M_faceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = MDAL_M_faceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 0, f_v );

  f_v_count = MDAL_M_faceVerticesCountAt( m, 1 );
  EXPECT_EQ( f_v_count, 3 ); //triangle
  f_v = MDAL_M_faceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 1, f_v );

  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

