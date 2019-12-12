/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"
#include <limits>
#include <cmath>
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_date_time.hpp"


TEST( MdalDateTimeTest, Duration )
{
  std::vector<std::pair<MDAL::Duration, MDAL::Duration>> tests =
  {
    { MDAL::Duration( 2, MDAL::Duration::minutes ), MDAL::Duration( 120, MDAL::Duration::seconds ) },
    { MDAL::Duration( 90, MDAL::Duration::minutes ), MDAL::Duration( 1.5, MDAL::Duration::hours ) },
    { MDAL::Duration( 2, MDAL::Duration::weeks ), MDAL::Duration( 336, MDAL::Duration::hours ) },
    { MDAL::Duration( 90, MDAL::Duration::seconds ), MDAL::Duration( 1.5, MDAL::Duration::minutes ) },
    { MDAL::Duration( 36, MDAL::Duration::hours ), MDAL::Duration( 1.5, MDAL::Duration::days ) },

    { MDAL::Duration( 150, MDAL::Duration::minutes ), MDAL::Duration( 1.5, MDAL::Duration::hours ) + MDAL::Duration( 1, MDAL::Duration::hours )},
    { MDAL::Duration( 8, MDAL::Duration::days ), MDAL::Duration( 1, MDAL::Duration::weeks ) + MDAL::Duration( 24, MDAL::Duration::hours )},
    { MDAL::Duration( 6, MDAL::Duration::days ), MDAL::Duration( 1, MDAL::Duration::weeks ) - MDAL::Duration( 24, MDAL::Duration::hours )}
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.first, test.second );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::milliseconds ), test.second.value( MDAL::Duration::milliseconds ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::seconds ), test.second.value( MDAL::Duration::seconds ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::minutes ), test.second.value( MDAL::Duration::minutes ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::hours ), test.second.value( MDAL::Duration::hours ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::days ), test.second.value( MDAL::Duration::days ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::weeks ), test.second.value( MDAL::Duration::weeks ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::months_CF ), test.second.value( MDAL::Duration::months_CF ) );
    EXPECT_DOUBLE_EQ( test.first.value( MDAL::Duration::exact_years ), test.second.value( MDAL::Duration::exact_years ) );

    EXPECT_TRUE( test.first == test.second ) ;
    EXPECT_TRUE( test.first != test.second + MDAL::Duration( 1, MDAL::Duration::seconds ) );
    EXPECT_TRUE( test.first < test.second + MDAL::Duration( 1, MDAL::Duration::seconds ) );
    EXPECT_TRUE( test.first <= test.second + MDAL::Duration( 1, MDAL::Duration::seconds ) );
    EXPECT_TRUE( test.first + MDAL::Duration( 1, MDAL::Duration::seconds ) > test.second );
    EXPECT_TRUE( test.first + MDAL::Duration( 1, MDAL::Duration::seconds ) >= test.second );
  }
}


TEST( MdalDateTimeTest, DateTime )
{
  std::vector<std::pair<MDAL::DateTime, MDAL::DateTime>> dateTests =
  {
    {MDAL::DateTime(), MDAL::DateTime()},
    {MDAL::DateTime( 2019, 02, 28, 10, 2, 1, MDAL::DateTime::Gregorian ), MDAL::DateTime( 1551348121, MDAL::DateTime::Unix )},
    {MDAL::DateTime( 2019, 02, 28, 10, 0, 0, MDAL::DateTime::Gregorian ), MDAL::DateTime( 2458542.916666666667, MDAL::DateTime::JulianDay )},
    {MDAL::DateTime( 2457125.5, MDAL::DateTime::JulianDay ), MDAL::DateTime( 2015, 04, 13, 00, 0, 0, MDAL::DateTime::Gregorian )},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1425, 01, 02, 12, 0, 0, MDAL::DateTime::Proleptic_Gregorian )},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1424, 12, 24, 12, 0, 0, MDAL::DateTime::Julian )},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), MDAL::DateTime( 1424, 12, 24, 12, 0, 0, MDAL::DateTime::Gregorian )},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::Duration( 24, MDAL::Duration::hours ), MDAL::DateTime( 1424, 12, 25, 12, 0, 0, MDAL::DateTime::Gregorian )},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::Duration( 240, MDAL::Duration::minutes ), MDAL::DateTime( 1424, 12, 24, 16, 0, 0, MDAL::DateTime::Gregorian )},
  };

  for ( const auto &test : dateTests )
  {
    EXPECT_EQ( test.first, test.second );

    if ( test.first.isValid() && test.second.isValid() )
    {
      EXPECT_TRUE( test.first < test.second + MDAL::Duration( 2, MDAL::Duration::hours ) );
      EXPECT_TRUE( test.first <= test.second + MDAL::Duration( 2, MDAL::Duration::hours ) );
      EXPECT_TRUE( test.first + MDAL::Duration( 2, MDAL::Duration::hours ) > test.second );
      EXPECT_TRUE( test.first + MDAL::Duration( 2, MDAL::Duration::hours ) >= test.second );
    }

  }

  std::vector<std::pair<MDAL::DateTime, std::string>> dateStringTests =
  {
    {MDAL::DateTime(), ""},
    {MDAL::DateTime( 2019, 02, 28, 10, 2, 1, MDAL::DateTime::Gregorian ), "2019-02-28T10:02:01"},
    {MDAL::DateTime( 2457125.5, MDAL::DateTime::JulianDay ), "2015-04-13T00:00:00"},
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ), "1424-12-24T12:00:00"},
    {
      MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) + MDAL::Duration( 26, MDAL::Duration::hours )
      + MDAL::Duration( 20, MDAL::Duration::minutes ) + MDAL::Duration( 25, MDAL::Duration::seconds ), "1424-12-25T14:20:25"
    },
    {MDAL::DateTime( 2241532, MDAL::DateTime::JulianDay ) - MDAL::Duration( 240, MDAL::Duration::minutes ), "1424-12-24T08:00:00"},
  };

  for ( const auto &test : dateStringTests )
  {
    EXPECT_EQ( test.first.toStandartCalendarISO8601(), test.second );
  }
}



int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

