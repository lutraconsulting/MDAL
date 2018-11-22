/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <algorithm>
#include "mdal_utils.hpp"

#define NODATA std::numeric_limits<double>::quiet_NaN()

MDAL::Dataset::~Dataset() = default;

size_t MDAL::Dataset::valuesCount() const
{
  assert( parent );
  assert( parent->parent );
  if ( parent->isOnVertices() )
  {
    return parent->parent->verticesCount();
  }
  else
  {
    return parent->parent->facesCount();
  }
}

MDAL::MemoryDataset::~MemoryDataset() = default;

size_t MDAL::MemoryDataset::activeData( size_t indexStart, size_t count, char *buffer )
{
  assert( parent );
  if ( parent->isOnVertices() )
  {
    assert( active.size() > indexStart ); //checked in C API interface
    assert( active.size() >= indexStart + count ); //checked in C API interface
    char *src = active.data() + indexStart;
    memcpy( buffer, src, count );
  }
  else
  {
    memset( buffer, true, count );
    return true;
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
      buffer[i] = NODATA;
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
      buffer[2 * i] = NODATA;
      buffer[2 * i + 1] = NODATA;
    }
    else
    {
      buffer[2 * i] = value.x;
      buffer[2 * i + 1] = value.y;
    }
  }

  return count;
}

std::string MDAL::DatasetGroup::getMetadata( const std::string &key )
{
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      return pair.second;
    }
  }
  return std::string();
}

void MDAL::DatasetGroup::setMetadata( const std::string &key, const std::string &val )
{
  bool found = false;
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      found = true;
      pair.second = val;
    }
  }
  if ( !found )
    metadata.push_back( std::make_pair( key, val ) );
}

std::string MDAL::DatasetGroup::name()
{
  return getMetadata( "name" );
}

void MDAL::DatasetGroup::setName( const std::string &name )
{
  setMetadata( "name", name );
}

std::string MDAL::DatasetGroup::uri() const
{
  return mUri;
}

void MDAL::DatasetGroup::setUri( const std::string &uri )
{
  mUri = uri;
}

bool MDAL::DatasetGroup::isOnVertices() const
{
  return mIsOnVertices;
}

void MDAL::DatasetGroup::setIsOnVertices( bool isOnVertices )
{
  mIsOnVertices = isOnVertices;
}

bool MDAL::DatasetGroup::isScalar() const
{
  return mIsScalar;
}

void MDAL::DatasetGroup::setIsScalar( bool isScalar )
{
  mIsScalar = isScalar;
}

void MDAL::Mesh::setSourceCrs( const std::string &str )
{
  mCrs = MDAL::trim( str );
}

void MDAL::Mesh::setSourceCrsFromWKT( const std::string &wkt )
{
  setSourceCrs( wkt );
}

void MDAL::Mesh::setSourceCrsFromEPSG( int code )
{
  setSourceCrs( std::string( "EPSG:" ) + std::to_string( code ) );
}

void MDAL::Mesh::addBedElevationDataset()
{
  if ( faces.empty() )
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

size_t MDAL::Mesh::verticesCount() const
{
  return vertices.size();
}

size_t MDAL::Mesh::facesCount() const
{
  return faces.size();
}

std::string MDAL::Mesh::uri() const
{
  return mUri;
}

void MDAL::Mesh::setUri( const std::string &uri )
{
  mUri = uri;
}

std::string MDAL::Mesh::crs() const
{
  return mCrs;
}
