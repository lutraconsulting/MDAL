/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Lutra Consulting Limited
*/

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <memory>

#include <tinyxml2.h>
#include <map>

#include "mdal_xdmf.hpp"
#include "mdal_utils.hpp"
#include "mdal_data_model.hpp"

static tinyxml2::XMLElement *getCheckChild( tinyxml2::XMLElement *parent, std::string name )
{
  tinyxml2::XMLNode *child = parent->FirstChildElement( name.c_str() );
  if ( !child )
  {
    MDAL::debug( "XML Element not found: " + name );
    throw MDAL_Status::Err_UnknownFormat;
  }
  tinyxml2::XMLElement *elem = child->ToElement();
  if ( !elem )
  {
    MDAL::debug( "XML Node is not an element " + name );
    throw MDAL_Status::Err_UnknownFormat;
  }

  return elem;
}

static tinyxml2::XMLElement *getCheckSibling( tinyxml2::XMLElement *from, std::string name )
{
  tinyxml2::XMLNode *child = from->NextSiblingElement( name.c_str() );
  if ( !child )
  {
    MDAL::debug( "XML Element not found: " + name );
    throw MDAL_Status::Err_UnknownFormat;
  }
  tinyxml2::XMLElement *elem = child->ToElement();
  if ( !elem )
  {
    MDAL::debug( "XML Node is not an element " + name );
    throw MDAL_Status::Err_UnknownFormat;
  }

  return elem;
}


static std::string attribute( tinyxml2::XMLElement *elem, std::string name )
{
  std::string res;
  if ( !elem )
    return res;
  const char *buf = elem->Attribute( name.c_str() );
  if ( !buf )
    return res;

  res = buf;
  return res;
}

static std::string attribute( tinyxml2::XMLNode *node, std::string name )
{
  std::string res;
  if ( !node )
    return res;
  tinyxml2::XMLElement *elem = node->ToElement();
  return attribute( elem, name );
}

static double queryDoubleAttribute( tinyxml2::XMLElement *elem, std::string name )
{
  double val;
  tinyxml2::XMLError err = elem->QueryDoubleAttribute( name.c_str(), &val );
  if ( err != tinyxml2::XMLError::XML_SUCCESS )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return val;
}

static int queryIntAttribute( tinyxml2::XMLElement *elem, std::string name )
{
  int val;
  tinyxml2::XMLError err = elem->QueryIntAttribute( name.c_str(), &val );
  if ( err != tinyxml2::XMLError::XML_SUCCESS )
  {
    throw MDAL_Status::Err_UnknownFormat;
  }
  return val;
}


static bool checkEqual( const std::string &a, const std::string &b, const std::string &throwErrorMessage = "" )
{
  bool equals = a == b;

  if ( !throwErrorMessage.empty() && !equals )
  {
    MDAL::debug( throwErrorMessage );
    throw MDAL_Status::Err_UnknownFormat;
  }

  return equals;
}

// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////


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
  ret[0] = mHyperSlab[0][0] + indexStart;
  ret[1] = mHyperSlab[0][1];
  return ret;
}

size_t MDAL::XdmfDataset::scalarData( size_t indexStart, size_t count, double *buffer )
{
  assert( group()->isScalar() ); //checked in C API interface

  size_t nValues = mHyperSlab[2][0];
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = { copyValues, 1 };
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  const double *input = values.data();
  memcpy( buffer, input, copyValues * sizeof( double ) );
  return copyValues;
}


