#include <string>
#include <cassert>
#include <stddef.h>
#include <limits>

#include "mdal.h"
#include "mdal_loader.hpp"
#include "mdal_defines.hpp"

#define NODATA std::numeric_limits<double>::quiet_NaN()

static MDAL_Status sLastStatus;

const char *MDAL_Version()
{
  return "0.0.3";
}

MDAL_Status MDAL_LastStatus()
{
  return sLastStatus;
}

MeshH MDAL_LoadMesh( const char *meshFile )
{
  if ( !meshFile )
    return nullptr;

  std::string filename( meshFile );
  return static_cast< MeshH >( MDAL::Loader::load( filename, &sLastStatus ).release() );
}


void MDAL_CloseMesh( MeshH mesh )
{
  if ( mesh )
  {
    MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
    delete m;
  }
}


int MDAL_M_vertexCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->vertices.size() );
  return len;
}

double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->vertices.size() > i );
  return m->vertices[i].x;
}

double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->vertices.size() > i );
  return m->vertices[i].y;
}

int MDAL_M_faceCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->faces.size() );
  return len;
}

int MDAL_M_faceVerticesCountAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  assert( index > -1 );
  size_t i = static_cast<size_t>( index );
  assert( m->faces.size() > i );
  int len = static_cast<int>( m->faces[i].size() );
  return len;
}

int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  assert( face_index > -1 );
  size_t fi = static_cast<size_t>( face_index );
  assert( m->faces.size() > fi );
  assert( vertex_index > -1 );
  size_t vi = static_cast<size_t>( vertex_index );
  assert( m->faces[fi].size() > vi );
  int len = static_cast<int>( m->faces[fi][vi] );
  return len;
}

void MDAL_M_LoadDatasets( MeshH mesh, const char *datasetFile )
{
  if ( !datasetFile )
    return;

  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );

  std::string filename( datasetFile );
  MDAL::Loader::loadDatasets( m, datasetFile, &sLastStatus );
}

void MDAL_M_CloseDataset( DatasetH dataset )
{
  assert( dataset );

  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  d->free();
}

int MDAL_M_datasetCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasets.size() );
  return len;
}

DatasetH MDAL_M_dataset( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = static_cast< MDAL::Mesh * >( mesh );
  int len = static_cast<int>( m->datasets.size() );
  assert( len > index );
  size_t i = static_cast<size_t>( index );
  return static_cast< DatasetH >( m->datasets[i].get() );
}

bool MDAL_D_hasScalarData( DatasetH dataset )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isScalar;
}

bool MDAL_D_isOnVertices( DatasetH dataset )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isOnVertices;
}

int MDAL_D_metadataCount( DatasetH dataset )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->metadata.size() );
  return len;
}

const char *MDAL_D_metadataKey( DatasetH dataset, int index )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->metadata.size() );
  assert( len > index );
  size_t i = static_cast<size_t>( index );
  return d->metadata[i].first.c_str();
}

const char *MDAL_D_metadataValue( DatasetH dataset, int index )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->metadata.size() );
  assert( len > index );
  size_t i = static_cast<size_t>( index );
  return d->metadata[i].second.c_str();
}

int MDAL_D_valueCount( DatasetH dataset )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  return len;
}

double MDAL_D_value( DatasetH dataset, int valueIndex )
{
  return MDAL_D_valueX( dataset, valueIndex );
}

double MDAL_D_valueX( DatasetH dataset, int valueIndex )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  assert( len > valueIndex );
  size_t i = static_cast<size_t>( valueIndex );
  if ( d->values[i].noData )
  {
    return NODATA;
  }
  else
    return d->values[i].x;
}

double MDAL_D_valueY( DatasetH dataset, int valueIndex )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  int len = static_cast<int>( d->values.size() );
  assert( len > valueIndex );
  size_t i = static_cast<size_t>( valueIndex );
  if ( d->values[i].noData )
  {
    return NODATA;
  }
  else
    return d->values[i].y;
}

bool MDAL_D_isValid( DatasetH dataset )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  return d->isValid;
}

bool MDAL_D_active( DatasetH dataset, int faceIndex )
{
  assert( dataset );
  MDAL::Dataset *d = static_cast< MDAL::Dataset * >( dataset );
  size_t i = static_cast<size_t>( faceIndex );
  return d->isActive( i );
}
