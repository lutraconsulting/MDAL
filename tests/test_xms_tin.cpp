/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"


TEST( MeshTinTest, ParaboloidFile )
{
  std::string path = test_file( "/tin/paraboloid.m.tin" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 3 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "XMS_TIN" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 239 );
  double x = getVertexXCoordinatesAt( m, 2 );
  double y = getVertexYCoordinatesAt( m, 2 );
  double z = getVertexZCoordinatesAt( m, 2 );
  EXPECT_DOUBLE_EQ( 43.741403313474954, x );
  EXPECT_DOUBLE_EQ( 2.8822251839036763, y );
  EXPECT_DOUBLE_EQ( 37.262135141179115, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 347, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 3 );
  EXPECT_EQ( 3, f_v_count );
  int f_v = getFaceVerticesIndexAt( m, 3, 0 );
  EXPECT_EQ( 36, f_v );
  f_v = getFaceVerticesIndexAt( m, 3, 1 );
  EXPECT_EQ( 68, f_v );
  f_v = getFaceVerticesIndexAt( m, 3, 2 );
  EXPECT_EQ( 231, f_v );


  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( -43.822593540032521, minX );
  EXPECT_DOUBLE_EQ( 44.986275235744273, maxX );
  EXPECT_DOUBLE_EQ( -45.012860971759714, minY );
  EXPECT_DOUBLE_EQ( 43.78966755326303, maxY );

  // Bed elevation dataset
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 239, count );

  double value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 37.13464052406677, value );

  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, LinesFile )
{
  std::string path = test_file( "/2dm/lines.2dm" );

  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "2DM" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 4 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 1000.0, x );
  EXPECT_DOUBLE_EQ( 2000.0, y );
  EXPECT_DOUBLE_EQ( 20.0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 0, f_count );

  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( 3, e_count );

  std::vector<int> startVertices;
  std::vector<int> endVertices;
  getEdgeVertexIndices( m, e_count, startVertices, endVertices );
  EXPECT_TRUE( compareVectors( startVertices, {0, 1, 2} ) );
  EXPECT_TRUE( compareVectors( endVertices, {1, 2, 3} ) );


  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 1000, minX );
  EXPECT_DOUBLE_EQ( 3000, maxX );
  EXPECT_DOUBLE_EQ( 2000, minY );
  EXPECT_DOUBLE_EQ( 3000, maxY );

  std::vector<int> expectedStartVertexIndices = {1, 2, 3};
  std::vector<int> expectedEndVertexIndices = {2, 3, 4};

  std::vector<int> startVertexIndices;
  std::vector<int> endVertexIndices;
  getEdgeVertexIndices( m, e_count, startVertexIndices, endVertexIndices );

  // Bed elevation dataset
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  double value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 30, value );

  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, RegularGridFile )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );

  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 1976 );
  double x = getVertexXCoordinatesAt( m, 1000 );
  double y = getVertexYCoordinatesAt( m, 1000 );
  EXPECT_DOUBLE_EQ( 381473.785, x );
  EXPECT_DOUBLE_EQ( 168726.985, y );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 1875, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 0, f_v );

  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, Basement3CellElevationTest )
{
  std::string path = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int maxCount = MDAL_M_faceVerticesMaximumCount( m );
  EXPECT_EQ( maxCount, 4 );

  std::string driverName = MDAL_M_driverName( m );
  EXPECT_EQ( driverName, "2DM" );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 54 );
  double x = getVertexXCoordinatesAt( m, 4 );
  double y = getVertexYCoordinatesAt( m, 4 );
  double z = getVertexZCoordinatesAt( m, 4 );
  EXPECT_DOUBLE_EQ( 8.0, x );
  EXPECT_DOUBLE_EQ( 0.0, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 77, f_count );

  int f_v_count = getFaceVerticesCountAt( m, 0 );
  EXPECT_EQ( 3, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 0, 0 );
  EXPECT_EQ( 29, f_v );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 0, minX );
  EXPECT_DOUBLE_EQ( 20, maxX );
  EXPECT_DOUBLE_EQ( 0, minY );
  EXPECT_DOUBLE_EQ( 5, maxY );

  f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( f_v_count, 3 ); //triangle
  f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // Bed elevation dataset
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  {
    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 54, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0, value );
  }

  // Bed elevation dataset and face elevation dataset
  {

    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation (Face)" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 77, count );

    double value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 0.19500000000000001, value );
  }

  MDAL_CloseMesh( m );
}


TEST( Mesh2DMTest, Save2DMeshToFile )
{
  saveAndCompareMesh(
    test_file( "/2dm/quad_and_triangle.2dm" ),
    tmp_file( "/quad_and_triangle_saved.2dm" ),
    "2DM"
  );
}

TEST( Mesh2DMTest, Save1DMeshToFile )
{
  saveAndCompareMesh(
    test_file( "/2dm/lines.2dm" ),
    tmp_file( "/lines_saved.2dm" ),
    "2DM"
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

