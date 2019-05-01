/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
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
#include <memory>
#include <algorithm>

#include "mdal_selafin.hpp"
#include "mdal.h"
#include "mdal_utils.hpp"
#include <math.h>

MDAL::DriverSelafin::DriverSelafin():
  Driver( "SELAFIN",
          "Selafin File",
          "*.slf",
          Capability::ReadMesh
        )
{
}

MDAL::DriverSelafin *MDAL::DriverSelafin::create()
{
  return new DriverSelafin();
}

MDAL::DriverSelafin::~DriverSelafin() = default;

static std::string read_string( std::ifstream &in, int len )
{
  std::vector<char> ptr( static_cast<size_t>( len ) );

  in.read( ptr.data(), len );
  if ( !in )
    throw MDAL_Status::Err_UnknownFormat;

  std::string ret = std::string( ptr.data() );
  ret = MDAL::trim( ret );
  return ret;
}

static double read_double( std::ifstream &in, bool streamInFloatPrecision )
{
  double ret;
  unsigned char buffer[8];

  if ( streamInFloatPrecision )
  {
    float ret_f;
    if ( in.read( reinterpret_cast< char * >( &buffer ), 4 ) )
      if ( !in )
        throw MDAL_Status::Err_UnknownFormat;
    if ( MDAL::isNativeLittleEndian() )
    {
      std::reverse( reinterpret_cast< char * >( &buffer ), reinterpret_cast< char * >( &buffer ) + 4 );
    }
    memcpy( reinterpret_cast< char * >( &ret_f ),
            reinterpret_cast< char * >( &buffer ),
            4 );
    ret = static_cast<double>( ret_f );
  }
  else
  {
    if ( in.read( reinterpret_cast< char * >( &buffer ), 8 ) )
      if ( !in )
        throw MDAL_Status::Err_UnknownFormat;
    if ( MDAL::isNativeLittleEndian() )
    {
      std::reverse( reinterpret_cast< char * >( &buffer ), reinterpret_cast< char * >( &buffer ) + 8 );
    }
    memcpy( reinterpret_cast< char * >( &ret ),
            reinterpret_cast< char * >( &buffer ),
            4 );
  }
  return ret;
}


static int read_int( std::ifstream &in )
{
  unsigned char data[4];

  if ( in.read( reinterpret_cast< char * >( &data ), 4 ) )
    if ( !in )
      throw MDAL_Status::Err_UnknownFormat;
  if ( MDAL::isNativeLittleEndian() )
  {
    std::reverse( reinterpret_cast< char * >( &data ), reinterpret_cast< char * >( &data ) + 4 );
  }
  int var;
  memcpy( reinterpret_cast< char * >( &var ),
          reinterpret_cast< char * >( &data ),
          4 );
  return var;
}

static size_t read_sizet( std::ifstream &in )
{
  int var = read_int( in );
  return static_cast<size_t>( var );
}

static void record_boundary( std::ifstream &in )
{
  in.ignore( 4 );
  if ( !in )
    throw MDAL_Status::Err_UnknownFormat;
}

bool MDAL::DriverSelafin::getStreamPrecision( std::ifstream &in )
{
  record_boundary( in );
  std::string title = read_string( in, 72 );
  MDAL_UNUSED( title );
  std::string varType = read_string( in, 8 );
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
    throw MDAL_Status::Err_UnknownFormat;
  }
  record_boundary( in );
  return ret;
}

