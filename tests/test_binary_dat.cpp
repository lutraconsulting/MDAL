/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshBinaryDatTest, MissingMesh )
{
  MeshH m = nullptr;
  std::string path = test_file( "binary_dat/quad_and_triangle_binary.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
  MDAL_CloseMesh( m );
}

TEST( MeshBinaryDatTest, QuadAndTriangleFile )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/binary_dat/quad_and_triangle_binary.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water Depth (m)" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int active = getActive( ds, 0 );
  EXPECT_EQ( 1, active );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0, time );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}

TEST( MeshBinaryDatTest, RegularGridVectorFile )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/binary_dat/regular_grid_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 3, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Vel  dat_format" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 61, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 50 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int active = getActive( ds, 600 );
  EXPECT_EQ( 0, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1976, count );

  double value = getValueX( ds, 1000 );
  EXPECT_DOUBLE_EQ( 0, value );

  EXPECT_FALSE( hasReferenceTime( g ) );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 4.1666666666 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshBinaryDatTest, RegularGridScalarFile )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  path = test_file( "/binary_dat/regular_grid_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 3, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Dep  dat_format" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 61, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int active = getActive( ds, 0 );
  EXPECT_EQ( 0, active );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1976, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  MDAL_CloseMesh( m );
}

TEST( MeshBinaryDatTest, WriteScalarTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string scalarPath = tmp_file( "/2dm_WriteScalarTest.dat" );
  std::vector<double> vals = {1, 2, 3, 4, 5};
  std::vector<int> active = {1, 1};

  // Create a new dat file
  {

    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    DriverH driver = MDAL_driverFromName( "BINARY_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices ) );
    ASSERT_FALSE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnFaces ) );
    ASSERT_FALSE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVolumes ) );
    ASSERT_FALSE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataInvalidLocation ) );

    DatasetGroupH g = MDAL_M_addDatasetGroup(
                        m,
                        "scalarGrp",
                        MDAL_DataLocation::DataOnVertices,
                        true,
                        driver,
                        scalarPath.c_str()
                      );
    ASSERT_NE( g, nullptr );

    ASSERT_EQ( MDAL_G_metadataCount( g ), 1 );
    MDAL_G_setMetadata( g, "testkey", "testvalue" );
    ASSERT_EQ( MDAL_G_metadataCount( g ), 2 );

    MDAL_G_addDataset( g,
                       0.0,
                       vals.data(),
                       active.data()
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       active.data()
                     );

    ASSERT_TRUE( MDAL_G_isInEditMode( g ) );
    MDAL_G_closeEditMode( g );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( g ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    ASSERT_EQ( g, MDAL_D_group( MDAL_G_dataset( g, 0 ) ) );

    MDAL_CloseMesh( m );
  }

  // Ok, now try to load it from the new
  // file and test the
  // values are there
  {
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, scalarPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    int active = getActive( ds, 0 );
    EXPECT_EQ( 1, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshBinaryDatTest, WriteVectorTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string vectorPath = tmp_file( "/2dm_WriteVectorTest.dat" );
  std::vector<double> vals = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
  std::vector<int> active = {1, 1};

  // Create a new dat file
  {

    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    DriverH driver = MDAL_driverFromName( "BINARY_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices ) );

    DatasetGroupH g = MDAL_M_addDatasetGroup(
                        m,
                        "vectorGrp",
                        MDAL_DataLocation::DataOnVertices,
                        false,
                        driver,
                        vectorPath.c_str()
                      );
    ASSERT_NE( g, nullptr );

    MDAL_G_addDataset( g,
                       0.0,
                       vals.data(),
                       active.data()
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       active.data()
                     );

    ASSERT_TRUE( MDAL_G_isInEditMode( g ) );
    MDAL_G_closeEditMode( g );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( g ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    ASSERT_EQ( g, MDAL_D_group( MDAL_G_dataset( g, 0 ) ) );

    MDAL_CloseMesh( m );
  }

  // Ok, now try to load it from the new
  // file and test the
  // values are there
  {
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, vectorPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    int active = getActive( ds, 0 );
    EXPECT_EQ( 1, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5, count );

    double value = getValueX( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    value = getValueY( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    MDAL_CloseMesh( m );
  }
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
