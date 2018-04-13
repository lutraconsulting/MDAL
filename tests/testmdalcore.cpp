/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"

TEST( HelloTest, PositiveNos )
{
  ASSERT_EQ( 6, 6 );
}

TEST( HelloTest2, NegativeNos )
{
  ASSERT_EQ( -1.0, -1.0 );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}

