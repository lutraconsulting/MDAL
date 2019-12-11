#include "mdal_date_time.hpp"
#include "mdal_utils.hpp"


constexpr double MILLISECONDS_IN_SECOND = 1000;
constexpr double MILLISECONDS_IN_MINUTE = 1000 * 60;
constexpr double MILLISECONDS_IN_HOUR = 1000 * 60 * 60;
constexpr double MILLISECONDS_IN_DAY = 1000 * 60 * 60 * 24;
constexpr double MILLISECONDS_IN_WEEK = 1000 * 60 * 60 * 24 * 7;

//https://www.unidata.ucar.edu/software/netcdf-java/current/CDM/CalendarDateTime.html
constexpr double MILLISECONDS_IN_EXACT_YEAR = 3.15569259747e10; //CF Compliant
constexpr double MILLISECONDS_IN_MONTH_CF = MILLISECONDS_IN_EXACT_YEAR / 12; //CF Compliant

constexpr double SECONDS_IN_DAY = 60 * 60 * 24;
constexpr double MINUTES_IN_DAY = 60 * 24;
constexpr double HOURS_IN_DAY = 24;


MDAL::DateTime::DateTime(): mValid( false )
{}

MDAL::DateTime::DateTime( const MDAL::DateTime &other ): mJulianTime( other.mJulianTime ), mValid( other.mValid )
{}

MDAL::DateTime::DateTime( int year, int month, int day, int hours, int minutes, double seconds, MDAL::DateTime::Calendar calendar )
{
  DateTimeValues value{year, month, day, hours, minutes, seconds};

  switch ( calendar )
  {
    case MDAL::DateTime::Gregorian:
      setWithGregorianJulianCalendarDate( value );
      break;
    case MDAL::DateTime::Proleptic_Gregorian:
      setWithGregorianCalendarDate( value );
      break;
    case MDAL::DateTime::Julian:
      setWithJulianCalendarDate( value );
      break;
  }
}

MDAL::DateTime::DateTime( double value, Epoch epoch ):
  mValid( true )
{
  switch ( epoch )
  {
    case MDAL::DateTime::Unix:
      mJulianTime = ( DateTime( 1970, 01, 01, 0, 0, 0, Gregorian ) + Duration( value, Duration::seconds ) ).mJulianTime;
      break;
    case MDAL::DateTime::JulianDay:
      mJulianTime = int64_t( value * MILLISECONDS_IN_DAY );
      break;
  }
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
  return mJulianTime / MILLISECONDS_IN_DAY;
}

std::string MDAL::DateTime::toJulianDayString() const
{
  return std::to_string( toJulianDay() ); ///TODO : maybe change the precison
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

bool MDAL::DateTime::isValid() const {return mValid;}

MDAL::DateTime::DateTime( int64_t julianTime ): mJulianTime( julianTime )
{}

MDAL::DateTime::DateTimeValues MDAL::DateTime::dateTimeGregorianJulianCalendar() const
{
  //https://fr.wikipedia.org/wiki/Jour_julien
  DateTimeValues values;
  int Z = int( mJulianTime / MILLISECONDS_IN_DAY  + 0.5 ); //integer part of julian days count
  double F = mJulianTime / MILLISECONDS_IN_DAY + 0.5 - Z; //fractional part of julian days count;
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

  values.hours = int( F * HOURS_IN_DAY );
  F = F - values.hours / HOURS_IN_DAY;
  values.minutes = int( F * MINUTES_IN_DAY );
  F = F  - values.minutes / MINUTES_IN_DAY;
  values.seconds = F * SECONDS_IN_DAY;

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

  mValid = true;
  mJulianTime = int64_t( julianDay * MILLISECONDS_IN_DAY +
                         ( values.hours ) * MILLISECONDS_IN_HOUR + //the formula set the day with hours at 00h
                         values.minutes * MILLISECONDS_IN_MINUTE +
                         values.seconds * MILLISECONDS_IN_SECOND );
}

void MDAL::DateTime::setWithJulianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  //https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html
  if ( values.month <= 2 )
  {
    values.year--;
    values.month += 12;
  }

  int E = int( 365.25 * ( values.year + 4716 ) );
  int F = int( 30.6001 * ( values.month + 1 ) );
  double julianDay = values.day + E + F - 1524.5;

  mValid = true;
  mJulianTime = int64_t( julianDay * MILLISECONDS_IN_DAY +
                         ( values.hours ) * MILLISECONDS_IN_HOUR + //the formula set the day with hours at 00h
                         values.minutes * MILLISECONDS_IN_MINUTE +
                         values.seconds * MILLISECONDS_IN_SECOND );
}

