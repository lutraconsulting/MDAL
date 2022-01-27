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
  MDAL_DriverH driver = MDAL_driverFromName( "H2I" );
  EXPECT_EQ( strcmp( MDAL_DR_filters( driver ), "*.json" ), 0 );
  EXPECT_TRUE( MDAL_DR_meshLoadCapability( driver ) );
  EXPECT_FALSE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( MDAL_DR_faceVerticesMaximumCount( driver ), 4 );
}

TEST( MeshH2iTest, LoadMesh )
{
  std::string path = test_file( "/h2i/de_tol_small/metadata.json" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "H2I:\"" + path + "\":de tol small" );

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

  ////////////////////////////
  /// water depth dataset group
  group = MDAL_M_datasetGroup( m, 3 );
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
  compareDurationInHours( MDAL_D_time( dataset ), 4 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.617, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0.116, 0.001 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), 11944 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.804, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0, 0.001 ) );

  ////////////////////////////
  /// water level group
  group = MDAL_M_datasetGroup( m, 5 );
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
  EXPECT_TRUE( compareDurationInHours( MDAL_D_time( dataset ), 4.0 / 3600 ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.799999775394419, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), -1.799999775394419 ) );

  dataset = MDAL_G_dataset( group, 200 );
  EXPECT_TRUE( compareDurationInHours( MDAL_D_time( dataset ), 11944.0 / 3600 ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.9818994301404425 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.40068558242701124 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), -1.535845010541368 ) );


  ////////////////////////////
  /// discharge dataset group
  group = MDAL_M_datasetGroup( m, 1 );
  ASSERT_EQ( std::string( "unit discharge" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_FALSE( MDAL_G_hasScalarData( group ) );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  ASSERT_EQ( MDAL_G_datasetCount( group ), 301 );
  ASSERT_EQ( MDAL_G_metadataCount( group ), 3 );
  EXPECT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m2/s" ) );
  EXPECT_EQ( std::string( "type" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), std::string( "discharge" ) );
  MDAL_G_minimumMaximum( group, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.41628520143256503 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0 ) );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 200 ), 0 ) );

  dataset = MDAL_G_dataset( group, 1 );
  compareDurationInHours( MDAL_D_time( dataset ), 4 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 2.2033972458650335e-07 ) );

  double vx = getValueX( dataset, 250 );
  double vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 250 ), -1.9082423691870207e-08 ) );
  EXPECT_TRUE( MDAL::equals( getValueY( dataset, 250 ), 2.142051116262502e-08 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), 11944 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.3516141096101225 ) );
  vx = getValueX( dataset, 250 );
  vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 250 ), -0.011015540811333473 ) );
  EXPECT_TRUE( MDAL::equals( getValueY( dataset, 250 ), -0.0065293025932666735 ) );

  ////////////////////////////
  /// velocity dataset group
  group = MDAL_M_datasetGroup( m, 2 );
  ASSERT_EQ( std::string( "velocity" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_FALSE( MDAL_G_hasScalarData( group ) );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  ASSERT_EQ( MDAL_G_datasetCount( group ), 301 );
  ASSERT_EQ( MDAL_G_metadataCount( group ), 3 );
  EXPECT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m/s" ) );
  EXPECT_EQ( std::string( "type" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), std::string( "velocity" ) );
  MDAL_G_minimumMaximum( group, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.9170254202699033 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0 ) );
  vx = getValueX( dataset, 200 );
  vy = getValueY( dataset, 200 );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 200 ), 0 ) );
  EXPECT_TRUE( MDAL::equals( getValueY( dataset, 200 ), 0 ) );


  dataset = MDAL_G_dataset( group, 1 );
  compareDurationInHours( MDAL_D_time( dataset ), 4 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.1972175260805653e-05 ) );

  vx = getValueX( dataset, 250 );
  vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 250 ), -3.3873910858682106e-08 ) );
  EXPECT_TRUE( MDAL::equals( getValueY( dataset, 250 ), 2.637530545593399e-08 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), 11944 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.9122031636962096 ) );
  vx = getValueX( dataset, 250 );
  vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( getValueX( dataset, 250 ), -0.011063229489062722 ) );
  EXPECT_TRUE( MDAL::equals( getValueY( dataset, 250 ), -0.006451940286861343 ) );

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

