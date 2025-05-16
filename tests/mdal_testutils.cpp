/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "gtest/gtest.h"

#include "mdal_testutils.hpp"
#include "mdal_config.hpp"
#include "mdal_utils.hpp"
#include <vector>
#include <math.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>

#ifdef _MSC_VER
#include <locale>
#include <codecvt>
#include <stringapiset.h>
#endif

const char *data_path()
{
  return TESTDATA;
}

const char *drivers_path()
{
  return DRIVERS_PATH;
}


std::string test_file( const std::string &basename )
{
  std::string path( data_path() );
  path += basename;
  return path;
}

std::string tmp_file( const std::string &basename )
{
  std::string path( data_path() + std::string( "/tmp" ) );
  path += basename;
  return path;
}

void copy( const std::string &src, const std::string &dest )
{
  std::ifstream srcS;
  std::ofstream dstS;

  srcS.open( src, std::ios::in | std::ios::binary );
  dstS.open( dest, std::ios::out | std::ios::binary );
  dstS << srcS.rdbuf();
}

bool deleteFile( const std::string &path )
{
  if ( fileExists( path ) )
  {
#ifdef _MSC_VER
    std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
    std::wstring wStr = converter.from_bytes( path );
    return DeleteFileW( wStr.c_str() );
#else
    return remove( path.c_str() ) == 0;
#endif
  }
  return true;
}

bool fileExists( const std::string &filename )
{
#ifdef _MSC_VER
  std::ifstream in;
  std::wstring_convert< std::codecvt_utf8_utf16< wchar_t > > converter;
  std::wstring wStr = converter.from_bytes( filename );
  in.open( wStr, std::ifstream::in | std::ifstream::binary );
  if ( !in.is_open() )
    return false;
#else
  std::ifstream in( filename );
#endif

  return in.good();
}

int getActive( MDAL_DatasetH dataset, int index )
{
  bool hasFlag = MDAL_D_hasActiveFlagCapability( dataset );
  if ( hasFlag )
  {
    int active;
    int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::ACTIVE_INTEGER, &active );
    if ( nValuesRead != 1 )
      return -2;
    return static_cast<bool>( active );
  }
  else
  {
    return -1;
  }
}

double getValue( MDAL_DatasetH dataset, int index )
{
  double val;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::SCALAR_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val;
}

double getValueX( MDAL_DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[0];
}

double getValueY( MDAL_DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[1];
}

int getLevelsCount3D( MDAL_DatasetH dataset, int index )
{
  int count;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, &count );
  if ( nValuesRead != 1 )
    return -1;

  return count;
}

double getLevelZ3D( MDAL_DatasetH dataset, int index )
{
  double z;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, &z );
  if ( nValuesRead != 1 )
    return -1;

  return z;
}

double getValue3D( MDAL_DatasetH dataset, int index )
{
  double val;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::SCALAR_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val;
}

double getValue3DX( MDAL_DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[0];
}

double getValue3DY( MDAL_DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[1];
}

int get3DFrom2D( MDAL_DatasetH dataset, int index )
{
  int index3d;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::FACE_INDEX_TO_VOLUME_INDEX_INTEGER, &index3d );
  if ( nValuesRead != 1 )
    return -1;

  return index3d;
}

bool compareVectors( const std::vector<int> &a, const std::vector<int> &b )
{
  if ( a.size() != b.size() )
    return false;

  for ( size_t i = 0; i < a.size(); ++i )
    if ( a[i] != b[i] )
      return false;

  return true;
}

bool compareVectors( const std::vector<double> &a, const std::vector<double> &b )
{
  double eps = 1e-4;
  if ( a.size() != b.size() )
    return false;

  for ( size_t i = 0; i < a.size(); ++i )
    if ( fabs( a[i] - b[i] ) > eps )
      return false;

  return true;
}

