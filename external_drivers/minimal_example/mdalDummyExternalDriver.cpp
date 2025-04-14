/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include <map>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <limits>

#include "mdal_external_driver.h"

//-----------------------------------------------------------------
//          Mesh structure

#define MAX_VERTEX_PER_FACE 4

static std::string sName( "Dynamic_driver_test" );
static std::string sLongName( "Dynamic driver test" );
static std::string sFilters( "" );

struct Vertex
{
  double x, y, z;
};

typedef std::vector<size_t> Face;
typedef std::pair<size_t, size_t> Edge ;

struct Dataset
{
  double time;
  std::vector<double> values;
  std::vector<int> isFaceActive;
  std::vector<int> volumeCounts;
  std::vector<double> volumeLevels;
};

struct Datasetgroup
{
  std::string name;
  std::vector<std::pair<std::string, std::string>> metadata;
  std::string dataType;
  bool scalar;
  std::string referenceTime;
  std::vector<Dataset> dataset;
};

struct Mesh
{
  std::string name;
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  std::vector<Edge> edges;

  std::string crs = "EPSG::32620";

  std::vector<Datasetgroup> datasetGroups;
};

//-----------------------------------------------------------------

static std::vector<std::string> split( const std::string &str,
                                       const char delimiter )
{
  std::vector<std::string> list;
  std::string::const_iterator start = str.begin();
  std::string::const_iterator end = str.end();
  std::string::const_iterator next;
  std::string token;
  do
  {
    next = std::find( start, end, delimiter );
    token = std::string( start, next );
    if ( !token.empty() )
      list.push_back( token );

    if ( next == end )
      break;
    else
      start = next + 1;
  }
  while ( true );
  return list;
}

static Mesh parseMesh( const std::string &uri )
{
  Mesh mesh;
  std::ifstream file( uri, std::ifstream::in );
  if ( !file.is_open() )
    return mesh;
  std::string line;
  if ( std::getline( file, line ) )
    mesh.name = line;

  // vertices
  while ( std::getline( file, line ) && line != "---" )
  {
    Vertex vert;
    std::vector<std::string> vertString = split( line, ',' );
    vert.x = atof( vertString.at( 0 ).c_str() );
    vert.y = atof( vertString.at( 1 ).c_str() );
    vert.z = atof( vertString.at( 2 ).c_str() );
    mesh.vertices.push_back( vert );
  }

  // faces and edges
  while ( std::getline( file, line ) && line != "---" )
  {
    std::vector <size_t> elem;
    std::vector<std::string> elemString = split( line, ',' );
    for ( const std::string &str : elemString )
      elem.push_back( atoi( str.c_str() ) );
    if ( elem.size() == 2 )
    {
      mesh.edges.emplace_back( elem.at( 0 ), elem.at( 1 ) );
    }
    else if ( elem.size() > 2 )
      mesh.faces.emplace_back( std::move( elem ) );
  }

  // datasets
  while ( std::getline( file, line ) )
  {
    while ( line != "---" && !file.eof() )
    {
      Datasetgroup group;
      group.name = line;
      for ( int i = 0; i < 2; ++i )
        if ( getline( file, line ) )
        {
          std::vector<std::string> meta = split( line, ',' );
          if ( meta.size() == 2 )
            group.metadata.push_back(
              std::pair<std::string, std::string>( meta.at( 0 ), meta.at( 1 ) ) );
        }

      if ( getline( file, line ) )
        group.dataType = line;

      if ( getline( file, line ) )
        group.scalar = line == "Scalar";

      if ( getline( file, line ) )
        group.referenceTime = line;

      while ( getline( file, line ) && line != "---" )
      {
        Dataset dataset;
        dataset.time = atof( line.c_str() );
        if ( getline( file, line ) )
        {
          std::vector<std::string> valuesStr = split( line, ',' );
          for ( const std::string &valStr : valuesStr )
            dataset.values.push_back( atof( valStr.c_str() ) );
        }
        else
          break;

        if ( group.dataType == "onFace" )
        {
          if ( getline( file, line ) )
          {
            std::vector<std::string> valuesStr = split( line, ',' );
            for ( const std::string &valStr : valuesStr )
              dataset.isFaceActive.push_back( atoi( valStr.c_str() ) );
          }
        }
        else if ( group.dataType == "onVolume" )
        {
          if ( getline( file, line ) )
          {
            std::vector<std::string> valuesStr = split( line, ',' );
            for ( const std::string &valStr : valuesStr )
              dataset.volumeCounts.push_back( atoi( valStr.c_str() ) );
          }
          if ( getline( file, line ) )
          {
            std::vector<std::string> valuesStr = split( line, ',' );
            for ( const std::string &valStr : valuesStr )
              dataset.volumeLevels.push_back( atof( valStr.c_str() ) );
          }
        }

        group.dataset.emplace_back( std::move( dataset ) );
      }

      mesh.datasetGroups.emplace_back( std::move( group ) );

      getline( file, line );
    }
  }

  return mesh;
}

