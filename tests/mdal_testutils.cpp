#define xstr(a) str(a)
#define str(a) #a

#include "mdal_testutils.hpp"
#include "mdal_config.hpp"
#include <vector>
#include <math.h>
#include <assert.h>
#include <fstream>
#include <stdio.h>

const char *data_path()
{
  return TESTDATA;
}


std::string test_file( std::string basename )
{
  std::string path( data_path() );
  path += basename;
  return path;
}

std::string tmp_file( std::string basename )
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

void deleteFile( const std::string &path )
{
  if ( fileExists( path ) )
    remove( path.c_str() );
}

bool fileExists( const std::string &filename )
{
  std::ifstream in( filename );
  return in.good();
}

int getActive( DatasetH dataset, int index )
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

double getValue( DatasetH dataset, int index )
{
  double val;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::SCALAR_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val;
}

double getValueX( DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[0];
}

double getValueY( DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[1];
}

int getLevelsCount3D( DatasetH dataset, int index )
{
  int count;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VERTICAL_LEVEL_COUNT_INTEGER, &count );
  if ( nValuesRead != 1 )
    return -1;

  return count;
}

double getLevelZ3D( DatasetH dataset, int index )
{
  double z;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VERTICAL_LEVEL_DOUBLE, &z );
  if ( nValuesRead != 1 )
    return -1;

  return z;
}

double getValue3D( DatasetH dataset, int index )
{
  double val;
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::SCALAR_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val;
}

double getValue3DX( DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[0];
}

double getValue3DY( DatasetH dataset, int index )
{
  double val[2];
  int nValuesRead = MDAL_D_data( dataset, index, 1, MDAL_DataType::VECTOR_2D_VOLUMES_DOUBLE, &val );
  if ( nValuesRead != 1 )
    return 0;

  return val[1];
}

int get3DFrom2D( DatasetH dataset, int index )
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

bool compareMeshFrames( MeshH meshA, MeshH meshB )
{
  // Vertices
  int orignal_v_count = MDAL_M_vertexCount( meshA );
  int saved_v_count = MDAL_M_vertexCount( meshB );
  if ( orignal_v_count != saved_v_count ) return false;

  std::vector<double> coordsA = getCoordinates( meshA, orignal_v_count );
  std::vector<double> coordsB = getCoordinates( meshB, saved_v_count );
  if ( !compareVectors( coordsA, coordsB ) )
    return false;

  // Faces
  int orignal_f_count = MDAL_M_faceCount( meshA );
  int saved_f_count = MDAL_M_faceCount( meshB );
  if ( orignal_f_count != saved_f_count ) return false;

  std::vector<int> verticesA = faceVertexIndices( meshA, orignal_f_count );
  std::vector<int> verticesB = faceVertexIndices( meshB, saved_f_count );
  if ( !compareVectors( verticesA, verticesB ) )
    return false;

  return true;
}

std::vector<double> getCoordinates( MeshH mesh, int verticesCount )
{
  MeshVertexIteratorH iterator = MDAL_M_vertexIterator( mesh );
  std::vector<double> coordinates( static_cast<size_t>( 3 * verticesCount ) );
  MDAL_VI_next( iterator, verticesCount, coordinates.data() );
  MDAL_VI_close( iterator );
  return coordinates;
}

double _getVertexCoordinatesAt( MeshH mesh, int index, int coordIndex )
{
  // coordIndex = 0 x
  // coordIndex = 1 y
  // coordIndex = 2 z
  std::vector<double> coordinates = getCoordinates( mesh, index + 1 );
  double val = coordinates[static_cast<size_t>( index * 3 + coordIndex )];
  return val;
}

double getVertexXCoordinatesAt( MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 0 );
}

double getVertexYCoordinatesAt( MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 1 );
}

double getVertexZCoordinatesAt( MeshH mesh, int index )
{
  return _getVertexCoordinatesAt( mesh, index, 2 );
}

std::vector<int> faceVertexIndices( MeshH mesh, int faceCount )
{
  MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
  int faceOffsetsBufferLen = faceCount + 1;
  int vertexIndicesBufferLen = faceOffsetsBufferLen * MDAL_M_faceVerticesMaximumCount( mesh );
  std::vector<int> faceOffsetsBuffer( static_cast<size_t>( faceOffsetsBufferLen ) );
  std::vector<int> vertexIndicesBuffer( static_cast<size_t>( vertexIndicesBufferLen ) );
  MDAL_FI_next( iterator, faceOffsetsBufferLen, faceOffsetsBuffer.data(),
                vertexIndicesBufferLen, vertexIndicesBuffer.data() );
  MDAL_FI_close( iterator );
  return vertexIndicesBuffer;
}

int getFaceVerticesCountAt( MeshH mesh, int faceIndex )
{
  MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
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

int getFaceVerticesIndexAt( MeshH mesh, int faceIndex, int index )
{
  MeshFaceIteratorH iterator = MDAL_M_faceIterator( mesh );
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

void init_test()
{

}

void finalize_test()
{
}
