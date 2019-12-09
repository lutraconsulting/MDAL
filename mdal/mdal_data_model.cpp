/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#include "mdal_data_model.hpp"
#include <assert.h>
#include <math.h>
#include <algorithm>
#include "mdal_utils.hpp"

MDAL::Dataset::~Dataset() = default;

MDAL::Dataset::Dataset( MDAL::DatasetGroup *parent )
  : mParent( parent )
{
  assert( mParent );
}

size_t MDAL::Dataset::valuesCount() const
{
  const MDAL_DataLocation location = group()->dataLocation();

  switch ( location )
  {
    case MDAL_DataLocation::DataOnVertices2D: return mesh()->verticesCount();
    case MDAL_DataLocation::DataOnFaces2D: return mesh()->facesCount();
    case MDAL_DataLocation::DataOnVolumes3D: return volumesCount();
    default: return 0;
  }
}

size_t MDAL::Dataset::activeData( size_t, size_t, int * )
{
  assert( !supportsActiveFlag() );
  return 0;
}

MDAL::Statistics MDAL::Dataset::statistics() const
{
  return mStatistics;
}

void MDAL::Dataset::setStatistics( const MDAL::Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DatasetGroup *MDAL::Dataset::group() const
{
  return mParent;
}

MDAL::Mesh *MDAL::Dataset::mesh() const
{
  return mParent->mesh();
}

double MDAL::Dataset::time( Duration::Unit unit ) const
{
  return mTime.value( unit );
}

void MDAL::Dataset::setTime( double time, Duration::Unit unit )
{
  mTime = Duration( time, unit );
}

void MDAL::Dataset::setTime( const MDAL::Duration &time )
{
  mTime = time;
}

bool MDAL::Dataset::supportsActiveFlag() const
{
  return mSupportsActiveFlag;
}

void MDAL::Dataset::setSupportsActiveFlag( bool value )
{
  mSupportsActiveFlag = value;
}

bool MDAL::Dataset::isValid() const
{
  return mIsValid;
}

MDAL::Dataset2D::Dataset2D( MDAL::DatasetGroup *parent )
  : Dataset( parent )
{
}

MDAL::Dataset2D::~Dataset2D() = default;


size_t MDAL::Dataset2D::volumesCount() const { return 0; }

size_t MDAL::Dataset2D::maximumVerticalLevelsCount() const { return 0; }

size_t MDAL::Dataset2D::verticalLevelCountData( size_t, size_t, int * ) { return 0; }

size_t MDAL::Dataset2D::verticalLevelData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset2D::faceToVolumeData( size_t, size_t, int * ) { return 0; }

size_t MDAL::Dataset2D::scalarVolumesData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset2D::vectorVolumesData( size_t, size_t, double * ) { return 0; }

MDAL::Dataset3D::Dataset3D( MDAL::DatasetGroup *parent, size_t volumes, size_t maxVerticalLevelCount )
  : Dataset( parent )
  , mVolumesCount( volumes )
  , mMaximumVerticalLevelsCount( maxVerticalLevelCount )
{
}

MDAL::Dataset3D::~Dataset3D() = default;

size_t MDAL::Dataset3D::volumesCount() const
{
  return mVolumesCount;
}

size_t MDAL::Dataset3D::maximumVerticalLevelsCount() const
{
  return mMaximumVerticalLevelsCount;
}

size_t MDAL::Dataset3D::scalarData( size_t, size_t, double * ) { return 0; }

size_t MDAL::Dataset3D::vectorData( size_t, size_t, double * ) { return 0; }

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri,
                                  const std::string &name )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
  setName( name );
}

std::string MDAL::DatasetGroup::driverName() const
{
  return mDriverName;
}

MDAL::DatasetGroup::~DatasetGroup() = default;

MDAL::DatasetGroup::DatasetGroup( const std::string &driverName,
                                  MDAL::Mesh *parent,
                                  const std::string &uri )
  : mDriverName( driverName )
  , mParent( parent )
  , mUri( uri )
{
  assert( mParent );
}

std::string MDAL::DatasetGroup::getMetadata( const std::string &key )
{
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      return pair.second;
    }
  }
  return std::string();
}

void MDAL::DatasetGroup::setMetadata( const std::string &key, const std::string &val )
{
  bool found = false;
  for ( auto &pair : metadata )
  {
    if ( pair.first == key )
    {
      found = true;
      pair.second = val;
    }
  }
  if ( !found )
    metadata.push_back( std::make_pair( key, val ) );
}

