/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2022 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_h2i.hpp"

#include <fstream>
#include <nlohmann_json/json.hpp>

#include "mdal_utils.hpp"
#include "mdal_logger.hpp"
#include "mdal_memory_data_model.hpp"


#define DRIVER_NAME "H2I"

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
    return false;

  const std::string linksFilePath =  metadata.dirPath + '/' + metadata.linksFile;
  if ( !MDAL::fileExists( linksFilePath ) )
    return false;

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
    Json jsonMeta;
    jsonMeta = Json::parse( jsonString );

    metadata.meshName = jsonMeta["name"].get<std::string>();
    metadata.crs = jsonMeta["crs"].get<std::string>();
    metadata.nodesFile = jsonMeta["topology"]["2d_nodes_file"].get<std::string>();
    metadata.linksFile = jsonMeta["topology"]["2d_links_file"].get<std::string>();
    metadata.referenceTime = jsonMeta["timesteps"]["start_datetime"];
    metadata.timeStepFile = jsonMeta["timesteps"]["timesteps_file"];

    for ( Json::iterator it = jsonMeta["results"].begin(); it != jsonMeta["results"].end(); ++it )
    {
      MetadataH2iDataset metaDataset;
      metaDataset.layer = ( *it )["layer"];
      metaDataset.file = ( *it )["result_file"];
      metaDataset.type = ( *it )["type"];
      metaDataset.units = ( *it )["units"];
      metaDataset.topology_file = ( *it )["topology_file"];

      metadata.datasetGroups.push_back( metaDataset );
    }

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
  // this map stores created vertex indexes (size_t) considering the position
  // in a grid (int,int) that have a resolution of minimum quad width

  double xMin = std::numeric_limits<double>::max();
  double yMin = std::numeric_limits<double>::max();
  double totalWidth = 0;
  double totalHeight = 0;

  int xIntervalsCount = 0;
  int yIntervalsCount = 0;
};

std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::createMeshFrame( const MDAL::DriverH2i::MetadataH2i &metadata, std::vector<double> &topography, LinksH2i links )
{
  double minSize = 0;
  double maxSize = 0;
  double xMin = 0;
  double xMax = 0;
  double yMin = 0;
  double yMax = 0;

  std::vector<CellH2i> cells;

  parseNodeFile( cells, metadata, minSize, maxSize, xMin, xMax, yMin, yMax );
  parseLinkFile( cells, links, metadata );

  std::vector<Vertex>vertices;
  VertexFactory vertexFactory( vertices );
  vertexFactory.setUp( xMin, xMax, yMin, yMax, minSize, maxSize );

  std::vector<Face> faces( cells.size() );
  topography.resize( cells.size() );

  size_t maxVerticesCount = 0;
  for ( size_t ci = 0; ci < cells.size(); ++ci )
  {
    const CellH2i &cell = cells.at( ci );
    Face &face = faces[ci];
    for ( size_t side = 0; side < 4; ++side )
    {
      face.push_back( vertexFactory.getVertex( cell.x, cell.y, side, false, cell.size ) );

      if ( cell.neighborsCellCountPerSide.at( side ) == 2 ) //two quads neighbors, need another vertex on the mid of this side
        face.push_back( vertexFactory.getVertex( cell.x, cell.y, side, true, cell.size ) );
    }

    if ( face.size() > maxVerticesCount )
      maxVerticesCount = face.size();

    topography[ci] = cell.z;
  }

  std::unique_ptr<MDAL::MemoryMesh> mesh( new MemoryMesh( name(), toInt( maxVerticesCount ), metadata.metadataFilePath ) );

  mesh->setVertices( std::move( vertices ) );
  mesh->setFaces( std::move( faces ) );
  mesh->setSourceCrs( metadata.crs );

  return mesh;
}


