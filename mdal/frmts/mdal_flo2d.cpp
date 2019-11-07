/*
 MDAL - mMesh Data Abstraction Library (MIT License)
 Copyright (C) 2016 Lutra Consulting
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_flo2d.hpp"
#include <vector>
#include <map>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <cstring>

#include "mdal_utils.hpp"
#include "mdal_hdf5.hpp"

struct VertexCompare
{
  bool operator()( const MDAL::Vertex &lhs, const MDAL::Vertex &rhs ) const
  {
    double resX = 0;
    resX += lhs.x * 1000000;
    resX += lhs.y * 1000;

    double resY = 0;
    resY += rhs.x * 1000000;
    resY += rhs.y * 1000;

    return resX < resY;
  }
};

static std::string fileNameFromDir( const std::string &mainFileName, const std::string &name )
{
  std::string dir = MDAL::dirName( mainFileName );
  return MDAL::pathJoin( dir, name );
}

static double getDouble( double val )
{
  if ( MDAL::equals( val, 0.0, 1e-8 ) )
  {
    return MDAL_NAN;
  }
  else
  {
    return val;
  }
}

static double getDouble( const std::string &val )
{
  double valF = MDAL::toDouble( val );
  return getDouble( valF );
}

void MDAL::DriverFlo2D::addStaticDataset(
  std::vector<double> &vals,
  const std::string &groupName,
  const std::string &datFileName )
{
  std::shared_ptr<DatasetGroup> group = std::make_shared< DatasetGroup >(
                                          name(),
                                          mMesh.get(),
                                          datFileName,
                                          groupName
                                        );
  group->setIsOnVertices( false );
  group->setIsScalar( true );

  std::shared_ptr<MDAL::MemoryDataset> dataset = std::make_shared< MemoryDataset >( group.get() );
  assert( vals.size() == dataset->valuesCount() );
  dataset->setTime( 0.0 );
  double *values = dataset->values();
  memcpy( values, vals.data(), vals.size() * sizeof( double ) );
  dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
  group->datasets.push_back( dataset );
  group->setStatistics( MDAL::calculateStatistics( group ) );
  mMesh->datasetGroups.push_back( group );
}

void MDAL::DriverFlo2D::parseCADPTSFile( const std::string &datFileName, std::vector<CellCenter> &cells )
{
  std::string cadptsFile( fileNameFromDir( datFileName, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  std::ifstream cadptsStream( cadptsFile, std::ifstream::in );
  std::string line;
  // CADPTS.DAT - COORDINATES OF CELL CENTERS (ELEM NUM, X, Y)
  while ( std::getline( cadptsStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 3 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
    CellCenter cc;
    cc.id = MDAL::toSizeT( lineParts[1] ) - 1; //numbered from 1
    cc.x = MDAL::toDouble( lineParts[1] );
    cc.y = MDAL::toDouble( lineParts[2] );
    cc.conn.resize( 4 );
    cells.push_back( cc );
  }
}

void MDAL::DriverFlo2D::parseFPLAINFile( std::vector<double> &elevations,
    const std::string &datFileName,
    std::vector<CellCenter> &cells )
{
  elevations.clear();
  // FPLAIN.DAT - CONNECTIVITY (ELEM NUM, ELEM N, ELEM E, ELEM S, ELEM W, MANNING-N, BED ELEVATION)
  std::string fplainFile( fileNameFromDir( datFileName, "FPLAIN.DAT" ) );
  if ( !MDAL::fileExists( fplainFile ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  std::ifstream fplainStream( fplainFile, std::ifstream::in );
  std::string line;

  while ( std::getline( fplainStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 7 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
    size_t cc_i = MDAL::toSizeT( lineParts[0] ) - 1; //numbered from 1
    for ( size_t j = 0; j < 4; ++j )
    {
      cells[cc_i].conn[j] = MDAL::toInt( lineParts[j + 1] ) - 1; //numbered from 1, 0 boundary Vertex
    }
    elevations.push_back( MDAL::toDouble( lineParts[6] ) );
  }
}

static void addDatasetToGroup( std::shared_ptr<MDAL::DatasetGroup> group, std::shared_ptr<MDAL::MemoryDataset> dataset )
{
  if ( group && dataset && dataset->valuesCount() > 0 )
  {
    dataset->setStatistics( MDAL::calculateStatistics( dataset ) );
    group->datasets.push_back( dataset );
  }
}

void MDAL::DriverFlo2D::parseTIMDEPFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // TIMDEP.OUT
  // this file is optional, so if not present, reading is skipped
  // time (separate line)
  // For every Vertex:
  // FLO2D: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y
  // FLO2DPro: ELEM NUMber (indexed from 1), depth, velocity, velocity x, velocity y, water surface elevation
  std::string inFile( fileNameFromDir( datFileName, "TIMDEP.OUT" ) );
  if ( !MDAL::fileExists( inFile ) )
  {
    return;
  }

  std::ifstream inStream( inFile, std::ifstream::in );
  std::string line;

  size_t nVertexs = mMesh->verticesCount();
  size_t ntimes = 0;

  double time = 0.0;
  size_t face_idx = 0;

  std::shared_ptr<DatasetGroup> depthDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Depth"
      );
  depthDsGroup->setIsOnVertices( false );
  depthDsGroup->setIsScalar( true );


  std::shared_ptr<DatasetGroup> waterLevelDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Water Level"
      );
  waterLevelDsGroup->setIsOnVertices( false );
  waterLevelDsGroup->setIsScalar( true );

  std::shared_ptr<DatasetGroup> flowDsGroup = std::make_shared< DatasetGroup >(
        name(),
        mMesh.get(),
        datFileName,
        "Velocity"
      );
  flowDsGroup->setIsOnVertices( false );
  flowDsGroup->setIsScalar( false );

  std::shared_ptr<MDAL::MemoryDataset> flowDataset;
  std::shared_ptr<MDAL::MemoryDataset> depthDataset;
  std::shared_ptr<MDAL::MemoryDataset> waterLevelDataset;

  while ( std::getline( inStream, line ) )
  {
    line = MDAL::rtrim( line );
    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() == 1 )
    {
      time = MDAL::toDouble( line );
      ntimes++;

      if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
      if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
      if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

      depthDataset  = std::make_shared< MemoryDataset >( depthDsGroup.get() );
      flowDataset = std::make_shared< MemoryDataset >( flowDsGroup.get() );
      waterLevelDataset = std::make_shared< MemoryDataset >( waterLevelDsGroup.get() );

      depthDataset->setTime( time );
      flowDataset->setTime( time );
      waterLevelDataset->setTime( time );

      face_idx = 0;

    }
    else if ( ( lineParts.size() == 5 ) || ( lineParts.size() == 6 ) )
    {
      // new Vertex for time
      if ( !depthDataset || !flowDataset || !waterLevelDataset ) throw MDAL_Status::Err_UnknownFormat;
      if ( face_idx == nVertexs ) throw MDAL_Status::Err_IncompatibleMesh;

      // this is magnitude: getDouble(lineParts[2]);
      flowDataset->values()[2 * face_idx] = getDouble( lineParts[3] );
      flowDataset->values()[2 * face_idx + 1] = getDouble( lineParts[4] );

      double depth = getDouble( lineParts[1] );
      depthDataset->values()[face_idx] = depth;

      if ( !std::isnan( depth ) ) depth += elevations[face_idx];
      waterLevelDataset->values()[face_idx] = depth;

      face_idx ++;

    }
    else
    {
      throw MDAL_Status::Err_UnknownFormat;
    }
  }

  if ( depthDataset ) addDatasetToGroup( depthDsGroup, depthDataset );
  if ( flowDataset ) addDatasetToGroup( flowDsGroup, flowDataset );
  if ( waterLevelDataset ) addDatasetToGroup( waterLevelDsGroup, waterLevelDataset );

  depthDsGroup->setStatistics( MDAL::calculateStatistics( depthDsGroup ) );
  flowDsGroup->setStatistics( MDAL::calculateStatistics( flowDsGroup ) );
  waterLevelDsGroup->setStatistics( MDAL::calculateStatistics( waterLevelDsGroup ) );

  mMesh->datasetGroups.push_back( depthDsGroup );
  mMesh->datasetGroups.push_back( flowDsGroup );
  mMesh->datasetGroups.push_back( waterLevelDsGroup );
}


void MDAL::DriverFlo2D::parseDEPTHFile( const std::string &datFileName, const std::vector<double> &elevations )
{
  // this file is optional, so if not present, reading is skipped
  std::string depthFile( fileNameFromDir( datFileName, "DEPTH.OUT" ) );
  if ( !MDAL::fileExists( depthFile ) )
  {
    return; //optional file
  }

  std::ifstream depthStream( depthFile, std::ifstream::in );
  std::string line;

  size_t nFaces = mMesh->facesCount();
  std::vector<double> maxDepth( nFaces );
  std::vector<double> maxWaterLevel( nFaces );

  size_t vertex_idx = 0;

  // DEPTH.OUT - COORDINATES (ELEM NUM, X, Y, MAX DEPTH)
  while ( std::getline( depthStream, line ) )
  {
    line = MDAL::rtrim( line );
    if ( vertex_idx == nFaces ) throw MDAL_Status::Err_IncompatibleMesh;

    std::vector<std::string> lineParts = MDAL::split( line, ' ' );
    if ( lineParts.size() != 4 )
    {
      throw MDAL_Status::Err_UnknownFormat;
    }

    double val = getDouble( lineParts[3] );
    maxDepth[vertex_idx] = val;

    //water level
    if ( !std::isnan( val ) ) val += elevations[vertex_idx];
    maxWaterLevel[vertex_idx] = val;


    vertex_idx++;
  }

  addStaticDataset( maxDepth, "Depth/Maximums", datFileName );
  addStaticDataset( maxWaterLevel, "Water Level/Maximums", datFileName );
}


void MDAL::DriverFlo2D::parseVELFPVELOCFile( const std::string &datFileName )
{
  // these files are optional, so if not present, reading is skipped
  size_t nFaces = mMesh->facesCount();
  std::vector<double> maxVel( nFaces );

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELFP.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELFP.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL) - Maximum floodplain flow velocity;
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nFaces ) throw MDAL_Status::Err_IncompatibleMesh;

      line = MDAL::rtrim( line );
      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL_Status::Err_UnknownFormat;
      }

      double val = getDouble( lineParts[3] );
      maxVel[vertex_idx] = val;

      vertex_idx++;
    }
  }

  {
    std::string velocityFile( fileNameFromDir( datFileName, "VELOC.OUT" ) );
    if ( !MDAL::fileExists( velocityFile ) )
    {
      return; //optional file
    }

    std::ifstream velocityStream( velocityFile, std::ifstream::in );
    std::string line;

    size_t vertex_idx = 0;

    // VELOC.OUT - COORDINATES (ELEM NUM, X, Y, MAX VEL)  - Maximum channel flow velocity
    while ( std::getline( velocityStream, line ) )
    {
      if ( vertex_idx == nFaces ) throw MDAL_Status::Err_IncompatibleMesh;

      line = MDAL::rtrim( line );
      std::vector<std::string> lineParts = MDAL::split( line, ' ' );
      if ( lineParts.size() != 4 )
      {
        throw MDAL_Status::Err_UnknownFormat;
      }

      double val = getDouble( lineParts[3] );
      if ( !std::isnan( val ) )  // overwrite value from VELFP if it is not 0
      {
        maxVel[vertex_idx] = val;
      }

      vertex_idx++;
    }
  }

  addStaticDataset( maxVel, "Velocity/Maximums", datFileName );
}

double MDAL::DriverFlo2D::calcCellSize( const std::vector<CellCenter> &cells )
{
  // find first cell that is not izolated from the others
  // and return its distance to the neighbor's cell center
  for ( size_t i = 0; i < cells.size(); ++i )
  {
    for ( size_t j = 0; j < 4; ++j )
    {
      int idx = cells[i].conn[0];
      if ( idx > -1 )
      {
        if ( ( j == 0 ) || ( j == 2 ) )
        {
          return fabs( cells[static_cast<size_t>( idx )].y - cells[i].y );
        }
        else
        {
          return fabs( cells[static_cast<size_t>( idx )].x - cells[i].x );
        }
      }
    }
  }
  throw MDAL_Status::Err_IncompatibleMesh;
}

MDAL::Vertex MDAL::DriverFlo2D::createVertex( size_t position, double half_cell_size, const CellCenter &cell )
{
  MDAL::Vertex n;
  n.x = cell.x;
  n.y = cell.y;

  switch ( position )
  {
    case 0:
      n.x += half_cell_size;
      n.y -= half_cell_size;
      break;

    case 1:
      n.x += half_cell_size;
      n.y += half_cell_size;
      break;

    case 2:
      n.x -= half_cell_size;
      n.y += half_cell_size;
      break;

    case 3:
      n.x -= half_cell_size;
      n.y -= half_cell_size;
      break;
  }

  return n;
}

void MDAL::DriverFlo2D::createMesh( const std::vector<CellCenter> &cells, double half_cell_size )
{
  // Create all Faces from cell centers.
  // Vertexs must be also created, they are not stored in FLO-2D files
  // try to reuse Vertexs already created for other Faces by usage of unique_Vertexs set.
  Faces faces;
  Vertices vertices;
  std::map<Vertex, size_t, VertexCompare> unique_vertices; //vertex -> id
  size_t vertex_idx = 0;

  for ( size_t i = 0; i < cells.size(); ++i )
  {
    Face e( 4 );

    for ( size_t position = 0; position < 4; ++position )
    {
      Vertex n = createVertex( position, half_cell_size, cells[i] );
      const auto iter = unique_vertices.find( n );
      if ( iter == unique_vertices.end() )
      {
        unique_vertices[n] = vertex_idx;
        vertices.push_back( n );
        e[position] = vertex_idx;
        ++vertex_idx;
      }
      else
      {
        e[position] = iter->second;
      }
    }

    faces.push_back( e );
  }

  mMesh.reset(
    new MemoryMesh(
      name(),
      vertices.size(),
      faces.size(),
      4, //maximum quads
      computeExtent( vertices ),
      mDatFileName
    )
  );
  mMesh->faces = faces;
  mMesh->vertices = vertices;
}

bool MDAL::DriverFlo2D::parseHDF5Datasets( const std::string &datFileName )
{
  //return true on error

  size_t nFaces =  mMesh->facesCount();

  std::string timedepFileName = fileNameFromDir( datFileName, "TIMDEP.HDF5" );
  if ( !fileExists( timedepFileName ) ) return true;

  HdfFile file( timedepFileName );
  if ( !file.isValid() ) return true;

  HdfGroup timedataGroup = file.group( "TIMDEP NETCDF OUTPUT RESULTS" );
  if ( !timedataGroup.isValid() ) return true;

  std::vector<std::string> groupNames = timedataGroup.groups();

  for ( const std::string &grpName : groupNames )
  {
    HdfGroup grp = timedataGroup.group( grpName );
    if ( !grp.isValid() ) return true;

    HdfAttribute groupType = grp.attribute( "Grouptype" );
    if ( !groupType.isValid() ) return true;

    /* Min and Max arrays in TIMDEP.HDF5 files have dimensions 1xntimesteps .
        HdfDataset minDs = grp.dataset("Mins");
        if (!minDs.isValid()) return true;

        HdfDataset maxDs = grp.dataset("Maxs");
        if (!maxDs.isValid()) return true;
    */

    HdfDataset timesDs = grp.dataset( "Times" );
    if ( !timesDs.isValid() ) return true;
    size_t timesteps = timesDs.elementCount();

    HdfDataset valuesDs = grp.dataset( "Values" );
    if ( !valuesDs.isValid() ) return true;

    bool isVector = MDAL::contains( groupType.readString(), "vector", ContainsBehaviour::CaseInsensitive );

    // Some sanity checks
    size_t expectedSize = mMesh->facesCount() * timesteps;
    if ( isVector ) expectedSize *= 2;
    if ( valuesDs.elementCount() != expectedSize ) return true;

    // Read data
    std::vector<double> times = timesDs.readArrayDouble();
    std::vector<float> values = valuesDs.readArray();

    // Create dataset now
    std::shared_ptr<DatasetGroup> ds = std::make_shared< DatasetGroup >(
                                         name(),
                                         mMesh.get(),
                                         datFileName,
                                         grpName
                                       );
    ds->setIsOnVertices( false );
    ds->setIsScalar( !isVector );

    for ( size_t ts = 0; ts < timesteps; ++ts )
    {
      std::shared_ptr< MemoryDataset > output = std::make_shared< MemoryDataset >( ds.get() );
      output->setTime( times[ts] );

      if ( isVector )
      {
        // vector
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = 2 * ( ts * nFaces + i );
          double x = getDouble( static_cast<double>( values[idx] ) );
          double y = getDouble( static_cast<double>( values[idx + 1] ) );
          output->values()[2 * i] = x;
          output->values()[2 * i + 1] = y;
        }
      }
      else
      {
        // scalar
        for ( size_t i = 0; i < nFaces; ++i )
        {
          size_t idx = ts * nFaces + i;
          double val = getDouble( static_cast<double>( values[idx] ) );
          output->values()[i] = val;
        }
      }
      addDatasetToGroup( ds, output );
    }

    // TODO use mins & maxs arrays
    ds->setStatistics( MDAL::calculateStatistics( ds ) );
    mMesh->datasetGroups.push_back( ds );

  }

  return false;
}

