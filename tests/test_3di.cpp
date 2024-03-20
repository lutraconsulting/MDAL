/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <math.h>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_config.hpp"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

TEST( Mesh3DiTest, Mesh2Dgroundwater )
{
  std::string path = test_file( "/3di/2d_groundwater/results_3di.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "3Di:\"" + path + "\":Mesh2D;;3Di:\"" + path + "\":Mesh2D_groundwater;;3Di:\"" + path + "\":Mesh2D_surface_water" );
  std::string uri = "3Di:\"" + path + "\":Mesh2D_groundwater";
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 6623 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 103160.5, x );
  EXPECT_DOUBLE_EQ( 517079.25, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  x = getVertexXCoordinatesAt( m, 6622 );
  y = getVertexYCoordinatesAt( m, 6622 );
  z = getVertexZCoordinatesAt( m, 6622 );
  EXPECT_DOUBLE_EQ( 109480.5, x );
  EXPECT_DOUBLE_EQ( 520199.25, y );
  EXPECT_DOUBLE_EQ( 0.0, z );


  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 5659, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 3, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 5, f_v );

  // test last face
  f_v_count = getFaceVerticesCountAt( m, 5658 );
  EXPECT_EQ( 4, f_v_count ); //quad
  f_v = getFaceVerticesIndexAt( m, 5658, 0 );
  EXPECT_EQ( 2548, f_v );
  f_v = getFaceVerticesIndexAt( m, 5658, 1 );
  EXPECT_EQ( 6621, f_v );
  f_v = getFaceVerticesIndexAt( m, 5658, 2 );
  EXPECT_EQ( 6622, f_v );
  f_v = getFaceVerticesIndexAt( m, 5658, 3 );
  EXPECT_EQ( 2549, f_v );


  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

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
  ASSERT_EQ( f_count, count );

  std::vector<double> elevations( f_count );
  int nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::SCALAR_DOUBLE, elevations.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_DOUBLE_EQ( elevations.at( 0 ), -1.0 );
  ASSERT_DOUBLE_EQ( elevations.at( 0 ), getValue( ds, 0 ) );
  ASSERT_DOUBLE_EQ( elevations.at( 1 ), -2.4745922559837705 );
  ASSERT_DOUBLE_EQ( elevations.at( 1 ), getValue( ds, 1 ) );
  ASSERT_DOUBLE_EQ( elevations.at( 5651 ), -2.577424985766411 );
  ASSERT_DOUBLE_EQ( elevations.at( 5651 ), getValue( ds, 5651 ) );
  ASSERT_DOUBLE_EQ( elevations.at( 5652 ), -2.6462171195369018 );
  ASSERT_DOUBLE_EQ( elevations.at( 5652 ), getValue( ds, 5652 ) );
  ASSERT_TRUE( std::isnan( elevations.at( 5658 ) ) );
  ASSERT_TRUE( std::isnan( getValue( ds, 5658 ) ) );


  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "water volume" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 5, MDAL_G_datasetCount( g ) );

  std::vector<double> values( f_count );

  ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::SCALAR_DOUBLE, values.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_DOUBLE_EQ( values.at( 0 ), 1601.4951171875 );
  ASSERT_DOUBLE_EQ( values.at( 0 ), getValue( ds, 0 ) );
  ASSERT_DOUBLE_EQ( values.at( 1 ), 2153.223876953125 );
  ASSERT_DOUBLE_EQ( values.at( 1 ), getValue( ds, 1 ) );
  ASSERT_DOUBLE_EQ( values.at( 5651 ), 9.358907699584961 );
  ASSERT_DOUBLE_EQ( values.at( 5651 ), getValue( ds, 5651 ) );
  ASSERT_DOUBLE_EQ( values.at( 5652 ), 11.230487823486328 );
  ASSERT_DOUBLE_EQ( values.at( 5652 ), getValue( ds, 5652 ) );
  ASSERT_DOUBLE_EQ( values.at( 5658 ), 0.0 );
  ASSERT_DOUBLE_EQ( values.at( 5658 ), getValue( ds, 5658 ) );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -10.534321784973145, min );
  EXPECT_DOUBLE_EQ( 3263.248291015625, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -10.542006492614746, min );
  EXPECT_DOUBLE_EQ( 3324.185302734375, max );


  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "flow velocity in cell centre" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 5, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( f_count, count );

  std::vector<double> vectorData( 2 * f_count );
  nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::VECTOR_2D_DOUBLE, vectorData.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_DOUBLE_EQ( vectorData.at( 0 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 0 ), getValueX( ds, 0 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 1 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 1 ), getValueY( ds, 0 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11316 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 11316 ), getValueX( ds, 5658 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11317 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 11317 ), getValueY( ds, 5658 ) );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 14140.721410168577, min );
  EXPECT_DOUBLE_EQ( 14140.721410168577, max );

  EXPECT_TRUE( compareReferenceTime( g, "2000-01-01T12:00:00" ) );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.7500505555555556 ) );

  MDAL_CloseMesh( m );
}

