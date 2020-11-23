/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"


TEST( MeshDynamicDriverTest, openMesh )
{
#ifdef WIN32
  size_t requiredSize = 0;
  getenv_s( &requiredSize, NULL, 0, "MDAL_DRIVER_PATH" );
  if ( requiredSize == 0 )
  {
    _putenv_s( "MDAL_DRIVER_PATH", DRIVERS_PATH );
  }
#endif

#ifndef WIN32
  setenv( "MDAL_DRIVER_PATH", DRIVERS_PATH, 0 );
#endif

  std::string path = test_file( "/dynamic_driver/mesh_1.msh" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  // Vertices
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );

  EXPECT_EQ( getVertexXCoordinatesAt( m, 0 ), 1000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 0 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 0 ), 0.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 1 ), 2000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 1 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 1 ), 1.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 2 ), 3000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 2 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 2 ), 2.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 3 ), 2000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 3 ), 3000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 3 ), 3.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 4 ), 1000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 4 ), 3000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 4 ), 4.0 );

  // Faces
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 2 );
  EXPECT_EQ( getFaceVerticesCountAt( m, 0 ), 4 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 0 ), 0 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 1 ), 1 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 2 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 3 ), 4 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 1 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 0 ), 1 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 1 ), 2 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 2 ), 3 );

  // Edges
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( e_count, 3 );
  std::vector<int> start;
  std::vector<int> end;
  getEdgeVertexIndices( m, e_count, start, end );
  EXPECT_EQ( start.at( 0 ), 0 );
  EXPECT_EQ( end.at( 0 ), 1 );
  EXPECT_EQ( start.at( 1 ), 3 );
  EXPECT_EQ( end.at( 1 ), 4 );
  EXPECT_EQ( start.at( 2 ), 4 );
  EXPECT_EQ( end.at( 2 ), 2 );

  double xMin, xMax, yMin, yMax;
  MDAL_M_extent( m, &xMin, &xMax, &yMin, &yMax );

  EXPECT_EQ( xMin, 1000 );
  EXPECT_EQ( xMax, 3000 );
  EXPECT_EQ( yMin, 2000 );
  EXPECT_EQ( yMax, 3000 );

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( crs, "EPSG::32620" );


  // Dataset
  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 6 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  EXPECT_TRUE( compareReferenceTime( g, "1990-02-03T01:02:00" ) );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 3, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "DatasetGroup_1" ), std::string( name ) );

  const char *metaKey = MDAL_G_metadataKey( g, 1 );
  EXPECT_EQ( std::string( "unit" ), std::string( metaKey ) );
  const char *metaValue = MDAL_G_metadataValue( g, 1 );
  EXPECT_EQ( std::string( "m" ), std::string( metaValue ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );
  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.0, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2.0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 3.0, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4.0, value );
  value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 5.0, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );

  /////////
  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  EXPECT_TRUE( compareReferenceTime( g, "1990-02-03T01:05:00" ) );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 3, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "DatasetGroup_6" ), std::string( name ) );

  metaKey = MDAL_G_metadataKey( g, 2 );
  EXPECT_EQ( std::string( "long unit" ), std::string( metaKey ) );
  metaValue = MDAL_G_metadataValue( g, 2 );
  EXPECT_EQ( std::string( "square meter per second" ), std::string( metaValue ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );
  ASSERT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.0, value );
  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 2.0, value );
  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 3.0, value );
  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2.0, value );

  EXPECT_EQ( getActive( ds, 0 ), 1 );
  EXPECT_EQ( getActive( ds, 1 ), 0 );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 2.236, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 3.606, max, 0.001 ) );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_TRUE( MDAL::equals( 1.0, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 3.606, max, 0.001 ) );

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
