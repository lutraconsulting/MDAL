/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

TEST( MeshXmdfTest, MissingMesh )
{
  MDAL_MeshH m = nullptr;
  std::string path = test_file( "/xmdf/xmdf_format.xmdf" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
}

TEST( MeshXmdfTest, RegularGridScalarDataset )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
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
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
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
  MDAL_DatasetH ds = MDAL_G_dataset( g, 50 );
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
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/regular_grid.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

  /*
   *  VECTOR DATASET (MAXIMUMS)
   */
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
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
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
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
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/custom_groups.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
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
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
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

TEST( MeshXmdfTest, withReferenceTime )
{
  // XMDF created with various TUFLOW utilities
  std::string path = test_file( "/xmdf/withReferenceTime/hydraul_006.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/withReferenceTime/PTM_005_QGIS_Axis.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 12, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  EXPECT_TRUE( hasReferenceTime( g ) );
  EXPECT_TRUE( compareReferenceTime( g, "1990-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshXmdfTest, unlockWhenClose )
{
  std::string tmpXmdf = tmp_file( "/temp.xmdf" );
  copy( test_file( "/xmdf/withReferenceTime/PTM_005_QGIS_Axis.xmdf" ), tmpXmdf );

  ASSERT_TRUE( fileExists( tmpXmdf ) );

  std::string path = test_file( "/xmdf/withReferenceTime/hydraul_006.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_M_LoadDatasets( m, tmpXmdf.c_str() );
  ASSERT_EQ( 12, MDAL_M_datasetGroupCount( m ) );

  MDAL_CloseMesh( m );

  deleteFile( tmpXmdf );

  ASSERT_FALSE( fileExists( tmpXmdf ) );
}

TEST( MeshXmdfTest, HydroAs2D )
{
  std::string path = test_file( "/xmdf/hydro-as-2d/hydro_as-2d.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );

  ASSERT_EQ( MDAL_M_vertexCount( m ), 300 );
  ASSERT_EQ( MDAL_M_faceCount( m ), 245 );

  path = test_file( "/xmdf/hydro-as-2d/veloc.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );

  EXPECT_EQ( MDAL_M_datasetGroupCount( m ), 3 );

  path = test_file( "/xmdf/hydro-as-2d/results.h5" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 9 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "EH" ), std::string( name ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  double val = getValue( ds, 100 );
  EXPECT_TRUE( MDAL::equals( val, 0.0773996, 0.00001 ) );


  g = MDAL_M_datasetGroup( m, 4 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "EH_abs" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 150 );
  EXPECT_TRUE( MDAL::equals( val, 1.09848, 0.0001 ) );


  g = MDAL_M_datasetGroup( m, 5 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "FT" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 150 );
  EXPECT_TRUE( MDAL::equals( val, 0.0695, 0.00001 ) );

  g = MDAL_M_datasetGroup( m, 6 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Froude" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 150 );
  EXPECT_TRUE( MDAL::equals( val, 0.587074, 0.00001 ) );

  g = MDAL_M_datasetGroup( m, 7 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "INT" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 150 );
  EXPECT_TRUE( MDAL::equals( val, 0.0695, 0.00001 ) );

  g = MDAL_M_datasetGroup( m, 8 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "q_spez" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 150 );
  EXPECT_TRUE( MDAL::equals( val, 0.0336903, 0.00001 ) );

  path = test_file( "/xmdf/hydro-as-2d/veloc.h5" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 11 );
  g = MDAL_M_datasetGroup( m, 9 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "veloc_HYDRO_AS-2D" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_FALSE( MDAL_G_hasScalarData( g ) );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  double valX = getValueX( ds, 200 );
  double valY = getValueY( ds, 200 );
  EXPECT_TRUE( MDAL::equals( valX, 0.253106, 0.00001 ) );
  EXPECT_TRUE( MDAL::equals( valY, 0.347786, 0.00001 ) );

  g = MDAL_M_datasetGroup( m, 10 );
  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "veloc_magnitudeHYDRO_AS-2D" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_dataLocation( g ), DataOnVertices );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 4 );
  EXPECT_FALSE( hasReferenceTime( g ) );
  ds = MDAL_G_dataset( g, 2 );
  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 300, count );
  val = getValue( ds, 180 );
  EXPECT_TRUE( MDAL::equals( val, 0.474482, 0.00001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshXmdfTest, withFinalgroup )
{
  std::string path = test_file( "/xmdf/withFinal/final_mindt_example.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/xmdf/withFinal/final_mindt_example.xmdf" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  EXPECT_EQ( MDAL_M_datasetGroupCount( m ), 3 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Minimum dt/Final" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 1 );

  g = MDAL_M_datasetGroup( m, 2 );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Minimum dt" ), std::string( name ) );
  EXPECT_EQ( MDAL_G_datasetCount( g ), 3 );

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

