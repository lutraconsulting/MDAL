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
          Capability::ReadMesh
        )
{
}

MDAL::DriverPly *MDAL::DriverPly::create()
{
  return new DriverPly();
}

MDAL::DriverPly::~DriverPly() = default;

size_t MDAL::DriverPly::getIndex( std::vector<std::string> v, std::string in )
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

std::unique_ptr<MDAL::Mesh> MDAL::DriverPly::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();

  libply::File file( meshFile );
  const libply::ElementsDefinition &definitions = file.definitions();
  Vertices vertices( 0 );
  Faces faces( 0 );
  Edges edges( 0 );
  size_t maxSizeFace = 0;

  //datastructures that will contain all of the datasets, categorised by vertex, face and edge datasets
  std::vector<std::vector<double>> vertexDatasets; // conatains the data
  std::vector<std::string> vProp2Ds; // contains the dataset names
  std::vector<std::vector<double>> faceDatasets;
  std::vector<std::string> fProp2Ds;
  std::vector<std::vector<double>> edgeDatasets;
  std::vector<std::string> eProp2Ds;

  for ( const libply::Element &el : definitions )
  {
    if ( el.name == "vertex" )
    {
      libply::ElementReadCallback vertexCallback = [&vertices, el]( libply::ElementBuffer & e )
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
            // TODO raise error
          }
        }
        vertices.push_back( vertex );
      };
      file.setElementReadCallback( "vertex", vertexCallback );
    }
    else if ( el.name == "face" )
    {
      libply::ElementReadCallback faceCallback = [&faces, el, &maxSizeFace]( libply::ElementBuffer & e )
      {
        Face face;
        maxSizeFace = 3;
        face.resize( 3 );
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex_indices" )
          {
            if ( !p.isList )
            {
              // TODO raise error
            }
            for ( size_t j = 0; j < maxSizeFace; j++ )
            {
                //std::cout << std::to_string( int( e[j] ) ) << std::endl;
                face[j] = int( e[j] );
            }
          }
        }
        faces.push_back( face );
      };
      file.setElementReadCallback( "face", faceCallback );
    } else if ( el.name == "edge" )
    {
      libply::ElementReadCallback edgeCallback = [&edges, el]( libply::ElementBuffer & e )
      {
        Edge edge;
        for ( size_t i = 0; i < el.properties.size(); i++ )
        {
          libply::Property p = el.properties[i];
          if ( p.name == "vertex1" )
          {
            edge.startVertex = int( e[i] );
          } else if ( p.name == "vertex2" )
          {
            edge.endVertex = int( e[i] );
          }
        }
        edges.push_back( edge );
      };
      file.setElementReadCallback( "edge", edgeCallback );
    }
  }

  file.read();

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
  //mesh->setSourceCrs( proj );

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