// helper to return string data - without having to deal with memory too much.
// returned pointer is valid only next call. also not thread-safe.
const char *_return_str( const std::string &str )
{
  static std::string lastStr;
  lastStr = str;
  return lastStr.c_str();
}

static std::map<int, Mesh> sMeshes;
static int sIdGenerator = 0;

//**************************************************************************
//
//              Driver API
//
//**************************************************************************

#ifdef __cplusplus
extern "C" {
#endif
const char *MDAL_DRIVER_driverName()
{
  return sName.c_str();
}
const char *MDAL_DRIVER_driverLongName()
{
  return sLongName.c_str();
}
const char *MDAL_DRIVER_filters()
{
  return sFilters.c_str();
}
int MDAL_DRIVER_capabilities()
{
  return 1;
}
int MDAL_DRIVER_maxVertexPerFace()
{
  return MAX_VERTEX_PER_FACE;
}

bool MDAL_DRIVER_canReadMesh( const char *uri )
{
  std::vector<std::string> uriSplit = split( uri, '.' );
  if ( uriSplit.size() > 1 && uriSplit.back() == "msh" )
  {
    std::ifstream in( uri );
    if ( in.good() )
      return true;
  }

  return false;
}

int MDAL_DRIVER_openMesh( const char *uri, const char * )
{
  if ( MDAL_DRIVER_canReadMesh( uri ) )
  {
    int id = sIdGenerator++;
    sMeshes[id] = parseMesh( uri );
    return id;
  }

  return -1;
}

void MDAL_DRIVER_closeMesh( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
    sMeshes.erase( sMeshes.find( meshId ) );
}


int MDAL_DRIVER_M_vertexCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    return int( sMeshes[meshId].vertices.size() );
  }
  return -1;
}

int MDAL_DRIVER_M_faceCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    return static_cast<int>( sMeshes[meshId].faces.size() );
  }
  return -1;
}

int MDAL_DRIVER_M_edgeCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    return static_cast<int>( sMeshes[meshId].edges.size() );
  }
  return -1;
}

void MDAL_DRIVER_M_extent( int meshId, double *xMin, double *xMax, double *yMin, double *yMax )
{
  *xMin = std::numeric_limits<double>::quiet_NaN();
  *xMax = std::numeric_limits<double>::quiet_NaN();
  *yMin = std::numeric_limits<double>::quiet_NaN();
  *yMax = std::numeric_limits<double>::quiet_NaN();

  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    *xMin = std::numeric_limits<double>::max();
    *xMax = -std::numeric_limits<double>::max();
    *yMin = std::numeric_limits<double>::max();
    *yMax = -std::numeric_limits<double>::max();

    for ( const Vertex &v : mesh.vertices )
    {
      if ( v.x < *xMin )
        *xMin = v.x;
      if ( v.x > *xMax )
        *xMax = v.x;
      if ( v.y < *yMin )
        *yMin = v.y;
      if ( v.y > *yMax )
        *yMax = v.y;
    }
  }
}

const char *MDAL_DRIVER_M_projection( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    return sMeshes[meshId].crs.c_str();
  }
  return _return_str( "" );
}

int MDAL_DRIVER_M_vertices( int meshId, int startIndex, int count, double *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    // I wonder which form we need : an iterator as MDAL interface (need to create a class whatever the driver).
    // Or a function like this with start index and count (here the caller keep reponsability to know the position,
    // and there will be only one caller : common implementation of dynamic driver of MDAL. simpler to implement?
    const Mesh &mesh = sMeshes[meshId];
    if ( startIndex >= static_cast<int>( mesh.vertices.size() ) )
      return -1;
    size_t effectiveVertCount = std::min( size_t( count ), mesh.vertices.size() - startIndex );
    std::vector<double> coordinates( effectiveVertCount * 3 );
    for ( size_t i = 0; i < effectiveVertCount; ++i )
    {
      coordinates[3 * i] = mesh.vertices.at( i ).x;
      coordinates[3 * i + 1] = mesh.vertices.at( i ).y;
      coordinates[3 * i + 2] = mesh.vertices.at( i ).z;
    }

    memcpy( buffer, coordinates.data(), effectiveVertCount * 3 * sizeof( double ) );

    return static_cast<int>( effectiveVertCount );
  }
  return -1;
}

