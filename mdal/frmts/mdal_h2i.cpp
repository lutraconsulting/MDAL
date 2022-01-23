/***************************************************************************
  mdal_h2i.cpp - mdal_h2i

 ---------------------
 begin                : 21.1.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mdal_h2i.hpp"

#include <iostream>
#include <nlohmann_json/json.hpp>

#include "mdal_utils.hpp"
#include "mdal_logger.hpp"

#include "mdal_memory_data_model.hpp"


#define DRIVER_NAME "H2i"

MDAL::DriverH2i::DriverH2i():
  Driver( DRIVER_NAME,
          "H2i Mesh File",
          "*.json",
          Capability::ReadMesh )
{}

MDAL::DriverH2i *MDAL::DriverH2i::create()
{
  return new DriverH2i();
}

bool MDAL::DriverH2i::canReadMesh( const std::string &uri )
{
  MetadataH2i metadata;
  if ( !parseJsonFile( uri, metadata ) )
    return false;

  const std::string nodesFilePath = metadata.dirPath + '/' + metadata.nodesFile;
  if ( !MDAL::fileExists( nodesFilePath ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), nodesFilePath + " could not be opened" );
    return false;
  }

  const std::string linksFilePath =  metadata.dirPath + '/' + metadata.linksFile;
  if ( !MDAL::fileExists( linksFilePath ) )
  {
    MDAL::Log::error( MDAL_Status::Err_FileNotFound, name(), linksFilePath + " could not be opened" );
    return false;
  }

  return true;
}

std::string MDAL::DriverH2i::buildUri( const std::string &meshFile )
{
  MetadataH2i metadata;

  if ( !parseJsonFile( meshFile, metadata ) )
    return std::string();

  return MDAL::buildMeshUri( meshFile, metadata.meshName, name() );
}

bool MDAL::DriverH2i::parseJsonFile( const std::string filePath, MetadataH2i &metadata )
{
  using Json = nlohmann::json;

  std::ifstream file = MDAL::openInputFile( filePath );
  if ( !file.is_open() )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), filePath + " could not be opened" );
    return false;
  }

  std::stringstream stream;
  stream << file.rdbuf();
  std::string jsonString = stream.str();

  try
  {
    Json jsonFile;
    jsonFile = Json::parse( jsonString );

    metadata.meshName = jsonFile["name"].get<std::string>();
    metadata.crs = jsonFile["crs"].get<std::string>();
    metadata.nodesFile = jsonFile["topology"]["2d_nodes_file"].get<std::string>();
    metadata.linksFile = jsonFile["topology"]["2d_links_file"].get<std::string>();
    metadata.referenceTime = jsonFile["timesteps"]["start_datetime"];
    metadata.timeStepFile = jsonFile["timesteps"]["timesteps_file"];

    metadata.metadataFilePath = filePath;
    metadata.dirPath = MDAL::dirName( filePath );

  }
  catch ( Json::exception e )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), filePath + ": " + e.what() );
    return false;
  }

  return true;
}

struct VertexFactory
{

  VertexFactory( std::vector<MDAL::Vertex> &vertices ):
    verticesRef( vertices )
  {}

  //! Returns vertex index
  size_t getVertex( double xCenter, double yCenter, int posInCell, bool onMidSide, double cellSize )
  {
    double xVertex, yVertex;
    switch ( posInCell )
    {
      case 0:
        xVertex = xCenter - ( onMidSide ? 0 : cellSize / 2 );
        yVertex = yCenter - cellSize / 2;
        break;
      case 1:
        xVertex = xCenter + cellSize / 2;
        yVertex = yCenter - ( onMidSide ? 0 : cellSize / 2 );
        break;
      case 2:
        xVertex = xCenter + ( onMidSide ? 0 : cellSize / 2 );
        yVertex = yCenter + cellSize / 2;
        break;
      case 3:
        xVertex = xCenter - cellSize / 2;
        yVertex = yCenter + ( onMidSide ? 0 : cellSize / 2 );
        break;
      default:
        xVertex = std::numeric_limits<double>::quiet_NaN();
        yVertex = std::numeric_limits<double>::quiet_NaN();
        break;
    }

    if ( std::isnan( xVertex ) || std::isnan( yVertex ) )
      return -1;

    int xPos = int( ( xVertex - xMin ) * xIntervalsCount / totalWidth + 0.5 );
    int yPos = int( ( yVertex - yMin ) * yIntervalsCount / totalHeight + 0.5 );

    auto it = createdVertexPosition.find( {xPos, yPos} );
    if ( it == createdVertexPosition.end() )
    {
      int vertIndex;
      vertIndex = verticesRef.size();
      verticesRef.push_back( {xVertex, yVertex, 0} );
      createdVertexPosition[ {xPos, yPos}] = vertIndex;

      return vertIndex;
    }
    else
    {
      return it->second;
    }
  }

  void setUp( double xmin,
              double xmax,
              double ymin,
              double ymax,
              double minCellSize,
              double maxCellSize )
  {

    xMin = xmin - maxCellSize / 2;
    yMin = ymin - maxCellSize / 2;
    totalWidth = xmax - xmin + maxCellSize;
    totalHeight = ymax - ymin + maxCellSize;

    xIntervalsCount = int ( totalWidth / minCellSize + 0.5 );
    yIntervalsCount = int ( totalHeight / minCellSize + 0.5 );
  }

  std::vector<MDAL::Vertex> &verticesRef;
  std::map<std::pair<int, int>, size_t> createdVertexPosition;

  double xMin;
  double yMin;
  double totalWidth;
  double totalHeight;

  int xIntervalsCount;
  int yIntervalsCount;
};

std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::createMeshFrame( const MDAL::DriverH2i::MetadataH2i &metadata )
{
  double minSize = 0;
  double maxSize = 0;
  double xMin = 0;
  double xMax = 0;
  double yMin = 0;
  double yMax = 0;

  std::vector<CellH2i> cells;

  parseNodeFile( cells, metadata, minSize, maxSize, xMin, xMax, yMin, yMax );
  parseLinkFile( cells, metadata );

  std::vector<Vertex>vertices;
  VertexFactory vertexFactory( vertices );
  vertexFactory.setUp( xMin, xMax, yMin, yMax, minSize, maxSize );

  std::vector<Face> faces( cells.size() );

  std::map<std::pair<int, int>, int> gridPositionToVertex;

  size_t maxVerticesCount = 0;
  for ( size_t ci = 0; ci < cells.size(); ++ci )
  {
    const CellH2i &cell = cells.at( ci );
    Face &face = faces[ci];
    for ( size_t side = 0; side < 4; ++side )
    {
      face.push_back( vertexFactory.getVertex( cell.x, cell.y, side, false, cell.size ) );

      if ( cell.neighborsCellCountperSide.at( side ) == 2 )
        face.push_back( vertexFactory.getVertex( cell.x, cell.y, side, true, cell.size ) );
    }

    if ( face.size() > maxVerticesCount )
      maxVerticesCount = face.size();
  }

  std::unique_ptr<MDAL::MemoryMesh> mesh( new MemoryMesh( name(), toInt( maxVerticesCount ), metadata.metadataFilePath ) );

  mesh->setVertices( std::move( vertices ) );
  mesh->setFaces( std::move( faces ) );
  mesh->setSourceCrs( metadata.crs );

  return mesh;
}


std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::load( const std::string &meshFile, const std::string & )
{
  MetadataH2i metadata;
  if ( !parseJsonFile( meshFile, metadata ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), meshFile + " could not be opened" );
    return nullptr;
  }

  std::unique_ptr<Mesh> mesh = createMeshFrame( metadata );

  DateTime referenceTime;
  std::vector<RelativeTimestamp> timeSteps;
  parseTime( metadata, referenceTime, timeSteps );

  return mesh;
}

void MDAL::DriverH2i::parseNodeFile( std::vector<MDAL::DriverH2i::CellH2i> &cells,
                                     const MDAL::DriverH2i::MetadataH2i &meta,
                                     double &minSize,
                                     double &maxSize,
                                     double &xMin,
                                     double &xMax,
                                     double &yMin,
                                     double &yMax ) const
{
  const std::string filePath = meta.dirPath + '/' + meta.nodesFile;
  std::ifstream nodeFile = MDAL::openInputFile( filePath );

  if ( !nodeFile.is_open() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + filePath );

  minSize = std::numeric_limits<double>::max();
  maxSize = 0;
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();;


  std::string line;
  while ( std::getline( nodeFile, line ) )
  {
    const std::vector<std::string> lineElements = split( line, ' ' );
    if ( lineElements.size() != 7 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + filePath );

    CellH2i cell;
    cell.x = toDouble( lineElements.at( 1 ) );
    cell.y = toDouble( lineElements.at( 2 ) );
    cell.size = toDouble( lineElements.at( 4 ) );
    cell.zmin = toDouble( lineElements.at( 5 ) );
    cell.zmax = toDouble( lineElements.at( 6 ) );

    cell.neighborsCellCountperSide = std::vector<int>( 4, 0 );

    if ( cell.size < minSize )
      minSize = cell.size;

    if ( cell.size > maxSize )
      maxSize = cell.size;

    if ( cell.x < xMin )
      xMin = cell.x;

    if ( cell.x > xMax )
      xMax = cell.x;

    if ( cell.y < yMin )
      yMin = cell.y;

    if ( cell.y > yMax )
      yMax = cell.y;

    cells.push_back( cell );
  }
}

void MDAL::DriverH2i::parseLinkFile( std::vector<MDAL::DriverH2i::CellH2i> &cells, const MDAL::DriverH2i::MetadataH2i &metadata ) const
{
  const std::string filePath = metadata.dirPath + '/' + metadata.linksFile;
  std::ifstream nodeFile = MDAL::openInputFile( filePath );

  if ( !nodeFile.is_open() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + filePath );

  std::string line;
  while ( std::getline( nodeFile, line ) )
  {
    const std::vector<std::string> lineElements = split( line, ' ' );
    if ( lineElements.size() != 10 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + filePath );

    int ori = toInt( lineElements.at( 1 ) );
    int cellFromIndex = toInt( lineElements.at( 2 ) ) - 1;
    int cellToIndex = toInt( lineElements.at( 3 ) ) - 1;
    int dir = toDouble( lineElements.at( 4 + ori ) ) > toDouble( lineElements.at( 6 + ori ) ) ? 1 : -1;

    CellH2i &cellFrom = cells.at( cellFromIndex );
    CellH2i &cellTo = cells.at( cellToIndex );

    size_t posInCellFrom = static_cast<int>( ( ori + dir + 4 ) % 4 );
    size_t posInCellTo = static_cast<int>( ( ori - dir + 4 ) % 4 );

    if ( posInCellFrom > 3 || posInCellTo > 3 ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Data invalid: " + filePath );

    cellFrom.neighborsCellCountperSide[posInCellFrom]++;
    cellTo.neighborsCellCountperSide[posInCellTo]++;
  }
}

void MDAL::DriverH2i::parseTime( const MDAL::DriverH2i::MetadataH2i &metadata, MDAL::DateTime &referenceTime, std::vector<MDAL::RelativeTimestamp> &timeSteps )
{
  referenceTime = DateTime( metadata.referenceTime );

  const std::string timeFilePath = metadata.dirPath + '/' + metadata.timeStepFile;
  std::ifstream timeFile = MDAL::openInputFile( timeFilePath );

  if ( !timeFile.is_open() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + timeFilePath );

  timeSteps.clear();
  std::string line;
  while ( std::getline( timeFile, line ) )
  {
    const std::vector<std::string> lineElements = split( line, ' ' );
    if ( lineElements.size() != 2 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + timeFilePath );

    timeSteps.emplace_back( toDouble( lineElements.at( 1 ) ), RelativeTimestamp::seconds );
  }
}