std::string MDAL::DatasetGroup::name()
{
  return getMetadata( "name" );
}

void MDAL::DatasetGroup::setName( const std::string &name )
{
  setMetadata( "name", name );
}

std::string MDAL::DatasetGroup::uri() const
{
  return mUri;
}

MDAL::Statistics MDAL::DatasetGroup::statistics() const
{
  return mStatistics;
}

void MDAL::DatasetGroup::setStatistics( const Statistics &statistics )
{
  mStatistics = statistics;
}

MDAL::DateTime MDAL::DatasetGroup::referenceTime() const
{
  return mReferenceTime;
}

void MDAL::DatasetGroup::setReferenceTime( const DateTime &referenceTime )
{
  mReferenceTime = referenceTime;
}

MDAL::Mesh *MDAL::DatasetGroup::mesh() const
{
  return mParent;
}

size_t MDAL::DatasetGroup::maximumVerticalLevelsCount() const
{
  size_t maxLevels = 0;
  for ( const std::shared_ptr<Dataset> &ds : datasets )
  {
    const size_t maxDsLevels = ds->maximumVerticalLevelsCount();
    if ( maxDsLevels > maxLevels )
      return maxLevels = maxDsLevels;
  }
  return maxLevels;
}

bool MDAL::DatasetGroup::isInEditMode() const
{
  return mInEditMode;
}

void MDAL::DatasetGroup::startEditing()
{
  mInEditMode = true;
}

void MDAL::DatasetGroup::stopEditing()
{
  mInEditMode = false;
}

MDAL_DataLocation MDAL::DatasetGroup::dataLocation() const
{
  return mDataLocation;
}

void MDAL::DatasetGroup::setDataLocation( MDAL_DataLocation dataLocation )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mDataLocation = dataLocation;
}

bool MDAL::DatasetGroup::isScalar() const
{
  return mIsScalar;
}

void MDAL::DatasetGroup::setIsScalar( bool isScalar )
{
  // datasets are initialized (e.g. values array, active array) based
  // on this property. Do not allow to modify later on.
  assert( datasets.empty() );
  mIsScalar = isScalar;
}

MDAL::Mesh::Mesh(
  const std::string &driverName,
  size_t verticesCount,
  size_t facesCount,
  size_t faceVerticesMaximumCount,
  MDAL::BBox extent,
  const std::string &uri )
  : mDriverName( driverName )
  , mVerticesCount( verticesCount )
  , mFacesCount( facesCount )
  , mFaceVerticesMaximumCount( faceVerticesMaximumCount )
  , mExtent( extent )
  , mUri( uri )
{
}

std::string MDAL::Mesh::driverName() const
{
  return mDriverName;
}

MDAL::Mesh::~Mesh() = default;

std::shared_ptr<MDAL::DatasetGroup> MDAL::Mesh::group( const std::string &name )
{
  for ( auto grp : datasetGroups )
  {
    if ( grp->name() == name )
      return grp;
  }
  return std::shared_ptr<MDAL::DatasetGroup>();
}

void MDAL::Mesh::setSourceCrs( const std::string &str )
{
  mCrs = MDAL::trim( str );
}

void MDAL::Mesh::setSourceCrsFromWKT( const std::string &wkt )
{
  setSourceCrs( wkt );
}

void MDAL::Mesh::setSourceCrsFromEPSG( int code )
{
  setSourceCrs( std::string( "EPSG:" ) + std::to_string( code ) );
}

void MDAL::Mesh::setSourceCrsFromPrjFile( const std::string &filename )
{
  const std::string proj = MDAL::readFileToString( filename );
  setSourceCrs( proj );
}

size_t MDAL::Mesh::verticesCount() const
{
  return mVerticesCount;
}

size_t MDAL::Mesh::facesCount() const
{
  return mFacesCount;
}

std::string MDAL::Mesh::uri() const
{
  return mUri;
}

MDAL::BBox MDAL::Mesh::extent() const
{
  return mExtent;
}

std::string MDAL::Mesh::crs() const
{
  return mCrs;
}

size_t MDAL::Mesh::faceVerticesMaximumCount() const
{
  return mFaceVerticesMaximumCount;
}

MDAL::MeshVertexIterator::~MeshVertexIterator() = default;

MDAL::MeshFaceIterator::~MeshFaceIterator() = default;