TEST( Mesh3DiTest, Mesh2Dsurface_water )
{
  std::string path = test_file( "/3di/2d_groundwater/results_3di.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "3Di:\"" + path + "\":Mesh2D;;3Di:\"" + path + "\":Mesh2D_groundwater;;3Di:\"" + path + "\":Mesh2D_surface_water" );
  std::string uri = "3Di:\"" + path + "\":Mesh2D_surface_water";
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 6661 );
  double x = getVertexXCoordinatesAt( m, 0 );
  double y = getVertexYCoordinatesAt( m, 0 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 103160.5, x );
  EXPECT_DOUBLE_EQ( 517079.25, y );
  EXPECT_DOUBLE_EQ( 0.0, z );

  x = getVertexXCoordinatesAt( m, 6660 );
  y = getVertexYCoordinatesAt( m, 6660 );
  z = getVertexZCoordinatesAt( m, 6660 );
  EXPECT_DOUBLE_EQ( 103080.5, x );
  EXPECT_DOUBLE_EQ( 520599.25, y );
  EXPECT_DOUBLE_EQ( 0.0, z );


  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 5697, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 3, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 5, f_v );

  // test last face
  f_v_count = getFaceVerticesCountAt( m, 5696 );
  EXPECT_EQ( 4, f_v_count ); //quad
  f_v = getFaceVerticesIndexAt( m, 5696, 0 );
  EXPECT_EQ( 6659, f_v );
  f_v = getFaceVerticesIndexAt( m, 5696, 1 );
  EXPECT_EQ( 87, f_v );
  f_v = getFaceVerticesIndexAt( m, 5696, 2 );
  EXPECT_EQ( 89, f_v );
  f_v = getFaceVerticesIndexAt( m, 5696, 3 );
  EXPECT_EQ( 6660, f_v );


  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 9, MDAL_M_datasetGroupCount( m ) );

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
  ASSERT_EQ( f_count, count );

  std::vector<double> elevations( f_count );
  int nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::SCALAR_DOUBLE, elevations.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_TRUE( std::isnan( elevations.at( 0 ) ) );
  ASSERT_TRUE( std::isnan( getValue( ds, 0 ) ) );
  ASSERT_DOUBLE_EQ( elevations.at( 1 ), -1.0 );
  ASSERT_DOUBLE_EQ( elevations.at( 1 ), getValue( ds, 1 ) );
  ASSERT_DOUBLE_EQ( elevations.at( 5652 ), -1.0 );
  ASSERT_DOUBLE_EQ( elevations.at( 5652 ), getValue( ds, 5652 ) );
  ASSERT_DOUBLE_EQ( elevations.at( 5653 ), -1.200136884248908 );
  ASSERT_DOUBLE_EQ( elevations.at( 5653 ), getValue( ds, 5653 ) );
  ASSERT_TRUE( std::isnan( elevations.at( 5696 ) ) );
  ASSERT_TRUE( std::isnan( getValue( ds, 5696 ) ) );


  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "water volume" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 5, MDAL_G_datasetCount( g ) );

  std::vector<double> values( f_count );

  ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );
  nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::SCALAR_DOUBLE, values.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_DOUBLE_EQ( values.at( 0 ), 747.270751953125 );
  ASSERT_DOUBLE_EQ( values.at( 0 ), getValue( ds, 0 ) );
  ASSERT_DOUBLE_EQ( values.at( 1 ), 2457.123291015625 );
  ASSERT_DOUBLE_EQ( values.at( 1 ), getValue( ds, 1 ) );
  ASSERT_DOUBLE_EQ( values.at( 5651 ), 3.6316468715667725 );
  ASSERT_DOUBLE_EQ( values.at( 5651 ), getValue( ds, 5651 ) );
  ASSERT_DOUBLE_EQ( values.at( 5652 ), 3.4241414070129395 );
  ASSERT_DOUBLE_EQ( values.at( 5652 ), getValue( ds, 5652 ) );
  ASSERT_DOUBLE_EQ( values.at( 5696 ), 0.0 );
  ASSERT_DOUBLE_EQ( values.at( 5696 ), getValue( ds, 5696 ) );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -1.9183223898266988e-08, min );
  EXPECT_DOUBLE_EQ( 6402.35595703125, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -5.2793296845266013e-08, min );
  EXPECT_DOUBLE_EQ( 7457.4951171875, max );


  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "flow velocity in cell centre" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 5, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( f_count, count );

  std::vector<double> vectorData( 2 * f_count );
  nValuesRead = MDAL_D_data( ds, 0, f_count, MDAL_DataType::VECTOR_2D_DOUBLE, vectorData.data() );
  ASSERT_EQ( f_count, nValuesRead );

  ASSERT_DOUBLE_EQ( vectorData.at( 0 ), -0.07972680032253265 );
  ASSERT_DOUBLE_EQ( vectorData.at( 0 ), getValueX( ds, 0 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 1 ), 0.008351435884833336 );
  ASSERT_DOUBLE_EQ( vectorData.at( 1 ), getValueY( ds, 0 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11304 ), 0.00043742090929299593 );
  ASSERT_DOUBLE_EQ( vectorData.at( 11304 ), getValueX( ds, 5652 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11305 ), -0.0002279738400829956 );
  ASSERT_DOUBLE_EQ( vectorData.at( 11305 ), getValueY( ds, 5652 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11392 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 11392 ), getValueX( ds, 5696 ) );
  ASSERT_DOUBLE_EQ( vectorData.at( 11393 ), -9999. );
  ASSERT_DOUBLE_EQ( vectorData.at( 11393 ), getValueY( ds, 5696 ) );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0., min );
  EXPECT_DOUBLE_EQ( 14140.721410168577, max );

  EXPECT_TRUE( compareReferenceTime( g, "2000-01-01T12:00:00" ) );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.7500505555555556 ) );

  MDAL_CloseMesh( m );
}