void MDAL::DriverFlo2D::parseOUTDatasets( const std::string &datFileName, const std::vector<double> &elevations )
{
  // Create Depth and Velocity datasets Time varying datasets
  parseTIMDEPFile( datFileName, elevations );

  // Maximum Depth and Water Level
  parseDEPTHFile( datFileName, elevations );

  // Maximum Velocity
  parseVELFPVELOCFile( datFileName );
}

MDAL::DriverFlo2D::DriverFlo2D()
  : Driver(
      "FLO2D",
      "Flo2D",
      "*.nc",
      Capability::ReadMesh | Capability::WriteDatasets )
{

}

MDAL::DriverFlo2D *MDAL::DriverFlo2D::create()
{
  return new DriverFlo2D();
}

bool MDAL::DriverFlo2D::canRead( const std::string &uri )
{
  std::string cadptsFile( fileNameFromDir( uri, "CADPTS.DAT" ) );
  if ( !MDAL::fileExists( cadptsFile ) )
  {
    return false;
  }

  std::string fplainFile( fileNameFromDir( uri, "FPLAIN.DAT" ) );
  if ( !MDAL::fileExists( fplainFile ) )
  {
    return false;
  }

  return true;
}

std::unique_ptr< MDAL::Mesh > MDAL::DriverFlo2D::load( const std::string &resultsFile, MDAL_Status *status )
{
  mDatFileName = resultsFile;
  if ( status ) *status = MDAL_Status::None;
  mMesh.reset();
  std::vector<CellCenter> cells;

  try
  {
    // Parse mMesh info
    parseCADPTSFile( mDatFileName, cells );
    std::vector<double> elevations;
    parseFPLAINFile( elevations, mDatFileName, cells );
    double cell_size = calcCellSize( cells );

    // Create mMesh
    createMesh( cells, cell_size / 2.0 );

    // create output for bed elevation
    addStaticDataset( elevations, "Bed Elevation", mDatFileName );

    if ( parseHDF5Datasets( mDatFileName ) )
    {
      // some problem with HDF5 data, try text files
      parseOUTDatasets( mDatFileName, elevations );
    }
  }

  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}

