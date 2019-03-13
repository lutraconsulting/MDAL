/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Christophe Coulet - Arteliagroup
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
  this->Reader->GetTitleFormat( titleFormat);
  if ( !startsWith( titleFormat, "SERAFIN" ) && !startsWith( titleFormat, "SERAFIND" ) )
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
  int Ndp = this->Reader->GetNodeByElements();

  // Allocate memory
  std::vector<Vertex> vertices( Nvertex );
  std::vector<Face> faces( Nfaces );

  float* XValues = new float[Nvertex];
  float* YValues = new float[Nvertex];
  //float* ZValues = new float[Nvertex];
  
  this->Reader->GetXValues(0, Nvertex, XValues);
  this->Reader->GetYValues(0, Nvertex, YValues);
  //this->Reader->GetZValues(0, Nvertex, ZValues, 0);
  
  for ( size_t vertexIndex = 0; vertexIndex < Nvertex; vertexIndex++)
  {
    Vertex &vertex = vertices[vertexIndex];
    vertex.x = double( XValues[vertexIndex] );
    vertex.y = double( YValues[vertexIndex] );
    vertex.z = 0; //double( ZValues[vertexIndex] );
  } 
 
  // Allocate memory
  int* Ikle = new int[Nfaces * Ndp];
  
  this->Reader->WriteConnectivity(Ikle);
  
  for ( size_t faceIndex = 0; faceIndex < Nfaces; faceIndex++)
  {
    Face &face = faces[faceIndex];
    face.resize( 3 );
    face[0] = Ikle[(faceIndex)*3 + 0];
    face[1] = Ikle[(faceIndex)*3 + 1];
    face[2] = Ikle[(faceIndex)*3 + 2];
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