std::unique_ptr<MDAL::Mesh> MDAL::DriverH2i::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();

  MetadataH2i metadata;
  if ( !parseJsonFile( meshFile, metadata ) )
  {
    MDAL::Log::error( MDAL_Status::Err_UnknownFormat, name(), meshFile + " could not be opened" );
    return nullptr;
  }

  std::vector<double> topography;
  LinksH2i links = std::make_shared<std::vector<LinkH2i>>();

  std::unique_ptr<Mesh> mesh = createMeshFrame( metadata, topography, links );

  // Create a topography dataset group (data on faces) from elevation in H2i nodes
  std::shared_ptr<DatasetGroup> topographyGroup = std::make_shared<DatasetGroup>( name(), mesh.get(), meshFile, "topography" );
  topographyGroup->setDataLocation( MDAL_DataLocation::DataOnFaces );

  std::shared_ptr<MemoryDataset2D> topographyDataset = std::make_shared<MemoryDataset2D>( topographyGroup.get() );
  memcpy( topographyDataset->values(), topography.data(), topography.size()*sizeof( double ) );
  topographyDataset->setStatistics( MDAL::calculateStatistics( topographyDataset ) );

  topographyGroup->datasets.push_back( topographyDataset );
  topographyGroup->setStatistics( MDAL::calculateStatistics( topographyGroup ) );
  mesh->datasetGroups.push_back( topographyGroup );

  // read time information
  DateTime referenceTime;
  std::vector<RelativeTimestamp> timeSteps;
  parseTime( metadata, referenceTime, timeSteps );

  // build the dataset groups for all temporal quantities.
  const std::vector<MetadataH2iDataset> &metaGroups = metadata.datasetGroups;
  for ( const MetadataH2iDataset &metadatasetGroup : metaGroups )
  {
    std::shared_ptr<DatasetGroup> group =
      std::make_shared<DatasetGroup>( name(), mesh.get(), metadatasetGroup.file, metadatasetGroup.layer );

    std::string datasetGroupFile = metadata.dirPath + '/' + metadatasetGroup.file;
    std::shared_ptr<std::ifstream> in = std::make_shared<std::ifstream>( datasetGroupFile, std::ifstream::binary );

    if ( !in->is_open() )
      continue;

    group->setReferenceTime( referenceTime );
    group->setDataLocation( MDAL_DataLocation::DataOnFaces );
    group->setMetadata( "units", metadatasetGroup.units );
    group->setMetadata( "type", metadatasetGroup.type );

    if ( metadatasetGroup.topology_file == "2d_nodes_file" )
    {
      for ( size_t datasetIndex = 0; datasetIndex < timeSteps.size(); ++datasetIndex )
      {
        std::shared_ptr<DatasetH2iOnNode> dataset = std::make_shared<DatasetH2iOnNode>( group.get(), in, datasetIndex );
        dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
        group->datasets.push_back( dataset );
        dataset->clear(); // Lazy loading, so we clear the loaded data during statistic calculation
        dataset->setTime( timeSteps.at( datasetIndex ) );
      }
    }
    else if ( metadatasetGroup.topology_file == "2d_links_file" )
    {
      group->setIsScalar( false );

      bool isFlux = metadatasetGroup.type == "discharge";
      if ( isFlux )
      {
        std::string fluxUnit = metadatasetGroup.units;
        if ( MDAL::contains( fluxUnit, "3/s" ) )
          fluxUnit = MDAL::replace( fluxUnit, "3/s", "2/s" );
        group->setMetadata( "units", "m2/s" );
        group->setName( "unit " + metadatasetGroup.layer );
      }

      for ( size_t datasetIndex = 0; datasetIndex < timeSteps.size(); ++datasetIndex )
      {
        std::shared_ptr<DatasetH2iOnLink> dataset = std::make_shared<DatasetH2iOnLink>( group.get(), in, links, datasetIndex, isFlux );
        dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
        group->datasets.push_back( dataset );
        dataset->clear();
        dataset->setTime( timeSteps.at( datasetIndex ) );
      }
    }
    else
      continue;

    group->setStatistics( MDAL::calculateStatistics( group ) );
    mesh->datasetGroups.push_back( group );
  }
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
    cell.z = ( toDouble( lineElements.at( 5 ) ) + toDouble( lineElements.at( 6 ) ) ) / 2;

    cell.neighborsCellCountPerSide = std::vector<int>( 4, 0 );

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

