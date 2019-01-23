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

struct StringTestData
{
  StringTestData( const std::string &input,
                  const std::vector<std::string> &results ):
    mInput( input ), mExpectedResult( results ) {}

  std::string mInput;
  std::vector<std::string> mExpectedResult;
};


TEST( MdalUtilsTest, Split )
{
  std::vector<StringTestData> tests =
  {
    StringTestData( "a;b;c", {"a", "b", "c"} ),
    StringTestData( "a;;b;c", {"a", "b", "c"} ),
    StringTestData( "a;b;", {"a", "b"} ),
    StringTestData( ";b;", {"b"} ),
    StringTestData( "a", {"a"} ),
    StringTestData( "", {} )
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.mExpectedResult, MDAL::split( test.mInput, ';' ) );
    EXPECT_EQ( test.mExpectedResult, MDAL::split( test.mInput, ";" ) );
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

