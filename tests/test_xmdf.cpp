/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshXmdfTest, MissingMesh )
{
  MeshH m = nullptr;
  std::string path = test_file( "/xmdf/xmdf_format.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
}

TEST( MeshXmdfTest, RegularGridScalarDataset )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/regular_grid.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  double minX, maxX, minY, maxY;
  MDAL_M_extent( m, &minX, &maxX, &minY, &maxY );
  EXPECT_DOUBLE_EQ( 381449.78499999997, minX );
  EXPECT_DOUBLE_EQ( 381599.78499999997, maxX );
  EXPECT_DOUBLE_EQ( 168700.98499999999, minY );
  EXPECT_DOUBLE_EQ( 168750.98499999999, maxY );

  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

  /*
   *  STATIC DATASET (Depth)
   */
  DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( MDAL_G_mesh( g ), m );

  std::string driverName = MDAL_G_driverName( g );
  EXPECT_EQ( driverName, "XMDF" );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 61, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 50 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );


  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1976, count );

  {
    std::vector<int> active( 5 );
    std::vector<int> expectedActive = {0, 0, 0, 0, 0};
    int nValuesRead = MDAL_D_data( ds, 0, 5, MDAL_DataType::ACTIVE_INTEGER, active.data() );
    ASSERT_EQ( 5,  nValuesRead );
    EXPECT_EQ( active, expectedActive );
  }

  {
    std::vector<int> active( 5 );
    std::vector<int> expectedActive = {1, 1, 1, 1, 1};
    int nValuesRead = MDAL_D_data( ds, 60, 5, MDAL_DataType::ACTIVE_INTEGER, active.data() );
    ASSERT_EQ( 5,  nValuesRead );
    EXPECT_EQ( active, expectedActive );

    std::vector<double> values( 5 );
    std::vector<double> expectedValue = { 0.173723, 0.572754, 0.285215, 0.661351, 0.369279 };
    nValuesRead = MDAL_D_data( ds, 60, 5, MDAL_DataType::SCALAR_DOUBLE, values.data() );
    ASSERT_EQ( 5,  nValuesRead );
    if ( !compareVectors( values, expectedValue ) )
    {
      EXPECT_EQ( values, expectedValue );
    }

    double value = getValue( ds, 60 );
    EXPECT_DOUBLE_EQ( 0.17372334003448486, value );
  }

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0.90217632055282593, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 1.0765361785888672, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 4.166666666666, time ) );

  EXPECT_FALSE( hasReferenceTime( g ) );

  MDAL_CloseMesh( m );
}

TEST( MeshXmdfTest, RegularGridVectorMaxDataset )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/regular_grid.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

  /*
   *  VECTOR DATASET (MAXIMUMS)
   */
  DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Vector Velocity/Maximums" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1976, count );

  {
    std::vector<int> active( 3 );
    std::vector<int> expectedActive = {1, 1, 1};
    int nValuesRead = MDAL_D_data( ds, 66, 3, MDAL_DataType::ACTIVE_INTEGER, active.data() );
    ASSERT_EQ( 3,  nValuesRead );
    EXPECT_EQ( active, expectedActive );

    std::vector<double> values( 3 * 2 ); // it is x1, y1, x2, y2, ...
    std::vector<double> expectedValue = { -0.00459212, 0.000718806,
                                          -0.0700743, 0,
                                          -0.00374553, -0.00105742
                                        };

    nValuesRead = MDAL_D_data( ds, 66, 3, MDAL_DataType::VECTOR_2D_DOUBLE, values.data() );
    ASSERT_EQ( 3,  nValuesRead );
    if ( !compareVectors( values, expectedValue ) )
    {
      EXPECT_EQ( values, expectedValue );
    }

    double value = getValueX( ds, 66 );
    EXPECT_DOUBLE_EQ( -0.0045921225100755692, value );

    value = getValueY( ds, 66 );
    EXPECT_DOUBLE_EQ( 0.00071880628820508718, value );
  }

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0.38855308294296265, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0.38855308294296265, max );

  EXPECT_FALSE( hasReferenceTime( g ) );

  MDAL_CloseMesh( m );
}

TEST( MeshXmdfTest, CustomGroupsDataset )
{
  // XMDF created with various TUFLOW utilities
  // where we have missing the standard groups like Temporal
  std::string path = test_file( "/2dm/M01_5m_002.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/custom_groups.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Vector Velocity" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 37, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 20893, count );

  {
    std::vector<int> active( 3 );
    std::vector<int> expectedActive = {0, 0, 0};
    int nValuesRead = MDAL_D_data( ds, 66, 3, MDAL_DataType::ACTIVE_INTEGER, active.data() );
    ASSERT_EQ( 3,  nValuesRead );
    EXPECT_EQ( active, expectedActive );

    std::vector<double> values( 3 );
    std::vector<double> expectedValue = {180, 180, 180};

    nValuesRead = MDAL_D_data( ds, 66, 3, MDAL_DataType::SCALAR_DOUBLE, values.data() );
    ASSERT_EQ( 3,  nValuesRead );
    if ( !compareVectors( values, expectedValue ) )
    {
      EXPECT_EQ( values, expectedValue );
    }
  }

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 180, max );
  EXPECT_DOUBLE_EQ( 180, min );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 180, max );
  EXPECT_DOUBLE_EQ( -179.99665832519531, min );

  ds = MDAL_G_dataset( g, 1 );
  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 0.083333333333, time ) );

  EXPECT_FALSE( hasReferenceTime( g ) );

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