void MDAL::DriverH2i::parseLinkFile( std::vector<CellH2i> &cells, LinksH2i links, const MetadataH2i &meta ) const
{
  const std::string filePath = meta.dirPath + '/' + meta.linksFile;
  std::ifstream nodeFile = MDAL::openInputFile( filePath );

  if ( !nodeFile.is_open() ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Could not open file " + filePath );

  links->clear();
  std::string line;
  while ( std::getline( nodeFile, line ) )
  {
    const std::vector<std::string> lineElements = split( line, ' ' );
    if ( lineElements.size() != 10 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + filePath );

    int ori = toInt( lineElements.at( 1 ) );
    size_t cellFromIndex = toSizeT( lineElements.at( 2 ) ) - 1;
    size_t cellToIndex = toSizeT( lineElements.at( 3 ) ) - 1;
    int dir = toDouble( lineElements.at( 4 + ori ) ) > toDouble( lineElements.at( 6 + ori ) ) ? 1 : -1;

    CellH2i &cellFrom = cells.at( cellFromIndex );
    CellH2i &cellTo = cells.at( cellToIndex );

    size_t posInCellFrom = static_cast<int>( ( ori + dir + 4 ) % 4 );
    size_t posInCellTo = static_cast<int>( ( ori - dir + 4 ) % 4 );

    if ( posInCellFrom > 3 || posInCellTo > 3 ) throw MDAL::Error( MDAL_Status::Err_InvalidData, "Data invalid: " + filePath );

    cellFrom.neighborsCellCountPerSide[posInCellFrom]++;
    cellTo.neighborsCellCountPerSide[posInCellTo]++;

    links->push_back( {cellFromIndex,
                       cellToIndex,
                       static_cast<unsigned char>( ori ),
                       dir * std::min( 1 / cellFrom.size, 1.0 ),
                       dir * std::min( 1 / cellTo.size, 1.0 ),
                       std::min( cellFrom.size, cellTo.size )} );
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

MDAL::DatasetH2iOnNode::DatasetH2iOnNode( MDAL::DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex )
  : Dataset2D( grp )
  , mIn( in )
  , mDatasetIndex( datasetIndex )
{}

size_t MDAL::DatasetH2iOnNode::scalarData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataloaded )
    loadData();
  size_t nValues = valuesCount();

  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;

  size_t copyValues = std::min( nValues - indexStart, count );
  memcpy( buffer, mValues.data() + indexStart, copyValues * sizeof( double ) );
  return copyValues;
}

void MDAL::DatasetH2iOnNode::clear()
{
  mValues.clear();
  mDataloaded = false;
}

void MDAL::DatasetH2iOnNode::loadData()
{
  mIn->seekg( beginingInFile() );
  int datasetSize = 0;
  bool changeEndianness = false;
  MDAL::readValue( datasetSize, *mIn, changeEndianness );

  if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) )
  {
    changeEndianness = true;
    mIn->seekg( beginingInFile() );
    MDAL::readValue( datasetSize, *mIn, changeEndianness );
    if ( datasetSize != MDAL::toInt( valuesCount() *sizeof( double ) ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + group()->uri() );
  }

  mValues.resize( valuesCount() );
  for ( size_t i = 0; i < valuesCount(); ++i )
  {
    if ( !readValue( mValues[i], *mIn, changeEndianness ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error when reading file: " + group()->uri() );
  }
}

std::streampos MDAL::DatasetH2iOnNode::beginingInFile() const
{
  return ( sizeof( int ) * 2 + sizeof( double ) * valuesCount() ) * mDatasetIndex;
}

MDAL::DatasetH2iOnLink::DatasetH2iOnLink( MDAL::DatasetGroup *grp,
    std::shared_ptr<std::ifstream> in,
    LinksH2i links,
    size_t datasetIndex, bool isFlux )
  : Dataset2D( grp )
  , mIn( in )
  , mLinks( links )
  , mDatasetIndex( datasetIndex )
  , mIsFlux( isFlux )
{}

size_t MDAL::DatasetH2iOnLink::vectorData( size_t indexStart, size_t count, double *buffer )
{
  if ( !mDataloaded )
    loadData();
  size_t nValues = mValues.size() / 2;

  if ( ( count < 1 ) || ( indexStart  >= nValues ) )
    return 0;

  size_t copyValues = 2 * std::min( nValues - indexStart, count );
  memcpy( buffer, &mValues.data()[+indexStart * 2], copyValues * sizeof( double ) );
  return copyValues / 2;
}

void MDAL::DatasetH2iOnLink::clear()
{
  mValues.clear();
  mDataloaded = false;
}

void MDAL::DatasetH2iOnLink::loadData()
{
  mIn->seekg( beginingInFile() );
  int datasetSize = 0;
  bool changeEndianness = false;
  MDAL::readValue( datasetSize, *mIn, changeEndianness );

  if ( datasetSize != MDAL::toInt( mLinks->size() *sizeof( double ) ) )
  {
    changeEndianness = true;
    mIn->seekg( beginingInFile() );
    MDAL::readValue( datasetSize, *mIn, changeEndianness );
    if ( datasetSize != MDAL::toInt( mLinks->size() *sizeof( double ) ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format not recognized: " + group()->uri() );
  }

  mValues = std::vector<double>( valuesCount() * 2, 0 );

  for ( size_t i = 0; i < mLinks->size(); ++i )
  {
    double linkValue = 0;
    if ( readValue( linkValue, *mIn, changeEndianness ) )
    {
      const LinkH2i &link = mLinks->at( i );

      mValues[2 * link.cell0 + link.ori] += ( linkValue * link.ratioSize0 / 2 ) * ( mIsFlux ? 1 : link.minSize );
      mValues[2 * link.cell1 + link.ori] += ( linkValue * link.ratioSize1 / 2 ) * ( mIsFlux ? 1 : link.minSize );
    }
    else  throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Error when reading file: " + group()->uri() );

  }

  mDataloaded = true;
}

std::streampos MDAL::DatasetH2iOnLink::beginingInFile() const
{
  return ( sizeof( int ) * 2 + sizeof( double ) * mLinks->size() ) * mDatasetIndex;
}