void MDAL::DriverSelafin::parseFile( std::vector<std::string> &var_names,
                                     double *xOrigin,
                                     double *yOrigin,
                                     size_t *nElem,
                                     size_t *nPoint,
                                     size_t *nPointsPerElem,
                                     std::vector<size_t> &ikle,
                                     std::vector<double> &x,
                                     std::vector<double> &y,
                                     std::vector<timestep_map> &data )
{
  if ( !MDAL::fileExists( mFileName ) )
  {
    throw MDAL_Status::Err_FileNotFound;
  }

  std::ifstream in( mFileName, std::ifstream::in | std::ifstream::binary );
  if ( !in )
    throw MDAL_Status::Err_FileNotFound; // Couldn't open the file

  // get length of file:
  in.seekg( 0, in.end );
  long long length = in.tellg();
  in.seekg( 0, in.beg );

  /* 1 record containing the title of the study (72 characters) and a 8 characters
  string indicating the type of format (SERAFIN or SERAFIND)
  */
  bool streamInFloatPrecision = getStreamPrecision( in );

  /* 1 record containing the two integers NBV(1) and NBV(2) (number of linear
     and quadratic variables, NBV(2) with the value of 0 for Telemac, as
     quadratic values are not saved so far)
  */
  record_boundary( in );
  size_t NBV1 = read_sizet( in );
  int NBV2 = read_int( in );
  MDAL_UNUSED( NBV2 );
  record_boundary( in );

  /* NBV(1) records containing the names and units of each variab
     le (over 32 characters)
  */
  for ( size_t i = 0; i < NBV1; ++i )
  {
    record_boundary( in );
    var_names.push_back( read_string( in, 32 ) );
    record_boundary( in );
  }

  /* 1 record containing the integers table IPARAM (10 integers, of which only
      the 6 are currently being used),

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

  record_boundary( in );

  int IPARAM[11]; // keep the numbering from the document
  for ( int i = 1; i < 11; ++i )
  {
    IPARAM[i] = read_int( in );
  }
  *xOrigin = IPARAM[3];
  *yOrigin = IPARAM[4];

  if ( IPARAM[7] != 0 )
  {
    // would need additional parsing
    throw MDAL_Status::Err_MissingDriver;
  }

  record_boundary( in );


  /*
    if IPARAM (10) = 1: a record containing the computation starting date
  */

  if ( IPARAM[10] == 1 )
  {
    record_boundary( in );
    int dummy_date_data;
    dummy_date_data = read_int( in ); //year
    dummy_date_data = read_int( in ); //month
    dummy_date_data = read_int( in ); //hour
    dummy_date_data = read_int( in ); //minute
    dummy_date_data = read_int( in ); //second
    dummy_date_data = read_int( in ); //milisecond
    MDAL_UNUSED( dummy_date_data )
    record_boundary( in );
  }

  /* 1 record containing the integers NELEM,NPOIN,NDP,1 (number of
     elements, number of points, number of points per element and the value 1)
   */
  record_boundary( in );
  *nElem = read_sizet( in );
  *nPoint = read_sizet( in );
  *nPointsPerElem = read_sizet( in );
  int dummy = read_int( in );
  MDAL_UNUSED( dummy );
  record_boundary( in );

  /* 1 record containing table IKLE (integer array
     of dimension (NDP,NELEM)
     which is the connectivity table.

     Attention: in TELEMAC-2D, the dimensions of this array are (NELEM,NDP))
  */

  record_boundary( in );
  ikle.resize( ( *nElem ) * ( *nPointsPerElem ) );
  for ( size_t i = 0; i < ikle.size(); ++i )
  {
    ikle[i] = read_sizet( in );
    -- ikle[i];  //numbered from 1
  }
  record_boundary( in );

  /* 1 record containing table IPOBO (integer array of dimension NPOIN); the
     value of one element is 0 for an internal point, and
     gives the numbering of boundary points for the others
  */
  record_boundary( in );
  std::vector<int> iPointBoundary( *nPoint );
  for ( size_t i = 0; i < iPointBoundary.size(); ++i )
  {
    iPointBoundary[i] = read_int( in );
  }
  record_boundary( in );

  /* 1 record containing table X (real array of dimension NPOIN containing the
     abscissae of the points)
  */

  record_boundary( in );
  x.resize( *nPoint );
  for ( size_t i = 0; i < x.size(); ++i )
  {
    x[i] = read_double( in, streamInFloatPrecision );
  }
  record_boundary( in );

  /* 1 record containing table Y (real array of dimension NPOIN containing the
     abscissae of the points)
  */

  record_boundary( in );
  y.resize( *nPoint );
  for ( size_t i = 0; i < y.size(); ++i )
  {
    y[i] = read_double( in, streamInFloatPrecision );
  }
  record_boundary( in );

  /* Next, for each time step, the following are found:
     - 1 record containing time T (real),
     - NBV(1)+NBV(2) records containing the results tables for each variable at time
  */
  data.resize( var_names.size() );

  size_t nTimesteps = ( length - in.tellg() ) / ( 12 + ( 4 + ( *nPoint ) * 4 + 4 ) * var_names.size() );
  for ( size_t nT = 0; nT < nTimesteps; ++nT )
  {
    record_boundary( in );
    double time = read_double( in, streamInFloatPrecision );
    record_boundary( in );

    for ( size_t i = 0; i < var_names.size(); ++i )
    {
      record_boundary( in );

      timestep_map &datait = data[i];
      std::vector<double> datavals( *nPoint );
      for ( size_t e = 0; e < datavals.size(); ++e )
      {
        datavals[e] = read_double( in, streamInFloatPrecision );
      }
      datait[time] = datavals;
      record_boundary( in );
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
      throw MDAL_Status::Err_IncompatibleMesh; //we can add it, but it is uncommon for this format
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
      nodes.size(),
      elements.size(),
      3, //Triangles
      computeExtent( nodes ),
      mFileName
    )
  );
  mMesh->faces = elements;
  mMesh->vertices = nodes;
}

