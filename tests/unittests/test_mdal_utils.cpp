/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <limits>
#include <cmath>
#include <string>
#include <vector>

//mdal
#include "mdal.h"
#include "mdal_utils.hpp"
#include "mdal_testutils.hpp"

struct SplitTestData
{
  SplitTestData( const std::string &input,
                 const std::vector<std::string> &results ):
    mInput( input ), mExpectedResult( results ) {}

  std::string mInput;
  std::vector<std::string> mExpectedResult;
};


TEST( MdalUtilsTest, SplitString )
{
  std::vector<SplitTestData> tests =
  {
    SplitTestData( "a;b;c", {"a", "b", "c"} ),
    SplitTestData( "a;;b;c", {"a", "b", "c"} ),
    SplitTestData( "a;b;", {"a", "b"} ),
    SplitTestData( ";b;", {"b"} ),
    SplitTestData( "a", {"a"} ),
    SplitTestData( "", {} )
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.mExpectedResult, MDAL::split( test.mInput, ";" ) );
  }

  // now test for string with multiple chars
  std::vector<SplitTestData> tests2 =
  {
    SplitTestData( "a;;;b;c", {"a", "b;c"} ),
    SplitTestData( "a;;;b;;;c", {"a", "b", "c"} ),
    SplitTestData( "a;;b;c", {"a;;b;c"} ),
    SplitTestData( "b;;;", {"b"} )
  };
  for ( const auto &test : tests2 )
  {
    EXPECT_EQ( test.mExpectedResult, MDAL::split( test.mInput, ";;;" ) );
  }
}

TEST( MdalUtilsTest, SplitChar )
{
  std::vector<SplitTestData> tests =
  {
    SplitTestData( "a;b;c", {"a", "b", "c"} ),
    SplitTestData( "a;;b;c", {"a", "b", "c"} ),
    SplitTestData( "a;b;", {"a", "b"} ),
    SplitTestData( ";b;", {"b"} ),
    SplitTestData( "a", {"a"} ),
    SplitTestData( "", {} )
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.mExpectedResult, MDAL::split( test.mInput, ';' ) );
  }
}

TEST( MdalUtilsTest, TimeParsing )
{
  std::vector<std::pair<std::string, double>> tests =
  {
    { "seconds since 2001-05-05 00:00:00", 3600 },
    { "minutes since 2001-05-05 00:00:00", 60 },
    { "hours since 1900-01-01 00:00:0.0", 1 },
    { "hours", 1 },
    { "days since 1961-01-01 00:00:00", 1.0 / 24.0 },
    { "invalid format of time", 1 }
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::parseTimeUnits( test.first ) );
  }
}

TEST( MdalUtilsTest, CF_TimeUnitParsing )
{
  std::vector<std::pair<std::string, MDAL::RelativeTimestamp::Unit>> tests =
  {
    { "seconds since 2001-05-05 00:00:00", MDAL::RelativeTimestamp::seconds },
    { "minutes since 2001-05-05 00:00:00", MDAL::RelativeTimestamp::minutes },
    { "hours since 1900-01-01 00:00:0.0", MDAL::RelativeTimestamp::hours },
    { "days since 1961-01-01 00:00:00", MDAL::RelativeTimestamp::days },
    { "weeks since 1961-01-01 00:00:00", MDAL::RelativeTimestamp::weeks },
    { "month since 1961-01-01 00:00:00", MDAL::RelativeTimestamp::months_CF },
    { "months since 1961-01-01 00:00:00", MDAL::RelativeTimestamp::months_CF },
    { "year since 1961-01-01 00:00:00", MDAL::RelativeTimestamp::exact_years },
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::parseCFTimeUnit( test.first ) );
  }
}

TEST( MdalUtilsTest, CF_ReferenceTimePArsing )
{
  std::vector<std::pair<std::string, MDAL::DateTime>> tests =
  {
    { "seconds since 2001-05-05 00:00:00", MDAL::DateTime( 2001, 5, 5, 00, 00, 00 ) },
    { "hours since 1900-05-05 10:00:0.0", MDAL::DateTime( 1900, 5, 5, 10, 00, 00 ) },
    { "days since 1200-05-05 00:05:00", MDAL::DateTime( 1200, 5, 5, 00, 5, 00 ) },
    { "weeks since 1961-05-05 00:01:10", MDAL::DateTime( 1961, 5, 5, 00, 1, 10 ) },
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::parseCFReferenceTime( test.first, "gregorian" ) );
  }
}

