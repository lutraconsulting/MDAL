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

  EXPECT_EQ( MDAL_M_faceCount( m ), 724 );
  EXPECT_EQ( MDAL_M_vertexCount( m ), 841 );

  double x = getVertexXCoordinatesAt( m, 5 );
  double y = getVertexYCoordinatesAt( m, 5 );

  EXPECT_EQ( x, 126627.0 );
  EXPECT_EQ( y, 463967.0 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 16 ), 6 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 0 ), 24 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 1 ), 23 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 2 ), 26 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 3 ), 27 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 4 ), 28 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 5 ), 29 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 500 ), 5 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 0 ), 533 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 1 ), 586 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 2 ), 587 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 3 ), 588 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 4 ), 534 );

  EXPECT_EQ( std::string( "epsg:28992" ), std::string( MDAL_M_projection( m ) ) );

  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( m, 0 );
  ASSERT_EQ( std::string( "topography" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( group ) );
  EXPECT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  EXPECT_EQ( MDAL_G_datasetCount( group ), 1 );

  MDAL_DatasetH dataset = MDAL_G_dataset( group, 0 );
  double max = -std::numeric_limits<double>::max();
  double min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -3.9 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.415 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 15 ), -1.59, 0.001 ) );

  group = MDAL_M_datasetGroup( m, 1 );
  ASSERT_EQ( std::string( "thin water depth" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( group ) );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  ASSERT_EQ( MDAL_G_datasetCount( group ), 301 );
  ASSERT_EQ( MDAL_G_metadataCount( group ), 3 );
  EXPECT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m" ) );
  EXPECT_EQ( std::string( "type" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), std::string( "depth" ) );
  MDAL_G_minimumMaximum( group, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.8501256272955278 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0, 0.001 ) );

  dataset = MDAL_G_dataset( group, 1 );
  compareDurationInHours( MDAL_D_time( dataset ), MDAL::RelativeTimestamp( 4, MDAL::RelativeTimestamp::seconds ).value( MDAL::RelativeTimestamp::hours ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.617, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0.116, 0.001 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), MDAL::RelativeTimestamp( 4, MDAL::RelativeTimestamp::seconds ).value( MDAL::RelativeTimestamp::hours ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.804, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0, 0.001 ) );

  group = MDAL_M_datasetGroup( m, 3 );
  ASSERT_EQ( std::string( "water level" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( group ) );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  ASSERT_EQ( MDAL_G_datasetCount( group ), 301 );
  ASSERT_EQ( MDAL_G_metadataCount( group ), 3 );
  EXPECT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m+datum" ) );
  EXPECT_EQ( std::string( "type" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), std::string( "level" ) );
  MDAL_G_minimumMaximum( group, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -2.0669352841761794 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.40068597138927825 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.8, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), -1.8, 0.001 ) );

  dataset = MDAL_G_dataset( group, 1 );
  compareDurationInHours( MDAL_D_time( dataset ), MDAL::RelativeTimestamp( 4, MDAL::RelativeTimestamp::seconds ).value( MDAL::RelativeTimestamp::hours ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.799999775394419, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), -1.799999775394419 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), MDAL::RelativeTimestamp( 4, MDAL::RelativeTimestamp::seconds ).value( MDAL::RelativeTimestamp::hours ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.9818994301404425 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.40068558242701124 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), -1.535845010541368 ) );

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

