/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2023 Lutra Consulting Ltd.
*/

#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

#include <regex>

TEST( MeshMike21Test, Driver )
{
  MDAL_DriverH driver = MDAL_driverFromName( "Mike21" );
  EXPECT_EQ( strcmp( MDAL_DR_filters( driver ), "*.mesh" ), 0 );
  EXPECT_TRUE( MDAL_DR_meshLoadCapability( driver ) );
  EXPECT_TRUE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( strcmp( MDAL_DR_saveMeshSuffix( driver ), "mesh" ), 0 );
  EXPECT_EQ( MDAL_DR_faceVerticesMaximumCount( driver ), 6 );
}

TEST( MeshMike21Test, MissingFile )
{
  MDAL_MeshH m = MDAL_LoadMesh( "non/existent/path.mesh" );
  EXPECT_EQ( nullptr, m );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_FileNotFound, s );
}

TEST( MeshMike21Test, WrongFile )
{
  std::string path = test_file( "/mike21/not_a_mesh_file.mesh" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
}

TEST( MeshMike21Test, WrongVertexCount )
{
  std::string path = test_file( "/mike21/wrong_vertex_count.mesh" );
  path = "Mike21:\"" + path + "\"";
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_InvalidData, s );
}

TEST( MeshMike21Test, WrongElementType )
{
  std::string path = test_file( "/mike21/wrong_element_type.mesh" );
  path = "Mike21:\"" + path + "\"";
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_InvalidData, s );
}

TEST( MeshMike21Test, WrongElementLineFormat )
{
  std::string path = test_file( "/mike21/wrong_element_line_format.mesh" );
  path = "Mike21:\"" + path + "\"";
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_InvalidData, s );
}

TEST( MeshMike21Test, WrongFaceCount )
{
  std::string path = test_file( "/mike21/wrong_face_count.mesh" );
  path = "Mike21:\"" + path + "\"";
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_EQ( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_InvalidData, s );
}

TEST( MeshMike21Test, ReadSmallMesh )
{
  std::string path = test_file( "/mike21/small.mesh" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Mike21:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12 );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 9 );
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 0, e_count );

  EXPECT_STREQ( MDAL_M_projection( m ), "LONG/LAT" );

  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( m ), 4 );

  EXPECT_EQ( MDAL_M_datasetGroupCount( m ), 2 );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 0 ) ), "VertexType" );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 1 ) ), "Bed Elevation" );
}

TEST( MeshMike21Test, ReadOdenseRoughQuads )
{
  std::string path = test_file( "/mike21/odense_rough_quads.mesh" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Mike21:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( s, MDAL_Status::None );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 535 );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 724 );
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 0, e_count );

  EXPECT_STREQ( MDAL_M_projection( m ), "UTM-33" );

  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( m ), 4 );

  EXPECT_EQ( MDAL_M_datasetGroupCount( m ), 2 );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 0 ) ), "VertexType" );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 1 ) ), "Bed Elevation" );
}

TEST( MeshMike21Test, ReadOdenseRough )
{
  std::string path = test_file( "/mike21/odense_rough.mesh" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Mike21:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 399 );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 654 );
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 0, e_count );

  EXPECT_STREQ( MDAL_M_projection( m ), "UTM-33" );

  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( m ), 3 );

  EXPECT_EQ( MDAL_M_datasetGroupCount( m ), 2 );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 0 ) ), "VertexType" );
  EXPECT_STREQ( MDAL_G_name( MDAL_M_datasetGroup( m, 1 ) ), "Bed Elevation" );
}

TEST( MeshMike21Test, SaveMike21MeshToFile )
{
  saveAndCompareMesh(
    test_file( "/mike21/odense_rough_comparison.mesh" ),
    tmp_file( "/odense_rough_saved.mesh" ),
    "Mike21"
  );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

