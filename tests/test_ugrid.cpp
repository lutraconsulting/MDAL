/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <math.h>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327
#endif

TEST( MeshUgridTest, SaveDFlow11Manzese )
{
  saveAndCompareMesh(
    test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ),
    tmp_file( "/manzese_1d2d_small_map_saveTest.nc" ),
    "Ugrid",
    "mesh2d"
  );
}

TEST( MeshUgridTest, SaveQuadAndTriangle )
{
  saveAndCompareMesh(
    test_file( "/2dm/quad_and_triangle.2dm" ),
    tmp_file( "/quad_and_triangle_saveTest.nc" ),
    "Ugrid"
  );
}

TEST( MeshUgridTest, DFlow11Manzese )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" );
  std::string uri = "\"" + path + "\":mesh2d";

  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":mesh1d;;Ugrid:\"" + path + "\":mesh2d" );

  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 3042 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 1824, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //quad
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 25, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 50, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 51, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 3 );
  EXPECT_EQ( 26, f_v );

  ASSERT_EQ( 10, MDAL_M_datasetGroupCount( m ) );

  // ///////////
  // Dataset
  // ///////////
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Total bed shear stress" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 6, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 1824, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 0.0, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 15.907056943512494, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 0.0, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "2017-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow11ManzeseNodeZValue )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" );
  std::string uri = "\"" + path + "\":mesh2d";
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":mesh1d;;Ugrid:\"" + path + "\":mesh2d" );
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  double z = getVertexZCoordinatesAt( m, 0 );
  ASSERT_EQ( z, 42.8397 );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow11Simplebox )
{
  std::string path = test_file( "/ugrid/D-Flow1.1/simplebox_hex7_map.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":mesh2d" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 720 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 810, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 479, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 1 );
  EXPECT_EQ( 524, f_v );
  f_v = getFaceVerticesIndexAt( m, 1, 2 );
  EXPECT_EQ( 480, f_v );

  // ///////////
  // dataset
  // ///////////
  ASSERT_EQ( 10, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 7 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "water depth at pressure points" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 13, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 810, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 3.1245244868590514, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 2.1346136585097848, min );
  EXPECT_DOUBLE_EQ( 5.8788940865351824, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 2.1346136585097848, min );
  EXPECT_DOUBLE_EQ( 6.3681219945588952, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( .0097222222222222224, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "2001-05-05T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow12RivierGridClm )
{
  std::string path = test_file( "/ugrid/D-Flow1.2/bw_11_zonder_riviergrid_met_1dwtg_clm.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":Mesh2D" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12310 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 11987, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // dataset
  // ///////////
  ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water level" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 45, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 6, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 2, min );
  EXPECT_DOUBLE_EQ( 12, max );

  // this dataset contains only discrete set of results
  // Mesh2D_s1:flag_values = "1 2 3 4 5 6 7 8 9 10 11 12" ;
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 2, min );
  EXPECT_DOUBLE_EQ( 12, max );

  double time = MDAL_D_time( ds );
  EXPECT_TRUE( compareDurationInHours( 183.5, time ) );

  EXPECT_TRUE( compareReferenceTime( g, "2002-10-15T00:00:00" ) );

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( "EPSG:28992", crs );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, DFlow12RivierGridMap )
{
  std::string path = test_file( "/ugrid/D-Flow1.2/bw_11_zonder_riviergrid_met_1dwtg_map.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":Mesh2D" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12310 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 11987, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 4, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 0, f_v );

  // ///////////
  // scalar dataset
  // ///////////
  ASSERT_EQ( 6, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Water level" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 23, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 3 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 3.8664888639861923, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( -0.3397185302284324, min );
  EXPECT_DOUBLE_EQ( 12.898470433826372, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -0.3397185302284324, min );
  EXPECT_DOUBLE_EQ( 12.898470433826372, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 184, time );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "Flow element center velocity vector" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 23, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 20 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 11987, count );

  value = getValueX( ds, 82 );
  EXPECT_DOUBLE_EQ( 0, value );

  value = getValueX( ds, 6082 );
  EXPECT_DOUBLE_EQ( 0.042315615419157751, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 0.66413616798770714, max );

  EXPECT_TRUE( compareReferenceTime( g, "2002-10-15T00:00:00" ) );

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( "EPSG:28992", crs );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, UGRIDFormatWithoutTime )
{
  std::string path = test_file( "/ugrid/without_time/TINUGRID.tin" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":TIN" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( 5, v_count );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 4, f_count );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, ADCIRC )
{
  std::string path = test_file( "/ugrid/ADCIRC/ADCIRC_BG_20190910_1t.nc" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "Ugrid:\"" + path + "\":mesh_topology" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  // ///////////
  // Vertices
  // ///////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 12769 );

  // ///////////
  // Faces
  // ///////////
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( 23860, f_count );

  // test face 1
  int f_v_count = getFaceVerticesCountAt( m, 1 );
  EXPECT_EQ( 3, f_v_count ); //triangle
  int f_v = getFaceVerticesIndexAt( m, 1, 0 );
  EXPECT_EQ( 1, f_v );

  // ///////////
  // scalar dataset
  // ///////////
  ASSERT_EQ( 3, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 2 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "sea surface height" ), std::string( name ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 12769, count );

  double value = getValue( ds, 500 );
  EXPECT_DOUBLE_EQ( 0.50850147008895874, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0.44549721479415894, min );
  EXPECT_DOUBLE_EQ( 0.91381251811981201, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.29059699177742004, min );
  EXPECT_DOUBLE_EQ( 0.91381251811981201, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 435576.5, time );

  // ///////////
  // Vector Dataset
  // ///////////
  g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "barotropic current" ), std::string( name ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 12769, count );

  value = getValueX( ds, 82 );
  EXPECT_DOUBLE_EQ( 0, value );

  value = getValueX( ds, 6082 );
  EXPECT_DOUBLE_EQ( -0.026552680879831314, value );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0, min );
  EXPECT_DOUBLE_EQ( 1.3282330120641679, max );

  EXPECT_TRUE( compareReferenceTime( g, "1970-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, 1DMeshTest )
{
  std::string path = test_file( "/ugrid/1dtest/dflow1d_map.nc" );

  std::string uri = "Ugrid:\"" + path + "\":" + "mesh1d";
  std::string uriToMeshNames = "Ugrid:\"" + path + "\"";

  EXPECT_EQ( MDAL_MeshNames( uriToMeshNames.c_str() ), "Ugrid:\"" + path + "\":mesh1d" ); // ignores network variable
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );

  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::Warn_InvalidElements, s );

  /////////////
  // Vertices
  /////////////
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 8 );

  /////////////
  // Edges
  /////////////
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( e_count, 7 );

  std::vector<int> startNodeEdgeIndices;
  std::vector<int> endNodeEdgeIndices;
  getEdgeVertexIndices( m, e_count, startNodeEdgeIndices, endNodeEdgeIndices );

  std::vector<int> expectedStartNodes {0, 1, 2, 3, 4, 5, 6};
  std::vector<int> expectedEndNodes {1, 2, 3, 4, 5, 6, 7};

  EXPECT_TRUE( compareVectors( startNodeEdgeIndices, expectedStartNodes ) );
  EXPECT_TRUE( compareVectors( endNodeEdgeIndices, expectedEndNodes ) );

  MDAL_DatasetGroupH dg = MDAL_M_datasetGroup( m, 3 );
  ASSERT_NE( dg, nullptr );

  ASSERT_EQ( std::string( "Flow element center velocity" ), std::string( MDAL_G_name( dg ) ) );
  ASSERT_EQ( MDAL_G_hasScalarData( dg ), true );

  MDAL_DataLocation dg_location = MDAL_G_dataLocation( dg );
  EXPECT_EQ( dg_location, MDAL_DataLocation::DataOnVertices );

  MDAL_DatasetH ds = MDAL_G_dataset( dg, 15 );
  ASSERT_NE( ds, nullptr );

  int ds_count = MDAL_D_valueCount( ds );
  EXPECT_EQ( ds_count, 8 );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( min, 1.4307828088040137e-14 );
  EXPECT_DOUBLE_EQ( max, 5.90487869582277e-13 );

  MDAL_CloseMesh( m );
}

TEST( MeshUgridTest, classifiedVariable )
{
  std::string path = test_file( "/ugrid/classified/simplebox_clm.nc" );
  MDAL_MeshH mesh = MDAL_LoadMesh( path.c_str() );

  EXPECT_NE( mesh, nullptr );

  MDAL_DatasetGroupH group = MDAL_M_datasetGroup( mesh, 1 );
  ASSERT_EQ( std::string( "Water depth at pressure points" ), std::string( MDAL_G_name( group ) ) );

  int metaDataCount = MDAL_G_metadataCount( group );
  EXPECT_EQ( 3, metaDataCount );
  ASSERT_EQ( std::string( "units" ), std::string( MDAL_G_metadataKey( group, 1 ) ) );
  ASSERT_EQ( std::string( MDAL_G_metadataValue( group, 1 ) ), std::string( "m" ) );
  ASSERT_EQ( std::string( "classification" ), std::string( MDAL_G_metadataKey( group, 2 ) ) );
  std::string classification( "0,0.5;;0.5,1;;1,5;;5,10;;10" );
  ASSERT_EQ( std::string( MDAL_G_metadataValue( group, 2 ) ), classification );

  MDAL_CloseMesh( mesh );
}

TEST( MeshUgridTest, magnitude_direction )
{
  std::vector<std::string> files;
  files.push_back( "simplebox_to_direction_clm.nc" );
  files.push_back( "simplebox_from_direction_clm.nc" );
  for ( const std::string &file : files )
  {
    std::string path = test_file( std::string( "/ugrid/magnitude_direction/" ) + file );
    MDAL_MeshH mesh = MDAL_LoadMesh( path.c_str() );
    EXPECT_NE( mesh, nullptr );
    ASSERT_EQ( 6, MDAL_M_datasetGroupCount( mesh ) );

    MDAL_DatasetGroupH group;
    if ( file == "simplebox_to_direction_clm.nc" )
    {
      group = MDAL_M_datasetGroup( mesh, 3 );
      ASSERT_EQ( std::string( "Flow element center velocity to direction relative true north" ), std::string( MDAL_G_name( group ) ) );
    }
    else
    {
      group = MDAL_M_datasetGroup( mesh, 2 );
      ASSERT_EQ( std::string( "Flow element center velocity from direction relative true north" ), std::string( MDAL_G_name( group ) ) );
    }
    MDAL_DatasetH dataset = MDAL_G_dataset( group, 10 );
    int directionIndex = getValue( dataset, 20 );
    EXPECT_EQ( directionIndex, 36 );

    if ( file == "simplebox_to_direction_clm.nc" )
    {
      group = MDAL_M_datasetGroup( mesh, 2 );
    }
    else
    {
      group = MDAL_M_datasetGroup( mesh, 3 );
    }
    ASSERT_EQ( std::string( "Flow element center velocity magnitude" ), std::string( MDAL_G_name( group ) ) );
    dataset = MDAL_G_dataset( group, 10 );
    int magIndex = getValue( dataset, 20 );
    EXPECT_EQ( magIndex, 3 );

    double direction = -( ( directionIndex - 1 ) * 10 + 5 ); //supposed to be clokcwise angle
    double magnitude = ( magIndex - 1 ) * 0.2 + 0.1;

    if ( file == "simplebox_from_direction_clm.nc" )
      direction = direction + 180;

    group = MDAL_M_datasetGroup( mesh, 1 );
    ASSERT_EQ( std::string( "Flow element center velocity" ), std::string( MDAL_G_name( group ) ) );
    dataset = MDAL_G_dataset( group, 10 );
    double x = getValueX( dataset, 20 );
    double y = getValueY( dataset, 20 );

    EXPECT_TRUE( MDAL::equals( x, magnitude * cos( 2 * M_PI * direction / 360 ) ) );
    EXPECT_TRUE( MDAL::equals( y, magnitude * sin( 2 * M_PI * direction / 360 ) ) );

    MDAL_CloseMesh( mesh );
  }
}

TEST( MeshUgridTest, VoidMesh )
{
  std::string path = test_file( "/ugrid/meshCreation/void_mesh.nc" );

  std::string uri = "Ugrid:\"" + path + "\":" + "mesh2d";
  std::string uriToMeshNames = "Ugrid:\"" + path + "\"";

  EXPECT_EQ( MDAL_MeshNames( uriToMeshNames.c_str() ), "Ugrid:\"" + path + "\":mesh2d" );
  MDAL_MeshH m = MDAL_LoadMesh( uri.c_str() );

  ASSERT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 0 );

  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 0 );

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