int MDAL_DRIVER_M_faces( int meshId, int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBufer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( startFaceIndex >= static_cast<int>( mesh.faces.size() ) )
      return -1;
    size_t effectiveFaceCount = std::min( size_t( faceCount ), mesh.faces.size() - startFaceIndex );
    size_t faceIndex = startFaceIndex;
    size_t maxFaceIndex = startFaceIndex + effectiveFaceCount - 1;
    std::vector<int> faceSizes;
    faceSizes.reserve( effectiveFaceCount );
    std::vector<int> vertexIndices;
    vertexIndices.reserve( vertexIndicesBufferLen );

    while ( faceIndex <= maxFaceIndex && vertexIndices.size() < size_t( vertexIndicesBufferLen + MAX_VERTEX_PER_FACE ) )
    {
      const Face &face = mesh.faces.at( faceIndex );
      for ( size_t i = 0; i < face.size(); ++i )
      {
        vertexIndices.push_back( static_cast<int>( face.at( i ) ) );
      }
      faceSizes.push_back( static_cast<int>( vertexIndices.size() ) );
      ++faceIndex;
    }

    memcpy( faceOffsetsBuffer, faceSizes.data(), faceSizes.size() * sizeof( int ) );
    memcpy( vertexIndicesBufer, vertexIndices.data(), vertexIndices.size() * sizeof( int ) );

    return static_cast<int>( faceSizes.size() );
  }

  return -1;
}

int MDAL_DRIVER_M_edges( int meshId, int startEdgeIndex, int edgeCount, int *startVertexIndices, int *endVertexIndices )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( startEdgeIndex >= static_cast<int>( mesh.edges.size() ) )
      return -1;
    size_t effectiveEdgesCount = std::min( size_t( edgeCount ), mesh.edges.size() - startEdgeIndex );
    std::vector<int> startIndices;
    startIndices.reserve( effectiveEdgesCount );
    std::vector<int> endIndices;
    endIndices.reserve( effectiveEdgesCount );
    for ( size_t i = 0; i < effectiveEdgesCount; ++i )
    {
      startIndices.push_back( static_cast<int>( mesh.edges.at( i ).first ) );
      endIndices.push_back( static_cast<int>( mesh.edges.at( i ).second ) );
    }

    memcpy( startVertexIndices, startIndices.data(), sizeof( int )*effectiveEdgesCount );
    memcpy( endVertexIndices, endIndices.data(), sizeof( int )*effectiveEdgesCount );

    return static_cast<int>( effectiveEdgesCount );
  }

  return -1;
}

int MDAL_DRIVER_M_datasetGroupCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    return static_cast<int>( mesh.datasetGroups.size() );
  }

  return -1;
}

const char *MDAL_DRIVER_G_groupName( int meshId, int groupIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= int( mesh.datasetGroups.size() ) )
      return _return_str( "" );
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    return datasetGroup.name.c_str();

  }

  return _return_str( "" );
}

const char *MDAL_DRIVER_G_referenceTime( int meshId, int groupIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= int( mesh.datasetGroups.size() ) )
      return  _return_str( "" );
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );

    return datasetGroup.referenceTime.c_str();
  }

  return  _return_str( "" );
}

int MDAL_DRIVER_G_metadataCount( int meshId, int groupIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );

    return static_cast<int>( datasetGroup.metadata.size() );
  }

  return -1;
}

//! Returns the metadata key
const char *MDAL_DRIVER_G_metadataKey( int meshId, int groupIndex, int metaDataIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= int( mesh.datasetGroups.size() ) )
      return _return_str( "" );
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    return datasetGroup.metadata.at( metaDataIndex ).first.c_str();
  }

  return _return_str( "" );
}

//! Returns the metadata value
const char *MDAL_DRIVER_G_metadataValue( int meshId, int groupIndex, int metaDataIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= int( mesh.datasetGroups.size() ) )
      return _return_str( "" );
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    return datasetGroup.metadata.at( metaDataIndex ).second.c_str();
  }

  return _return_str( "" );
}

bool MDAL_DRIVER_G_datasetsDescription( int meshId, int groupIndex, bool *isScalar, int *dataLocation, int *datasetCount )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= int( mesh.datasetGroups.size() ) )
      return false;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    if ( !isScalar || !dataLocation || !datasetCount )
      return false;
    *isScalar = datasetGroup.scalar;
    if ( datasetGroup.dataType == "onVertex" )
      *dataLocation = 1;
    if ( datasetGroup.dataType == "onFace" )
      *dataLocation = 2;
    if ( datasetGroup.dataType == "onVolume" )
      *dataLocation = 3;
    if ( datasetGroup.dataType == "onEdge" )
      *dataLocation = 4;

    *datasetCount = static_cast<int>( datasetGroup.dataset.size() );

    return true;
  }

  return false;
}