TEST( Mesh3DiTest, Mesh2D4cells301steps )
{
  std::string path = test_file( "/3di/2d_4cells301steps/results_3di.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "3Di:\"" + path + "\":Mesh2D;;3Di:\"" + path + "\":Mesh2D_groundwater;;3Di:\"" + path + "\":Mesh2D_surface_water" );
  std::string uri = "3Di:\"" + path + "\":Mesh2D";
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 9 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );

  std::vector<double> expectedCoords =
  {
    0.0, 0.0, 0.0,
    12, 0, 0.0,
    12, 12, 0.0,
    0, 12, 0.0,
    12, 24, 0.0,
    0, 24, 0.0,
    24, 0, 0.0,
    24, 12, 0.0,
    24, 24, 0.0
  };
  EXPECT_EQ( expectedCoords.size(), 9 * 3 );

  std::vector<double> coordinates = getCoordinates( m, 9 );

  EXPECT_TRUE( compareVectors( expectedCoords, coordinates ) );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 3, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 2, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 4, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 5, f_v );

  // ///////////
  // Bed elevation dataset
  // ///////////
  ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );

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
  ASSERT_EQ( 4, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 0, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 0, value );

  // ///////////
  // Scalar Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "waterlevel" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 301, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 1, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.32025772825098286, min );
  EXPECT_DOUBLE_EQ( 1, max );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "flow velocity in cell centre" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 301, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 80 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 0, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 8.4487915942199819e-14, max );

  EXPECT_TRUE( compareReferenceTime( g, "2014-01-01T00:00:00" ) );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.22222222222 ) );

  MDAL_CloseMesh( m );
}

TEST( Mesh3DiTest, Mesh2D16cells7steps )
{
  std::string path = test_file( "/3di/2d_16cells7steps/results_3di.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "3Di:\"" + path + "\":Mesh2D;;3Di:\"" + path + "\":Mesh2D_groundwater;;3Di:\"" + path + "\":Mesh2D_surface_water" );
  std::string uri = "3Di:\"" + path + "\":Mesh2D";
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 25 );
  double z = getVertexZCoordinatesAt( m, 0 );
  EXPECT_DOUBLE_EQ( 0.0, z );
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 16, f_count );

  ASSERT_EQ( 7, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( 7, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  EXPECT_TRUE( compareReferenceTime( g, "2014-01-01T00:00:00" ) );

  ds = MDAL_G_dataset( g, 6 );
  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( time, 0.01666666667 ) );

  MDAL_CloseMesh( m );
}

TEST( Mesh3DiTest, Mesh1D )
{
  std::string path = test_file( "/3di/1d_loon/results_3di.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "3Di:\"" + path + "\":Mesh1D;;3Di:\"" + path + "\":Mesh2D;;3Di:\"" + path + "\":Mesh2D_groundwater;;3Di:\"" + path + "\":Mesh2D_surface_water" );
  std::string uri = path + "\":Mesh1D";
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  const char *projection = MDAL_M_projection( m );
  EXPECT_EQ( std::string( "EPSG:28992" ), std::string( projection ) );

  EXPECT_EQ( MDAL_M_vertexCount( m ), 109 );
  EXPECT_EQ( MDAL_M_edgeCount( m ), 204 );
  EXPECT_EQ( MDAL_M_faceCount( m ), 0 );

  EXPECT_EQ( getVertexXCoordinatesAt( m, 1 ), 237014 ) ;
  EXPECT_EQ( getVertexYCoordinatesAt( m, 1 ), 559119.1 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 1 ), 7.12 );

  ASSERT_EQ( 14, MDAL_M_datasetGroupCount( m ) );
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 11 );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( DataOnVertices, MDAL_G_dataLocation( g ) );
  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );
  ASSERT_TRUE( MDAL::equals( 8.162201881408691, getValue( ds, ( 5 ) ) ) );

  g = MDAL_M_datasetGroup( m, 10 );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( DataOnEdges, MDAL_G_dataLocation( g ) );
  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 2 );
  ASSERT_NE( ds, nullptr );
  ASSERT_TRUE( MDAL::equals( -0.44919684529304504, getValue( ds, ( 5 ) ) ) );

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

