/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <cmath>
#include <limits>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

TEST( MeshHec2dTest, simpleArea )
{
  std::string path = test_file( "/hec2d/1area/test.p01.hdf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "HEC2D:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 773 );

  std::vector<double> expectedCoords =
  {
    -5.5731e-06, 90.0, 0.00,
      9.999988, 90.00000297, 0.00,
      9.999988, 99.962230818, 0.00,
      19.999988, 90.00000297, 0.00
    };
  EXPECT_EQ( expectedCoords.size(), 4 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 4 );

  EXPECT_TRUE( compareVectors( expectedCoords, coordinates ) );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 851, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 3, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 1, f_v );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 8, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );



  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 851, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 9.65625, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water Surface" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 41, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 851, count );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 9.699999809265137, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 10.390625, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 10.390625, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0.0, time );

  EXPECT_TRUE( compareDurationInHours( 0, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "1899-12-30T00:00:00" ) );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Velocity" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 41, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 35 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 851, count );

  value = getValueX( ds, 15 );
  EXPECT_TRUE( MDAL::equals( -0.044146, value, 0.000001 ) );
  value = getValueY( ds, 15 );
  EXPECT_TRUE( MDAL::equals( 0.0001003014, value, 0.00000001 ) );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 7.3807e-5, min, 0.000001 ) );
  EXPECT_TRUE( MDAL::equals( 0.0452122, max, 0.000001 ) );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_TRUE( MDAL::equals( 0.3837422465, max, 0.000001 ) );

  MDAL_CloseMesh( m );
}

TEST( MeshHec2dTest, MultiAreas )
{
  std::string path = test_file( "/hec2d/2areas/baldeagle_multi2d.hdf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "HEC2D:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "PROJCS[\"NAD_1983_StatePlane_Pennsylvania_North_FIPS_3701_Feet\",GEOGCS[\"GCS_North_American_1983\",DATUM[\"D_North_American_1983\",SPHEROID[\"GRS_1980\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Lambert_Conformal_Conic\"],PARAMETER[\"False_Easting\",1968500],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",-77.75],PARAMETER[\"Standard_Parallel_1\",40.88333333333333],PARAMETER[\"Standard_Parallel_2\",41.95],PARAMETER[\"Latitude_Of_Origin\",40.16666666666666],UNIT[\"Foot_US\",0.30480060960121924]]" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 783 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 725, f_count );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 8, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 725, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 635.30230712890625, value );

  value = getValue( ds, 700 );
  EXPECT_TRUE( std::isnan( value ) );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water Surface" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 7, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 725, count );

  value = getValue( ds, 100 );
  EXPECT_DOUBLE_EQ( 606.6416015625, value );

  value = getValue( ds, 700 );
  EXPECT_DOUBLE_EQ( 655.0142211914062, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 605.7783203125, min );
  EXPECT_DOUBLE_EQ( 706.2740478515625, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 576.4375, min );
  EXPECT_DOUBLE_EQ( 706.2740478515625, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 60, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "1999-01-01T12:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshHec2dTest, model_505 )
{
  std::string path = test_file( "/hec2d/2dmodel_5.0.5/temp.p01.hdf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "HEC2D:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( projection ), std::string( "PROJCS[\"PNG94_PNGMG94_zone_55\",GEOGCS[\"GCS_PNG94\",DATUM[\"D_Papua_New_Guinea_Geodetic_Datum_1994\",SPHEROID[\"GRS_1980\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"Meter\",1]]" ) );
  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 69 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 73, f_count );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 8, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 73, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 40.490013122558594, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water Surface" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 61, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 73, count );

  value = getValue( ds, 10 );
  EXPECT_TRUE( std::isnan( value ) ); //dry face

  value = getValue( ds, 50 );
  EXPECT_TRUE( MDAL::equals( 34.785, value, 0.001 ) );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 32, min );
  EXPECT_DOUBLE_EQ( 43.28509521484375, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 32, min );
  EXPECT_DOUBLE_EQ( 43.28509521484375, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 0.083333335816860199, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "2018-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshHec2dTest, hec_6_0 )
{
  std::string path = test_file( "/hec2d/2d_6.0/Prueba.p01.hdf" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "HEC2D:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( projection ), std::string( "PROJCS[\"ETRS89 / UTM zone 30N\",GEOGCS[\"ETRS89\",DATUM[\"D_ETRS_1989\",SPHEROID[\"GRS_1980\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",-3],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0],UNIT[\"Meter\",1]]" ) );
  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 4287 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4403, f_count );

  // ///////////
  // Dataset groups
  // ///////////

  ASSERT_EQ( 5, MDAL_M_datasetGroupCount( m ) );

  // Bed Elevation
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4403, count );

  double value = getValue( ds, 0 );
  EXPECT_TRUE( MDAL::equals( 812.59869, value, 0.001 ) );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 802.0603, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 907.402, max, 0.001 ) );

  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water Surface" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 361, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 200 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4403, count );

  value = getValue( ds, 10 );
  EXPECT_TRUE( MDAL::equals( 807.006, value, 0.001 ) );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 806.957, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 909.976, max, 0.001 ) );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 0.5555555555555556, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "2020-12-23T00:00:00" ) );

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
