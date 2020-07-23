/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"
#include <limits>
#include <cmath>
#include <string>
#include <vector>
#include <fstream>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"
#include "mdal_datetime.hpp"


TEST( MdalDateTimeTest, Duration )
{
  std::vector<std::pair<MDAL::RelativeTimestamp, MDAL::RelativeTimestamp>> tests =
  {
    { MDAL::RelativeTimestamp( 2, MDAL::RelativeTimestamp::minutes ), MDAL::RelativeTimestamp( 120, MDAL::RelativeTimestamp::seconds ) },
    { MDAL::RelativeTimestamp( 90, MDAL::RelativeTimestamp::minutes ), MDAL::RelativeTimestamp( 1.5, MDAL::RelativeTimestamp::hours ) },
    { MDAL::RelativeTimestamp( 2, MDAL::RelativeTimestamp::weeks ), MDAL::RelativeTimestamp( 336, MDAL::RelativeTimestamp::hours ) },
    { MDAL::RelativeTimestamp( 90, MDAL::RelativeTimestamp::seconds ), MDAL::RelativeTimestamp( 1.5, MDAL::RelativeTimestamp::minutes ) },
    { MDAL::RelativeTimestamp( 36, MDAL::RelativeTimestamp::hours ), MDAL::RelativeTimestamp( 1.5, MDAL::RelativeTimestamp::days ) },
  };
  for ( const std::pair<MDAL::RelativeTimestamp, MDAL::RelativeTimestamp> &test : tests )
  {
    EXPECT_EQ( test.first, test.second );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::milliseconds ), test.second.value( MDAL::RelativeTimestamp::milliseconds ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::seconds ), test.second.value( MDAL::RelativeTimestamp::seconds ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::minutes ), test.second.value( MDAL::RelativeTimestamp::minutes ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::hours ), test.second.value( MDAL::RelativeTimestamp::hours ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::days ), test.second.value( MDAL::RelativeTimestamp::days ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::weeks ), test.second.value( MDAL::RelativeTimestamp::weeks ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::months_CF ), test.second.value( MDAL::RelativeTimestamp::months_CF ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::RelativeTimestamp::exact_years ), test.second.value( MDAL::RelativeTimestamp::exact_years ) );

    EXPECT_TRUE( test.first == test.second ) ;
  }
}

TEST( MdalDateTimeTest, DateTime )
{
  std::vector<std::pair<MDAL::DateTime, MDAL::DateTime>> dateTests =
  {
    { MDAL::DateTime(), MDAL::DateTime()},
    { MDAL::DateTime( 2019, 02, 28, 10, 2, 1, MDAL::DateTime::Gregorian ), MDAL::DateTime( 1551348121, MDAL::DateTime::Unix ) },
    { MDAL::DateTime( 2019, 02, 28, 10, 0, 0, MDAL::DateTime::Gregorian ), MDAL::DateTime( 2458542.916666666667, MDAL::DateTime::JulianDay ) },
    { MDAL::DateTime( 2457125.5, MDAL::DateTime::JulianDay ), MDAL::DateTime( 2015, 04, 13, 00, 0, 0, MDAL::DateTime::Gregorian ) },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1425, 01, 02, 12, 0, 0, MDAL::DateTime::ProlepticGregorian ) },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1424, 12, 24, 12, 0, 0, MDAL::DateTime::Julian ) },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1424, 12, 24, 12, 0, 0, MDAL::DateTime::Gregorian ) },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::RelativeTimestamp( 24, MDAL::RelativeTimestamp::hours ), MDAL::DateTime( 1424, 12, 25, 12, 0, 0, MDAL::DateTime::Gregorian ) },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::RelativeTimestamp( 240, MDAL::RelativeTimestamp::minutes ), MDAL::DateTime( 1424, 12, 24, 16, 0, 0, MDAL::DateTime::Gregorian ) },
  };

  for ( const std::pair<MDAL::DateTime, MDAL::DateTime> &test : dateTests )
  {
    EXPECT_EQ( test.first, test.second );

    if ( test.first.isValid() && test.second.isValid() )
    {
      EXPECT_TRUE( test.first < test.second + MDAL::RelativeTimestamp( 2, MDAL::RelativeTimestamp::hours ) );
    }

  }

  std::vector<std::pair<MDAL::DateTime, std::string>> dateStringTests =
  {
    { MDAL::DateTime(), "" },
    { MDAL::DateTime( 2019, 02, 28, 10, 2, 1, MDAL::DateTime::Gregorian ), "2019-02-28T10:02:01" },
    { MDAL::DateTime( 2457125.5, MDAL::DateTime::JulianDay ), "2015-04-13T00:00:00" },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), "1425-01-02T12:00:00" },
    {
      MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::RelativeTimestamp( 26, MDAL::RelativeTimestamp::hours )
      + MDAL::RelativeTimestamp( 20, MDAL::RelativeTimestamp::minutes ) + MDAL::RelativeTimestamp( 25, MDAL::RelativeTimestamp::seconds ), "1425-01-03T14:20:25"
    },
    { MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) - MDAL::RelativeTimestamp( 240, MDAL::RelativeTimestamp::minutes ), "1425-01-02T08:00:00" },
  };

  for ( const std::pair<MDAL::DateTime, std::string> &test : dateStringTests )
  {
    EXPECT_EQ( test.first.toStandardCalendarISO8601(), test.second );
  }
}

TEST( MdalDateTimeTest, ConvertJulianDay )
{
  std::string path = test_file( "/datetime/julianDay.txt" );
  std::ifstream stream( path, std::ifstream::in );

  EXPECT_TRUE( stream.is_open() );

  while ( !stream.eof() )
  {
    std::string line;
    std::getline( stream, line );

    std::vector<std::string> dates = MDAL::split( line, ' ' );

    if ( dates.size() != 2 )
      continue;
    std::string stringDate = dates[0];
    double julianDay = MDAL::toDouble( dates[1] );

    MDAL::DateTime dateTime( julianDay, MDAL::DateTime::JulianDay );

    EXPECT_EQ( stringDate, dateTime.toStandardCalendarISO8601() );
  }
}

TEST( MdalDateTimeTest, ConvertFromISO8601 )
{
  std::string timeISO8601 = "1900-02-01T01:02Z";

  MDAL::DateTime dateTimeFromStr( timeISO8601 );
  MDAL::DateTime dateTime( 1900, 2, 1, 1, 2 );

  EXPECT_EQ( dateTimeFromStr, dateTime );

  timeISO8601 = "1900-02-01T01:02:59Z";
  dateTimeFromStr = MDAL::DateTime( timeISO8601 );
  dateTime = MDAL::DateTime( 1900, 2, 1, 1, 2, 59 );

  EXPECT_EQ( dateTimeFromStr, dateTime );


  timeISO8601 = "1900-02-01T01:02:59.789Z";
  dateTimeFromStr = MDAL::DateTime( timeISO8601 );
  dateTime = MDAL::DateTime( 1900, 2, 1, 1, 2, 59.789 );

  EXPECT_EQ( dateTimeFromStr, dateTime );
}
