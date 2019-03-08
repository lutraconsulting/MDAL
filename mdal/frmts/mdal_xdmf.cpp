/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <map>
#include <cmath>

#include "mdal_xdmf.hpp"
#include "mdal_utils.hpp"
#include "mdal_data_model.hpp"
#include "mdal_xml.hpp"

MDAL::XdmfDataset::XdmfDataset( MDAL::DatasetGroup *grp, const MDAL::HyperSlab &slab, const HdfDataset &valuesDs, double time )
  : MDAL::Dataset( grp )
  , mHdf5DatasetValues( valuesDs )
  , mHyperSlab( slab )
{
  setTime( time );
}

MDAL::XdmfDataset::~XdmfDataset() = default;

std::vector<hsize_t> MDAL::XdmfDataset::offsets( size_t indexStart )
{
  std::vector<hsize_t> ret( 2 );
  ret[0] = mHyperSlab.startX + indexStart;
  ret[1] = mHyperSlab.startY;
  return ret;
}

std::vector<hsize_t> MDAL::XdmfDataset::selections( size_t copyValues )
{
  std::vector<hsize_t> ret( 2 );
  if ( mHyperSlab.countInFirstColumn )
  {
    ret[0] = copyValues;
    ret[1] = mHyperSlab.isScalar ? 1 : 3;
  }
  else
  {
    ret[0] = mHyperSlab.isScalar ? 1 : 3;
    ret[1] = copyValues;
  }
  return ret;
}

size_t MDAL::XdmfDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface
  assert( mHyperSlab.isScalar );

  size_t nValues = mHyperSlab.count;
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = selections( copyValues );
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  if ( values.empty() )
    return 0;

  const double *input = values.data();
  memcpy( buffer, input, copyValues * sizeof( double ) );
  return copyValues;
}


size_t MDAL::XdmfDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface
  assert( !mHyperSlab.isScalar );

  size_t nValues = mHyperSlab.count;
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = selections( copyValues );
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  if ( values.empty() )
    return 0;

  const double *input = values.data();
  for ( size_t j = 0; j < copyValues; ++j )
  {
    buffer[2 * j] = input[3 * j];
    buffer[2 * j + 1] = input[3 * j + 1];
  }
  return copyValues;
}

size_t MDAL::XdmfDataset::activeData( size_t indexStart, size_t count, int *buffer )
{
  MDAL_UNUSED( indexStart );
  memset( buffer, 1, count * sizeof( int ) );
  return count;
}

// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////

MDAL::HyperSlab MDAL::DriverXdmf::parseHyperSlab( const std::string &str, size_t dimX )
{
  std::stringstream slabSS( str );
  std::vector<std::vector<size_t>> data( 3, std::vector<size_t>( dimX ) );
  size_t i = 0;
  size_t number;
  while ( slabSS >> number )
  {
    data[i / dimX][i % dimX] = number;
    i++;
  }
  if ( i != 3 * dimX )
  {
    MDAL::debug( "hyperSlab dimensions mismatch" );
    throw MDAL_Status::Err_UnknownFormat;
  }

  MDAL::HyperSlab slab;
  slab.startX = data[0][0];
  slab.startY = data[0][1];
  size_t countX = data[2][0];
  size_t countY = data[2][1];

  if ( ( data[1][0] != 1 ) || ( data[1][1] != 1 ) )
  {
    MDAL::debug( " only hyperSlab with stride 1 are supported " );
    throw MDAL_Status::Err_UnknownFormat;
  }

  // sort
  if ( ( countX < countY ) && ( countY = ! 3 ) )
  {
    std::swap( countX, countY );
    slab.countInFirstColumn = false;
  }
  else
  {
    slab.countInFirstColumn = true;
  }
  slab.count = countX;

  if ( countY == 1 )
  {
    slab.isScalar = true;
  }
  else if ( countY == 3 )
  {
    slab.isScalar = false;
  }
  else
  {
    MDAL::debug( "hyperSlab dimensions mismatch, not scalar or vector" );
    throw MDAL_Status::Err_UnknownFormat;
  }

  return slab;
}

