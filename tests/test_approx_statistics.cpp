/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2026 Lutra Consulting Limited
*/
#include "gtest/gtest.h"
#include <string>
#include <vector>
#include <cmath>
#include <limits>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

namespace
{
  const int kDatasetCount = 10;
  const int kPeakIndex = 5;
  const double kPeakValue = 1000.0;

  // Build a multi-timestep scalar group persisted to a SELAFIN file:
  // kDatasetCount datasets where dataset i holds the constant value i,
  // except dataset kPeakIndex whose first value is the outlier kPeakValue.
  // Approximate sampling that misses kPeakIndex will not capture the
  // global maximum.
  MDAL_MeshH buildMultiTimestepMesh( const std::string &savedFile )
  {
    std::string sourceFile = test_file( "/slf/example.slf" );
    MDAL_MeshH sourceMesh = MDAL_LoadMesh( sourceFile.c_str() );
    EXPECT_NE( sourceMesh, nullptr );
    MDAL_SaveMesh( sourceMesh, savedFile.c_str(), "SELAFIN" );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    MDAL_CloseMesh( sourceMesh );

    MDAL_MeshH mesh = MDAL_LoadMesh( savedFile.c_str() );
    EXPECT_NE( mesh, nullptr );

    MDAL_DriverH driver = MDAL_driverFromName( "SELAFIN" );
    MDAL_DatasetGroupH g = MDAL_M_addDatasetGroup( mesh,
                           "TestGroup",
                           DataOnVertices,
                           true /*scalar*/,
                           driver,
                           savedFile.c_str() );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

    const size_t v_count = MDAL_M_vertexCount( mesh );
    for ( int i = 0; i < kDatasetCount; ++i )
    {
      std::vector<double> values( v_count, static_cast<double>( i ) );
      if ( i == kPeakIndex )
        values[0] = kPeakValue;
      MDAL_G_addDataset( g, static_cast<double>( i ), values.data(), nullptr );
      EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    }
    MDAL_G_closeEditMode( g );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    return mesh;
  }

  MDAL_DatasetGroupH lastGroup( MDAL_MeshH mesh )
  {
    return MDAL_M_datasetGroup( mesh, MDAL_M_datasetGroupCount( mesh ) - 1 );
  }
}

TEST( MeshApproxStatisticsTest, VersionMatchesSinceTags )
{
  // The load flag and lazy/approximate statistics API is documented as
  // \since MDAL 1.4.0, so MDAL_Version() must report that version.
  EXPECT_STREQ( "1.4.0", MDAL_Version() );
}

TEST( MeshApproxStatisticsTest, ExactFallbacksEqualExact )
{
  std::string file = tmp_file( "/approx_stats_fallback.slf" );
  MDAL_MeshH m = buildMultiTimestepMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double minE = NAN, maxE = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  EXPECT_DOUBLE_EQ( kPeakValue, maxE );

  // sampleCount 0, negative, == count and > count all fall back to exact
  for ( int sampleCount : { 0, -5, kDatasetCount, 999 } )
  {
    double minA = NAN, maxA = NAN;
    MDAL_G_minimumMaximumApprox( g, sampleCount, &minA, &maxA );
    EXPECT_DOUBLE_EQ( minE, minA ) << "sampleCount=" << sampleCount;
    EXPECT_DOUBLE_EQ( maxE, maxA ) << "sampleCount=" << sampleCount;
  }

  MDAL_CloseMesh( m );
}

TEST( MeshApproxStatisticsTest, SampleCountOneUsesMiddleDataset )
{
  // For n=10 and sampleCount=1 the middle dataset (index 4) is sampled:
  // constant value 4, and the outlier at index 5 is missed.
  std::string file = tmp_file( "/approx_stats_sc1.slf" );
  MDAL_MeshH m = buildMultiTimestepMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 1, &minA, &maxA );
  EXPECT_DOUBLE_EQ( 4.0, minA );
  EXPECT_DOUBLE_EQ( 4.0, maxA );

  MDAL_CloseMesh( m );
}

