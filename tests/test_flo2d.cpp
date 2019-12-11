/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <cmath>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( MeshFlo2dTest, WriteBarnHDF5_New )
{
  std::string path = test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" );
  std::string newFile = tmp_file( "/flow2d_BarnHDF5_New.hdf5" );

  // cleanup from previous run
  deleteFile( newFile );

  // FLO-2D is only on face datasets
  size_t f_count = 521;
  std::vector<double> valsScalar( f_count, 1.1 );
  std::vector<double> valsVector( 2 * f_count, 2.2 );

  // Create a new dat file
  {
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    DriverH driver = MDAL_driverFromName( "FLO2D" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnFaces2D ) );
    ASSERT_FALSE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices2D ) );
    ASSERT_FALSE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVolumes3D ) );

    ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

    // add scalar group
    DatasetGroupH g = MDAL_M_addDatasetGroup(
                        m,
                        "scalarGrp",
                        MDAL_DataLocation::DataOnFaces2D,
                        true,
                        driver,
                        newFile.c_str()
                      );
    ASSERT_NE( g, nullptr );
    MDAL_G_addDataset( g,
                       0.0,
                       valsScalar.data(),
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       valsScalar.data(),
                       nullptr
                     );
    ASSERT_TRUE( MDAL_G_isInEditMode( g ) );
    MDAL_G_closeEditMode( g );
    ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( g ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );

    // add vector group
    DatasetGroupH gV = MDAL_M_addDatasetGroup(
                         m,
                         "vectorGrp",
                         MDAL_DataLocation::DataOnFaces2D,
                         false,
                         driver,
                         newFile.c_str()
                       );
    ASSERT_NE( gV, nullptr );
    MDAL_G_addDataset( gV,
                       0.0,
                       valsVector.data(),
                       nullptr
                     );
    MDAL_G_addDataset( gV,
                       1.0,
                       valsVector.data(),
                       nullptr
                     );
    ASSERT_TRUE( MDAL_G_isInEditMode( gV ) );
    MDAL_G_closeEditMode( gV );
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( gV ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( gV ) );

    MDAL_CloseMesh( m );
  }

  // Ok, now try to load it from the new
  // file and test the
  // values are there
  {
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );
    MDAL_M_LoadDatasets( m, newFile.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

    // scalar group
    {
      DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
      ASSERT_NE( g, nullptr );

      const char *name = MDAL_G_name( g );
      EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

      bool scalar = MDAL_G_hasScalarData( g );
      EXPECT_EQ( true, scalar );

      MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
      EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

      ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
      DatasetH ds = MDAL_G_dataset( g, 0 );
      ASSERT_NE( ds, nullptr );

      bool valid = MDAL_D_isValid( ds );
      EXPECT_EQ( true, valid );

      EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

      int count = MDAL_D_valueCount( ds );
      ASSERT_EQ( f_count, count );

      double value = getValue( ds, 2 );
      EXPECT_DOUBLE_EQ( 1.1000000238418579, value );
    }

    // vector group
    {
      DatasetGroupH g = MDAL_M_datasetGroup( m, 6 );
      ASSERT_NE( g, nullptr );

      const char *name = MDAL_G_name( g );
      EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

      bool scalar = MDAL_G_hasScalarData( g );
      EXPECT_EQ( false, scalar );

      MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
      EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

      ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
      DatasetH ds = MDAL_G_dataset( g, 0 );
      ASSERT_NE( ds, nullptr );

      bool valid = MDAL_D_isValid( ds );
      EXPECT_EQ( true, valid );

      EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

      int count = MDAL_D_valueCount( ds );
      ASSERT_EQ( f_count, count );

      double value = getValueX( ds, 5 );
      EXPECT_DOUBLE_EQ( 2.2000000476837158, value );
    }

    MDAL_CloseMesh( m );
  }
}

