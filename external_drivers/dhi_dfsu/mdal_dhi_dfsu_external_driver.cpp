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

#include "mdal_external_driver.h"
#include "mdal_dhi_dfsu.hpp"

#include "limits.h"

#undef min
#undef max

static std::map<int, std::unique_ptr<Mesh>> sMeshes;
static int sIdGenerator = 0;

static std::string sName( "DHI" );
static std::string sLongName( "DHI dfsu" );
static std::string sFilters( "*.dfsu" );

#define MAX_VERTEX_PER_FACE 4

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
  return Mesh::canRead( uri );
}

int MDAL_DRIVER_openMesh( const char *uri, const char * )
{
  std::unique_ptr<Mesh> mesh = Mesh::loadMesh( uri );
  if ( mesh )
  {
    std::pair<std::map<int, std::unique_ptr<Mesh>>::iterator, bool> ret = sMeshes.emplace( sIdGenerator++, mesh.release() );
    return ret.first->first;
  }
  else
    return -1;
}

void MDAL_DRIVER_closeMesh( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    mesh->close();
    sMeshes.erase( sMeshes.find( meshId ) );
  }
}


int MDAL_DRIVER_M_vertexCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->verticesCount();
  }

  return -1;
}

int MDAL_DRIVER_M_faceCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->facesCount();
  }

  return -1;
}

int MDAL_DRIVER_M_edgeCount( int )
{
  return -1;
}

void MDAL_DRIVER_M_extent( int meshId, double *xMin, double *xMax, double *yMin, double *yMax )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    mesh->extent( xMin, xMax, yMin, yMax );
  }
}

const char *MDAL_DRIVER_M_projection( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->projection().c_str();
  }

  return "";
}

int MDAL_DRIVER_M_vertices( int meshId, int startIndex, int count, double *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    int returnedCount = std::max( 0, std::min( count, mesh->verticesCount() - startIndex ) );

    if ( returnedCount > 0 )
    {
      double *start = mesh->vertexCoordinates( startIndex );
      memcpy( buffer, start, returnedCount * 3 * sizeof( double ) );
    }

    return returnedCount;
  }

  return -1;
}

int MDAL_DRIVER_M_faces( int meshId, int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->connectivity( startFaceIndex, faceCount, faceOffsetsBuffer, vertexIndicesBufferLen, vertexIndicesBuffer );
  }

  return -1;
}

int MDAL_DRIVER_M_edges( int, int, int, int *, int * )
{
  return 0;
}

int MDAL_DRIVER_M_datasetGroupCount( int meshId )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->datasetGroupsCount();
  }
  return -1;
}

const char *MDAL_DRIVER_G_groupName( int meshId, int groupIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->datasetgroup( groupIndex )->name().c_str();
  }
  return "";
}

const char *MDAL_DRIVER_G_referenceTime( int meshId, int )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->referenceTime().c_str();
  }
  return "";
}

int MDAL_DRIVER_G_metadataCount( int meshId, int index )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    if ( index >= 0 && index < static_cast<int>( mesh->datasetGroupsCount() ) )
      return static_cast<int>( mesh->datasetgroup( index )->metadata().size() );
  }
  return -1;
}

//! Returns the metadata key
const char *MDAL_DRIVER_G_metadataKey( int meshId, int groupIndex, int metaDataIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    if ( groupIndex >= 0 && groupIndex < static_cast<int>( mesh->datasetGroupsCount() ) )
      if ( metaDataIndex >= 0 && metaDataIndex < mesh->datasetgroup( groupIndex )->metadata().size() )
        return mesh->datasetgroup( groupIndex )->metadata().at( metaDataIndex ).first.c_str();
  }
  return "";
}

//! Returns the metadata value
const char *MDAL_DRIVER_G_metadataValue( int meshId, int groupIndex, int metaDataIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    if ( groupIndex >= 0 && groupIndex < static_cast<int>( mesh->datasetGroupsCount() ) )
      if ( metaDataIndex >= 0 && metaDataIndex < mesh->datasetgroup( groupIndex )->metadata().size() )
        return mesh->datasetgroup( groupIndex )->metadata().at( metaDataIndex ).second.c_str();
  }
  return "";
}

bool MDAL_DRIVER_G_datasetsDescription( int meshId, int groupIndex, bool *isScalar, int *dataLocation, int *datasetCount )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    DatasetGroup *dsg = mesh->datasetgroup( groupIndex );
    *isScalar = dsg->isScalar();
    *dataLocation = 2;
    *datasetCount = dsg->datasetCount();
    return true;
  }
  return false;
}

MDAL_LIB_EXPORT double MDAL_DRIVER_D_time( int meshId, int, int datasetIndex, bool *ok )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    return mesh->time( datasetIndex );
  }
  *ok = false;
  return 0;
}

int MDAL_DRIVER_D_data( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    DatasetGroup *dsg = mesh->datasetgroup( groupIndex );
    if ( !dsg )
      return 0;
    Dataset *ds = dsg->dataset( datasetIndex );
    if ( !ds )
      return 0;
    return ds->getData( indexStart, count, buffer );
  }

  return 0;
}


bool MDAL_DRIVER_D_hasActiveFlagCapability( int, int, int )
{
  return true;
}

//! Returns the dataset active flags
int MDAL_DRIVER_D_activeFlags( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    DatasetGroup *dsg = mesh->datasetgroup( groupIndex );
    if ( !dsg )
      return 0;
    Dataset *ds = dsg->dataset( datasetIndex );
    if ( !ds )
      return 0;
    return ds->getActive( indexStart, count, buffer );
  }

  return 0;
}

int MDAL_DRIVER_D_maximumVerticalLevelCount(int meshId, int groupIndex, int datasetIndex)
{
    if (sMeshes.find(meshId) != sMeshes.end())
    {
    }

    return -1;
}

int MDAL_DRIVER_D_volumeCount(int meshId, int groupIndex, int datasetIndex)
{
    if (sMeshes.find(meshId) != sMeshes.end())
    {
    }

    return -1;
}

int MDAL_DRIVER_D_verticalLevelCountData(int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int* buffer)
{
    if (sMeshes.find(meshId) != sMeshes.end())
    {
    }

    return -1;
}

int MDAL_DRIVER_D_verticalLevelData(int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double* buffer)
{
    if (sMeshes.find(meshId) != sMeshes.end())
    {
    }

    return -1;
}

int MDAL_DRIVER_D_faceToVolumeData(int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int* buffer)
{
    if (sMeshes.find(meshId) != sMeshes.end())
    {

    }

    return -1;
}

MDAL_LIB_EXPORT void MDAL_DRIVER_D_unload( int meshId, int groupIndex, int datasetIndex )
{
  if ( sMeshes.find( meshId ) != sMeshes.end() )
  {
    Mesh *mesh = sMeshes[meshId].get();
    DatasetGroup *dsg = mesh->datasetgroup( groupIndex );
    if ( !dsg )
      return;
    Dataset *ds = dsg->dataset( datasetIndex );
    if ( ds )
      return ds->unload();
  }
}

#ifdef __cplusplus
}//////////////////////////
#endif
