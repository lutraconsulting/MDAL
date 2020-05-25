/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <iosfwd>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <memory>
#include <algorithm>

#include "mdal_selafin.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include <math.h>
#include "mdal_logger.hpp"

MDAL::SerafinStreamReader::SerafinStreamReader() = default;

void MDAL::SerafinStreamReader::initialize( const std::string &fileName )
{
  mFileName = fileName;
  if ( !MDAL::fileExists( mFileName ) )
  {
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "Did not find file " + mFileName );
  }

  mIn = std::ifstream( mFileName, std::ifstream::in | std::ifstream::binary );
  if ( !mIn )
    throw MDAL::Error( MDAL_Status::Err_FileNotFound, "File " + mFileName + " could not be open" ); // Couldn't open the file

  // get length of file:
  mIn.seekg( 0, mIn.end );
  mFileSize = mIn.tellg();
  mIn.seekg( 0, mIn.beg );

  mStreamInFloatPrecision = getStreamPrecision();
  mIsNativeLittleEndian = MDAL::isNativeLittleEndian();
}

bool MDAL::SerafinStreamReader::getStreamPrecision( )
{
  ignore_array_length( );
  ignore( 72 );
  std::string varType = read_string_without_length( 8 );
  bool ret;
  if ( varType == "SERAFIN" )
  {
    ret = true;
  }
  else if ( varType == "SERAFIND" )
  {
    ret = false;
  }
  else
  {
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Not found stream precision" );
  }
  ignore_array_length( );
  return ret;
}