void MDAL::DateTime::setWithGregorianJulianCalendarDate( MDAL::DateTime::DateTimeValues values )
{
  //https://quasar.as.utexas.edu/BillInfo/JulianDatesG.html

  mValid = true;

  if ( values.year > 1582 ||
       ( values.year == 1582 && ( values.month > 10 || ( values.month == 10 && values.day >= 15 ) ) ) ) //gregorian calendar
  {
    setWithGregorianCalendarDate( values );
  }
  else
    setWithJulianCalendarDate( values );
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
                            msStr; /// TODO use another way to translate seconds with fraction part

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
      mDuration = int64_t( duration * MILLISECONDS_IN_SECOND );
      break;
    case MDAL::Duration::minutes:
      mDuration = int64_t( duration * MILLISECONDS_IN_MINUTE );
      break;
    case MDAL::Duration::hours:
      mDuration = int64_t( duration * MILLISECONDS_IN_HOUR );
      break;
    case MDAL::Duration::days:
      mDuration = int64_t( duration * MILLISECONDS_IN_DAY );
      break;
    case MDAL::Duration::weeks:
      mDuration = int64_t( duration * MILLISECONDS_IN_WEEK );
      break;
    case MDAL::Duration::months_CF:
      mDuration = int64_t( duration * MILLISECONDS_IN_MONTH_CF );
      break;
    case MDAL::Duration::exact_years:
      mDuration = int64_t( duration * MILLISECONDS_IN_EXACT_YEAR );
      break;
  }
}

MDAL::Duration::Duration( const MDAL::Duration &other )
{
  mDuration = other.mDuration;
}

double MDAL::Duration::value( MDAL::Duration::Unit unit ) const
{
  switch ( unit )
  {
    case MDAL::Duration::milliseconds:
      return double( mDuration );
    case MDAL::Duration::seconds:
      return mDuration / MILLISECONDS_IN_SECOND;
    case MDAL::Duration::minutes:
      return mDuration  / MILLISECONDS_IN_MINUTE;
    case MDAL::Duration::hours:
      return mDuration / MILLISECONDS_IN_HOUR;
    case MDAL::Duration::days:
      return double( mDuration ) / MILLISECONDS_IN_DAY;
    case MDAL::Duration::weeks:
      return double( mDuration )  / MILLISECONDS_IN_WEEK;
    case MDAL::Duration::months_CF:
      return double( mDuration ) / MILLISECONDS_IN_MONTH_CF;
    case MDAL::Duration::exact_years:
      return double( mDuration )  / MILLISECONDS_IN_EXACT_YEAR;
  }
}

MDAL::Duration &MDAL::Duration::operator=( const MDAL::Duration &other )
{
  mDuration = other.mDuration;
  return *this;
}

MDAL::Duration MDAL::Duration::operator-( const MDAL::Duration &other ) const
{
  return Duration( mDuration - other.mDuration );
}

MDAL::Duration MDAL::Duration::operator+( const MDAL::Duration &other ) const
{
  return Duration( mDuration + other.mDuration );
}

MDAL::Duration &MDAL::Duration::operator+=( const MDAL::Duration &other )
{
  mDuration += other.mDuration;
  return *this;
}

MDAL::Duration &MDAL::Duration::operator-=( const MDAL::Duration &other )
{
  mDuration -= other.mDuration;
  return *this;
}

bool MDAL::Duration::operator==( const MDAL::Duration &other ) const
{
  return mDuration == other.mDuration;
}

bool MDAL::Duration::operator!=( const MDAL::Duration &other ) const
{
  return mDuration != other.mDuration;
}

bool MDAL::Duration::operator<( const MDAL::Duration &other ) const
{
  return mDuration < other.mDuration;
}

bool MDAL::Duration::operator>( const MDAL::Duration &other ) const
{
  return mDuration > other.mDuration;
}

bool MDAL::Duration::operator>=( const MDAL::Duration &other ) const
{
  return mDuration >= other.mDuration;
}

bool MDAL::Duration::operator<=( const MDAL::Duration &other ) const
{
  return mDuration <= other.mDuration;
}

MDAL::Duration::Duration( int64_t ms ): mDuration( ms )
{}