MDAL::DateTime MDAL::DateTime::fromStandartValue( int year, int month, int day, int hours, int minutes, double seconds )
{
  DateTime dateTime;
  if ( month > 0 && day > 0 && hours >= 0 && minutes >= 0 && seconds >= 0 )
  {
    dateTime.mValid = true;
    DateTimeValues values{year, month, day, hours, minutes, seconds};
    dateTime.setWithGregorianJulianCalendarValues( values );
  }

  return dateTime;
}

std::string MDAL::DateTime::toStandartCalendarISO8601() const
{
  DateTimeValues value = dateTimeGregorianJulianCalendar();
  if ( mValid )
    return toString( value );
  else
    return "none";
}

double MDAL::DateTime::toJulianDay() const
{
  return mJulianTime / 24.0 / 3600 / 1000;
}

MDAL::DateTime &MDAL::DateTime::operator=( const MDAL::DateTime &other )
{
  mJulianTime = other.mJulianTime;
  mValid = other.mValid;
  return *this;
}

MDAL::DateTime MDAL::DateTime::operator+( const MDAL::Duration &duration ) const
{
  if ( !mValid )
    return DateTime();
  return DateTime( mJulianTime + duration.mDuration );
}

MDAL::DateTime &MDAL::DateTime::operator+=( const MDAL::Duration &duration )
{
  if ( !mValid )
    return *this;
  mJulianTime += duration.mDuration;
  return *this;
}

MDAL::DateTime &MDAL::DateTime::operator-=( const MDAL::Duration &duration )
{
  if ( !mValid )
    return *this;
  mJulianTime -= duration.mDuration;
  return *this;
}

MDAL::DateTime MDAL::DateTime::operator-( const MDAL::Duration &duration ) const
{
  if ( !mValid )
    return DateTime();
  return DateTime( mJulianTime - duration.mDuration );
}

bool MDAL::DateTime::operator==( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return true;

  return ( mValid && other.mValid ) && ( mJulianTime == other.mJulianTime );
}

bool MDAL::DateTime::operator!=( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return true;

  return !operator==( other );
}

bool MDAL::DateTime::operator<( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return false;
  return ( mValid && other.mValid ) && ( mJulianTime < other.mJulianTime );
}

bool MDAL::DateTime::operator>( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return false;
  return ( mValid && other.mValid ) && ( mJulianTime > other.mJulianTime );
}

bool MDAL::DateTime::operator>=( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return true;
  return ( mValid && other.mValid ) && ( mJulianTime >= other.mJulianTime );
}

bool MDAL::DateTime::operator<=( const MDAL::DateTime &other ) const
{
  if ( !mValid && !other.mValid )
    return true;
  return ( mValid && other.mValid ) && ( mJulianTime <= other.mJulianTime );
}

MDAL::DateTime::DateTime( int64_t julianTime ): mJulianTime( julianTime )
{}

MDAL::DateTime::DateTimeValues MDAL::DateTime::dateTimeGregorianJulianCalendar() const
{
  //https://fr.wikipedia.org/wiki/Jour_julien
  DateTimeValues values;
  int Z = int( mJulianTime / 24.0 / 3600 / 1000 + 0.5 ); //integer part of julian days count
  double F = mJulianTime / 24.0 / 3600 / 1000 + 0.5 - Z; //fractional part of julian days count;
  int S;
  if ( Z < 2299161 )
    S = Z;
  else
  {
    int alpha = int( ( Z - 1867216.25 ) / 36524.25 );
    S = Z + 1 + alpha - int( alpha / 4 );
  }

  int B = S + 1524;
  int C = int( ( B - 122.1 ) / 365.25 );
  int D = int( 365.25 * C );
  int E = int( ( B - D ) / 30.6001 );

  values.day = B - D - int( 30.6001 * E );
  if ( E < 14 )
    values.month = E - 1;
  else
    values.month = E - 13;

  if ( values.month > 2 )
    values.year = C - 4716;
  else
    values.year = C - 4715;

  values.hours = int( F * 24 );
  F = F / 24 - values.hours;
  values.minutes = int( F * 60 );
  F = F / 60 - values.minutes;
  values.seconds = F * 60;

  return values;
}