void compareMeshFrames( MDAL_MeshH meshA, MDAL_MeshH meshB )
{
  // Vertices
  int orignal_v_count = MDAL_M_vertexCount( meshA );
  int saved_v_count = MDAL_M_vertexCount( meshB );
  EXPECT_EQ( orignal_v_count, saved_v_count );

  std::vector<double> coordsA = getCoordinates( meshA, orignal_v_count );
  std::vector<double> coordsB = getCoordinates( meshB, saved_v_count );
  EXPECT_TRUE( compareVectors( coordsA, coordsB ) );

  // Edges
  int orignal_e_count = MDAL_M_edgeCount( meshA );
  int saved_e_count = MDAL_M_edgeCount( meshB );
  EXPECT_EQ( orignal_e_count, saved_e_count );

  std::vector<int> original_start;
  std::vector<int> original_end;
  std::vector<int> saved_start;
  std::vector<int> saved_end;

  getEdgeVertexIndices( meshA, orignal_e_count, original_start, original_end );
  getEdgeVertexIndices( meshB, saved_e_count, saved_start, saved_end );

  EXPECT_TRUE( compareVectors( original_start, saved_start ) );
  EXPECT_TRUE( compareVectors( original_end, saved_end ) );

  // Faces
  int orignal_f_count = MDAL_M_faceCount( meshA );
  int saved_f_count = MDAL_M_faceCount( meshB );
  EXPECT_EQ( orignal_f_count, saved_f_count );

  std::vector<int> verticesA = faceVertexIndices( meshA, orignal_f_count );
  std::vector<int> verticesB = faceVertexIndices( meshB, saved_f_count );

  EXPECT_TRUE( compareVectors( verticesA, verticesB ) );
}

void compareMeshMetadata( MDAL_MeshH meshA, MDAL_MeshH meshB )
{
  // Metadata count
  const int orignal_m_count = MDAL_M_metadataCount( meshA );
  const int saved_m_count = MDAL_M_metadataCount( meshB );
  EXPECT_EQ( orignal_m_count, saved_m_count );

  // Metadata values
  for ( int i = 0; i < orignal_m_count; ++i )
  {
    const std::string keyA( MDAL_M_metadataKey( meshA, i ) );
    const std::string valA( MDAL_M_metadataValue( meshA, i ) );
    for ( int j = 0; j < saved_m_count; ++j )
    {
      const std::string keyB( MDAL_M_metadataKey( meshB, j ) );
      const std::string valB( MDAL_M_metadataValue( meshB, j ) );

      if ( keyA == keyB && valA == valB )
        break;
      else if ( j == saved_m_count - 1 )
        FAIL() << "Mesh metadata do not match: " << keyA << ": " << valA;
    }
  }
}

std::vector<double> getCoordinates( MDAL_MeshH mesh, int verticesCount )
{
  MDAL_MeshVertexIteratorH iterator = MDAL_M_vertexIterator( mesh );
  std::vector<double> coordinates( static_cast<size_t>( 3 * verticesCount ) );
  MDAL_VI_next( iterator, verticesCount, coordinates.data() );
  MDAL_VI_close( iterator );
  return coordinates;
}

void getEdgeVertexIndices( MDAL_MeshH mesh, int edgesCount, std::vector<int> &start, std::vector<int> &end )
{
  MDAL_MeshEdgeIteratorH iterator = MDAL_M_edgeIterator( mesh );
  start.clear();
  start.resize( static_cast<size_t>( edgesCount ) );
  end.clear();
  end.resize( static_cast<size_t>( edgesCount ) );
  MDAL_EI_next( iterator, edgesCount, start.data(), end.data() );
  MDAL_EI_close( iterator );
}

double _getVertexCoordinatesAt( MDAL_MeshH mesh, int index, int coordIndex )
{
  // coordIndex = 0 x
  // coordIndex = 1 y
  // coordIndex = 2 z
  std::vector<double> coordinates = getCoordinates( mesh, index + 1 );
  double val = coordinates[static_cast<size_t>( index * 3 + coordIndex )];
  return val;
}

double getVertexXCoordinatesAt( MDAL_MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 0 );
}

double getVertexYCoordinatesAt( MDAL_MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 1 );
}

double getVertexZCoordinatesAt( MDAL_MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 2 );
}

std::vector<int> faceVertexIndices( MDAL_MeshH mesh, int faceCount )
{
  MDAL_MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
  int faceOffsetsBufferLen = faceCount;
  int vertexIndicesBufferLen = faceOffsetsBufferLen * MDAL_M_faceVerticesMaximumCount( mesh );
  std::vector<int> faceOffsetsBuffer( static_cast<size_t>( faceOffsetsBufferLen ) );
  std::vector<int> vertexIndicesBuffer( static_cast<size_t>( vertexIndicesBufferLen ) );
  MDAL_FI_next( iterator, faceOffsetsBufferLen, faceOffsetsBuffer.data(),
                vertexIndicesBufferLen, vertexIndicesBuffer.data() );
  MDAL_FI_close( iterator );
  return vertexIndicesBuffer;
}

