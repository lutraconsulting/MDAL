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

TEST( MdalUtilsTest, TrimString )
{
  std::vector<SplitTestData> tests =
  {
    SplitTestData( "", {"", "", ""} ),
    SplitTestData( " ", {"", "", ""} ),
    SplitTestData( " a", {"a", " a", "a"} ),
    SplitTestData( "a ", {"a", "a", "a "} ),
    SplitTestData( " a ", {"a", " a", "a "} ),
    SplitTestData( " a b ", {"a b", " a b", "a b "} ),
    SplitTestData( "\na b ", {"a b", "\na b", "a b "} )
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.mExpectedResult[0], MDAL::trim( test.mInput ) );
    EXPECT_EQ( test.mExpectedResult[1], MDAL::rtrim( test.mInput ) );
    EXPECT_EQ( test.mExpectedResult[2], MDAL::ltrim( test.mInput ) );
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

TEST( MdalUtilsTest, StartsWidth )
{
  // case sensitive
  EXPECT_EQ( false, MDAL::startsWith( "abcs", "" ) );
  EXPECT_EQ( false, MDAL::startsWith( "abcs", "", MDAL::ContainsBehaviour::CaseSensitive ) );

  std::vector<std::pair<std::string, bool>> tests =
  {
    { "abcd", true },
    { " abcd", false },
    { "ab", false },
    { "", false },
    { "abc ", true },
    { "cccc", false },
    { "ABC", false }
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::startsWith( test.first, "abc" ) );
    EXPECT_EQ( test.second, MDAL::startsWith( test.first, "abc", MDAL::ContainsBehaviour::CaseSensitive ) );
  }

  // case insensitive
  EXPECT_EQ( false, MDAL::startsWith( "abcs", "", MDAL::ContainsBehaviour::CaseInsensitive ) );
  tests =
  {
    { "abcd", true },
    { " abcd", false },
    { "ab", false },
    { "", false },
    { "abc ", true },
    { "cccc", false },
    { "ABC", true },
    { "AbC", true }
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::startsWith( test.first, "abc", MDAL::ContainsBehaviour::CaseInsensitive ) );
  }
}

TEST( MdalUtilsTest, EndsWidth )
{
  // case sensitive
  EXPECT_EQ( false, MDAL::endsWith( "abcs", "", MDAL::ContainsBehaviour::CaseSensitive ) );

  std::vector<std::pair<std::string, bool>> tests =
  {
    { "abcd", true },
    { " abcd", true },
    { "ab", false },
    { "", false },
    { "abcd ", false },
    { "cccc", false },
    { "aa ABCD", false }
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::endsWith( test.first, "cd", MDAL::ContainsBehaviour::CaseSensitive ) );
  }

  // case insensitive
  EXPECT_EQ( false, MDAL::endsWith( "abcs", "", MDAL::ContainsBehaviour::CaseInsensitive ) );
  tests =
  {
    { "abCd", true },
    { " abcd", true },
    { "ab", false },
    { "", false },
    { "abcd ", false },
    { "cccc", false },
    { "ABCD", true },
    { "aa AbcD", true }
  };
  for ( const auto &test : tests )
  {
    EXPECT_EQ( test.second, MDAL::endsWith( test.first, "cd", MDAL::ContainsBehaviour::CaseInsensitive ) );
  }
}

struct LoadMeshUri
{
  LoadMeshUri( std::string u, std::string d, std::string mf, std::string mn, int mid ) :
    uri( u ),
    expectedDriver( d ),
    expectedMeshFile( mf ),
    expectedMeshName( mn ),
    expectedMeshID( mid ) {};

  std::string uri;
  std::string expectedDriver;
  std::string expectedMeshFile;
  std::string expectedMeshName;
  int expectedMeshID;
};

TEST( MdalUtilsTest, ParseLoadMeshUri )
{
  std::string inputUri, driverName, meshFile, specificMeshName;
  int specificMeshID;

  std::vector<LoadMeshUri> tests
  {
    LoadMeshUri( "Ugrid:\"mesh.nc\":mesh1d", "Ugrid", "mesh.nc", "mesh1d", 0 ),
    LoadMeshUri( "Ugrid:\"mesh.nc\":1", "Ugrid", "mesh.nc", "", 1 ),
    LoadMeshUri( "\"mesh.nc\":mesh1d", "", "mesh.nc", "mesh1d", 0 ),
    LoadMeshUri( "\"mesh.nc\":1", "", "mesh.nc", "", 1 ),
    LoadMeshUri( "Ugrid:\"mesh.nc\"", "Ugrid", "mesh.nc", "", 0 ),
    LoadMeshUri( "\"mesh.nc\"", "", "mesh.nc", "", 0 ),
    LoadMeshUri( "mesh.nc", "", "mesh.nc", "", 0 )
  };

  for ( const LoadMeshUri &it : tests )
  {
    MDAL::parseDriverAndMeshFromUri( it.uri, driverName, meshFile, specificMeshName, specificMeshID );

    EXPECT_EQ( driverName, it.expectedDriver );
    EXPECT_EQ( meshFile, it.expectedMeshFile );
    EXPECT_EQ( specificMeshName, it.expectedMeshName );
    EXPECT_EQ( specificMeshID, it.expectedMeshID );
  }
}
