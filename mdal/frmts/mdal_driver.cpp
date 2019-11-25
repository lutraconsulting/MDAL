/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
*/

#include <string.h>
#include "mdal_driver.hpp"
#include "mdal_utils.hpp"
#include "mdal_memory_data_model.hpp"

MDAL::Driver::Driver( const std::string &name,
                      const std::string &longName,
                      const std::string &filters,
                      int capabilityFlags )
  : mName( name )
  , mLongName( longName )
  , mFilters( filters )
  , mCapabilityFlags( capabilityFlags )
{

}

MDAL::Driver::~Driver() = default;

std::string MDAL::Driver::name() const
{
  return mName;
}

std::string MDAL::Driver::longName() const
{
  return mLongName;
}

std::string MDAL::Driver::filters() const
{
  return mFilters;
}

bool MDAL::Driver::hasCapability( MDAL::Capability capability ) const
{
  return capability == ( mCapabilityFlags & capability );
}

bool MDAL::Driver::canReadMesh( const std::string &uri )
{
  return false;
}

bool MDAL::Driver::canReadDatasets( const std::string &uri )
{
  return false;
}

int MDAL::Driver::faceVerticesMaximumCount() const
{
  return -1;
}

std::unique_ptr< MDAL::Mesh > MDAL::Driver::load( const std::string &uri, MDAL_Status *status )
{
  MDAL_UNUSED( uri );
  MDAL_UNUSED( status );
  return std::unique_ptr< MDAL::Mesh >();
}

void MDAL::Driver::load( const std::string &uri, Mesh *mesh, MDAL_Status *status )
{
  MDAL_UNUSED( uri );
  MDAL_UNUSED( mesh );
  MDAL_UNUSED( status );
  return;
}

void MDAL::Driver::save( const std::string &uri, MDAL::Mesh *mesh, MDAL_Status *status )
{
  MDAL_UNUSED( uri );
  MDAL_UNUSED( mesh );
  MDAL_UNUSED( status );
}

void MDAL::Driver::createDatasetGroup( MDAL::Mesh *mesh, const std::string &groupName, MDAL_DataLocation dataLocation, bool hasScalarData, const std::string &datasetGroupFile )
{
  std::shared_ptr<MDAL::DatasetGroup> grp(
    new MDAL::DatasetGroup( name(),
                            mesh,
                            datasetGroupFile )
  );
  grp->setName( groupName );
  grp->setDataLocation( dataLocation );
  grp->setIsScalar( hasScalarData );
  grp->startEditing();
  mesh->datasetGroups.push_back( grp );
}

void MDAL::Driver::createDataset( MDAL::DatasetGroup *group, double time, const double *values, const int *active )
{
  std::shared_ptr<MDAL::MemoryDataset2D> dataset = std::make_shared< MemoryDataset2D >( group );
  dataset->setTime( time );
  size_t count = dataset->valuesCount();

  if ( !group->isScalar() )
    count *= 2;

  memcpy( dataset->values(), values, sizeof( double ) * count );
  if ( active && dataset->active() )
    memcpy( dataset->active(), active, sizeof( int ) * dataset->mesh()->facesCount() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
}

bool MDAL::Driver::persist( MDAL::DatasetGroup *group )
{
  MDAL_UNUSED( group );
  return true; // failure
}
