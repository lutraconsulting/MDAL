/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Runette Software Ltd.
*/

#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <limits>
#include <algorithm>
#include <string.h>

#include "mdal_ply.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_logger.hpp"
#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "libplyxx/libplyxx.h"

#define DRIVER_NAME "PLY"

MDAL::DriverPly::DriverPly() :
  Driver( DRIVER_NAME,
          "Stanford PLY Ascii Mesh File",
          "*.ply",
          Capability::ReadMesh |
          Capability::SaveMesh |
          Capability::WriteDatasetsOnVertices |
          Capability::WriteDatasetsOnFaces |
          Capability::WriteDatasetsOnEdges
        )
{
}

MDAL::DriverPly *MDAL::DriverPly::create()
{
  return new DriverPly();
}

MDAL::DriverPly::~DriverPly() = default;

size_t getIndex( std::vector<std::string> v, std::string in )
{
  std::vector<std::string>::iterator it = std::find( v.begin(), v.end(), in );
  return ( size_t )std::distance( v.begin(), it );
}

// check for the magic number which in  a PLY file is "ply"
bool MDAL::DriverPly::canReadMesh( const std::string &uri )
{
  std::ifstream in( uri, std::ifstream::in );
  std::string line;
  if ( !MDAL::getHeaderLine( in, line ) || !startsWith( line, "ply" ) )
  {
    return false;
  }
  return true;
}

std::string MDAL::DriverPly::saveMeshOnFileSuffix() const
{
  return "ply";
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverPly::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();
  Vertices vertices( 0 );
  Faces faces( 0 );
  Edges edges( 0 );
  size_t maxSizeFace = 0;

  size_t vertexCount = 0;
  size_t faceCount = 0;
  size_t edgeCount = 0;

  //datastructures that will contain all of the datasets, categorised by vertex, face and edge datasets
  std::vector<std::vector<double>> vertexDatasets; // conatains the data
  std::vector<std::string> vProp2Ds; // contains the dataset name
  std::vector<std::vector<double>> faceDatasets;
  std::vector<std::string> fProp2Ds;
  std::vector<std::vector<double>> edgeDatasets;
  std::vector<std::string> eProp2Ds;

  libply::File file( meshFile );
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) { return nullptr; }
  const libply::ElementsDefinition &definitions = file.definitions();
  const libply::Metadata &metadata = file.metadata();
  for ( const libply::Element &element : definitions )
  {
    if ( element.name == "vertex" )
    {
      vertexCount = element.size;
    }
    else if ( element.name == "face" )
    {
      faceCount = element.size;
    }
    else if ( element.name == "edge" )
    {
      edgeCount = element.size;
    }
    for ( const libply::Property &property : element.properties )
    {
      if ( element.name == "vertex" &&
           property.name != "X" &&
           property.name != "x" &&
           property.name != "Y" &&
           property.name != "y" &&
           property.name != "Z" &&
           property.name != "z"
         )
      {
        vProp2Ds.push_back( property.name );
        vertexDatasets.push_back( * new std::vector<double> );
      }
      else if ( element.name == "face" &&
                property.name != "vertex_indices"
              )
      {
        fProp2Ds.push_back( property.name );
        faceDatasets.push_back( * new std::vector<double> );
      }
      else if ( element.name == "edge" &&
                property.name != "vertex1" &&
                property.name != "vertex2"
              )
      {
        eProp2Ds.push_back( property.name );
        edgeDatasets.push_back( * new std::vector<double> );
      }
    }
  }

  for ( const libply::Element &el : definitions )
  {
    if ( el.name == "vertex" )
    {
      libply::ElementReadCallback vertexCallback = [&vertices, &el, &vProp2Ds, &vertexDatasets]( libply::ElementBuffer & e )
      {
        Vertex vertex;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "X" || p.name == "x" )
          {
            vertex.x = e[i];
          }
          else if ( p.name == "Y" || p.name == "y" )
          {
            vertex.y = e[i];
          }
          else if ( p.name == "Z" || p.name == "z" )
          {
            vertex.z = e[i];
          }
          else
          {
            int dsIdx = getIndex( vProp2Ds, p.name );
            std::vector<double> *ds = & vertexDatasets[dsIdx];
            ds->push_back( e[i] );
          }
        }
        vertices.push_back( vertex );
      };
      file.setElementReadCallback( "vertex", vertexCallback );
    }
    else if ( el.name == "face" )
    {
      libply::ElementReadCallback faceCallback = [&faces, &el, &maxSizeFace, &fProp2Ds, &faceDatasets]( libply::ElementBuffer & e )
      {
        Face face;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex_indices" )
          {
            if ( !p.isList )
            {
              MDAL::Log::error( MDAL_Status::Err_InvalidData, "PLY: triangles are not lists" );
            }
            else
            {
              libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
              if ( maxSizeFace < lp->size() ) maxSizeFace = lp->size();
              face.resize( lp->size() );
              for ( size_t j = 0; j < lp->size(); j++ )
              {
                face[j] = int( lp->value( j ) );
              }
            }
          }
          else
          {
            int dsIdx = getIndex( fProp2Ds, p.name );
            std::vector<double> *ds = & faceDatasets[dsIdx];
            ds->push_back( e[i] );
          }
        }
        faces.push_back( face );
      };
      file.setElementReadCallback( "face", faceCallback );
    }
    else if ( el.name == "edge" )
    {
      libply::ElementReadCallback edgeCallback = [&edges, &el, &eProp2Ds, &edgeDatasets]( libply::ElementBuffer & e )
      {
        Edge edge;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex1" )
          {
            edge.startVertex = int( e[i] );
          }
          else if ( p.name == "vertex2" )
          {
            edge.endVertex = int( e[i] );
          }
          else
          {
            int dsIdx = getIndex( eProp2Ds, p.name );
            std::vector<double> *ds = & edgeDatasets[dsIdx];
            ds->push_back( e[i] );
          }
        }
        edges.push_back( edge );
      };
      file.setElementReadCallback( "edge", edgeCallback );
    }
  }

  file.read();
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) { return nullptr; }
  if ( vertices.size() != vertexCount ||
       faces.size() != faceCount ||
       edges.size() != edgeCount
     )
  {
    MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_InvalidData, "Incomplete Mesh" );
    return nullptr;
  }

  std::unique_ptr< MemoryMesh > mesh(
    new MemoryMesh(
      DRIVER_NAME,
      maxSizeFace,
      meshFile
    )
  );
  mesh->setFaces( std::move( faces ) );
  mesh->setVertices( std::move( vertices ) );
  mesh->setEdges( std::move( edges ) );
  if ( metadata.find( "crs" ) != metadata.end() )
  {
    mesh->setSourceCrs( metadata.at( "crs" ) );
  }


  // Add Bed Elevation
  MDAL::addBedElevationDatasetGroup( mesh.get(), mesh->vertices() );

  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), vProp2Ds[i], DataOnVertices, true );
    addDataset2D( group.get(), vertexDatasets[i] );
  }

  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), fProp2Ds[i], DataOnFaces, true );
    addDataset2D( group.get(), faceDatasets[i] );
  }

  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    std::shared_ptr<DatasetGroup> group = addDatasetGroup( mesh.get(), eProp2Ds[i], DataOnEdges, true );
    addDataset2D( group.get(), edgeDatasets[i] );
  }

  /*
  * Clean up
  */
  for ( size_t i = 0; i < vertexDatasets.size(); ++i )
  {
    vertexDatasets.pop_back();
  };

  for ( size_t i = 0; i < faceDatasets.size(); ++i )
  {
    faceDatasets.pop_back();
  };

  for ( size_t i = 0; i < edgeDatasets.size(); ++i )
  {
    edgeDatasets.pop_back();
  };

  return std::unique_ptr<Mesh>( mesh.release() );

}