std::string MDAL::SerafinStreamReader::read_string( size_t len )
{
  size_t length = read_sizet();
  if ( length != len ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to read string" );
  std::string ret = read_string_without_length( len );
  ignore_array_length();
  return ret;
}

std::vector<double> MDAL::SerafinStreamReader::read_double_arr( size_t len )
{
  size_t length = read_sizet();
  if ( mStreamInFloatPrecision )
  {
    if ( length != len * 4 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading double array" );
  }
  else
  {
    if ( length != len * 8 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading double array" );
  }
  std::vector<double> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_double();
  }
  ignore_array_length();
  return ret;
}

std::vector<double> MDAL::SerafinStreamReader::read_double_arr( const std::streampos &position, size_t offset, size_t len )
{
  std::vector<double> ret( len );
  std::streamoff off;
  if ( mStreamInFloatPrecision )
    off = offset * 4;
  else
    off = offset * 8;

  mIn.seekg( position + off );
  for ( size_t i = 0; i < len; ++i )
    ret[i] = read_double();

  return ret;
}

std::vector<int> MDAL::SerafinStreamReader::read_int_arr( size_t len )
{
  size_t length = read_sizet();
  if ( length != len * 4 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading int array" );
  std::vector<int> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_int();
  }
  ignore_array_length();
  return ret;
}

std::vector<int> MDAL::SerafinStreamReader::read_int_arr( const std::streampos &position, size_t offset, size_t len )
{
  std::vector<int> ret( len );
  std::streamoff off = offset * 8;

  mIn.seekg( position + off );
  for ( size_t i = 0; i < len; ++i )
    ret[i] = read_int();

  return ret;
}

std::vector<size_t> MDAL::SerafinStreamReader::read_size_t_arr( size_t len )
{
  size_t length = read_sizet();
  if ( length != len * 4 ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading sizet array" );
  std::vector<size_t> ret( len );
  for ( size_t i = 0; i < len; ++i )
  {
    ret[i] = read_sizet();
  }
  ignore_array_length();
  return ret;
}

std::string MDAL::SerafinStreamReader::read_string_without_length( size_t len )
{
  std::vector<char> ptr( len );
  mIn.read( ptr.data(), static_cast<int>( len ) );
  if ( !mIn )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open stream for reading string without length" );

  size_t str_length = 0;
  for ( size_t i = len; i > 0; --i )
  {
    if ( ptr[i - 1] != 0x20 )
    {
      str_length = i;
      break;
    }
  }
  std::string ret( ptr.data(), str_length );
  return ret;
}

double MDAL::SerafinStreamReader::read_double( )
{
  double ret;

  if ( mStreamInFloatPrecision )
  {
    float ret_f;
    if ( !readValue( ret_f, mIn, mIsNativeLittleEndian ) )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Reading double failed" );
    ret = static_cast<double>( ret_f );
  }
  else
  {
    if ( !readValue( ret, mIn, mIsNativeLittleEndian ) )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Reading double failed" );
  }
  return ret;
}


int MDAL::SerafinStreamReader::read_int( )
{
  unsigned char data[4];

  if ( mIn.read( reinterpret_cast< char * >( &data ), 4 ) )
    if ( !mIn )
      throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to open stream for reading int" );
  if ( mIsNativeLittleEndian )
  {
    std::reverse( reinterpret_cast< char * >( &data ), reinterpret_cast< char * >( &data ) + 4 );
  }
  int var;
  memcpy( reinterpret_cast< char * >( &var ),
          reinterpret_cast< char * >( &data ),
          4 );
  return var;
}

size_t MDAL::SerafinStreamReader::read_sizet()
{
  int var = read_int( );
  return static_cast<size_t>( var );
}

bool MDAL::SerafinStreamReader::checkIntArraySize( size_t len )
{
  return ( len * 4 == read_sizet() );
}

bool MDAL::SerafinStreamReader::checkDoubleArraySize( size_t len )
{
  if ( mStreamInFloatPrecision )
  {
    return ( len * 4 ) == read_sizet();
  }
  else
  {
    return ( len * 8 ) == read_sizet();
  }
}

size_t MDAL::SerafinStreamReader::remainingBytes()
{
  return static_cast<size_t>( mFileSize - mIn.tellg() );
}

void MDAL::SerafinStreamReader::ignore( int len )
{
  mIn.ignore( len );
  if ( !mIn )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "Unable to ignore characters (invalid stream)" );
}

void MDAL::SerafinStreamReader::ignore_array_length( )
{
  ignore( 4 );
}

// //////////////////////////////
// DRIVER
// //////////////////////////////
MDAL::DriverSelafin::DriverSelafin():
  Driver( "SELAFIN",
          "Selafin File",
          "*.slf",
          Capability::ReadMesh
        ),
  mReader( new SerafinStreamReader )
{
}

MDAL::DriverSelafin *MDAL::DriverSelafin::create()
{
  return new DriverSelafin();
}

MDAL::DriverSelafin::~DriverSelafin() = default;


void MDAL::DriverSelafin::parseFile( std::vector<std::string> &var_names,
                                     std::map<std::string, std::streampos> &streamPositions,
                                     double *xOrigin,
                                     double *yOrigin,
                                     size_t *nElem,
                                     size_t *nPoint,
                                     size_t *nPointsPerElem,
                                     std::vector<size_t> &ikle,
                                     std::vector<double> &x,
                                     std::vector<double> &y,
                                     std::vector<timestep_map> &data,
                                     DateTime &referenceTime )
{

  /* 1 record containing the title of the study (72 characters) and a 8 characters
  string indicating the type of format (SERAFIN or SERAFIND)
  */
  mReader->initialize( mFileName );

  /* 1 record containing the two integers NBV(1) and NBV(2) (number of linear
     and quadratic variables, NBV(2) with the value of 0 for Telemac, as
     quadratic values are not saved so far), numbered from 1 in docs
  */
  std::vector<size_t> nbv = mReader->read_size_t_arr( 2 );

  /* NBV(1) records containing the names and units of each variab
     le (over 32 characters)
  */
  for ( size_t i = 0; i < nbv[0]; ++i )
  {
    var_names.push_back( mReader->read_string( 32 ) );
  }

  /* 1 record containing the integers table IPARAM (10 integers, of which only
      the 6 are currently being used), numbered from 1 in docs

      - if IPARAM (3) != 0: the value corresponds to the x-coordinate of the
      origin of the mesh,

      - if IPARAM (4) != 0: the value corresponds to the y-coordinate of the
      origin of the mesh,

      - if IPARAM (7) != 0: the value corresponds to the number of  planes
      on the vertical (3D computation),

      - if IPARAM (8) != 0: the value corresponds to the number of
      boundary points (in parallel),

      - if IPARAM (9) != 0: the value corresponds to the number of interface
      points (in parallel),

      - if IPARAM(8) or IPARAM(9) != 0: the array IPOBO below is replaced
      by the array KNOLG (total initial number of points). All the other
      numbers are local to the sub-domain, including IKLE
  */
  std::vector<int> params = mReader->read_int_arr( 10 );
  *xOrigin = static_cast<double>( params[2] );
  *yOrigin = static_cast<double>( params[3] );

  if ( params[6] != 0 )
  {
    // would need additional parsing
    throw MDAL::Error( MDAL_Status::Err_MissingDriver, "File " + mFileName + " would need additional parsing" );
  }

  /*
    if IPARAM (10) = 1: a record containing the computation starting date
  */

  if ( params[9] == 1 )
  {
    std::vector<int> datetime = mReader->read_int_arr( 6 );
    referenceTime = DateTime( datetime[0], datetime[1], datetime[2], datetime[3], datetime[4], double( datetime[5] ) );
  }

  /* 1 record containing the integers NELEM,NPOIN,NDP,1 (number of
     elements, number of points, number of points per element and the value 1)
   */
  std::vector<size_t> numbers = mReader->read_size_t_arr( 4 );
  *nElem = numbers[0];
  *nPoint = numbers[1];
  *nPointsPerElem = numbers[2];

  /* 1 record containing table IKLE (integer array
     of dimension (NDP,NELEM)
     which is the connectivity table.

     Attention: in TELEMAC-2D, the dimensions of this array are (NELEM,NDP))
  */
  size_t size = ( *nElem ) * ( *nPointsPerElem );
  if ( ! mReader->checkIntArraySize( size ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading connectivity table" );
  streamPositions["ikle"] = mReader->passThroughIntArray( size );

  //ikle = mReader->read_size_t_arr( ( *nElem ) * ( *nPointsPerElem ) );
//  for ( size_t i = 0; i < ikle.size(); ++i )
//  {
//    -- ikle[i];  //numbered from 1
//  }

  /* 1 record containing table IPOBO (integer array of dimension NPOIN); the
     value of one element is 0 for an internal point, and
     gives the numbering of boundary points for the others
  */
  size = *nPoint;
  if ( ! mReader->checkIntArraySize( size ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading IPOBO table" );
  streamPositions["IPOBO"] = mReader->passThroughIntArray( size );; // +4 to ignore array lenght

  /* 1 record containing table X (real array of dimension NPOIN containing the
     abscisse of the points)
  */
  size = *nPoint;
  if ( ! mReader->checkDoubleArraySize( size ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading abscisse values" );
  streamPositions["abscisse"] = mReader->passThroughDoubleArray( size );

  /* 1 record containing table Y (real array of dimension NPOIN containing the
     ordinate of the points)
  */
  size = *nPoint;
  if ( ! mReader->checkDoubleArraySize( size ) ) throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading abscisse values" );
  streamPositions["ordinate"] = mReader->passThroughDoubleArray( size );


  /* Next, for each time step, the following are found:
     - 1 record containing time T (real),
     - NBV(1)+NBV(2) records containing the results tables for each variable at time
  */
  data.resize( var_names.size() );

  size_t nTimesteps = mReader->remainingBytes() / ( 12 + ( 4 + ( *nPoint ) * 4 + 4 ) * var_names.size() );
  for ( size_t nT = 0; nT < nTimesteps; ++nT )
  {
    std::vector<double> times = mReader->read_double_arr( 1 );
    double time = times[0];

    for ( size_t i = 0; i < var_names.size(); ++i )
    {
      timestep_map &datait = data[i];
      std::vector<double> datavals = mReader->read_double_arr( *nPoint );
      datait[time] = datavals;
    }
  }
}

void MDAL::DriverSelafin::createMesh(
  double xOrigin,
  double yOrigin,
  size_t nElems,
  size_t nPoints,
  size_t nPointsPerElem,
  std::vector<size_t> &ikle,
  std::vector<double> &x,
  std::vector<double> &y )
{
  Vertices nodes( nPoints );
  Vertex *nodesPtr = nodes.data();
  for ( size_t n = 0; n < nPoints; ++n, ++nodesPtr )
  {
    nodesPtr->x = xOrigin + x[n];
    nodesPtr->y = yOrigin + y[n];
  }

  Faces elements( nElems );
  for ( size_t e = 0; e < nElems; ++e )
  {
    if ( nPointsPerElem != 3 )
    {
      throw MDAL::Error( MDAL_Status::Err_IncompatibleMesh, "Creating mesh failed, wrong number of points per element (3)" ); //we can add it, but it is uncommon for this format
    }

    // elemPtr->setId(e);
    elements[e].resize( 3 );
    for ( size_t p = 0; p < 3; p++ )
    {
      size_t val = ikle[e * 3 + p];
      if ( val > nPoints - 1 )
      {
        elements[e][p] = 0;
      }
      else
      {
        elements[e][p] = val;
      }
    }
  }

  mMesh.reset(
    new MemoryMesh(
      "SELAFIN",
      3, //Triangles
      mFileName
    )
  );
}

void MDAL::DriverSelafin::addData( const std::vector<std::string> &var_names,
                                   const std::vector<timestep_map> &data,
                                   size_t nPoints,
                                   const DateTime &referenceTime )
{
  for ( size_t nName = 0; nName < var_names.size(); ++nName )
  {
    std::string var_name( var_names[nName] );
    var_name = MDAL::toLower( MDAL::trim( var_name ) );
    var_name = MDAL::replace( var_name, "/", "" ); // slash is represented as sub-dataset group but we do not have such subdatasets groups in Selafin

    bool is_vector = false;
    bool is_x = true;

    if ( MDAL::contains( var_name, "velocity u" ) || MDAL::contains( var_name, "along x" ) )
    {
      is_vector = true;
      var_name = MDAL::replace( var_name, "velocity u", "velocity" );
      var_name = MDAL::replace( var_name, "along x", "" );
    }
    else if ( MDAL::contains( var_name, "velocity v" ) || MDAL::contains( var_name, "along y" ) )
    {
      is_vector = true;
      is_x =  false;
      var_name =  MDAL::replace( var_name, "velocity v", "velocity" );
      var_name =  MDAL::replace( var_name, "along y", "" );
    }

    if ( MDAL::contains( var_name, "vitesse u" ) || MDAL::contains( var_name, "suivant x" ) )
    {
      is_vector = true;
      var_name = MDAL::replace( var_name, "vitesse u", "vitesse" );
      var_name = MDAL::replace( var_name, "suivant x", "" );
    }
    else if ( MDAL::contains( var_name, "vitesse v" ) || MDAL::contains( var_name, "suivant y" ) )
    {
      is_vector = true;
      is_x =  false;
      var_name =  MDAL::replace( var_name, "vitesse v", "vitesse" );
      var_name =  MDAL::replace( var_name, "suivant y", "" );
    }

    std::shared_ptr<DatasetGroup> group = mMesh->group( var_name );
    if ( !group )
    {
      group = std::make_shared< DatasetGroup >(
                mMesh->driverName(),
                mMesh.get(),
                mMesh->uri(),
                var_name
              );
      group->setDataLocation( MDAL_DataLocation::DataOnVertices );
      group->setIsScalar( !is_vector );

      mMesh->datasetGroups.push_back( group );
    }

    group->setReferenceTime( referenceTime );

    size_t i = 0;
    for ( timestep_map::const_iterator it = data[nName].begin(); it != data[nName].end(); ++it, ++i )
    {
      std::shared_ptr<MDAL::MemoryDataset2D> dataset;
      if ( group->datasets.size() > i )
      {
        dataset = std::dynamic_pointer_cast<MDAL::MemoryDataset2D>( group->datasets[i] );
      }
      else
      {
        dataset = std::make_shared< MemoryDataset2D >( group.get(), true );
        // see https://github.com/lutraconsulting/MDAL/issues/185
        dataset->setTime( it->first, RelativeTimestamp::seconds );
        group->datasets.push_back( dataset );
      }
      for ( size_t nP = 0; nP < nPoints; nP++ )
      {
        double val = it->second.at( nP );
        if ( MDAL::equals( val, 0 ) )
        {
          val = std::numeric_limits<double>::quiet_NaN();
        }
        if ( is_vector )
        {
          if ( is_x )
          {
            dataset->setValueX( nP, val );
          }
          else
          {
            dataset->setValueY( nP, val );
          }
        }
        else
        {
          dataset->setScalarValue( nP, val );
        }
      }
    }
  }

  // now activate faces and calculate statistics
  for ( auto group : mMesh->datasetGroups )
  {
    for ( auto dataset : group->datasets )
    {
      std::shared_ptr<MDAL::MemoryDataset2D> dts = std::dynamic_pointer_cast<MDAL::MemoryDataset2D>( dataset );
//      if ( dts )
//        dts->activateFaces( mMesh.get() );

      MDAL::Statistics stats = MDAL::calculateStatistics( dataset );
      dataset->setStatistics( stats );
    }

    MDAL::Statistics stats = MDAL::calculateStatistics( group );
    group->setStatistics( stats );
  }
}

bool MDAL::DriverSelafin::canReadMesh( const std::string &uri )
{
  if ( !MDAL::fileExists( uri ) ) return false;

  std::ifstream in( uri, std::ifstream::in | std::ifstream::binary );
  if ( !in ) return false;

  // The first four bytes of the file should contain the values (in hexadecimal): 00 00 00 50.
  // This actually indicates the start of a string of length 80 in the file.
  // At position 84 in the file, the eight next bytes should read (in hexadecimal): 00 00 00 50 00 00 00 04.
  unsigned char data[ 92 ];
  in.read( reinterpret_cast< char * >( &data ), 92 );
  if ( !in )
    return false;

  if ( data[0] != 0 || data[1] != 0 ||
       data[2] != 0 || data[3] != 0x50 )
    return false;

  if ( data[84 + 0] != 0 || data[84 + 1] != 0 ||
       data[84 + 2] != 0 || data[84 + 3] != 0x50 ||
       data[84 + 4] != 0 || data[84 + 5] != 0 ||
       data[84 + 6] != 0 || data[84 + 7] != 8 )
    return false;

  return true;
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverSelafin::load( const std::string &meshFile, const std::string & )
{
  MDAL::Log::resetLastStatus();
  mFileName = meshFile;
  mMesh.reset();

  std::vector<std::string> var_names;
  double xOrigin;
  double yOrigin;
  size_t nElems;
  size_t nPoints;
  size_t nPointsPerElem;
  std::vector<size_t> ikle;
  std::vector<double> x;
  std::vector<double> y;
  std::vector<timestep_map> data;
  DateTime referenceTime;
  std::map<std::string, std::streampos> streamPositions;

  try
  {
    parseFile( var_names, streamPositions,
               &xOrigin,
               &yOrigin,
               &nElems,
               &nPoints,
               &nPointsPerElem,
               ikle,
               x,
               y,
               data,
               referenceTime );


//    createMesh( xOrigin,
//                yOrigin,
//                nElems,
//                nPoints,
//                nPointsPerElem,
//                ikle,
//                x,
//                y );

    std::streampos xStart = streamPositions["abscisse"];
    std::streampos yStart = streamPositions["ordinate"];
    std::streampos ikleStart = streamPositions["ikle"];
    mMesh.reset( new MeshSelafin( mFileName, mReader, xStart, yStart, ikleStart, nPoints, nElems, xOrigin, yOrigin ) );

    addData( var_names, data, nPoints, referenceTime );
  }
  catch ( MDAL_Status error )
  {
    MDAL::Log::error( error, name(), "Error while loading file " + meshFile );
    mMesh.reset();
  }
  catch ( MDAL::Error err )
  {
    MDAL::Log::error( err, name() );
    mMesh.reset();
  }
  return std::unique_ptr<Mesh>( mMesh.release() );
}

size_t MDAL::MeshSelafinVertexIterator::next( size_t vertexCount, double *coordinates )
{
  size_t count = std::min( vertexCount, mTotalVerticesCount - mPosition );

  if ( count == 0 )
    return 0;

  std::vector<double> xValues = mReader->read_double_arr( mStartX, mPosition, count );
  std::vector<double> yValues = mReader->read_double_arr( mStartY, mPosition, count );

  if ( xValues.size() != count || yValues.size() != count )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading vertices" );

  for ( size_t i = 0; i < count; ++i )
  {
    coordinates[i * 3] = xValues.at( i ) + mXOrigin;
    coordinates[i * 3 + 1] = yValues.at( i ) + mYOrigin;
    coordinates[i * 3 + 2] = 0;
  }

  mPosition += count;

  return count;
}

size_t MDAL::MeshSelafinEdgeIterator::next( size_t edgeCount, int *startVertexIndices, int *endVertexIndices )
{
  MDAL_UNUSED( edgeCount )
  MDAL_UNUSED( startVertexIndices )
  MDAL_UNUSED( endVertexIndices )
  return 0;
}

size_t MDAL::MeshSelafinFaceIterator::next( size_t faceOffsetsBufferLen, int *faceOffsetsBuffer, size_t vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  assert( faceOffsetsBuffer );
  assert( vertexIndicesBuffer );
  assert( vertexIndicesBufferLen == 3 * faceOffsetsBufferLen );

  size_t count = std::min( faceOffsetsBufferLen, mTotalFacesCount - mPosition );

  if ( count == 0 )
    return 0;

  std::vector<int> indexes = mReader->read_int_arr( mStart, mPosition * 3, count * 3 );

  if ( indexes.size() != count * 3 )
    throw MDAL::Error( MDAL_Status::Err_UnknownFormat, "File format problem while reading faces" );

  int vertexLocalIndex = 0;

  for ( size_t i = 0; i < count; i++ )
  {
    for ( size_t j = 0; j < 3; ++j )
      vertexIndicesBuffer[vertexLocalIndex + j] = indexes[j + i * 3] - 1;
    vertexLocalIndex += 3;
    faceOffsetsBuffer[i] = vertexLocalIndex;
  }

  mPosition += count;

  return count;

}

std::unique_ptr<MDAL::MeshVertexIterator> MDAL::MeshSelafin::readVertices()
{
  return std::unique_ptr<MDAL::MeshVertexIterator>( new MeshSelafinVertexIterator(
           mReader, mXVerticesStart, mYVerticesStart, mVerticesCount, mXOrigin, mYOrigin ) );
}

std::unique_ptr<MDAL::MeshEdgeIterator> MDAL::MeshSelafin::readEdges()
{
  return std::unique_ptr<MDAL::MeshEdgeIterator>( new MeshSelafinEdgeIterator );
}

std::unique_ptr<MDAL::MeshFaceIterator> MDAL::MeshSelafin::readFaces()
{
  return std::unique_ptr<MDAL::MeshFaceIterator>( new MeshSelafinFaceIterator( mReader, mIkleTableStart, mFacesCount ) );
}

MDAL::BBox MDAL::MeshSelafin::extent() const
{
  if ( mIsExtentUpToDate )
    return mExtent;
  calculateExtent();
  return mExtent;
}

void MDAL::MeshSelafin::calculateExtent() const
{
  size_t bufferSize = 1000;
  std::unique_ptr<MeshSelafinVertexIterator> vertexIt(
    new MeshSelafinVertexIterator( mReader, mXVerticesStart, mYVerticesStart, mVerticesCount, mXOrigin, mYOrigin ) );
  size_t count = 0;
  BBox bbox;
  std::vector<Vertex> vertices( mVerticesCount );
  size_t index = 0;
  do
  {
    std::vector<double> verticesCoord( bufferSize * 3 );
    count = vertexIt->next( bufferSize, verticesCoord.data() );

    for ( size_t i = 0; i < count; ++i )
    {
      vertices[index + i].x = verticesCoord.at( i * 3 );
      vertices[index + i].y = verticesCoord.at( i * 3 + 1 );
      vertices[index + i].z = verticesCoord.at( i * 3 + 2 );
    }
    index += count;
  }
  while ( count == 0 );

  mExtent = MDAL::computeExtent( vertices );
  mIsExtentUpToDate = true;
}