void MDAL::DriverSelafin::addData( const std::vector<std::string> &var_names, const std::vector<timestep_map> &data, size_t nPoints, size_t nElems )
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

    std::shared_ptr<DatasetGroup> group = mMesh->group( var_name );
    if ( !group )
    {
      group = std::make_shared< DatasetGroup >(
                mMesh->driverName(),
                mMesh.get(),
                mMesh->uri(),
                var_name
              );
      group->setIsOnVertices( true );
      group->setIsScalar( !is_vector );

      mMesh->datasetGroups.push_back( group );
    }

    size_t i = 0;
    for ( timestep_map::const_iterator it = data[nName].begin(); it != data[nName].end(); ++it, ++i )
    {
      std::shared_ptr<MDAL::MemoryDataset> dataset;
      if ( group->datasets.size() > i )
      {
        dataset = std::dynamic_pointer_cast<MDAL::MemoryDataset>( group->datasets[i] );
      }
      else
      {
        dataset = std::make_shared< MemoryDataset >( group.get() );
        dataset->setTime( it->first );
        group->datasets.push_back( dataset );
      }
      double *values = dataset->values();

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
            values[2 * nP] = val;
          }
          else
          {
            values[2 * nP + 1] = val;
          }
        }
        else
        {
          values[nP] = val;
        }
      }
    }
  }

  // now activate faces and calculate statistics
  for ( auto group : mMesh->datasetGroups )
  {
    for ( auto dataset : group->datasets )
    {
      std::shared_ptr<MDAL::MemoryDataset> dts = std::dynamic_pointer_cast<MDAL::MemoryDataset>( dataset );
      MDAL::activateFaces( mMesh.get(), dts );

      MDAL::Statistics stats = MDAL::calculateStatistics( dataset );
      dataset->setStatistics( stats );
    }

    MDAL::Statistics stats = MDAL::calculateStatistics( group );
    group->setStatistics( stats );
  }
}

bool MDAL::DriverSelafin::canRead( const std::string &uri )
{
  if ( !MDAL::fileExists( uri ) ) return false;

  std::ifstream in( uri, std::ifstream::in | std::ifstream::binary );
  if ( !in ) return false;

  try
  {
    bool streamInFloatPrecision = getStreamPrecision( in );
    MDAL_UNUSED( streamInFloatPrecision );
    return true;
  }
  catch ( MDAL_Status )
  {
    return false;
  }
}

std::unique_ptr<MDAL::Mesh> MDAL::DriverSelafin::load( const std::string &meshFile, MDAL_Status *status )
{
  if ( status ) *status = MDAL_Status::None;
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

  try
  {
    parseFile( var_names,
               &xOrigin,
               &yOrigin,
               &nElems,
               &nPoints,
               &nPointsPerElem,
               ikle,
               x,
               y,
               data );

    createMesh( xOrigin,
                yOrigin,
                nElems,
                nPoints,
                nPointsPerElem,
                ikle,
                x,
                y );

    addData( var_names, data, nPoints, nElems );
  }
  catch ( MDAL_Status error )
  {
    if ( status ) *status = ( error );
    mMesh.reset();
  }

  return std::unique_ptr<Mesh>( mMesh.release() );
}
