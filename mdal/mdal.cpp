#include <string>
#include <cassert>

#include "mdal.h"
#include "mdal_loader.hpp"
#include "mdal_defines.hpp"

static Status sLastStatus;

const char *MDAL_Version()
{
#define xstr(s) str(s)
#define str(s) #s
  return xstr( MDAL_VERSION );
}

Status MDAL_LastStatus()
{
  return sLastStatus;
}

MeshH MDAL_LoadMesh( const char *meshFile )
{
  if ( !meshFile )
    return nullptr;

  std::string filename( meshFile );
  return ( MeshH ) MDAL::Loader::load( filename, &sLastStatus );
}


void MDAL_CloseMesh( MeshH mesh )
{
  if ( mesh )
  {
    MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
    delete m;
  }
}


int MDAL_M_vertexCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  return m->vertices.size();
}

double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( ( m->vertices.size() > index ) && index > -1 );
  return m->vertices[index].x;
}

double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( ( m->vertices.size() > index ) && index > -1 );
  return m->vertices[index].y;
}

int MDAL_M_faceCount( MeshH mesh )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  return m->faces.size();
}

int MDAL_M_faceVerticesCountAt( MeshH mesh, int index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( ( m->faces.size() > index ) && index > -1 );
  return m->faces[index].size();
}

int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index )
{
  assert( mesh );
  MDAL::Mesh *m = ( MDAL::Mesh * ) mesh;
  assert( ( m->faces.size() > face_index ) && face_index > -1 );
  assert( ( m->faces[face_index].size() > vertex_index ) && vertex_index > -1 );
  return m->faces[face_index][vertex_index];
}
