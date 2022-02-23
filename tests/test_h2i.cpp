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
  std::string path = test_file( "/h2i/de_tol_small/case1/metadata.json" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "H2I:\"" + path + "\":de tol small" );

  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );

  EXPECT_EQ( MDAL_M_faceCount( m ), 1141 );
  EXPECT_EQ( MDAL_M_vertexCount( m ), 1321 );

  double x = getVertexXCoordinatesAt( m, 5 );
  double y = getVertexYCoordinatesAt( m, 5 );

  EXPECT_TRUE( MDAL::equals( x, 126727.0 ) );
  EXPECT_TRUE( MDAL::equals( y, 464167.0 ) );

  EXPECT_EQ( getFaceVerticesCountAt( m, 16 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 0 ), 1311 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 1 ), 982 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 16, 2 ), 1241 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 500 ), 4 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 0 ), 1108 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 1 ), 1114 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 2 ), 1115 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 500, 3 ), 1129 );

  EXPECT_EQ( std::string( "epsg:28992" ), std::string( MDAL_M_projection( m ) ) );

  ////////////////////////////
  /// water depth dataset group
  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( m, 0 );
  group = MDAL_M_datasetGroup( m, 0 );
  ASSERT_EQ( std::string( "thin water depth" ), std::string( MDAL_G_name( group ) ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( group ) );
  ASSERT_EQ( MDAL_G_dataLocation( group ), MDAL_DataLocation::DataOnFaces );
  ASSERT_EQ( MDAL_G_datasetCount( group ), 301 );
  ASSERT_EQ( MDAL_G_metadataCount( group ), 3 );
  EXPECT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m" ) );
  EXPECT_EQ( std::string( "type" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  EXPECT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), std::string( "depth" ) );
  double min, max;
  MDAL_G_minimumMaximum( group, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.8518783400912862 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  MDAL_DatasetH dataset = MDAL_G_dataset( group, 0 );
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
  EXPECT_TRUE( MDAL::equals( max, 1.696, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 200 ), 0, 0.001 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), 11944 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );

  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.796, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 1010 ), 0.00498, 0.00001 ) );

  ////////////////////////////
  /// water level group
  group = MDAL_M_datasetGroup( m, 2 );
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
  EXPECT_TRUE( MDAL::equals( min, -2.2737410236741886 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.40068574307103255 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.8, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.400, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 1010 ), -1.440, 0.001 ) );

  dataset = MDAL_G_dataset( group, 1 );
  EXPECT_TRUE( compareDurationInHours( MDAL_D_time( dataset ), 4.0 / 3600 ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.7999999494328813 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 1010 ), -1.4399999607698168 ) );

  dataset = MDAL_G_dataset( group, 200 );
  EXPECT_TRUE( compareDurationInHours( MDAL_D_time( dataset ), 11944.0 / 3600 ) );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, -1.8584289966520853 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4006800596340498 ) );
  EXPECT_TRUE( MDAL::equals( getValue( dataset, 1010 ), -1.4270193439120757 ) );


  ////////////////////////////
  /// velocity dataset group
  group = MDAL_M_datasetGroup( m, 3 );
  ASSERT_EQ( std::string( "upwind velocity" ), std::string( MDAL_G_name( group ) ) );
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
  EXPECT_TRUE( MDAL::equals( max, 1.2733710589419995 ) );

  compareReferenceTime( group, "2001-01-01T00:00:00" );

  dataset = MDAL_G_dataset( group, 0 );
  compareDurationInHours( MDAL_D_time( dataset ), 0 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0 ) );
  double vx = getValueX( dataset, 200 );
  double vy = getValueY( dataset, 200 );
  EXPECT_TRUE( MDAL::equals( vx, 0 ) );
  EXPECT_TRUE( MDAL::equals( vy, 0 ) );

  dataset = MDAL_G_dataset( group, 30 );
  compareDurationInHours( MDAL_D_time( dataset ), 4 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 1.1414997205451265 ) );

  vx = getValueX( dataset, 250 );
  vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( vx, 0.012687793650851827 ) );
  EXPECT_TRUE( MDAL::equals( vy, 0.024070607273504109 ) );

  dataset = MDAL_G_dataset( group, 200 );
  compareDurationInHours( MDAL_D_time( dataset ), 11944 / 3600 );
  max = -std::numeric_limits<double>::max();
  min = std::numeric_limits<double>::max();
  MDAL_D_minimumMaximum( dataset, &min, &max );
  EXPECT_TRUE( MDAL::equals( min, 0 ) );
  EXPECT_TRUE( MDAL::equals( max, 0.4264180707596937 ) );
  vx = getValueX( dataset, 250 );
  vy = getValueY( dataset, 250 );
  EXPECT_TRUE( MDAL::equals( vx, 0.028074630083586842 ) );
  EXPECT_TRUE( MDAL::equals( vy, 0.028626587119002123 ) );

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