TEST( MeshFlo2dTest, WriteBarnHDF5_Append )
{
  std::string pathOrig = test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" );
  std::string cadtsFile = tmp_file( "/CADPTS.DAT" );
  std::string fplainFile = tmp_file( "/FPLAIN.DAT" );
  std::string appendedFile = tmp_file( "/TIMDEP.HDF5" );

  //prepare
  deleteFile( cadtsFile );
  deleteFile( fplainFile );
  deleteFile( appendedFile );

  copy( test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ), appendedFile );
  copy( test_file( "/flo2d/BarnHDF5/CADPTS.DAT" ), cadtsFile );
  copy( test_file( "/flo2d/BarnHDF5/FPLAIN.DAT" ), fplainFile );

  // FLO-2D is only on face datasets
  size_t f_count = 521;
  std::vector<double> valsScalar( f_count );
  std::vector<double> valsVector( 2 * f_count );

  for ( size_t i = 0; i < f_count; ++i )
  {
    double val = static_cast<double>( i );
    valsScalar[i] = val;
    valsVector[2 * i] = val;
    valsVector[2 * i + 1] = 2 * val;
  }

  // Create a new dat file
  {
    MeshH m = MDAL_LoadMesh( pathOrig.c_str() );
    ASSERT_NE( m, nullptr );
    DriverH driver = MDAL_driverFromName( "FLO2D" );
    ASSERT_NE( driver, nullptr );
    ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

    // add scalar group
    DatasetGroupH g = MDAL_M_addDatasetGroup(
                        m,
                        "scalarGrp",
                        MDAL_DataLocation::DataOnFaces2D,
                        true,
                        driver,
                        appendedFile.c_str()
                      );
    ASSERT_NE( g, nullptr );
    MDAL_G_addDataset( g,
                       0.0,
                       valsScalar.data(),
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       valsScalar.data(),
                       nullptr
                     );
    ASSERT_TRUE( MDAL_G_isInEditMode( g ) );
    MDAL_G_closeEditMode( g );
    ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( g ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );

    // add vector group
    DatasetGroupH gV = MDAL_M_addDatasetGroup(
                         m,
                         "vectorGrp",
                         MDAL_DataLocation::DataOnFaces2D,
                         false,
                         driver,
                         appendedFile.c_str()
                       );
    ASSERT_NE( gV, nullptr );
    MDAL_G_addDataset( gV,
                       0.0,
                       valsVector.data(),
                       nullptr
                     );
    MDAL_G_addDataset( gV,
                       1.0,
                       valsVector.data(),
                       nullptr
                     );
    ASSERT_TRUE( MDAL_G_isInEditMode( gV ) );
    MDAL_G_closeEditMode( gV );
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );
    ASSERT_FALSE( MDAL_G_isInEditMode( gV ) );
    ASSERT_EQ( 2, MDAL_G_datasetCount( gV ) );

    MDAL_CloseMesh( m );
  }

  // Ok, now try to load it from the new
  // file and test the
  // values are there
  {
    MeshH m = MDAL_LoadMesh( appendedFile.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

    // scalar group
    {
      DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
      ASSERT_NE( g, nullptr );

      const char *name = MDAL_G_name( g );
      EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

      bool scalar = MDAL_G_hasScalarData( g );
      EXPECT_EQ( true, scalar );

      MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
      EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

      ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
      DatasetH ds = MDAL_G_dataset( g, 0 );
      ASSERT_NE( ds, nullptr );

      bool valid = MDAL_D_isValid( ds );
      EXPECT_EQ( true, valid );

      EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

      int count = MDAL_D_valueCount( ds );
      ASSERT_EQ( f_count, count );

      double value = getValue( ds, 0 );
      EXPECT_TRUE( std::isnan( value ) );

      value = getValue( ds, 200 );
      EXPECT_DOUBLE_EQ( 200.0, value );

      value = getValue( ds, 500 );
      EXPECT_DOUBLE_EQ( 500.0, value );
    }

    // vector group
    {
      DatasetGroupH g = MDAL_M_datasetGroup( m, 6 );
      ASSERT_NE( g, nullptr );

      const char *name = MDAL_G_name( g );
      EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

      bool scalar = MDAL_G_hasScalarData( g );
      EXPECT_EQ( false, scalar );

      MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
      EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

      ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
      DatasetH ds = MDAL_G_dataset( g, 0 );
      ASSERT_NE( ds, nullptr );

      bool valid = MDAL_D_isValid( ds );
      EXPECT_EQ( true, valid );

      EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

      int count = MDAL_D_valueCount( ds );
      ASSERT_EQ( f_count, count );

      double value = getValueX( ds, 0 );
      EXPECT_TRUE( std::isnan( value ) );

      value = getValueX( ds, 200 );
      EXPECT_DOUBLE_EQ( 200.0, value );

      value = getValueX( ds, 500 );
      EXPECT_DOUBLE_EQ( 500.0, value );

      value = getValueY( ds, 0 );
      EXPECT_TRUE( std::isnan( value ) );

      value = getValueY( ds, 200 );
      EXPECT_DOUBLE_EQ( 2 * 200.0, value );

      value = getValueY( ds, 500 );
      EXPECT_DOUBLE_EQ( 2 * 500.0, value );

    }

    MDAL_CloseMesh( m );
  }

  //cleanup
  deleteFile( cadtsFile );
  deleteFile( fplainFile );
}

