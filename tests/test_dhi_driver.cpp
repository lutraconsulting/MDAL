/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

void initTest()
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
}

TEST( MeshDhiDriverTest, loadDriver )
{
  MDAL_DriverH driver = MDAL_driverFromName( "DHI" );
  ASSERT_TRUE( driver );

  ASSERT_TRUE( MDAL_DR_meshLoadCapability( driver ) );

  std::string name( MDAL_DR_name( driver ) );
  std::string longName( MDAL_DR_longName( driver ) );
  EXPECT_EQ( name, std::string( "DHI" ) );
  EXPECT_EQ( longName, std::string( "DHI dfsu" ) );
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
  EXPECT_TRUE( groupName == std::string( "Depth averaged velocity" ) );
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
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret = RUN_ALL_TESTS();
  return ret;
}
