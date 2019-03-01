/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Ltd.
 Christophe Coulet - Arteliagroup
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>

#include "mdal_slf.hpp"
#include "SerafinReader.h"
#include "mdal.h"
#include "mdal_utils.hpp"

#define DRIVER_NAME "SLF"

MDAL::MeshSlf::MeshSlf( size_t verticesCount,
                        size_t facesCount,
                        size_t faceVerticesMaximumCount,
                        MDAL::BBox extent,
                        const std::string &uri)
  : MemoryMesh( DRIVER_NAME,
                verticesCount,
                facesCount,
                faceVerticesMaximumCount,
                extent,
                uri )
{
}

MDAL::MeshSlf::~MeshSlf() = default;

MDAL::DriverSlf::DriverSlf():
  Driver( DRIVER_NAME,
          "Selafin File",
          "*.slf",
          Capability::ReadMesh
        )
{
}

MDAL::DriverSlf *MDAL::DriverSlf::create()
{
  return new DriverSlf();
}

MDAL::DriverSlf::~DriverSlf() = default;

bool MDAL::DriverSlf::canRead( const std::string &uri )
{
  this->FileStream = new std::ifstream(uri, std::ifstream::in | std::ifstream::binary );
  this->Reader = new SerafinReader( FileStream);
  this->Reader->GetTitle( title);
  if ( !endsWith( title, "SELAFIN " ) || !endsWith( title, "SELAFIND" ) )
  {
    return false;
  }
  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverSlf::load( const std::string &meshFile, MDAL_Status *status )
{
  mMeshFile = meshFile;

  if ( status ) *status = MDAL_Status::None;

  this->FileStream = new std::ifstream(mMeshFile, std::ifstream::in | std::ifstream::binary );
  this->Reader = new SerafinReader( FileStream);
  
  int Nvertex = this->Reader->GetNumberOfNodes();
  int Nfaces = this->Reader->GetNumberOfElement();

  // Allocate memory
  std::vector<Vertex> vertices( Nvertex );
  std::vector<Face> faces( Nfaces );

  float* XValues = NULL;
  float* YValues = NULL;
  float* ZValues = NULL;
  
  this->Reader->GetXValues(0, Nvertex, XValues);
  this->Reader->GetYValues(0, Nvertex, YValues);
  this->Reader->GetZValues(0, Nvertex, ZValues, 0);
  
  for ( size_t vertexIndex = 1; Nvertex; vertexIndex++)
  {
    Vertex &vertex = vertices[vertexIndex-1];
    vertex.x = double( XValues[vertexIndex] );
    vertex.y = double( YValues[vertexIndex] );
    vertex.z = double( ZValues[vertexIndex] );
  }
  
  int* Ikle = NULL;
  this->Reader->WriteConnectivity(Ikle);
  
  for ( size_t faceIndex = 1; Nfaces; faceIndex++)
  {
    Face &face = faces[faceIndex-1];
    face.resize( 3 );
    face[0] = Ikle[(faceIndex-1)*3 + 0];
    face[1] = Ikle[(faceIndex-1)*3 + 1];
    face[2] = Ikle[(faceIndex-1)*3 + 2];
  }
  
  std::unique_ptr< MeshSlf > mesh(
    new MeshSlf(
      vertices.size(),
      faces.size(),
      3, //Triangles
      computeExtent( vertices ),
      mMeshFile
    )
  );
  mesh->faces = faces;
  mesh->vertices = vertices;
  MDAL::addBedElevationDatasetGroup( mesh.get(), vertices );
  return std::unique_ptr<Mesh>( mesh.release() );
}
