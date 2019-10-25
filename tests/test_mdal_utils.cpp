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


int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