void MDAL::DriverXdmf::hdf5NamePath( const std::string &dataItemPath, std::string &filePath, std::string &hdf5Path )
{
  std::string dirName = MDAL::dirName( mDatFile );
  std::string path( dataItemPath );
  size_t endpos = path.find_last_not_of( " \t\n" );
  if ( std::string::npos != endpos )
  {
    path.erase( endpos + 1 );
  }
  size_t startpos = path.find_first_not_of( " \t\n" );
  if ( std::string::npos != startpos )
  {
    path.erase( 0, startpos );
  }

  std::vector<std::string> chunks = MDAL::split( path, ":" );
  if ( chunks.size() != 2 )
  {
    MDAL::debug( "must be in format fileName:hdfPath" );
    throw MDAL_Status::Err_UnknownFormat;
  }

  filePath = dirName + "/" + chunks[0];
  hdf5Path = chunks[1];
}

std::vector<size_t> MDAL::DriverXdmf::parseDimensions2D( const std::string &data )
{
  std::stringstream slabDimSS( data );
  std::vector<size_t> slabDim;
  size_t number;
  while ( slabDimSS >> number )
    slabDim.push_back( number );
  if ( slabDim.size() != 2 )
  {
    MDAL::debug( "Only two-dimensional slab array is supported" );
    throw MDAL_Status::Err_UnknownFormat;
  }
  return slabDim;
}

