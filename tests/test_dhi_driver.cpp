/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

TEST( MeshDhiDriverTest, loadDriverDfsu )
{
  MDAL_DriverH driver = MDAL_driverFromName( "DHI DFSU" );
  ASSERT_TRUE( driver );

  ASSERT_TRUE( MDAL_DR_meshLoadCapability( driver ) );

  std::string name( MDAL_DR_name( driver ) );
  std::string longName( MDAL_DR_longName( driver ) );
  EXPECT_EQ( name, std::string( "DHI DFSU" ) );
  EXPECT_EQ( longName, std::string( "DHI dfsu" ) );
}

TEST( MeshDhiDriverTest, loadDriverDfs2 )
{
  MDAL_DriverH driver = MDAL_driverFromName( "DHI DFS2" );
  ASSERT_TRUE( driver );

  ASSERT_TRUE( MDAL_DR_meshLoadCapability( driver ) );

  std::string name( MDAL_DR_name( driver ) );
  std::string longName( MDAL_DR_longName( driver ) );
  EXPECT_EQ( name, std::string( "DHI DFS2" ) );
  EXPECT_EQ( longName, std::string( "DHI dfs2" ) );
}

TEST( MeshDhiDriverTest, loadMesh )
{
  std::string path = test_file( "/dhi/small.dfsu" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 18811 );

  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_EQ( x, 3245279.0076439101 );
  EXPECT_EQ( y, 3480923.4169049999 );
  EXPECT_EQ( z, 13.496343612670898 );

  x = getVertexXCoordinatesAt( m, 10000 );
  y = getVertexYCoordinatesAt( m, 10000 );
  z = getVertexZCoordinatesAt( m, 10000 );
  EXPECT_EQ( x, 3245715.5411204398 );
  EXPECT_EQ( y, 3480983.0842166701 );
  EXPECT_EQ( z, 19.388742446899414 );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 36625 );
  int vertCount = getFaceVerticesCountAt( m, 0 );
  int vertInd = getFaceVerticesIndexAt( m, 0, 1 );
  EXPECT_EQ( vertCount, 4 );
  EXPECT_EQ( vertInd, 4 );
  vertCount = getFaceVerticesCountAt( m, 5000 );
  vertInd = getFaceVerticesIndexAt( m, 5000, 2 );
  EXPECT_EQ( vertCount, 3 );
  EXPECT_EQ( vertInd, 3906 );


  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH dsg = MDAL_M_datasetGroup( m, 0 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  EXPECT_TRUE( MDAL_G_hasScalarData( dsg ) );
  std::string groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "Surface elevation" ) );
  compareReferenceTime( dsg, "2018-01-01T00:08:20" );
  ASSERT_EQ( 5, MDAL_G_datasetCount( dsg ) );

  MDAL_DatasetH ds = MDAL_G_dataset( dsg, 0 );
  ASSERT_TRUE( ds );
  bool valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 36625, count );
  double time = MDAL_D_time( ds );
  EXPECT_EQ( 0, time );
  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  EXPECT_FALSE( getActive( ds, 1000 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  double value = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 14.4849767, value, 0.00001 ) );

  ds = MDAL_G_dataset( dsg, 1 );
  ASSERT_TRUE( ds );
  valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 0.1666666, time, 0.00001 ) );
  EXPECT_FALSE( getActive( ds, 1000 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  value = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 15.15912, value, 0.00001 ) );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
  dsg = MDAL_M_datasetGroup( m, 1 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  EXPECT_TRUE( MDAL_G_hasScalarData( dsg ) );
  groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "Total water depth" ) );
  compareReferenceTime( dsg, "2018-01-01T00:08:20" );
  ASSERT_EQ( 5, MDAL_G_datasetCount( dsg ) );

  ds = MDAL_G_dataset( dsg, 5 );
  EXPECT_FALSE( ds );

  ds = MDAL_G_dataset( dsg, 4 );
  ASSERT_TRUE( ds );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 0.6666667, time, 0.00001 ) );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  EXPECT_TRUE( getActive( ds, 100 ) );
  value = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 1.3017599582672119, value, 0.00001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshDhiDriverTest, meshWithVectorGroup )
{
  std::string path = test_file( "/dhi/OresundHD.dfsu" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 2057 );

  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_EQ( x, 359978.8125 );
  EXPECT_EQ( y, 6205400.0 );
  EXPECT_EQ( z, -1.9789566993713379 );

  x = getVertexXCoordinatesAt( m, 2000 );
  y = getVertexYCoordinatesAt( m, 2000 );
  z = getVertexZCoordinatesAt( m, 2000 );
  EXPECT_EQ( x, 330871.28125 );
  EXPECT_EQ( y, 6142677.5 );
  EXPECT_EQ( z, -2.0 );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 3636 );

  ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH dsg = MDAL_M_datasetGroup( m, 1 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  std::string groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "Velocity" ) );
  EXPECT_FALSE( MDAL_G_hasScalarData( dsg ) );

  ASSERT_EQ( 12, MDAL_G_datasetCount( dsg ) );

  MDAL_DatasetH ds = MDAL_G_dataset( dsg, 0 );
  ASSERT_TRUE( ds );
  bool valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  double time = MDAL_D_time( ds );
  EXPECT_EQ( 0, time );
  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  EXPECT_TRUE( getActive( ds, 200 ) );
  double valueX = getValueX( ds, 200 );
  double valueY = getValueY( ds, 200 );
  EXPECT_TRUE( MDAL::equals( 0.000000, valueX, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 0.000000, valueY, 0.00001 ) );

  ds = MDAL_G_dataset( dsg, 1 );
  ASSERT_TRUE( ds );
  valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 24.0, time, 0.00001 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  valueX = getValueX( ds, 100 );
  valueY = getValueY( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 0.237487, valueX, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.399625, valueY, 0.00001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshDhiDriverTest, meshWithVectorGroupDifferentName )
{
  std::string path = test_file( "/dhi/OdenseHD2D.dfsu" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 535 );

  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_EQ( x, 222397.9375 );
  EXPECT_EQ( y, 6163314.0 );
  EXPECT_EQ( z, -8.7315053939819336 );

  x = getVertexXCoordinatesAt( m, 500 );
  y = getVertexYCoordinatesAt( m, 500 );
  z = getVertexZCoordinatesAt( m, 500 );
  EXPECT_EQ( x, 213042.78125 );
  EXPECT_EQ( y, 6156939.5 );
  EXPECT_EQ( z, -0.29921221733093262 );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 724 );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH dsg = MDAL_M_datasetGroup( m, 1 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  std::string groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "Velocity" ) );
  EXPECT_FALSE( MDAL_G_hasScalarData( dsg ) );

  ASSERT_EQ( 13, MDAL_G_datasetCount( dsg ) );

  MDAL_DatasetH ds = MDAL_G_dataset( dsg, 0 );
  ASSERT_TRUE( ds );
  bool valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  double time = MDAL_D_time( ds );
  EXPECT_EQ( 0, time );
  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  EXPECT_TRUE( getActive( ds, 200 ) );
  double valueX = getValueX( ds, 200 );
  double valueY = getValueY( ds, 200 );
  EXPECT_TRUE( MDAL::equals( 0.000000, valueX, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 0.000000, valueY, 0.00001 ) );

  ds = MDAL_G_dataset( dsg, 1 );
  ASSERT_TRUE( ds );
  valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 24.0, time, 0.00001 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  valueX = getValueX( ds, 100 );
  valueY = getValueY( ds, 100 );
  EXPECT_TRUE( MDAL::equals( -0.04948761, valueX, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.02867248, valueY, 0.00001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshDhiDriverTest, loadMesh3DStackedWithVelocity )
{
  std::string path = test_file( "/dhi/HD-3D.dfsu" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 6146 );

  double x = getVertexXCoordinatesAt( m, 10 );
  double y = getVertexYCoordinatesAt( m, 10 );
  double z = getVertexZCoordinatesAt( m, 10 );

  EXPECT_TRUE( MDAL::equals( x, 455169.964160, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( y, 6145435.27265, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( z, -3.0825, 0.001 ) );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 11693 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 10 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 0 ), 28 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 1 ), 29 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 2 ), 30 );

  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 2 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );
  ASSERT_FALSE( MDAL_G_hasScalarData( g ) );

  std::string groupName = MDAL_G_name( g );
  EXPECT_TRUE( groupName == std::string( "Velocity" ) );

  EXPECT_EQ( 6, MDAL_G_datasetCount( g ) );
  double min, max;
  MDAL_G_minimumMaximum( g, &min, &max );

  EXPECT_TRUE( MDAL::equals( max, 1.90697, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( min, 0.0, 0.00001 ) );

  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 4, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 35079, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 1500 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 3, get3DFrom2D( ds, 1 ) );
  EXPECT_EQ( 10500, get3DFrom2D( ds, 3500 ) );

  int faceIndex = 0;
  EXPECT_TRUE( MDAL::equals( -4.529386, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -5.705848, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -6.882310, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -8.058772, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );

  faceIndex = 1500;
  EXPECT_TRUE( MDAL::equals( -1.112822, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.150430, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.188038, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.225645, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 0.0, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );

  ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 4, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 35079, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 1500 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 3, get3DFrom2D( ds, 1 ) );
  EXPECT_EQ( 10500, get3DFrom2D( ds, 3500 ) );

  faceIndex = 0;
  EXPECT_TRUE( MDAL::equals( -4.529386, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -5.705848, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -6.882310, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -8.058772, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 2.75812839e-09, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 2.75812817e-09, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 2.75812817e-09, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 2.94324898e-09, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 2.94324898e-09, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 2.94324898e-09, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );

  faceIndex = 1500;
  EXPECT_TRUE( MDAL::equals( -1.112822, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.150430, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.188038, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.225645, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 1.60614605e-10, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 1.61907321e-10, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 1.61939212e-10, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 1.39322054e-10, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 1.41126291e-10, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 1e-13 ) );
  EXPECT_TRUE( MDAL::equals( 1.41167314e-10, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 1e-13 ) );

  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );
  ASSERT_TRUE( MDAL_G_hasScalarData( g ) );

  groupName = MDAL_G_name( g );
  EXPECT_TRUE( groupName == std::string( "Vertical velocity" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshDhiDriverTest, loadMesh3DStackedSigmaZ )
{
  std::string path = test_file( "/dhi/Oresund3DSigmaZ.dfsu" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 2090 );

  double x = getVertexXCoordinatesAt( m, 10 );
  double y = getVertexYCoordinatesAt( m, 10 );
  double z = getVertexZCoordinatesAt( m, 10 );

  EXPECT_TRUE( MDAL::equals( x, 350695.21875, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( y, 6172595.0000, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( z, -1.6927, 0.001 ) );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 3700 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 10 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 0 ), 16 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 1 ), 6 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 10, 2 ), 9 );

  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 1 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  EXPECT_EQ( 34, MDAL_G_maximumVerticalLevelCount( g ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_EQ( 9, MDAL_G_datasetCount( g ) );
  double min, max;
  MDAL_G_minimumMaximum( g, &min, &max );

  EXPECT_TRUE( MDAL::equals( max, 34.956432, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( min, 7.7913579, 0.00001 ) );

  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 34, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 38227, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 20, getLevelsCount3D( ds, 1500 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 3, get3DFrom2D( ds, 1 ) );
  EXPECT_EQ( 35743, get3DFrom2D( ds, 3500 ) );

  int faceIndex = 0;
  EXPECT_TRUE( MDAL::equals( 0.00000, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.413406, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.826812, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.240218, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 22.57276, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 22.57276, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 22.57276, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 0.00001 ) );

  faceIndex = 1500;
  EXPECT_TRUE( MDAL::equals( -0.0000, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -2.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -3.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -4.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 4 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -5.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 5 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -6.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 6 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -7.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 7 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -8.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 8 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -9.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 9 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -10.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 10 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -11.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 11 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -12.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 12 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -13.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 13 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -14.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 14 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -15.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 15 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -16.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 16 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -17.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 17 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -18.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 18 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -19.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 19 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -20.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 20 ), 0.00001 ) );

  double val = getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 3 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 4 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 5 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 6 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 7 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 8 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 9 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 10 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 11 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 12 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 13 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 14 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.895956, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 15 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.9027424, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 16 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.9100285, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 17 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 26.9413280, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 18 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 27.0064373, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 19 ), 0.00001 ) );


  ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 34, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 38227, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 20, getLevelsCount3D( ds, 1500 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 3, get3DFrom2D( ds, 1 ) );
  EXPECT_EQ( 35743, get3DFrom2D( ds, 3500 ) );


  faceIndex = 0;
  double v = getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 );
  EXPECT_TRUE( MDAL::equals( 0.0498526, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.3801708, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.8101944, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.2402180, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );

  EXPECT_TRUE( MDAL::equals( 23.3483334, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 23.3587456, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 23.3790226, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 0.00001 ) );

  faceIndex = 1500;
  EXPECT_TRUE( MDAL::equals( 0.0708504, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.9527663, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -1.9763831, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -3.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -4.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 4 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -5.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 5 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -6.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 6 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -7.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 7 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -8.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 8 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -9.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 9 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -10.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 10 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -11.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 11 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -12.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 12 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -13.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 13 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -14.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 14 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -15.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 15 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -16.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 16 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -17.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 17 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -18.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 18 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -19.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 19 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -20.0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 20 ), 0.00001 ) );
  val = getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 );
  EXPECT_TRUE( MDAL::equals( 28.0670509, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 0 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1145458, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1560669, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 2 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1680126, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 3 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1663208, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 4 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1611271, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 5 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1555557, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 6 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1504440, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 7 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1459408, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 8 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1418915, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 9 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1381931, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 10 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1347809, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 11 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1316166, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 12 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1286697, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 13 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1259251, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 14 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1233749, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 15 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1210232, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 16 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1188717, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 17 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1169205, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 18 ), 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( 28.1144199, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 19 ), 0.00001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshDhiDriverTest, loadMeshDfs2 )
{
  std::string path = test_file( "/dhi/OresundHD.dfs2" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  int verticesCount = MDAL_M_vertexCount( m );
  ASSERT_EQ( verticesCount, 6624 );

  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_EQ( x, 336944.91817007872 );
  EXPECT_EQ( y, 6122282.7906238409 );
  EXPECT_EQ( z, 0.0 );

  x = getVertexXCoordinatesAt( m, 6000 );
  y = getVertexYCoordinatesAt( m, 6000 );
  z = getVertexZCoordinatesAt( m, 6000 );
  EXPECT_EQ( x, 317123.21823685983 );
  EXPECT_EQ( y, 6197474.2144151041 );
  EXPECT_EQ( z, 0.0 );

  int facesCount = MDAL_M_faceCount( m );
  ASSERT_EQ( facesCount, 6461 );
  int vertCount = getFaceVerticesCountAt( m, 0 );
  int vertInd = getFaceVerticesIndexAt( m, 0, 1 );
  EXPECT_EQ( vertCount, 4 );
  EXPECT_EQ( vertInd, 1 );
  vertCount = getFaceVerticesCountAt( m, 5000 );
  vertInd = getFaceVerticesIndexAt( m, 5000, 2 );
  EXPECT_EQ( vertCount, 4 );
  EXPECT_EQ( vertInd, 5143 );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH dsg = MDAL_M_datasetGroup( m, 0 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  EXPECT_TRUE( MDAL_G_hasScalarData( dsg ) );
  std::string groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "H Water Depth m" ) );
  compareReferenceTime( dsg, "1993-12-02T00:00:00" );
  ASSERT_EQ( 13, MDAL_G_datasetCount( dsg ) );

  MDAL_DatasetH ds = MDAL_G_dataset( dsg, 0 );
  ASSERT_TRUE( ds );
  bool valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 6461, count );
  double time = MDAL_D_time( ds );
  EXPECT_EQ( 0, time );
  EXPECT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );
  EXPECT_FALSE( getActive( ds, 1000 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  double value = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 5.699370, value, 0.00001 ) );

  ds = MDAL_G_dataset( dsg, 1 );
  ASSERT_TRUE( ds );
  valid = MDAL_D_isValid( ds );
  ASSERT_TRUE( valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 24.0, time, 0.00001 ) );
  EXPECT_FALSE( getActive( ds, 1000 ) );
  EXPECT_TRUE( getActive( ds, 100 ) );
  value = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 5.341166, value, 0.00001 ) );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
  dsg = MDAL_M_datasetGroup( m, 1 );
  ASSERT_TRUE( dsg );
  EXPECT_TRUE( MDAL_G_dataLocation( dsg ) == MDAL_DataLocation::DataOnFaces );
  EXPECT_FALSE( MDAL_G_hasScalarData( dsg ) );
  groupName = MDAL_G_name( dsg );
  EXPECT_TRUE( groupName == std::string( "Flux m^3/s/m" ) );
  compareReferenceTime( dsg, "1993-12-02T00:00:00" );
  ASSERT_EQ( 13, MDAL_G_datasetCount( dsg ) );

  ds = MDAL_G_dataset( dsg, 15 );
  EXPECT_FALSE( ds );

  ds = MDAL_G_dataset( dsg, 4 );
  ASSERT_TRUE( ds );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  time = MDAL_D_time( ds );
  EXPECT_TRUE( MDAL::equals( 96.0, time, 0.00001 ) );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( facesCount, count );
  EXPECT_TRUE( getActive( ds, 100 ) );
  double valueX = getValueX( ds, 100 );
  double valueY = getValueY( ds, 100 );
  EXPECT_TRUE( MDAL::equals( 0.30736675, valueX, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( -0.50450772, valueY, 0.00001 ) );

  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  set_mdal_driver_path( "dhi_dfsu" );
  int ret = RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
