/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_memory_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <cstring>
#include <algorithm>
#include "mdal_utils.hpp"

MDAL::MemoryDataset::~MemoryDataset() = default;

size_t MDAL::MemoryDataset::activeData( size_t indexStart, size_t count, int *buffer )
{
  assert( parent );
  if ( parent->isOnVertices() )
  {
    assert( active.size() > indexStart ); //checked in C API interface
    assert( active.size() >= indexStart + count ); //checked in C API interface
    int *src = active.data() + indexStart;
    memcpy( buffer, src, count * sizeof( int ) );
  }
  else
  {
    memset( buffer, true, count * sizeof( int ) );
  }

  return count;
}

size_t MDAL::MemoryDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( parent ); //checked in C API interface
  assert( parent->isScalar() ); //checked in C API interface
  assert( values.size() > indexStart ); //checked in C API interface
  assert( values.size() >= indexStart + count ); //checked in C API interface

  for ( size_t i = 0; i < count; ++i )
  {
    const MDAL::Value value = values[ indexStart + i ];
    if ( value.noData )
    {
      buffer[i] = MDAL_NAN;
    }
    else
    {
      buffer[i] = value.x;
    }
  }

  return count;
}

size_t MDAL::MemoryDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( parent ); //checked in C API interface
  assert( !parent->isScalar() ); //checked in C API interface
  assert( values.size() > indexStart ); //checked in C API interface
  assert( values.size() >= indexStart + count ); //checked in C API interface

  for ( size_t i = 0; i < count; ++i )
  {
    const MDAL::Value value = values[ indexStart + i ];
    if ( value.noData )
    {
      buffer[2 * i] = MDAL_NAN;
      buffer[2 * i + 1] = MDAL_NAN;
    }
    else
    {
      buffer[2 * i] = value.x;
      buffer[2 * i + 1] = value.y;
    }
  }

  return count;
}

MDAL::MemoryMesh::MemoryMesh( size_t verticesCount, size_t facesCount, size_t faceVerticesMaximumCount, MDAL::BBox extent, const std::string &uri )
  : MDAL::Mesh( verticesCount, facesCount, faceVerticesMaximumCount, extent, uri )
{

}

void MDAL::MemoryMesh::addBedElevationDataset( const Vertices &vertices, const Faces &faces )
{
  if ( 0 == facesCount() )
    return;

  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >();
  group->setIsOnVertices( true );
  group->setIsScalar( true );
  group->setName( "Bed Elevation" );
  group->setUri( uri() );
  group->parent = this;

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >();
  dataset->time = 0.0;
  dataset->values.resize( vertices.size() );
  dataset->active.resize( faces.size() );
  dataset->parent = group.get();
  std::fill( dataset->active.begin(), dataset->active.end(), 1 );
  for ( size_t i = 0; i < vertices.size(); ++i )
  {
    dataset->values[i].x = vertices[i].z;
  }
  group->datasets.push_back( dataset );
  datasetGroups.push_back( group );
}

std::unique_ptr<MDAL::MeshVertexIterator> MDAL::MemoryMesh::readVertices()
{
  std::unique_ptr<MDAL::MemoryMeshVertexIterator> it( new MemoryMeshVertexIterator( this ) );
  return it;
}

std::unique_ptr<MDAL::MeshFaceIterator> MDAL::MemoryMesh::readFaces()
{
  std::unique_ptr<MDAL::MemoryMeshFaceIterator> it( new MemoryMeshFaceIterator( this ) );
  return it;
}

MDAL::MemoryMesh::~MemoryMesh() = default;

MDAL::MemoryMeshVertexIterator::MemoryMeshVertexIterator( const MDAL::MemoryMesh *mesh )
  : mMemoryMesh( mesh )
{

}

MDAL::MemoryMeshVertexIterator::~MemoryMeshVertexIterator() = default;

size_t MDAL::MemoryMeshVertexIterator::next( size_t vertexCount, double *coordinates )
{
  assert( mMemoryMesh );
  assert( coordinates );

  size_t maxVertices = mMemoryMesh->verticesCount();

  if ( vertexCount > maxVertices )
    return 0;

  if ( mLastVertexIndex >= maxVertices )
    return 0;

  size_t i = 0;

  while ( true )
  {
    if ( mLastVertexIndex + i >= maxVertices )
      break;

    if ( i >= vertexCount )
      break;

    const Vertex v = mMemoryMesh->vertices[mLastVertexIndex + i];
    coordinates[3 * i] = v.x;
    coordinates[3 * i + 1] = v.y;
    coordinates[3 * i + 2] = v.z;

    ++i;
  }

  mLastVertexIndex += i;
  return i;
}

MDAL::MemoryMeshFaceIterator::MemoryMeshFaceIterator( const MDAL::MemoryMesh *mesh )
  : mMemoryMesh( mesh )
{
}

MDAL::MemoryMeshFaceIterator::~MemoryMeshFaceIterator() = default;

size_t MDAL::MemoryMeshFaceIterator::next(
  size_t faceOffsetsBufferLen, int *faceOffsetsBuffer,
  size_t vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  assert( mMemoryMesh );
  assert( faceOffsetsBuffer );
  assert( vertexIndicesBuffer );

  size_t maxFaces = mMemoryMesh->facesCount();
  size_t faceVerticesMaximumCount = mMemoryMesh->faceVerticesMaximumCount();
  size_t vertexIndex = 0;
  size_t faceIndex = 0;

  while ( true )
  {
    if ( vertexIndex + faceVerticesMaximumCount > vertexIndicesBufferLen )
      break;

    if ( faceIndex >= faceOffsetsBufferLen )
      break;

    if ( mLastFaceIndex + faceIndex >= maxFaces )
      break;

    const Face f = mMemoryMesh->faces[mLastFaceIndex + faceIndex];
    for ( size_t faceVertexIndex = 0; faceVertexIndex < f.size(); ++faceVertexIndex )
    {
      vertexIndicesBuffer[vertexIndex] = static_cast<int>( f[faceVertexIndex] );
      ++vertexIndex;
    }
    faceOffsetsBuffer[faceIndex] = static_cast<int>( vertexIndex );
    ++faceIndex;
  }

  if ( faceIndex > 0 )
  {
    // we actually got something read
    mLastFaceIndex += faceOffsetsBufferLen;
    faceOffsetsBuffer[mLastFaceIndex] = static_cast<int>( --vertexIndex );
  }
  return faceIndex;
}
