/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <string>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

static MDAL_MeshH mesh()
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  return m;
}

static MDAL_MeshH lines_mesh()
{
  std::string path = test_file( "/2dm/lines.2dm" );
  EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  return m;
}

TEST( MeshAsciiDatTest, MissingMesh )
{
  MDAL_MeshH m = nullptr;
  std::string path = test_file( "ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
}

TEST( MeshAsciiDatTest, MissingFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "non/existent/path.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_FileNotFound, s );
  EXPECT_EQ( 1, MDAL_M_datasetGroupCount( m ) );
  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, WrongFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/not_a_data_file.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_UnknownFormat, s );
  EXPECT_EQ( 1, MDAL_M_datasetGroupCount( m ) );
  MDAL_CloseMesh( m );
}

TEST( Mesh2DMTest, Mixed1D2D )
{
  std::string meshFile = test_file( "/2dm/quad_and_line.2dm" );
  MDAL_MeshH m = MDAL_LoadMesh( meshFile.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

  // for mixed meshes, vertex datasets are supported
  // we use the dataset from quad_and_triangle, since if the number of vertices
  // and number of elements matches, the actual file is the same for both 1D and 2D
  // data
  std::string vertexPath = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar.dat" );
  MDAL_M_LoadDatasets( m, vertexPath.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  // for mixed meshes, element datasets are not supported
  std::string facePath = test_file( "/ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, facePath.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, LinesFaceScalarFile )
{
  MDAL_MeshH m = lines_mesh();
  std::string path = test_file( "/ascii_dat/lines_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "EdgeScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 3, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 3, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 3, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 1, time );

  EXPECT_TRUE( compareReferenceTime( g,  "1950-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, LinesFaceVectorFile )
{
  MDAL_MeshH m = lines_mesh();
  std::string path = test_file( "/ascii_dat/lines_els_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "EdgeVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 3, count );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 4.2426406871192848, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 4.2426406871192848, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 1, time );

  EXPECT_TRUE( compareReferenceTime( g, "1950-01-07T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, LinesVertexScalarFile )
{
  MDAL_MeshH m = lines_mesh();
  std::string path = test_file( "/ascii_dat/lines_vertex_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}


TEST( MeshAsciiDatTest, LinesVertexVectorFile )
{
  MDAL_MeshH m = lines_mesh();
  std::string path = test_file( "/ascii_dat/lines_vertex_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 4, count );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0, time );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 1, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceScalarFileWithNumberingGaps )
{
  std::string meshPath = test_file( "/2dm/mesh_with_numbering_gaps.2dm" );
  MDAL_MeshH m = MDAL_LoadMesh( meshPath.c_str() );
  ASSERT_NE( m, nullptr );

  std::string path = test_file( "/ascii_dat/mesh_with_numbering_gaps_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "mesh_with_numbering_gaps_scalar" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 3, value );

  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4, value );

  value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 5, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 5, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 10, max );

  MDAL_CloseMesh( m );
}

TEST( XdmfTest, MeshNumberedFrom0 )
{
  std::string path = test_file( "/xdmf/simple/simpleXFMD.2dm" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // basically it should be denied with any ascii dat file
  std::string datDath = test_file( "/ascii_dat/mesh_with_numbering_gaps_scalar.dat" );
  MDAL_M_LoadDatasets( m, datDath.c_str() );
  s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::Err_IncompatibleMesh, s );
  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceScalarFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "FaceScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 2, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1, min );
  EXPECT_DOUBLE_EQ( 2, max );

  EXPECT_EQ( 0, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 0, MDAL_G_maximumVerticalLevelCount( g ) );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 1, time );

  EXPECT_TRUE( compareReferenceTime( g,  "1950-01-01T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleFaceVectorFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_els_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );

  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "FaceVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 2.8284271247461903, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 1.4142135623730951, min );
  EXPECT_DOUBLE_EQ( 2.8284271247461903, max );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 1, time );

  EXPECT_TRUE( compareReferenceTime( g, "1950-01-07T00:00:00" ) );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarOldFile )
{
  for ( int i = 0; i < 4; ++i )
  {
    std::string name = "quad_and_triangle_vertex_scalar_old" + std::to_string( i );
    MDAL_MeshH m = mesh();
    std::string path = test_file( "/ascii_dat/" + name + ".dat" );
    MDAL_M_LoadDatasets( m, path.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 1, meta_count );

    const char *key = MDAL_G_metadataKey( g, 0 );
    EXPECT_EQ( std::string( "name" ), std::string( key ) );

    const char *val = MDAL_G_metadataValue( g, 0 );
    EXPECT_EQ( name, std::string( val ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    value = getValue( ds, 1 );
    EXPECT_DOUBLE_EQ( 2, value );

    ds = MDAL_G_dataset( g, 1 );
    double time = MDAL_D_time( ds );
    if ( i == 3 )
    {
      // With timeunits
      EXPECT_DOUBLE_EQ( time, 1.0 );
    }
    else
    {
      // default timeunits
      EXPECT_DOUBLE_EQ( time, 3600.0 );
    }

    MDAL_CloseMesh( m );
  }
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexScalarFileWithTabs )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_scalar_tabs.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexScalarDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexVectorFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_vector.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 2, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "VertexVectorDataset" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0, time );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 1, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, QuadAndTriangleVertexVectorOldFile )
{
  MDAL_MeshH m = mesh();
  std::string path = test_file( "/ascii_dat/quad_and_triangle_vertex_vector_old.dat" );
  MDAL_M_LoadDatasets( m, path.c_str() );
  MDAL_Status s = MDAL_LastStatus();
  EXPECT_EQ( MDAL_Status::None, s );
  ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
  ASSERT_NE( g, nullptr );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 1, meta_count );

  const char *key = MDAL_G_metadataKey( g, 0 );
  EXPECT_EQ( std::string( "name" ), std::string( key ) );

  const char *val = MDAL_G_metadataValue( g, 0 );
  EXPECT_EQ( std::string( "quad_and_triangle_vertex_vector_old" ), std::string( val ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 1, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double time = MDAL_D_time( ds );
  EXPECT_DOUBLE_EQ( 0, time );

  double value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 1, value );

  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 2, value );

  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 1, value );

  MDAL_CloseMesh( m );
}

TEST( MeshAsciiDatTest, WriteScalarVertexTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string scalarPath = tmp_file( "/2dm_WriteScalarTest.dat" );
  std::vector<double> vals = {1, 2, 3, 4, 5};
  std::vector<int> active = {1, 0};

  // Create a new dat file
  {

    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, scalarPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    int active = getActive( ds, 0 );
    EXPECT_EQ( 1, active );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 5, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    ds = MDAL_G_dataset( g, 1 );
    ASSERT_NE( ds, nullptr );

    active = getActive( ds, 1 );
    EXPECT_EQ( 0, active );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshAsciiDatTest, WriteScalarFaceTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string scalarPath = tmp_file( "/2dm_WriteScalarTest_els.dat" );
  std::vector<double> vals = {1, 2};

  // Create a new dat file
  {

    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnFaces ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
                             m,
                             "scalarGrp",
                             MDAL_DataLocation::DataOnFaces,
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
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       nullptr
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, scalarPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 2, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshAsciiDatTest, WriteScalarEdgeTest )
{
  std::string path = test_file( "/2dm/lines.2dm" );
  std::string scalarPath = tmp_file( "/2dm_WriteScalarLinesTest_els.dat" );
  std::vector<double> vals = {1, 2, 3};

  // Create a new dat file
  {

    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnEdges ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
                             m,
                             "scalarGrp",
                             MDAL_DataLocation::DataOnEdges,
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
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       nullptr
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, scalarPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "scalarGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( true, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnEdges );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 3, count );

    double value = getValue( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshAsciiDatTest, WriteVectorVertexTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string vectorPath = tmp_file( "/2dm_WriteVectorTest.dat" );
  std::vector<double> vals = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5};
  std::vector<int> active = {1, 1};

  // Create a new dat file
  {

    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, vectorPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
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

TEST( MeshAsciiDatTest, WriteVectorFaceTest )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string vectorPath = tmp_file( "/2dm_WriteVectorTest_els.dat" );
  std::vector<double> vals = {1, 1, 2, 2};

  // Create a new dat file
  {

    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnFaces ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
                             m,
                             "vectorGrp",
                             MDAL_DataLocation::DataOnFaces,
                             false,
                             driver,
                             vectorPath.c_str()
                           );
    ASSERT_NE( g, nullptr );

    MDAL_G_addDataset( g,
                       0.0,
                       vals.data(),
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       nullptr
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, vectorPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

    int count = MDAL_D_valueCount( ds );
    ASSERT_EQ( 2, count );

    double value = getValueX( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    value = getValueY( ds, 0 );
    EXPECT_DOUBLE_EQ( 1, value );

    MDAL_CloseMesh( m );
  }
}

TEST( MeshAsciiDatTest, WriteVectorVertexTestNoActive )
{
  std::string path = test_file( "/2dm/quad_and_triangle.2dm" );
  std::string vectorPath = tmp_file( "/2dm_WriteVectorTestNoActive.dat" );
  std::vector<double> vals = {1, 1, 2, 2, 3, 3, 4, 4, 5, 5};

  // Create a new dat file
  {
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );

    ASSERT_EQ( 1, MDAL_M_datasetGroupCount( m ) );

    MDAL_DriverH driver = MDAL_driverFromName( "ASCII_DAT" );
    ASSERT_NE( driver, nullptr );
    ASSERT_TRUE( MDAL_DR_writeDatasetsCapability( driver, MDAL_DataLocation::DataOnVertices ) );

    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup(
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
                       nullptr
                     );
    MDAL_G_addDataset( g,
                       1.0,
                       vals.data(),
                       nullptr
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
    EXPECT_EQ( MDAL_MeshNames( path.c_str() ), "2DM:\"" + path + "\"" );
    MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
    ASSERT_NE( m, nullptr );
    MDAL_M_LoadDatasets( m, vectorPath.c_str() );
    MDAL_Status s = MDAL_LastStatus();
    EXPECT_EQ( MDAL_Status::None, s );
    ASSERT_EQ( 2, MDAL_M_datasetGroupCount( m ) );

    MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 1 );
    ASSERT_NE( g, nullptr );

    int meta_count = MDAL_G_metadataCount( g );
    ASSERT_EQ( 2, meta_count );

    const char *name = MDAL_G_name( g );
    EXPECT_EQ( std::string( "vectorGrp" ), std::string( name ) );

    bool scalar = MDAL_G_hasScalarData( g );
    EXPECT_EQ( false, scalar );

    MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
    EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

    ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );
    MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
    ASSERT_NE( ds, nullptr );

    bool valid = MDAL_D_isValid( ds );
    EXPECT_EQ( true, valid );

    EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

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