MDAL::DatasetGroups MDAL::DriverXdmf::parseXdmfXml( )
{
  size_t facesCount = mMesh->facesCount();
  std::map< std::string, std::shared_ptr<MDAL::DatasetGroup> > groups;
  std::map< std::string, std::shared_ptr<HdfFile> > hdfFiles;
  size_t nTimesteps = 0;

  XMLFile xmfFile;
  xmfFile.openFile( mDatFile );

  xmlNodePtr elem = xmfFile.getCheckRoot( "Xdmf" );
  elem = xmfFile.getCheckChild( elem, "Domain" );
  elem = xmfFile.getCheckChild( elem, "Grid" );

  xmfFile.checkAttribute( elem, "GridType", "Collection", "Expecting Collection Grid Type" );
  xmfFile.checkAttribute( elem, "CollectionType", "Temporal", "Expecting Temporal Collection Type" );

  elem = xmfFile.getCheckChild( elem, "Grid" );

  for ( xmlNodePtr gridNod = elem;
        gridNod != nullptr;
        gridNod = xmfFile.getCheckSibling( gridNod, "Grid", false ) )
  {
    ++nTimesteps;
    xmlNodePtr timeNod = xmfFile.getCheckChild( gridNod, "Time" );
    double time = xmfFile.queryDoubleAttribute( timeNod, "Value" );

    xmlNodePtr scalarNod = xmfFile.getCheckChild( gridNod, "Attribute" );

    for ( ;
          scalarNod != nullptr;
          scalarNod = xmfFile.getCheckSibling( scalarNod, "Attribute", false ) )
    {
      xmfFile.checkAttribute( scalarNod, "Center", "Cell", "Only cell centered data is currently supported" );
      if ( !xmfFile.checkAttribute( scalarNod, "AttributeType", "Scalar" ) &&
           !xmfFile.checkAttribute( scalarNod, "AttributeType", "Vector" ) )
      {
        MDAL::debug( "Only scalar and vector data are currently supported" );
        throw MDAL_Status::Err_UnknownFormat;
      }
      std::string groupName = xmfFile.attribute( scalarNod, "Name" );
      if ( groupName.empty() )
      {
        MDAL::debug( "Group name cannot be empty" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      xmlNodePtr itemNod = xmfFile.getCheckChild( scalarNod, "DataItem" );

      if ( xmfFile.checkAttribute( itemNod, "ItemType", "Function" ) )
      {
        // unsupported
        continue;
      }

      if ( !xmfFile.checkAttribute( itemNod, "ItemType", "HyperSlab" ) &&
           !xmfFile.checkAttribute( itemNod, "Type", "HyperSlab" ) )
      {
        MDAL::debug( "Expecting HyperSlab ItemType and Type" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      size_t dim = xmfFile.querySizeTAttribute( itemNod, "Dimensions" );
      if ( dim != facesCount )
      {
        MDAL::debug( "Dataset dimensions should correspond to the number of mesh elements" );
        throw MDAL_Status::Err_UnknownFormat;
      }
      xmlNodePtr slabNod = xmfFile.getCheckChild( itemNod, "DataItem" );
      xmfFile.checkAttribute( slabNod, "Format", "XML", "Only XML hyperSlab format supported" );

      std::string slabDimS = xmfFile.attribute( slabNod, "Dimensions" );
      std::vector<size_t> slabDim = parseDimensions2D( slabDimS );
      if ( slabDim[0] != 3 || ( slabDim[1] != 2 && slabDim[1] != 3 ) )
      {
        MDAL::debug( "Only two-dimensional slab array with dim 3x3 is supported (1)" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      std::string slabS = xmfFile.content( slabNod );
      const HyperSlab slab = parseHyperSlab( slabS, slabDim[1] );

      xmlNodePtr snapshotNod = xmfFile.getCheckSibling( slabNod, "DataItem" );

      xmfFile.checkAttribute( snapshotNod, "Format", "HDF", "Only HDF dataset format supported" );
      std::string snapshotDimS = xmfFile.attribute( snapshotNod, "Dimensions" );
      std::vector<size_t> snapshotDim = parseDimensions2D( snapshotDimS );

      std::string hdf5Name, hdf5Path;
      hdf5NamePath( xmfFile.content( snapshotNod ), hdf5Name, hdf5Path );

      std::shared_ptr<HdfFile> hdfFile;
      if ( hdfFiles.count( hdf5Name ) == 0 )
      {
        hdfFile = std::make_shared<HdfFile>( hdf5Name );
        hdfFiles[hdf5Name] = hdfFile;
      }
      else
      {
        hdfFile = hdfFiles[hdf5Name];
      }

      std::shared_ptr<MDAL::DatasetGroup> group;
      if ( groups.count( groupName ) == 0 )
      {
        group = std::make_shared<MDAL::DatasetGroup>(
                  "XDMF",
                  mMesh,
                  mDatFile,
                  groupName
                );
        group->setIsScalar( slab.isScalar );
        group->setIsOnVertices( false ); //only center-based implemented
        groups[groupName] = group;
      }
      else
      {
        group = groups[groupName];
        if ( group->isScalar() != slab.isScalar )
        {
          MDAL::debug( "Inconsistent groups" );
          throw MDAL_Status::Err_UnknownFormat;
        }
      }

      HdfDataset hdfDataset( hdfFile->id(), hdf5Path );
      std::shared_ptr<MDAL::XdmfDataset> xdmfDataset = std::make_shared<MDAL::XdmfDataset>(
            group.get(),
            slab,
            hdfDataset,
            time
          );
      // This basically forces to load all data to calculate statistics!
      const MDAL::Statistics stats = MDAL::calculateStatistics( xdmfDataset );
      xdmfDataset->setStatistics( stats );
      group->datasets.push_back( xdmfDataset );
    }
  }

  // check groups
  DatasetGroups ret;
  for ( const auto &group : groups )
  {
    std::shared_ptr<MDAL::DatasetGroup> grp = group.second;
    if ( grp->datasets.size() != nTimesteps )
    {
      MDAL::debug( "Invalid group, missing timesteps" );
      throw MDAL_Status::Err_UnknownFormat;
    }

    const MDAL::Statistics stats = MDAL::calculateStatistics( grp );
    grp->setStatistics( stats );
    // verify the integrity of the dataset
    if ( !std::isnan( stats.minimum ) )
      ret.push_back( grp );
  }

  return ret;
}


MDAL::DriverXdmf::DriverXdmf()
  : Driver( "XDMF",
            "XDMF",
            "*.xdmf;;*.xmf",
            Capability::ReadDatasets )
{
}

MDAL::DriverXdmf::~DriverXdmf() = default;

MDAL::DriverXdmf *MDAL::DriverXdmf::create()
{
  return new DriverXdmf();
}

bool MDAL::DriverXdmf::canRead( const std::string &uri )
{
  XMLFile xmfFile;
  try
  {
    xmfFile.openFile( uri );
    xmlNodePtr root = xmfFile.getCheckRoot( "Xdmf" );
    xmfFile.checkAttribute( root, "Version", "2.0", "Invalid version" );
  }
  catch ( MDAL_Status )
  {
    return false;
  }
  return true;
}

void MDAL::DriverXdmf::load( const std::string &datFile,
                             MDAL::Mesh *mesh,
                             MDAL_Status *status )
{
  assert( mesh );

  mDatFile = datFile;
  mMesh = mesh;

  if ( status ) *status = MDAL_Status::None;

  if ( !MDAL::fileExists( mDatFile ) )
  {
    if ( status ) *status = MDAL_Status::Err_FileNotFound;
    return;
  }

  try
  {
    // parse XML
    DatasetGroups groups = parseXdmfXml( );
    // add groups to mesh
    for ( const auto &group : groups )
    {
      mMesh->datasetGroups.push_back( group );
    }
  }
  catch ( MDAL_Status err )
  {
    if ( status ) *status = err;
  }
}