TEST( MeshApproxStatisticsTest, SampleCountThreeMissesMiddleOutlier )
{
  // For n=10 and sampleCount=3 the chosen indices are {0, 4, 9}
  // — the outlier at index 5 must be missed.
  std::string file = tmp_file( "/approx_stats_sc3.slf" );
  MDAL_MeshH m = buildMultiTimestepMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double minE = NAN, maxE = NAN, minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  MDAL_G_minimumMaximumApprox( g, 3, &minA, &maxA );

  EXPECT_DOUBLE_EQ( kPeakValue, maxE );  // exact captures the outlier
  EXPECT_LT( maxA, kPeakValue );         // approximate misses it
  EXPECT_LE( minE, minA );               // approximate range is contained in exact range
  EXPECT_GE( maxE, maxA );

  MDAL_CloseMesh( m );
}

TEST( MeshApproxStatisticsTest, ExactCacheUntouchedAfterApproximate )
{
  std::string file = tmp_file( "/approx_stats_cache.slf" );
  MDAL_MeshH m = buildMultiTimestepMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  // Call approximate first
  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 3, &minA, &maxA );
  EXPECT_LT( maxA, kPeakValue );

  // Exact must still return the true range
  double minE = NAN, maxE = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  EXPECT_DOUBLE_EQ( kPeakValue, maxE );

  MDAL_CloseMesh( m );
}

TEST( MeshLoadFlagsTest, SkipStatisticsThenLazyExact )
{
  // Build a multi-timestep selafin first (writes through the addDataset edit
  // mode path, which always computes stats — eager path is unaffected).
  std::string file = tmp_file( "/skipstats_eager.slf" );
  MDAL_MeshH eager = buildMultiTimestepMesh( file );
  ASSERT_NE( eager, nullptr );
  MDAL_CloseMesh( eager );

  // Reload with MDAL_LF_SkipStatistics: the SELAFIN driver must NOT compute
  // per-dataset stats during load.
  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_NE( m, nullptr );

  ASSERT_GE( MDAL_M_datasetGroupCount( m ), 1 );
  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  // Approximate min/max computes only on a sample of timesteps; the outlier
  // dataset (index 5) is missed when sampleCount=3.
  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 3, &minA, &maxA );
  EXPECT_FALSE( std::isnan( maxA ) );
  EXPECT_LT( maxA, kPeakValue );

  // Exact (lazy) computes on first access and caches: must return the outlier.
  double minE = NAN, maxE = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  EXPECT_DOUBLE_EQ( kPeakValue, maxE );

  // A second exact call returns the cached value (sanity).
  double minE2 = NAN, maxE2 = NAN;
  MDAL_G_minimumMaximum( g, &minE2, &maxE2 );
  EXPECT_DOUBLE_EQ( minE, minE2 );
  EXPECT_DOUBLE_EQ( maxE, maxE2 );

  MDAL_CloseMesh( m );
}

TEST( MeshLoadFlagsTest, NoSkipFlagPreservesEagerBehavior )
{
  // Without the flag, MDAL_LoadMeshWithFlags(uri, 0) must behave exactly
  // like MDAL_LoadMesh.
  std::string file = tmp_file( "/skipstats_default.slf" );
  MDAL_MeshH eager = buildMultiTimestepMesh( file );
  ASSERT_NE( eager, nullptr );
  MDAL_CloseMesh( eager );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), 0 );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double min = NAN, max = NAN;
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( kPeakValue, max );

  MDAL_CloseMesh( m );
}

TEST( MeshLoadFlagsTest, LoadDatasetsWithFlagsSkipsStatistics )
{
  // Attach the SELAFIN file as a dataset file onto an already-loaded mesh:
  // MDAL_M_LoadDatasetsWithFlags must honor MDAL_LF_SkipStatistics too.
  std::string file = tmp_file( "/skipstats_loaddatasets.slf" );
  MDAL_MeshH built = buildMultiTimestepMesh( file );
  ASSERT_NE( built, nullptr );
  MDAL_CloseMesh( built );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_NE( m, nullptr );
  const int groupsBefore = MDAL_M_datasetGroupCount( m );

  MDAL_M_LoadDatasetsWithFlags( m, file.c_str(), MDAL_LF_SkipStatistics );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
  ASSERT_GT( MDAL_M_datasetGroupCount( m ), groupsBefore );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 3, &minA, &maxA );
  EXPECT_FALSE( std::isnan( maxA ) );
  EXPECT_LT( maxA, kPeakValue );

  double minE = NAN, maxE = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  EXPECT_DOUBLE_EQ( kPeakValue, maxE );

  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret = RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