std::shared_ptr< MDAL::DatasetGroup> MDAL::DriverPly::addDatasetGroup( MDAL::Mesh *mesh, const std::string &name, const MDAL_DataLocation location, bool isScalar )
{
  if ( !mesh )
    return NULL;

  if ( location == DataOnFaces && mesh->facesCount() == 0 )
    return NULL;

  if ( location == DataOnEdges && mesh->edgesCount() == 0 )
    return NULL;

  std::shared_ptr< DatasetGroup > group = std::make_shared< DatasetGroup >( mesh->driverName(), mesh, name, name );
  group->setDataLocation( location );
  group->setIsScalar( isScalar );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mesh->datasetGroups.push_back( group );
  return group;
}

void MDAL::DriverPly::addDataset2D( MDAL::DatasetGroup *group, const std::vector<double> &values )
{
  if ( !group )
    return;

  MDAL::Mesh *mesh = group->mesh();

  if ( values.empty() )
    return;

  if ( 0 == mesh->verticesCount() )
    return;

  if ( group->dataLocation() == DataOnVertices )
  {
    assert( values.size() == mesh->verticesCount() );
  }

  if ( group->dataLocation() == DataOnFaces )
  {
    assert( values.size() == mesh->facesCount() );
    if ( mesh->facesCount() == 0 )
      return;
  }

  if ( group->dataLocation() == DataOnEdges )
  {
    assert( values.size() == mesh->edgesCount() );
    if ( mesh->edgesCount() == 0 )
      return;
  }

  std::shared_ptr< MDAL::MemoryDataset2D > dataset = std::make_shared< MemoryDataset2D >( group );
  dataset->setTime( 0.0 );
  memcpy( dataset->values(), values.data(), sizeof( double ) * values.size() );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
}