MDAL_LIB_EXPORT double MDAL_DRIVER_D_time( int meshId, int groupIndex, int datasetIndex, bool *ok )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    *ok = groupIndex < static_cast<int>( mesh.datasetGroups.size() ) ;
    if ( !( *ok ) )
      return 0;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    *ok = datasetIndex < static_cast<int>( datasetGroup.dataset.size() );
    if ( !( *ok ) )
      return 0;
    const Dataset &dataset = datasetGroup.dataset.at( datasetIndex );
    return dataset.time;
  }

  *ok = false;
  return 0;
}

int MDAL_DRIVER_D_data( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( datasetIndex );
    int totalDatasetValues = static_cast<int>( dataset.values.size() );
    if ( !datasetGroup.scalar )
      totalDatasetValues /= 2;
    int effectiveCount = std::min( count, totalDatasetValues - indexStart );

    int bufferValues = effectiveCount;
    if ( datasetGroup.scalar )
      for ( int i = 0; i < bufferValues; ++i )
        buffer[i] = dataset.values.at( i + indexStart );
    else
      for ( int i = 0; i < bufferValues * 2; ++i )
        buffer[i] = dataset.values.at( i + indexStart * 2 );

    return effectiveCount;
  }

  return -1;
}

bool MDAL_DRIVER_D_hasActiveFlagCapability( int meshId, int groupIndex, int )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return false;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( groupIndex );
    if ( datasetGroup.dataType == "onFace" )
      return true;
  }

  return false;
}

//! Returns the dataset active flags
int MDAL_DRIVER_D_activeFlags( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onFace" )
      return -1;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );
    int totalValues = static_cast<int>( dataset.isFaceActive.size() );
    int effectiveCount = std::min( count, totalValues - indexStart );

    for ( int i = 0; i < effectiveCount; ++i )
      buffer[i] = dataset.isFaceActive.at( size_t( i + indexStart ) );

    return effectiveCount;
  }

  return false;
}

int MDAL_DRIVER_D_maximumVerticalLevelCount( int meshId, int groupIndex, int datasetIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onVolume" )
      return 0;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );

    int maxLevelcount = 0;
    for ( const int levelCount : dataset.volumeCounts )
    {
      if ( maxLevelcount < levelCount )
        maxLevelcount = levelCount;
    }

    return maxLevelcount;

  }

  return -1;
}

int MDAL_DRIVER_D_volumeCount( int meshId, int groupIndex, int datasetIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onVolume" )
      return 0;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );

    int volCount = 0;
    for ( const int levelCount : dataset.volumeCounts )
    {
      volCount += levelCount;
    }

    return volCount;
  }

  return -1;
}

int MDAL_DRIVER_D_verticalLevelCountData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onVolume" )
      return 0;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );

    int totalValueCount = static_cast<int>( dataset.volumeCounts.size() );

    int effectiveCount = std::min( count,  totalValueCount - indexStart );

    int bufferValues = effectiveCount;
    for ( int i = 0; i < bufferValues; ++i )
      buffer[i] = dataset.volumeCounts.at( i + indexStart );

    return effectiveCount;
  }

  return -1;
}

int MDAL_DRIVER_D_verticalLevelData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onVolume" )
      return 0;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );

    int totalValueCount = static_cast<int>( dataset.volumeLevels.size() );

    int effectiveCount = std::min( count,  totalValueCount - indexStart );

    int bufferValues = effectiveCount;
    for ( int i = 0; i < bufferValues; ++i )
      buffer[i] = dataset.volumeLevels.at( i + indexStart );

    return effectiveCount;
  }

  return -1;
}

int MDAL_DRIVER_D_faceToVolumeData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    const Mesh &mesh = sMeshes[meshId];
    if ( groupIndex >= static_cast<int>( mesh.datasetGroups.size() ) )
      return -1;
    const Datasetgroup &datasetGroup = mesh.datasetGroups.at( size_t( groupIndex ) );
    if ( datasetGroup.dataType != "onVolume" )
      return 0;

    if ( datasetIndex >= static_cast<int>( datasetGroup.dataset.size() ) )
      return -1;
    const Dataset &dataset = datasetGroup.dataset.at( size_t( datasetIndex ) );

    std::vector<int> faceToVolume;
    int i = 0;
    for ( const int volCount : dataset.volumeCounts )
    {
      faceToVolume.push_back( i );
      i += volCount;
    }

    int totalValueCount = static_cast<int>( dataset.volumeLevels.size() );

    int effectiveCount = std::min( count,  totalValueCount - indexStart );

    int bufferValues = effectiveCount;
    for ( int i = 0; i < bufferValues; ++i )
      buffer[i] = faceToVolume.at( i + indexStart );

    return effectiveCount;
  }

  return -1;
}

MDAL_LIB_EXPORT void MDAL_DRIVER_D_unload( int, int, int )
{}

#ifdef __cplusplus
}//////////////////////////
#endif