void MDAL::DateTime::setWithGregorianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  //https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  int A = values.year / 100;
  int B = A / 4;
  int C = 2 - A + B;
  int E = int( 365.25 * ( values.year + 4716 ) );
  int F = int( 30.6001 * ( values.month + 1 ) );
  double julianDay = C + values.day + E + F - 1524.5;

  mJulianTime = int64_t( julianDay * 24 * 3600 * 1000 +
                         ( values.hours ) * 3600 * 1000 + //the formula set the day with hours at 00h
                         values.minutes * 60 * 1000 +
                         values.seconds * 1000 );
}

void MDAL::DateTime::setWithGregorianJulianCalendarValues( MDAL::DateTime::DateTimeValues values )
{
  //https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html

  int C = 0;
  int Y = values.year;
  int M = values.month;
  int D = values.day;
  if ( M <= 2 )
  {
    Y--;
    M += 12;
  }
  if ( values.year > 1582 ||
       ( values.year == 1582 && ( values.month > 10 || ( values.month == 10 && values.day >= 15 ) ) ) ) //gregorian calendar
  {
    int A = Y / 100;
    int B = A / 4;
    C = 2 - A + B;
  }

  int E = int( 365.25 * ( Y + 4716 ) );
  int F = int( 30.6001 * ( M + 1 ) );
  double julianDay = C + D + E + F - 1524.5;

  mJulianTime = int64_t( julianDay * 24 * 3600 * 1000 +
                         ( values.hours ) * 3600 * 1000 + //the formula set the day with hours at 00h
                         values.minutes * 60 * 1000 +
                         values.seconds * 1000 );
}

std::string MDAL::DateTime::toString( MDAL::DateTime::DateTimeValues values ) const
{
  std::string yearStr = std::to_string( values.year );

  int miliseconds = int( ( values.seconds - int( values.seconds ) ) * 1000 + 0.5 );
  std::string msStr;
  if ( miliseconds > 0 )
  {
    if ( miliseconds < 10 )
      msStr = prependZero( std::to_string( miliseconds ), 3 );
    else if ( miliseconds < 100 )
      msStr = prependZero( std::to_string( miliseconds ), 2 );

    msStr = std::string( "," ).append( msStr );
  }

  std::string strDateTime = prependZero( std::to_string( values.year ), 4 ) + "-" +
                            prependZero( std::to_string( values.month ), 2 ) + "-" +
                            prependZero( std::to_string( values.day ), 2 ) + "T" +
                            prependZero( std::to_string( values.hours ), 2 ) + ":" +
                            prependZero( std::to_string( values.minutes ), 2 ) + ":" +
                            prependZero( std::to_string( int( values.seconds ) ), 2 ) +
                            msStr; /// TODO use another way to translate seconds with fraction aprt

  return strDateTime;
}

MDAL::Duration MDAL::DateTime::operator-( const MDAL::DateTime &other ) const
{
  if ( !mValid || !other.mValid )
    return Duration();
  return Duration( mJulianTime - other.mJulianTime );
}

MDAL::Duration::Duration(): mDuration( 0 )
{}

MDAL::Duration::Duration( double duration, MDAL::Duration::Unit unit )
{
  switch ( unit )
  {
    case MDAL::Duration::milliseconds:
      mDuration = int64_t( duration );
      break;
    case MDAL::Duration::seconds:
      mDuration = int64_t( duration * 1000 );
      break;
    case MDAL::Duration::minutes:
      mDuration = int64_t( duration * 60 * 1000 );
      break;
    case MDAL::Duration::hours:
      mDuration = int64_t( duration * 60 * 60 * 1000 );
      break;
    case MDAL::Duration::days:
      mDuration = int64_t( duration * 24 * 60 * 60 * 1000 );
      break;
    case MDAL::Duration::weeks:
      mDuration = int64_t( duration * 7 * 24 * 60 * 60 * 1000 );
      break;
  }
}

double MDAL::Duration::value( MDAL::Duration::Unit unit ) const
{
  switch ( unit )
  {
    case MDAL::Duration::milliseconds:
      return double( mDuration );
    case MDAL::Duration::seconds:
      return double( mDuration ) / 1000 ;
    case MDAL::Duration::minutes:
      return double( mDuration ) / 60 / 1000 ;
    case MDAL::Duration::hours:
      return double( mDuration ) / 60 / 60 / 1000 ;
    case MDAL::Duration::days:
      return double( mDuration ) / 24 / 60 / 60 / 1000 ;
    case MDAL::Duration::weeks:
      return double( mDuration )  / 7 / 24 / 60 / 60 / 1000;
  }
}

MDAL::Duration::Duration( int64_t ms ): mDuration( ms )
{}