int getFaceVerticesCountAt( MDAL_MeshH mesh, int faceIndex )
{
  MDAL_MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
  int faceOffsetsBufferLen = faceIndex + 1;
  int vertexIndicesBufferLen = faceOffsetsBufferLen * MDAL_M_faceVerticesMaximumCount( mesh );
  std::vector<int> faceOffsetsBuffer( static_cast<size_t>( faceOffsetsBufferLen ) );
  std::vector<int> vertexIndicesBuffer( static_cast<size_t>( vertexIndicesBufferLen ) );
  MDAL_FI_next( iterator, faceOffsetsBufferLen, faceOffsetsBuffer.data(),
                vertexIndicesBufferLen, vertexIndicesBuffer.data() );
  MDAL_FI_close( iterator );
  int count;
  if ( faceIndex == 0 )
  {
    count = faceOffsetsBuffer[static_cast<size_t>( faceIndex )];
  }
  else
  {
    count = faceOffsetsBuffer[static_cast<size_t>( faceIndex )] - faceOffsetsBuffer[static_cast<size_t>( faceIndex - 1 )];
  }
  return count;
}

int getFaceVerticesIndexAt( MDAL_MeshH mesh, int faceIndex, int index )
{
  MDAL_MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
  int faceOffsetsBufferLen = faceIndex + 1;
  int vertexIndicesBufferLen = faceOffsetsBufferLen * MDAL_M_faceVerticesMaximumCount( mesh );
  std::vector<int> faceOffsetsBuffer( static_cast<size_t>( faceOffsetsBufferLen ) );
  std::vector<int> vertexIndicesBuffer( static_cast<size_t>( vertexIndicesBufferLen ) );
  MDAL_FI_next( iterator, faceOffsetsBufferLen, faceOffsetsBuffer.data(),
                vertexIndicesBufferLen, vertexIndicesBuffer.data() );
  MDAL_FI_close( iterator );
  int id;
  if ( faceIndex == 0 )
  {
    id = index;
  }
  else
  {
    id = faceOffsetsBuffer[static_cast<size_t>( faceIndex - 1 )] + index;
  }

  int faceVertexIndex = vertexIndicesBuffer[static_cast<size_t>( id )];
  return faceVertexIndex;
}

void set_mdal_driver_path( const std::string &dirname )
{
  std::string fullPath = std::string( drivers_path() ) +  "/" + dirname;
#ifdef WIN32
  size_t requiredSize = 0;
  getenv_s( &requiredSize, NULL, 0, "MDAL_DRIVER_PATH" );
  if ( requiredSize == 0 )
  {
    _putenv_s( "MDAL_DRIVER_PATH", fullPath.c_str() );
  }
#endif

#ifndef WIN32
  setenv( "MDAL_DRIVER_PATH", fullPath.c_str(), 0 );
#endif
}

void init_test()
{
}

void finalize_test()
{
}

bool compareDurationInHours( double h1, double h2 )
{
  return fabs( h1 - h2 ) < 1.0 / 3600 / 1000;
}

bool hasReferenceTime( MDAL_DatasetGroupH group )
{
  return std::strcmp( MDAL_G_referenceTime( group ), "" ) != 0;
}

bool compareReferenceTime( MDAL_DatasetGroupH group, const char *referenceTime )
{
  return std::strcmp( MDAL_G_referenceTime( group ), referenceTime ) == 0;
}

void saveAndCompareMesh( const std::string &filename, const std::string &savedFile, const std::string &driver, const std::string &meshName, bool compareMetadata )
{
  //test driver capability
  EXPECT_TRUE( MDAL_DR_saveMeshCapability( MDAL_driverFromName( driver.c_str() ) ) );

  std::string uri( filename );

  std::string savedUri = driver + ":\"" + savedFile + "\"";

  if ( !meshName.empty() )
  {
    uri = "\"" + uri + "\":" + meshName;
    savedUri = savedUri + ":" + meshName;
  }

  // Open mesh
  MDAL_MeshH meshToSave = MDAL_LoadMesh( uri.c_str() );
  EXPECT_NE( meshToSave, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // Save the mesh
  MDAL_SaveMeshWithUri( meshToSave, savedUri.c_str() );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // Load saved mesh
  MDAL_MeshH savedMesh = MDAL_LoadMesh( savedFile.c_str() );
  EXPECT_NE( savedMesh, nullptr );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // Compare saved with the original mesh
  compareMeshFrames( meshToSave, savedMesh );
  if ( compareMetadata )
    compareMeshMetadata( meshToSave, savedMesh );

  MDAL_CloseMesh( savedMesh );

  // Again but with other API method
  MDAL_SaveMesh( meshToSave,  savedFile.c_str(), driver.c_str() );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // Load saved mesh
  savedMesh = MDAL_LoadMesh( savedFile.c_str() );
  EXPECT_NE( savedMesh, nullptr );
  s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // Close meshed and delete all the files
  MDAL_CloseMesh( meshToSave );
  MDAL_CloseMesh( savedMesh );
  ASSERT_TRUE( deleteFile( savedFile ) );
}
