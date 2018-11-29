/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <algorithm>
#include "mdal_utils.hpp"

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

MDAL::Statistics MDAL::DatasetGroup::statistics() const
{
  return mStatistics;
}

void MDAL::DatasetGroup::setStatistics( const Statistics &statistics )
{
  mStatistics = statistics;
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

MDAL::Mesh::Mesh( size_t verticesCount, size_t facesCount, size_t faceVerticesMaximumCount, MDAL::BBox extent, const std::string &uri )
  : mVerticesCount( verticesCount )
  , mFacesCount( facesCount )
  , mFaceVerticesMaximumCount( faceVerticesMaximumCount )
  , mExtent( extent )
  , mUri( uri )
{
}

MDAL::Mesh::~Mesh() = default;

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

void MDAL::Mesh::setExtent( const BBox &extent )
{
  mExtent = extent;
}

void MDAL::Mesh::setFaceVerticesMaximumCount( const size_t &faceVerticesMaximumCount )
{
  mFaceVerticesMaximumCount = faceVerticesMaximumCount;
}

void MDAL::Mesh::setFacesCount( const size_t &facesCount )
{
  mFacesCount = facesCount;
}

void MDAL::Mesh::setVerticesCount( const size_t &verticesCount )
{
  mVerticesCount = verticesCount;
}

size_t MDAL::Mesh::verticesCount() const
{
  return mVerticesCount;
}

size_t MDAL::Mesh::facesCount() const
{
  return mFacesCount;
}

std::string MDAL::Mesh::uri() const
{
  return mUri;
}

MDAL::BBox MDAL::Mesh::extent() const
{
  return mExtent;
}

std::string MDAL::Mesh::crs() const
{
  return mCrs;
}

size_t MDAL::Mesh::faceVerticesMaximumCount() const
{
  return mFaceVerticesMaximumCount;
}

MDAL::MeshVertexIterator::~MeshVertexIterator() = default;

MDAL::MeshFaceIterator::~MeshFaceIterator() = default;