size_t MDAL::XdmfDataset::vectorData( size_t indexStart, size_t count, double *buffer )
{
  assert( !group()->isScalar() ); //checked in C API interface

  size_t nValues = mHyperSlab[2][0];
  if ( ( count < 1 ) || ( indexStart >= nValues ) )
    return 0;
  size_t copyValues = std::min( nValues - indexStart, count );

  std::vector<hsize_t> off = offsets( indexStart );
  std::vector<hsize_t> counts = { copyValues, 3};
  std::vector<double> values = mHdf5DatasetValues.readArrayDouble( off, counts );
  const double *input = values.data();
  for ( size_t j = 0; j < count; ++j )
  {
    buffer[ 2 * j] =  input[3 * j] ;
    buffer[ 2 * j + 1 ] =  input[3 * j + 1] ;
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

MDAL::DatasetGroups MDAL::DriverXdmf::parseXdmfXml( )
{
  size_t facesCount = mMesh->facesCount();
  std::map< std::string, std::shared_ptr<MDAL::DatasetGroup> > groups;
  std::map< std::string, std::shared_ptr<HdfFile> > hdfFiles;
  int nTimesteps = 0;

  tinyxml2::XMLDocument xmfFile;
  if ( xmfFile.LoadFile( mDatFile.c_str() ) != tinyxml2::XML_SUCCESS )
  {
    MDAL::debug( "Cannot open xmf file" );
    throw MDAL_Status::Err_UnknownFormat;
  }

  tinyxml2::XMLElement *elem = xmfFile.FirstChildElement( "Xdmf" );
  elem = getCheckChild( elem, "Domain" );
  elem = getCheckChild( elem, "Grid" );

  checkEqual( attribute( elem, "GridType" ), "Collection", "Expecting Collection Grid Type" );
  checkEqual( attribute( elem, "CollectionType" ), "Temporal", "Expecting Temporal Collection Type" );

  for ( tinyxml2::XMLNode *gridNod = elem->FirstChildElement( "Grid" );
        gridNod != nullptr;
        gridNod = gridNod->NextSiblingElement( "Grid" ) )
  {
    ++nTimesteps;
    tinyxml2::XMLElement *timeNod = getCheckChild( gridNod->ToElement(), "Time" );
    double time = queryDoubleAttribute( timeNod, "Value" );

    for ( tinyxml2::XMLNode *scalarNod = gridNod->ToElement()->FirstChildElement( "Attribute" );
          scalarNod != nullptr;
          scalarNod = scalarNod->NextSiblingElement( "Attribute" ) )
    {
      scalarNod = scalarNod->ToElement();
      checkEqual( attribute( scalarNod, "Center" ), "Cell", "Only cell centered data is currently supported" );
      std::string attributeType = attribute( scalarNod, "AttributeType" );
      if ( !checkEqual( attributeType, "Scalar" ) &&
           !checkEqual( attributeType, "Vector" ) )
      {
        MDAL::debug( "Only scalar and vector data are currently supported" );
        throw MDAL_Status::Err_UnknownFormat;
      }
      std::string groupName = attribute( scalarNod, "Name" );
      if ( groupName.empty() )
      {
        MDAL::debug( "Group name cannot be empty" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      tinyxml2::XMLElement *itemNod = getCheckChild( scalarNod->ToElement(), "DataItem" );

      if ( !checkEqual( attribute( itemNod, "ItemType" ), "HyperSlab" ) &&
           !checkEqual( attribute( itemNod, "Type" ), "HyperSlab" ) )
      {
        MDAL::debug( "Expecting HyperSlab ItemType and Type" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      size_t dim = static_cast<size_t>( queryIntAttribute( itemNod, "Dimensions" ) );
      if ( dim != facesCount )
      {
        MDAL::debug( "Dataset dimensions should correspond to the number of mesh elements" );
        throw MDAL_Status::Err_UnknownFormat;
      }
      tinyxml2::XMLElement *slabNod = getCheckChild( itemNod, "DataItem" );
      checkEqual( attribute( slabNod, "Format" ), "XML", "Only XML hyperSlab format supported" );

      std::string slabDimS = attribute( slabNod, "Dimensions" );
      std::stringstream slabDimSS( slabDimS );
      std::vector<int> slabDim;
      int number;
      while ( slabDimSS >> number )
        slabDim.push_back( number );
      if ( slabDim.size() > 2 )
      {
        MDAL::debug( "Only two-dimensional slab array is supported" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      std::string slabS = slabNod->GetText();
      std::stringstream slabSS( slabS );
      MDAL::HyperSlab slab( slabDim[0], std::vector<int>( slabDim[1] ) );
      int i = 0;
      while ( slabSS >> number )
      {
        slab[i / slabDim[0]][i % slabDim[0]] = number;
        i++;
      }
      if ( i != slabDim[0] * slabDim[1] )
      {
        MDAL::debug( "hyperSlab dimensions mismatch" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      tinyxml2::XMLElement *snapshotNod = getCheckSibling( slabNod, "DataItem" );
      checkEqual( attribute( snapshotNod, "Format" ), "HDF", "Only HDF dataset format supported" );
      std::string snapshotDimS = attribute( snapshotNod, "Dimensions" );
      std::stringstream snapshotDimSS( snapshotDimS );
      std::vector<int> snapshotDim;
      while ( snapshotDimSS >> number )
        snapshotDim.push_back( number );
      if ( slabDim.size() > 2 )
      {
        MDAL::debug( "Only two-dimensional hdf array is supported" );
        throw MDAL_Status::Err_UnknownFormat;
      }

      std::string dirName = MDAL::dirName( mDatFile );
      std::string path = snapshotNod->GetText();
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

      std::string hdf5Name = dirName + "/" + chunks[0];
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

      bool isScalar;
      if ( slab[2][1] == 1 )
        isScalar = true;
      else if ( slab[2][1] == 3 )
        isScalar = false;
      else
      {
        MDAL::debug( "Wrong HyperSlab value: second column should terminate by 1 for scalars and 3 for vectors" );
        throw MDAL_Status::Err_UnknownFormat;
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
        group->setIsScalar( isScalar );
        group->setIsOnVertices( false ); //only center-based implemented
        groups[groupName] = group;
      }
      else
      {
        group = groups[groupName];
        if ( group->isScalar() != isScalar )
        {
          MDAL::debug( "Inconsistent groups" );
          throw MDAL_Status::Err_UnknownFormat;
        }
      }

      HdfDataset hdfDataset( hdfFile->id(), chunks[1] );

      std::shared_ptr<MDAL::XdmfDataset> xdmfDataset = std::make_shared<MDAL::XdmfDataset>(
            group.get(),
            slab,
            hdfDataset,
            time
          );

      // This basically forces to load all data to calculate statistics!
      xdmfDataset->setStatistics( MDAL::calculateStatistics( xdmfDataset ) );
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

    grp->setStatistics( MDAL::calculateStatistics( grp ) );
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
  tinyxml2::XMLDocument xmfFile;
  if ( xmfFile.LoadFile( uri.c_str() ) != tinyxml2::XML_SUCCESS )
  {
    return false;
  }
  tinyxml2::XMLElement *elem = xmfFile.FirstChildElement( "Xdmf" );
  if ( !elem )
  {
    return false;
  }
  return attribute( elem, "Version" ) == std::string( "2.0" );
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
