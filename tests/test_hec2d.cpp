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
#include "mdal_testutils.hpp"

TEST( MeshHec2dTest, simpleArea )
{
  std::string path = test_file( "/hec2d/1area/test.p01.hdf" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
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

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

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

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 41, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

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

  MDAL_CloseMesh( m );
}

TEST( MeshHec2dTest, MultiAreas )
{
  std::string path = test_file( "/hec2d/2areas/baldeagle_multi2d.hdf" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
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

  DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Bed Elevation" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  bool onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  bool active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

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

  onVertices = MDAL_G_isOnVertices( g );
  EXPECT_EQ( false, onVertices );

  ASSERT_EQ( 7, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 5 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  active = getActive( ds, 0 );
  EXPECT_EQ( true, active );

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
  EXPECT_DOUBLE_EQ( 2.5, time );

  MDAL_CloseMesh( m );
}

TEST( MeshHec2dTest, model_505 )
{
  std::string path = test_file( "/hec2d/2dmodel_5.0.5/temp.p01.hdf" );
  MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_EQ( m, nullptr ); // NOT IMPLEMENTED
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
