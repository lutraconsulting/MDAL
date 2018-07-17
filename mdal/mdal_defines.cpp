/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include <mdal_defines.hpp>
#include <assert.h>

void MDAL::Dataset::free()
{
  values.clear();
  isValid = false;
  parent = nullptr;
}

bool MDAL::Dataset::isActive( size_t faceIndex )
{
  assert( parent );
  if ( parent->isOnVertices )
  {
    if ( active.size() > faceIndex )
      return active[faceIndex];
    else
      return false;
  }
  else
  {
    return true;
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

void MDAL::DatasetGroup::free()
{
  datasets.clear();
  metadata.clear();
}