void MDAL::DriverPly::save( const std::string &uri, MDAL::Mesh *mesh )
{
  MDAL::Log::resetLastStatus();

  DatasetGroups groups = mesh->datasetGroups;

  // vectors to hold the different types of group
  DatasetGroups vgroups;
  DatasetGroups fgroups;
  DatasetGroups egroups;

  for ( std::shared_ptr<DatasetGroup> group : groups )
  {
    if ( group->dataLocation() == MDAL_DataLocation::DataOnVertices )
    {
      vgroups.push_back( group );
    }
    else if ( group->dataLocation() == MDAL_DataLocation::DataOnFaces )
    {
      fgroups.push_back( group );
    }
    else if ( group->dataLocation() == MDAL_DataLocation::DataOnEdges )
    {
      egroups.push_back( group );
    }
  }


  libply::FileOut file( uri, libply::File::Format::ASCII );
  if ( MDAL::Log::getLastStatus() != MDAL_Status::None ) return;

  libply::ElementsDefinition definitions;
  std::vector<libply::Property> vproperties;
  vproperties.emplace_back( "X", libply::Type::FLOAT64, false );
  vproperties.emplace_back( "Y", libply::Type::FLOAT64, false );
  vproperties.emplace_back( "Z", libply::Type::FLOAT64, false );
  for ( std::shared_ptr<DatasetGroup> group : vgroups )
  {
    vproperties.emplace_back( group->name(), libply::Type::FLOAT64, false );
  }
  definitions.emplace_back( "vertex", mesh->verticesCount(), vproperties );
  if ( mesh->facesCount() > 0 )
  {
    std::vector<libply::Property> fproperties;
    fproperties.emplace_back( "vertex_indices", libply::Type::UINT32, true );
    for ( std::shared_ptr<DatasetGroup> group : fgroups )
    {
      vproperties.emplace_back( group->name(), libply::Type::FLOAT64, false );
    }
    definitions.emplace_back( "face", mesh->facesCount(), fproperties );
  }
  if ( mesh->edgesCount() > 0 )
  {
    std::vector<libply::Property> eproperties;
    eproperties.emplace_back( "vertex1", libply::Type::UINT32, false );
    eproperties.emplace_back( "vertex2", libply::Type::UINT32, false );
    for ( std::shared_ptr<DatasetGroup> group : egroups )
    {
      vproperties.emplace_back( group->name(), libply::Type::FLOAT64, false );
    }
    definitions.emplace_back( "edge", mesh->edgesCount(), eproperties );
  }

  file.setElementsDefinition( definitions );

  // write vertices
  std::unique_ptr<MDAL::MeshVertexIterator> vertices = mesh->readVertices();


  libply::ElementWriteCallback vertexCallback = [&vertices]( libply::ElementBuffer & e, size_t index )
  {
    double vertex[3];
    vertices->next( 1, vertex );
    e[0] = vertex[0];
    e[1] = vertex[1];
    e[2] = vertex[2];
  };

  // write faces
  std::vector<int> vertexIndices( mesh->faceVerticesMaximumCount() );
  std::unique_ptr<MDAL::MeshFaceIterator> faces = mesh->readFaces();

  libply::ElementWriteCallback faceCallback = [&faces, &vertexIndices]( libply::ElementBuffer & e, size_t index )
  {
    int faceOffsets[1];
    faces->next( 1, faceOffsets, vertexIndices.size(), vertexIndices.data() );
    libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[0] );
    lp->define( libply::Type::UINT32, faceOffsets[0] );
    for ( int j = 0; j < faceOffsets[0]; ++j )
    {
      lp->value( j ) = vertexIndices[j];
    };
  };


  // write edges

  std::unique_ptr<MDAL::MeshEdgeIterator> edges = mesh->readEdges();

  libply::ElementWriteCallback edgeCallback = [&edges]( libply::ElementBuffer & e, size_t index )
  {
    int startIndex;
    int endIndex;
    edges->next( 1, &startIndex, &endIndex );
    e[0] = startIndex;
    e[1] = endIndex;
  };

  file.setElementWriteCallback( "vertex", vertexCallback );
  if ( mesh->facesCount() > 0 ) file.setElementWriteCallback( "face", faceCallback );
  if ( mesh->edgesCount() > 0 ) file.setElementWriteCallback( "edge", edgeCallback );
  file.write();
}