TEST( MeshFlo2dTest, BarnHDF5 )
{
  std::string path = test_file( "/flo2d/BarnHDF5/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 571 );

  std::vector<double> expectedCoords =
  {
    0.000,        0.000, 0.000,
    100.000,       0.000, 0.000,
    200.000,        0.000, 0.000,
    300.000,      0.000, 0.000
  };
  EXPECT_EQ( expectedCoords.size(), 4 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 4 );

  compareVectors( expectedCoords, coordinates );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 521, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 5, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 1, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 521, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 4261.2799999999997, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 4262.8800000000001, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 4262.8299999999999, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4262.7700000000004, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "FLOW DEPTH" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 20, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 0.10124753560882101, time ) );

  const char *referenceTime;
  referenceTime = MDAL_G_referenceTime( g );
  EXPECT_EQ( std::string( "none" ), std::string( referenceTime ) );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 521, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 4262.8798828125, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 4259.18017578125, min );
  EXPECT_DOUBLE_EQ( 4520, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 4259.18017578125, min );
  EXPECT_DOUBLE_EQ( 4520, max );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Velocity" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 20, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 17 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 521, count );

  value = getValueX( ds, 234 );
  EXPECT_DOUBLE_EQ( 0.27071857452392578, value );

  value = getValueY( ds, 234 );
  EXPECT_DOUBLE_EQ( -0.62059682607650757, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0.1241119660936652, min );
  EXPECT_DOUBLE_EQ( 2.847882132344469, max );

  MDAL_CloseMesh( m );
}

TEST( MeshFlo2dTest, basic )
{
  std::vector<std::string> files;
  files.push_back( "basic" );
  files.push_back( "basic_with_dos_eol" );
  for ( const std::string &file : files )
  {
    std::string path = test_file( "/flo2d/" + file + "/BASE.OUT" );
    MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );

    // ///////////
    // Vertices
    // ///////////
    int v_count = MDAL_M_vertexCount( m );
    EXPECT_EQ( v_count, 16 );

    std::vector<double> expectedCoords =
    {
      1.59, 3.00, 0.00,
      2.59,  3.00, 0.00,
      3.59,  3.00, 0.00,
      1.59,  2.00, 0.00,
      2.59,  2.00, 0.00,
      3.59,  2.00, 0.00,
      1.59, 1.00, 0.00,
      2.59,  1.00, 0.00,
      3.59, 1.00, 0.00
    };
    EXPECT_EQ( expectedCoords.size(), 9 * 3 );

    std::vector<double> coordinates = getCoordinates( m, 9 );

    compareVectors( expectedCoords, coordinates );

    // ///////////
    // Faces
    // ///////////
    int f_count = MDAL_M_faceCount( m );
    EXPECT_EQ( 9, f_count );

    // test face 1
    int f_v_count = getFaceVerticesCountAt( m, 1 );
    EXPECT_EQ( 4, f_v_count ); //quad
    int f_v = getFaceVerticesIndexAt( m, 1, 0 );
    EXPECT_EQ( 4, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 1 );
    EXPECT_EQ( 5, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 2 );
    EXPECT_EQ( 1, f_v );
    f_v = getFaceVerticesIndexAt( m, 1, 3 );
    EXPECT_EQ( 0, f_v );

    // ///////////
    // Bed elevation dataset
    // ///////////
    ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

    DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 9, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1.48, value );

    // ///////////
    // Scalar Dataset
    // ///////////
    g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

    scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

    ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
    ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 9, count );

    value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 1, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1, min );
    EXPECT_DOUBLE_EQ( 1, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1, min );
    EXPECT_DOUBLE_EQ( 3, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 0.5, time ) );

    const char *referenceTime;
    referenceTime = MDAL_G_referenceTime( g );
    EXPECT_EQ( std::string( "none" ), std::string( referenceTime ) );

    // ///////////
    // Max Depth
    // ///////////
    g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );

    meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "Depth/Maximums" ), std::string( name ) );

    scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

    ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
    ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 9, count );

    value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 2, value );

    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 2, min );
    EXPECT_DOUBLE_EQ( 4, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 2, min );
    EXPECT_DOUBLE_EQ( 4, max );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshFlo2dTest, basic_required_files_only )
{
  std::string path = test_file( "/flo2d/basic_required_files_only/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 16 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 9, f_count );

  // ///////////
  // Bed elevation dataset
  // ///////////
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
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 9, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.48, value );

  const char *referenceTime;
  referenceTime = MDAL_G_referenceTime( g );
  EXPECT_EQ( std::string( "none" ), std::string( referenceTime ) );

  MDAL_CloseMesh( m );
}

TEST( MeshFlo2dTest, pro_16_02_14 )
{
  std::string path = test_file( "/flo2d/pro_16_02_14/BASE.OUT" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5443 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 5101, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5101, count );

  double value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4904.2299999999996, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Depth" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces2D );

  ASSERT_EQ( 4, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 150.0, time ) );

  const char *referenceTime;
  referenceTime = MDAL_G_referenceTime( g );
  EXPECT_EQ( std::string( "none" ), std::string( referenceTime ) );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5101, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 0.098000000000000004, value );


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

