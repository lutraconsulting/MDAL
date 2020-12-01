/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( XdmfTest, Basement3HumpsTest )
{
  std::string path = test_file( "/xdmf/basement3/3HumpsTest/3_humps_mesh.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/basement3/3HumpsTest/three_humps.xdmf" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path2.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  // normal scalar dataset
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "water_surface" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 11, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 18497, count );

    double value = getValue( ds, 145 );
    EXPECT_DOUBLE_EQ( 0.73621612176773543, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 2.9100000000000001, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 2.9100000000000001, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 20, time ) );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

  // FUNCTION: JOIN($0, $1, 0*$1) dataset
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "spec_discharge" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 11, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 18497, count );

    double valueX = getValueX( ds, 145 );
    EXPECT_DOUBLE_EQ( -0.01422131435481137, valueX );

    double valueY = getValueY( ds, 145 );
    EXPECT_DOUBLE_EQ( -0.30278839626026738, valueY );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 2.0280154310589538, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 4.9994493491047303, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 20, time ) );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

  MDAL_CloseMesh( m );
}


TEST( XdmfTest, Basement3Slopes )
{
  std::string path = test_file( "/xdmf/basement3/3Slopes/3Slopes_Counter.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/basement3/3Slopes/7_J_run.XMDF" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path2.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

  // normal dataset
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "friction_chezy" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 21030, count );

    double value = getValue( ds, 145 );
    EXPECT_DOUBLE_EQ( 383.91154621338592, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 378.67290092503674, min );
    EXPECT_DOUBLE_EQ( 494.81692722296532, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 494.816927222965329, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 1, time ) );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

  // FUNCTION: $1 - $0
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "delta_z" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 21030, count );

    double value = getValue( ds, 7493 );
    EXPECT_DOUBLE_EQ( 0.0030000000000001137, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( -8.3107706316809526e-05, min );
    EXPECT_DOUBLE_EQ( 0.0030000000000001137, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( -8.3107706316809526e-05, min );
    EXPECT_DOUBLE_EQ( 0.0030000000000001137, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 1, time ) );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

  // FUNCTION: $0 - $1
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 6 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "water_depth" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 21030, count );

    double value = getValue( ds, 7493 );
    EXPECT_DOUBLE_EQ( 3.0361991037085616, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1.0093761225415925, min );
    EXPECT_DOUBLE_EQ( 3.0788896191491268, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1.0093761225415925, min );
    EXPECT_DOUBLE_EQ( 3.0788896191491268, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 1, time );
  }

  // FUNCTION: sqrt($0/($2-$3)*$0/($2-$3) + $1/($2-$3)*$1/($2-$3))
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "flow_velocity_abs" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 21030, count );

    double value = getValue( ds, 7493 );
    EXPECT_DOUBLE_EQ( 7.0036239300095486, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
    EXPECT_DOUBLE_EQ( 18.579443815111134, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
    EXPECT_DOUBLE_EQ( 18.579443815111134, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 1, time );
  }

  MDAL_CloseMesh( m );
}


TEST( XdmfTest, Basement3SimpleChannel )
{
  std::string path = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/basement3/SimpleChannel/SimpleChannel.xdmf" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path2.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 4 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "water_surface" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 11, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 3 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 77, count );

    double value = getValue( ds, 30 );
    EXPECT_DOUBLE_EQ( 0.41129252269409461, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0.29132377244494195, min );
    EXPECT_DOUBLE_EQ( 0.48486230912036832, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0040000000000000001, min );
    EXPECT_DOUBLE_EQ( 0.48486232713374577, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 30, time ) );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

  MDAL_CloseMesh( m );
}


TEST( XdmfTest, Basement3SimpleGeometry )
{
  std::string path = test_file( "/xdmf/basement3/SimpleGeometry/test.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/basement3/SimpleGeometry/test.xmf" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path2.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 4, MDAL_M_datasetGroupCount( m ) );

  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "water_surface" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 3 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 9, count );

    double value = getValue( ds, 30 );
    EXPECT_DOUBLE_EQ( 0.0, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 1.9182098447965066, min );
    EXPECT_DOUBLE_EQ( 2.0899662203297678, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 1.8713530882459224, min );
    EXPECT_DOUBLE_EQ( 2.1451217674360481, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 6, time );
  }

  MDAL_CloseMesh( m );
}

TEST( XdmfTest, Simple )
{
  std::string path = test_file( "/xdmf/simple/simpleXFMD.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  std::string path2 = test_file( "/xdmf/simple/simpleXFMD.xmf" );
  EXPECT_TRUE( std::string( MDAL_MeshNames( path2.c_str() ) ).empty() );
  MDAL_M_LoadDatasets( m, path2.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  EXPECT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

  // ///////////
  // Scalar Dataset
  // ///////////
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "h" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 21, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5790, count );

    double value = getValue( ds, 210 );
    EXPECT_DOUBLE_EQ( 0.0, value );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.97462530517332602, value );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0, min );
    EXPECT_DOUBLE_EQ( 1.0916943102403027, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( -0.1568600200000731, min );
    EXPECT_DOUBLE_EQ( 1.0916943102403027, max );

    double time = MDAL_D_time( ds );
    EXPECT_DOUBLE_EQ( 100.895, time );

    // lets try another timestep too
    ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    value = getValue( ds, 5000 );
    EXPECT_DOUBLE_EQ( 0.19425453944902599, value );
  }

  // //////////////
  // Vector Dataset
  // //////////////
  {
    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "v" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 21, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5790, count );

    double valueX = getValueX( ds, 145 );
    EXPECT_DOUBLE_EQ( 3.0896201122474545, valueX );

    double valueY = getValueY( ds, 145 );
    EXPECT_DOUBLE_EQ( -0.02535337911766803, valueY );

    valueX = getValueX( ds, 196 );
    EXPECT_DOUBLE_EQ( 2.3246286772752853, valueX );

    valueY = getValueY( ds, 196 );
    EXPECT_DOUBLE_EQ( -0.27034955553581924, valueY );

    double min, max;
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 3.5179872000733399, max );

    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min );
    EXPECT_DOUBLE_EQ( 3.5179872000733399, max );

    double time = MDAL_D_time( ds );
    EXPECT_TRUE( compareDurationInHours( 100.895, time ) );

    // lets try another timestep too
    ds = MDAL_G_dataset( g, 10 );
    ASSERT_NE( ds, nullptr );

    valueX = getValueX( ds, 196 );
    EXPECT_DOUBLE_EQ( 0, valueX );

    valueY = getValueY( ds, 196 );
    EXPECT_DOUBLE_EQ( 0, valueY );

    EXPECT_FALSE( hasReferenceTime( g ) );
  }

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
