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

MDAL::Dataset::Dataset( MDAL::DatasetGroup *parent )
  : mParent( parent )
{
  assert( mParent );
}

size_t MDAL::Dataset::valuesCount() const
{
  if ( group()->isOnVertices() )
  {
    return mesh()->verticesCount();
  }
  else
  {
    return mesh()->facesCount();
  }
}

MDAL::Statistics MDAL::Dataset::statistics() const
{
  return mStatistics;
}

void MDAL::Dataset::setStatistics( const MDAL::Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DatasetGroup *MDAL::Dataset::group() const
{
  return mParent;
}

MDAL::Mesh *MDAL::Dataset::mesh() const
{
  return mParent->mesh();
}

double MDAL::Dataset::time() const
{
  return mTime;
}

void MDAL::Dataset::setTime( double time )
{
  mTime = time;
}

bool MDAL::Dataset::isValid() const
{
  return mIsValid;
}

void MDAL::Dataset::setIsValid( bool isValid )
{
  mIsValid = isValid;
}

MDAL::DatasetGroup::DatasetGroup( MDAL::Mesh *parent,
                                  const std::string &uri,
                                  const std::string &name )
  : mParent( parent )
  , mUri( uri )
{
  assert( mParent );
  setName( name );
}

MDAL::DatasetGroup::~DatasetGroup() = default;

MDAL::DatasetGroup::DatasetGroup( MDAL::Mesh *parent, const std::string &uri )
  : mParent( parent )
  , mUri( uri )
{
  assert( mParent );
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

MDAL::Statistics MDAL::DatasetGroup::statistics() const
{
  return mStatistics;
}

void MDAL::DatasetGroup::setStatistics( const Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::Mesh *MDAL::DatasetGroup::mesh() const
{
  return mParent;
}

bool MDAL::DatasetGroup::isOnVertices() const
{
  return mIsOnVertices;
}

void MDAL::DatasetGroup::setIsOnVertices( bool isOnVertices )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mIsOnVertices = isOnVertices;
}

bool MDAL::DatasetGroup::isScalar() const
{
  return mIsScalar;
}

void MDAL::DatasetGroup::setIsScalar( bool isScalar )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
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

void MDAL::Mesh::setFaceVerticesMaximumCount( size_t faceVerticesMaximumCount )
{
  mFaceVerticesMaximumCount = faceVerticesMaximumCount;
}

void MDAL::Mesh::setFacesCount( size_t facesCount )
{
  mFacesCount = facesCount;
}

void MDAL::Mesh::setVerticesCount( size_t verticesCount )
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
