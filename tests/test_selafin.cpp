/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 ARTELIA - Christophe Coulet
 (christophe dot coulet at arteliagroup dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

TEST( MeshSLFTest, MalpassetGeometry )
{
  std::string path = test_file( "/slf/example.slf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SELAFIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 13541 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 5905.615234375, x );
  EXPECT_DOUBLE_EQ( 4695.9560546875, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  x = getVertexXCoordinatesAt( m, 1000 );
  y = getVertexYCoordinatesAt( m, 1000 );
  z = getVertexZCoordinatesAt( m, 1000 );
  EXPECT_DOUBLE_EQ( 16275.708984375, x );
  EXPECT_DOUBLE_EQ( -936.93072509765625, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 26000, f_count );

  // ///////////
  // Edges
  // ///////////
  EXPECT_EQ( 0, MDAL_M_edgeCount( m ) );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!
  int f_v = getFaceVerticesIndexAt( m, 100, 0 );
  EXPECT_EQ( 6807, f_v );
  f_v = getFaceVerticesIndexAt( m, 100, 1 );
  EXPECT_EQ( 6277, f_v ); \
  f_v = getFaceVerticesIndexAt( m, 100, 2 );
  EXPECT_EQ( 6811, f_v );

  // Datasets
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "bottom" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 13541, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 70.0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 94.5398330688477, value );
  value = getValue( ds, 1000 );
  EXPECT_DOUBLE_EQ( 1.73051724061679e-008, value );
  value = getValue( ds, 9571 );
  EXPECT_DOUBLE_EQ( 7.5623664855957, value );

  MDAL_CloseMesh( m );
}

TEST( MeshSLFTest, MalpassetResultFrench )
{
  std::string path = test_file( "/slf/example_res_fr.slf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "SELAFIN:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "SELAFIN" );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 13541 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );
  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 26000, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //only triangles!

  int var_count = MDAL_M_datasetGroupCount( m );
  ASSERT_EQ( 4, var_count ); // 4 variables (Velocity, Water Depth, Free Surface and Bottom)

  // ///////////
  // Scalar Dataset
  // ///////////
  MDAL_DatasetGroupH r = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( r, nullptr );

  int meta_count = MDAL_G_metadataCount( r );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( r );
  EXPECT_EQ( std::string( "surface libre   m" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( r );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( r );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( r ) );
  MDAL_DatasetH ds = MDAL_G_dataset( r, 1 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 1.111111111, time ) );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 13541, count );

  double value = getValue( ds, 8667 );
  EXPECT_DOUBLE_EQ( 31.965662002563477, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -0.00673320097848773, min );
  EXPECT_DOUBLE_EQ( 100.00228118896484, max );

  MDAL_G_minimumMaximum( r, &min, &max );
  EXPECT_DOUBLE_EQ( -0.00673320097848773, min );
  EXPECT_DOUBLE_EQ( 100.00228118896484, max );

  // ///////////
  // Vector Dataset
  // ///////////
  r = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( r, nullptr );

  meta_count = MDAL_G_metadataCount( r );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( r );
  EXPECT_EQ( std::string( "vitesse       ms" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( r );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( r );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( r ) );
  ds = MDAL_G_dataset( r, 1 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 13541, count );

  value = getValueX( ds, 8667 );
  EXPECT_DOUBLE_EQ( 6.2320127487182617, value );
  value = getValueY( ds, 8667 );
  EXPECT_DOUBLE_EQ( -0.97271907329559326, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 0, min ) );
  EXPECT_TRUE( MDAL::equals( 7.5673562379016834, max ) );

  EXPECT_TRUE( compareReferenceTime( r, "1900-01-01T00:00:00" ) );

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