void MDAL::DriverFlo2D::addToHDF5File( DatasetGroup *group )
{
  // TODO: for testing only
  saveNewHDF5File( group );
}

void MDAL::DriverFlo2D::saveNewHDF5File( DatasetGroup *group )
{
  // Create file
  HdfFile file( group->uri(), true );

  // Create dataspace for dataset File Version
  std::vector<hsize_t> dimsSingle = {1};
  std::vector<hsize_t> dimsDouble = {1, 1};
  HdfDataspace dscFileVersion( dimsSingle, true );

  // Create float dataset File Version
  HdfDataset dsFileVersion( file.id(), "/File Version", true );
  dsFileVersion.writeFloat( dscFileVersion.id(), 99.99f );

  // Create dataspace for dataset File Type
  HdfDataspace dscFileType( dimsSingle, true );

  // Create string dataset File Type
  HdfDataset dsFileType( file.id(), "/File Type", true );
  dsFileType.writeString( file.id(), dscFileType.id(), "Xmdf" );

  // Create group TIMDEP NETCDF OUTPUT RESULTS
  HdfGroup groupTNOR( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS", true );

  // Crete dataspace for attribute
  HdfDataspace dscTNOR( dimsSingle, true );

  // Create attribute
  HdfAttribute attTNORGrouptype( groupTNOR.id(), "Grouptype", true );
  // Write string value to attribute
  attTNORGrouptype.writeString( dscTNOR.id(), "Generic" );

  // Group - Flow Depth
  HdfGroup groupFlowDepth( groupTNOR.id(), "/TIMDEP NETCDF OUTPUT RESULTS/FLOW DEPTH", true );
  HdfDataspace dscFlowDepthDataType( dimsSingle, true );
  HdfAttribute attFlowDepthDataType( groupFlowDepth.id(), "Data Type", true );
  attFlowDepthDataType.writeInt32( dscFlowDepthDataType.id(), 0 );

  HdfDataspace dscFlowDepthDatasetCompression( dimsSingle, true );
  HdfAttribute attFlowDepthDatasetCompression( groupFlowDepth.id(), "DatasetCompression", true );
  attFlowDepthDatasetCompression.writeInt32( dscFlowDepthDatasetCompression.id(), -1 );

  HdfDataspace dscFlowDepthDatasetUnits( dimsSingle, true );
  HdfAttribute attFlowDepthDatasetUnits( groupFlowDepth.id(), "DatasetUnits", true );
  attFlowDepthDatasetUnits.writeString( dscFlowDepthDatasetUnits.id(), "ft or m" );

  HdfDataspace dscFlowDepthGrouptype( dimsSingle, true );
  HdfAttribute attFlowDepthGrouptype( groupFlowDepth.id(), "Grouptype", true );
  attFlowDepthGrouptype.writeString( dscFlowDepthGrouptype.id(), "DATASET SCALAR" );

  HdfDataspace dscFlowDepthTimeUnits( dimsSingle, true );
  HdfAttribute attFlowDepthTimeUnits( groupFlowDepth.id(), "TimeUnits", true );
  attFlowDepthTimeUnits.writeString( dscFlowDepthTimeUnits.id(), "Hours" );

  std::vector<hsize_t> dimsMaxMinTimes = { 20 };
  float nullData[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  double nullDoubleData[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  float null2DData[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  std::vector<hsize_t> dimsValues = { 3, 3 };

  float null3DData[] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  std::vector<hsize_t> dimsValues3d = { 3, 3, 3 };

  HdfDataspace dscFlowDepthMaxs( dimsMaxMinTimes, true );
  HdfDataset dsFlowDepthMaxs( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/FLOW DEPTH/Maxs", true );
  dsFlowDepthMaxs.writeFloatArray( dscFlowDepthMaxs.id(), nullData ); // Replace with data array ?

  HdfDataspace dscFlowDepthMins( dimsMaxMinTimes, true );
  HdfDataset dsFlowDepthMins( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/FLOW DEPTH/Mins", true );
  dsFlowDepthMins.writeFloatArray( dscFlowDepthMins.id(), nullData ); // Replace with data array ?

  HdfDataspace dscFlowDepthTimes( dimsMaxMinTimes, true );
  HdfDataset dsFlowDepthTimes( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/FLOW DEPTH/Times", true );
  dsFlowDepthTimes.writeDoubleArray( dscFlowDepthTimes.id(), nullDoubleData ); // Replace with data array ?

  HdfDataspace dscFlowDepthValues( dimsValues, true );
  HdfDataset dsFlowDepthValues( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/FLOW DEPTH/Values", true );
  dsFlowDepthValues.writeFloatArray( dscFlowDepthValues.id(), null2DData ); // Replace with data array ?

  // Group - Floodplain Water Surface Elevation

  HdfGroup groupFWSE( groupTNOR.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Floodplain Water Surface Elevation", true );
  HdfDataspace dscFWSEDataType( dimsSingle, true );
  HdfAttribute attFWSEDataType( groupFWSE.id(), "Data Type", true );
  attFWSEDataType.writeInt32( dscFWSEDataType.id(), 0 );

  HdfDataspace dscFWSEDatasetCompression( dimsSingle, true );
  HdfAttribute attFWSEDatasetCompression( groupFWSE.id(), "DatasetCompression", true );
  attFWSEDatasetCompression.writeInt32( dscFWSEDatasetCompression.id(), -1 );

  HdfDataspace dscFWSEDatasetUnits( dimsSingle, true );
  HdfAttribute attFWSEDatasetUnits( groupFWSE.id(), "DatasetUnits", true );
  attFWSEDatasetUnits.writeString( dscFWSEDatasetUnits.id(), "ft or m" );

  HdfDataspace dscFWSEGrouptype( dimsSingle, true );
  HdfAttribute attFWSEGrouptype( groupFWSE.id(), "Grouptype", true );
  attFWSEGrouptype.writeString( dscFWSEGrouptype.id(), "DATASET SCALAR" );

  HdfDataspace dscFWSETimeUnits( dimsSingle, true );
  HdfAttribute attFWSETimeUnits( groupFWSE.id(), "TimeUnits", true );
  attFWSETimeUnits.writeString( dscFWSETimeUnits.id(), "Hours" );

  HdfDataspace dscFWSEMaxs( dimsMaxMinTimes, true );
  HdfDataset dsFWSEMaxs( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Floodplain Water Surface Elevation/Maxs", true );
  dsFWSEMaxs.writeFloatArray( dscFWSEMaxs.id(), nullData ); // Replace with data array ?

  HdfDataspace dscFWSEMins( dimsMaxMinTimes, true );
  HdfDataset dsFWSEMins( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Floodplain Water Surface Elevation/Mins", true );
  dsFWSEMins.writeFloatArray( dscFWSEMins.id(), nullData ); // Replace with data array ?

  HdfDataspace dscFWSETimes( dimsMaxMinTimes, true );
  HdfDataset dsFWSETimes( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Floodplain Water Surface Elevation/Times", true );
  dsFWSETimes.writeDoubleArray( dscFWSETimes.id(), nullDoubleData ); // Replace with data array ?

  HdfDataspace dscFWSEValues( dimsValues, true );
  HdfDataset dsFWSEValues( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Floodplain Water Surface Elevation/Values", true );
  dsFWSEValues.writeFloatArray( dscFWSEValues.id(), null2DData ); // Replace with data array ?

  // Group - Velocity

  HdfGroup groupVelocity( groupTNOR.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity", true );
  HdfDataspace dscVelocityDataType( dimsSingle, true );
  HdfAttribute attVelocityDataType( groupVelocity.id(), "Data Type", true );
  attVelocityDataType.writeInt32( dscVelocityDataType.id(), 0 );

  HdfDataspace dscVelocityDatasetCompression( dimsSingle, true );
  HdfAttribute attVelocityDatasetCompression( groupVelocity.id(), "DatasetCompression", true );
  attVelocityDatasetCompression.writeInt32( dscVelocityDatasetCompression.id(), -1 );

  HdfDataspace dscVelocityDatasetUnits( dimsSingle, true );
  HdfAttribute attVelocityDatasetUnits( groupVelocity.id(), "DatasetUnits", true );
  attVelocityDatasetUnits.writeString( dscVelocityDatasetUnits.id(), "fps or mps" );

  HdfDataspace dscVelocityGrouptype( dimsSingle, true );
  HdfAttribute attVelocityGrouptype( groupVelocity.id(), "Grouptype", true );
  attVelocityGrouptype.writeString( dscVelocityGrouptype.id(), "DATASET VECTOR" );

  HdfDataspace dscVelocityTimeUnits( dimsSingle, true );
  HdfAttribute attVelocityTimeUnits( groupVelocity.id(), "TimeUnits", true );
  attVelocityTimeUnits.writeString( dscVelocityTimeUnits.id(), "Hours" );

  HdfDataspace dscVelocityMaxs( dimsMaxMinTimes, true );
  HdfDataset dsVelocityMaxs( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity/Maxs", true );
  dsVelocityMaxs.writeFloatArray( dscVelocityMaxs.id(), nullData ); // Replace with data array ?

  HdfDataspace dscVelocityMins( dimsMaxMinTimes, true );
  HdfDataset dsVelocityMins( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity/Mins", true );
  dsVelocityMins.writeFloatArray( dscVelocityMins.id(), nullData ); // Replace with data array ?

  HdfDataspace dscVelocityTimes( dimsMaxMinTimes, true );
  HdfDataset dsVelocityTimes( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity/Times", true );
  dsVelocityTimes.writeDoubleArray( dscVelocityTimes.id(), nullDoubleData ); // Replace with data array ?

  HdfDataspace dscVelocityValues( dimsValues3d, true );
  HdfDataset dsVelocityValues( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity/Values", true );
  dsVelocityValues.writeFloatArray( dscVelocityValues.id(), null3DData ); // Replace with data array ?

  // Group - Velocity MAG

  HdfGroup groupVelocityMag( groupTNOR.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity MAG", true );
  HdfDataspace dscVelocityMagDataType( dimsSingle, true );
  HdfAttribute attVelocityMagDataType( groupVelocityMag.id(), "Data Type", true );
  attVelocityMagDataType.writeInt32( dscVelocityMagDataType.id(), 0 );

  HdfDataspace dscVelocityMagDatasetCompression( dimsSingle, true );
  HdfAttribute attVelocityMagDatasetCompression( groupVelocityMag.id(), "DatasetCompression", true );
  attVelocityMagDatasetCompression.writeInt32( dscVelocityMagDatasetCompression.id(), -1 );

  HdfDataspace dscVelocityMagDatasetUnits( dimsSingle, true );
  HdfAttribute attVelocityMagDatasetUnits( groupVelocityMag.id(), "DatasetUnits", true );
  attVelocityMagDatasetUnits.writeString( dscVelocityMagDatasetUnits.id(), "ft or m" );

  HdfDataspace dscVelocityMagGrouptype( dimsSingle, true );
  HdfAttribute attVelocityMagGrouptype( groupVelocityMag.id(), "Grouptype", true );
  attVelocityMagGrouptype.writeString( dscVelocityMagGrouptype.id(), "DATASET SCALAR" );

  HdfDataspace dscVelocityMagTimeUnits( dimsSingle, true );
  HdfAttribute attVelocityMagTimeUnits( groupVelocityMag.id(), "TimeUnits", true );
  attVelocityMagTimeUnits.writeString( dscVelocityMagTimeUnits.id(), "Hours" );

  HdfDataspace dscVelocityMagMaxs( dimsMaxMinTimes, true );
  HdfDataset dsVelocityMagMaxs( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity MAG/Maxs", true );
  dsVelocityMagMaxs.writeFloatArray( dscVelocityMagMaxs.id(), nullData ); // Replace with data array ?

  HdfDataspace dscVelocityMagMins( dimsMaxMinTimes, true );
  HdfDataset dsVelocityMagMins( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity MAG/Mins", true );
  dsVelocityMagMins.writeFloatArray( dscVelocityMagMins.id(), nullData ); // Replace with data array ?

  HdfDataspace dscVelocityMagTimes( dimsMaxMinTimes, true );
  HdfDataset dsVelocityMagTimes( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity MAG/Times", true );
  dsVelocityMagTimes.writeDoubleArray( dscVelocityMagTimes.id(), nullDoubleData ); // Replace with data array ?

  HdfDataspace dscVelocityMagValues( dimsValues, true );
  HdfDataset dsVelocityMagValues( file.id(), "/TIMDEP NETCDF OUTPUT RESULTS/Velocity MAG/Values", true );
  dsVelocityMagValues.writeFloatArray( dscVelocityMagValues.id(), null2DData ); // Replace with data array ?

}

bool MDAL::DriverFlo2D::persist( DatasetGroup *group )
{
  try
  {
    // Return true on error
    const std::string path = group->uri();
    if ( MDAL::fileExists( path ) )
    {
      // Add dataset to a existing file
      addToHDF5File( group );
    }
    else
    {
      // Create new HDF5 file with Flow2D structure
      saveNewHDF5File( group );
    }
    return false;
  }
  catch ( MDAL_Status error )
  {
    return true;
  }

}
