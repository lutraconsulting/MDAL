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

std::unique_ptr<MDAL::Mesh> MDAL::DriverPly::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();
  Vertices vertices( 0 );
  Faces faces( 0 );
  Edges edges( 0 );
  size_t maxSizeFace = 0;

  //datastructures that will contain all of the datasets, categorised by vertex, face and edge datasets
  std::vector<std::vector<double>> vertexDatasets; // conatains the data
  std::vector<std::string> vProp2Ds; // contains the dataset name
  std::vector<std::vector<double>> faceDatasets;
  std::vector<std::string> fProp2Ds;
  std::vector<std::vector<double>> edgeDatasets;
  std::vector<std::string> eProp2Ds;

  libply::File file( meshFile );
  const libply::ElementsDefinition &definitions = file.definitions();
  const libply::Metadata &metadata = file.metadata();
  for ( const libply::Element &element : definitions )
  {
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
              // TODO raise error
            }
            libply::ListProperty *lp = dynamic_cast<libply::ListProperty *>( &e[i] );
            if ( maxSizeFace < lp->size() ) maxSizeFace = lp->size();
            face.resize( lp->size() );
            for ( size_t j = 0; j < lp->size(); j++ )
            {
              face[j] = int( lp->value( j ) );
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
