/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2026 Lutra Consulting Limited
*/
#include "gtest/gtest.h"
#include <algorithm>
#include <fstream>
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
  const double kUniformPeak = 7.5;

  // Saves a copy of the SELAFIN example mesh to \a savedFile, reloads it and
  // opens a new scalar vertex dataset group on it, left in edit mode so the
  // caller can add datasets with MDAL_G_addDataset().
  MDAL_MeshH openMeshWithNewGroup( const std::string &savedFile, MDAL_DatasetGroupH *group )
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
    *group = MDAL_M_addDatasetGroup( mesh,
                                     "TestGroup",
                                     DataOnVertices,
                                     true /*scalar*/,
                                     driver,
                                     savedFile.c_str() );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    return mesh;
  }

  // Build a multi-timestep scalar group persisted to a SELAFIN file:
  // kDatasetCount datasets where dataset i holds the constant value i,
  // except dataset kPeakIndex whose first value is the outlier kPeakValue.
  // Approximate sampling that misses kPeakIndex will not capture the
  // global maximum.
  MDAL_MeshH buildMultiTimestepMesh( const std::string &savedFile )
  {
    MDAL_DatasetGroupH g = nullptr;
    MDAL_MeshH mesh = openMeshWithNewGroup( savedFile, &g );

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

  // Build a two-timestep group shaped like a real hydraulic run: a uniformly
  // zero initial condition followed by a timestep reaching kUniformPeak.
  MDAL_MeshH buildUniformFirstDatasetMesh( const std::string &savedFile )
  {
    MDAL_DatasetGroupH g = nullptr;
    MDAL_MeshH mesh = openMeshWithNewGroup( savedFile, &g );

    const size_t v_count = MDAL_M_vertexCount( mesh );
    std::vector<double> initial( v_count, 0.0 );
    MDAL_G_addDataset( g, 0.0, initial.data(), nullptr );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

    std::vector<double> flooded( v_count, 0.0 );
    flooded[v_count / 2] = kUniformPeak;
    MDAL_G_addDataset( g, 1.0, flooded.data(), nullptr );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

    MDAL_G_closeEditMode( g );
    EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    return mesh;
  }

  // Empties \a path in place. Any handle already open on it then reads a file
  // that can no longer serve the data it used to, the way a result file being
  // rewritten by a running solver does.
  void emptyFile( const std::string &path )
  {
    std::ofstream out( path.c_str(), std::ios::binary | std::ios::trunc );
    EXPECT_TRUE( out.is_open() );
  }

  // Every group and dataset range of \a mesh, in order, so that an eagerly
  // loaded mesh and a lazily loaded one can be compared value by value.
  std::vector<double> allRanges( MDAL_MeshH mesh )
  {
    std::vector<double> ranges;
    const int groupCount = MDAL_M_datasetGroupCount( mesh );
    for ( int i = 0; i < groupCount; ++i )
    {
      MDAL_DatasetGroupH g = MDAL_M_datasetGroup( mesh, i );
      double min = NAN, max = NAN;
      MDAL_G_minimumMaximum( g, &min, &max );
      ranges.push_back( min );
      ranges.push_back( max );

      const int datasetCount = MDAL_G_datasetCount( g );
      for ( int d = 0; d < datasetCount; ++d )
      {
        MDAL_D_minimumMaximum( MDAL_G_dataset( g, d ), &min, &max );
        ranges.push_back( min );
        ranges.push_back( max );
      }
    }
    return ranges;
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

TEST( MeshApproxStatisticsTest, SampleCountOneSamplesBothEndpoints )
{
  // A sampleCount of 1 is raised to 2, so the first and the last dataset are
  // sampled: for n=10 that is index 0 (constant 0) and index 9 (constant 9).
  // The outlier at index 5 is still missed.
  std::string file = tmp_file( "/approx_stats_sc1.slf" );
  MDAL_MeshH m = buildMultiTimestepMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );

  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 1, &minA, &maxA );
  EXPECT_DOUBLE_EQ( 0.0, minA );
  EXPECT_DOUBLE_EQ( static_cast<double>( kDatasetCount - 1 ), maxA );

  MDAL_CloseMesh( m );
}

TEST( MeshApproxStatisticsTest, SampleCountOneCoversGroupWithUniformFirstDataset )
{
  // Regression: a two-timestep group whose first dataset is a uniform initial
  // condition. Sampling a single middle dataset used to return the degenerate
  // range [0, 0] while the exact range is [0, kUniformPeak].
  std::string file = tmp_file( "/approx_stats_uniform_first.slf" );
  MDAL_MeshH m = buildUniformFirstDatasetMesh( file );
  ASSERT_NE( m, nullptr );

  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( 2, MDAL_G_datasetCount( g ) );

  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 1, &minA, &maxA );
  EXPECT_DOUBLE_EQ( 0.0, minA );
  EXPECT_DOUBLE_EQ( kUniformPeak, maxA );

  double minE = NAN, maxE = NAN;
  MDAL_G_minimumMaximum( g, &minE, &maxE );
  EXPECT_DOUBLE_EQ( minE, minA );
  EXPECT_DOUBLE_EQ( maxE, maxA );

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

TEST( MeshLoadFlagsTest, UnreadableFileReportsNaNAndIsNotCached )
{
  // The file changes under an open mesh handle: a solver still writing its
  // results, a network share, removable media. The deferred statistics must
  // report the failure instead of caching a range they could not compute.
  std::string file = tmp_file( "/skipstats_truncated.slf" );
  copy( test_file( "/slf/example_res_fr.slf" ), file );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_NE( m, nullptr );
  ASSERT_GT( MDAL_M_datasetGroupCount( m ), 0 );
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );
  ASSERT_GT( MDAL_G_datasetCount( g ), 0 );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  emptyFile( file );

  // twice: nothing may be cached, so the second call must try again and fail
  // again rather than quietly return a range nobody could compute
  for ( int attempt = 1; attempt <= 2; ++attempt )
  {
    double min = 0, max = 0;

    MDAL_ResetStatus();
    MDAL_G_minimumMaximum( g, &min, &max );
    EXPECT_TRUE( std::isnan( min ) ) << "attempt " << attempt;
    EXPECT_TRUE( std::isnan( max ) ) << "attempt " << attempt;
    EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None ) << "attempt " << attempt;

    MDAL_ResetStatus();
    MDAL_D_minimumMaximum( ds, &min, &max );
    EXPECT_TRUE( std::isnan( min ) ) << "attempt " << attempt;
    EXPECT_TRUE( std::isnan( max ) ) << "attempt " << attempt;
    EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None ) << "attempt " << attempt;

    MDAL_ResetStatus();
    MDAL_G_minimumMaximumApprox( g, 2, &min, &max );
    EXPECT_TRUE( std::isnan( min ) ) << "attempt " << attempt;
    EXPECT_TRUE( std::isnan( max ) ) << "attempt " << attempt;
    EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None ) << "attempt " << attempt;
  }

  MDAL_CloseMesh( m );
  deleteFile( file );
}

TEST( MeshLoadFlagsTest, UnreadableFileDoesNotAbortDataAccess )
{
  // The same scenario on the entry points that read rather than compute: they
  // must report the failure, not let the driver exception escape the C API
  // and terminate the host application.
  std::string file = tmp_file( "/skipstats_truncated_data.slf" );
  copy( test_file( "/slf/example_res_fr.slf" ), file );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_NE( m, nullptr );
  const int vertexCount = MDAL_M_vertexCount( m );
  const int faceCount = MDAL_M_faceCount( m );
  ASSERT_GT( vertexCount, 0 );
  ASSERT_GT( faceCount, 0 );
  ASSERT_GT( MDAL_M_datasetGroupCount( m ), 0 );
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );
  ASSERT_GT( MDAL_G_datasetCount( g ), 0 );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  const bool isScalar = MDAL_G_hasScalarData( g );
  const int valueCount = MDAL_D_valueCount( ds );
  ASSERT_GT( valueCount, 0 );

  emptyFile( file );

  MDAL_ResetStatus();
  std::vector<double> values( static_cast<size_t>( valueCount ) * ( isScalar ? 1 : 2 ) );
  EXPECT_EQ( 0, MDAL_D_data( ds, 0, valueCount,
                             isScalar ? MDAL_DataType::SCALAR_DOUBLE : MDAL_DataType::VECTOR_2D_DOUBLE,
                             values.data() ) );
  EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None );

  MDAL_ResetStatus();
  std::vector<double> coordinates( static_cast<size_t>( vertexCount ) * 3 );
  MDAL_MeshVertexIteratorH vertexIterator = MDAL_M_vertexIterator( m );
  ASSERT_NE( vertexIterator, nullptr );
  EXPECT_EQ( 0, MDAL_VI_next( vertexIterator, vertexCount, coordinates.data() ) );
  EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None );
  MDAL_VI_close( vertexIterator );

  MDAL_CloseMesh( m );
  deleteFile( file );
}

TEST( MeshApproxStatisticsTest, DatasetMinimumMaximumOnFlagLoadedDataset )
{
  // MDAL_D_minimumMaximum() on a dataset of a mesh loaded with
  // MDAL_LF_SkipStatistics: nothing was pre-computed, so the first call reads
  // the dataset. The answer must match a manual pass over MDAL_D_data().
  std::string file = tmp_file( "/skipstats_dataset_minmax.slf" );
  MDAL_MeshH built = buildMultiTimestepMesh( file );
  ASSERT_NE( built, nullptr );
  MDAL_CloseMesh( built );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( file.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_NE( m, nullptr );
  MDAL_DatasetGroupH g = lastGroup( m );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( kDatasetCount, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, kPeakIndex );
  ASSERT_NE( ds, nullptr );

  const int valueCount = MDAL_D_valueCount( ds );
  ASSERT_GT( valueCount, 0 );
  std::vector<double> values( static_cast<size_t>( valueCount ) );
  ASSERT_EQ( valueCount, MDAL_D_data( ds, 0, valueCount, MDAL_DataType::SCALAR_DOUBLE, values.data() ) );
  double expectedMin = values[0];
  double expectedMax = values[0];
  for ( double value : values )
  {
    expectedMin = std::min( expectedMin, value );
    expectedMax = std::max( expectedMax, value );
  }
  EXPECT_DOUBLE_EQ( kPeakValue, expectedMax );

  double min = NAN, max = NAN;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( expectedMin, min );
  EXPECT_DOUBLE_EQ( expectedMax, max );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

  // the result is cached, so a second call answers the same
  double min2 = NAN, max2 = NAN;
  MDAL_D_minimumMaximum( ds, &min2, &max2 );
  EXPECT_DOUBLE_EQ( min, min2 );
  EXPECT_DOUBLE_EQ( max, max2 );

  MDAL_CloseMesh( m );
}

TEST( MeshApproxStatisticsTest, EditModeGroupFollowsTheDatasetsAddedSoFar )
{
  // A group still in edit mode reports the range of what it holds and does
  // not freeze it, so datasets added later widen it. Up to MDAL 1.3 such a
  // group returned NaN.
  std::string file = tmp_file( "/approx_stats_editmode.slf" );
  MDAL_DatasetGroupH g = nullptr;
  MDAL_MeshH m = openMeshWithNewGroup( file, &g );
  ASSERT_NE( m, nullptr );
  ASSERT_NE( g, nullptr );
  ASSERT_TRUE( MDAL_G_isInEditMode( g ) );

  const size_t v_count = MDAL_M_vertexCount( m );
  for ( int i = 0; i < 3; ++i )
  {
    std::vector<double> values( v_count, static_cast<double>( i ) );
    MDAL_G_addDataset( g, static_cast<double>( i ), values.data(), nullptr );
  }

  double min = NAN, max = NAN;
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 2.0, max );

  // the cache is not frozen: three more datasets, with a wider range
  for ( int i = 3; i < 6; ++i )
  {
    std::vector<double> values( v_count, -10.0 * i );
    MDAL_G_addDataset( g, static_cast<double>( i ), values.data(), nullptr );
  }
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -50.0, min );
  EXPECT_DOUBLE_EQ( 2.0, max );

  double minA = NAN, maxA = NAN;
  MDAL_G_minimumMaximumApprox( g, 2, &minA, &maxA );
  EXPECT_LE( min, minA );
  EXPECT_GE( max, maxA );

  MDAL_G_closeEditMode( g );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
  EXPECT_FALSE( MDAL_G_isInEditMode( g ) );
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( -50.0, min );
  EXPECT_DOUBLE_EQ( 2.0, max );
  MDAL_CloseMesh( m );

  // and the same range comes back from the file, eagerly and lazily
  for ( int flags : { 0, static_cast<int>( MDAL_LF_SkipStatistics ) } )
  {
    MDAL_MeshH reloaded = MDAL_LoadMeshWithFlags( file.c_str(), flags );
    ASSERT_NE( reloaded, nullptr );
    double rMin = NAN, rMax = NAN;
    MDAL_G_minimumMaximum( lastGroup( reloaded ), &rMin, &rMax );
    EXPECT_DOUBLE_EQ( -50.0, rMin ) << "flags=" << flags;
    EXPECT_DOUBLE_EQ( 2.0, rMax ) << "flags=" << flags;
    MDAL_CloseMesh( reloaded );
  }
}

TEST( MeshApproxStatisticsTest, GroupWithASingleDataset )
{
  std::string file = tmp_file( "/approx_stats_one_dataset.slf" );
  MDAL_DatasetGroupH g = nullptr;
  MDAL_MeshH m = openMeshWithNewGroup( file, &g );
  ASSERT_NE( m, nullptr );

  const size_t v_count = MDAL_M_vertexCount( m );
  std::vector<double> values( v_count );
  for ( size_t i = 0; i < v_count; ++i )
    values[i] = static_cast<double>( i % 7 );
  MDAL_G_addDataset( g, 0.0, values.data(), nullptr );
  MDAL_G_closeEditMode( g );
  MDAL_CloseMesh( m );

  for ( int flags : { 0, static_cast<int>( MDAL_LF_SkipStatistics ) } )
  {
    MDAL_MeshH reloaded = MDAL_LoadMeshWithFlags( file.c_str(), flags );
    ASSERT_NE( reloaded, nullptr );
    MDAL_DatasetGroupH gg = lastGroup( reloaded );
    ASSERT_EQ( 1, MDAL_G_datasetCount( gg ) );

    double min = NAN, max = NAN;
    MDAL_G_minimumMaximum( gg, &min, &max );
    EXPECT_DOUBLE_EQ( 0.0, min ) << "flags=" << flags;
    EXPECT_DOUBLE_EQ( 6.0, max ) << "flags=" << flags;

    // every sampleCount falls back to the exact range of the only dataset
    for ( int sampleCount : { 1, 3, -5 } )
    {
      double minA = NAN, maxA = NAN;
      MDAL_G_minimumMaximumApprox( gg, sampleCount, &minA, &maxA );
      EXPECT_DOUBLE_EQ( min, minA ) << "flags=" << flags << " sampleCount=" << sampleCount;
      EXPECT_DOUBLE_EQ( max, maxA ) << "flags=" << flags << " sampleCount=" << sampleCount;
    }
    MDAL_CloseMesh( reloaded );
  }
}

TEST( MeshApproxStatisticsTest, GroupWithoutAnyDataset )
{
  std::string file = tmp_file( "/approx_stats_no_dataset.slf" );
  MDAL_DatasetGroupH g = nullptr;
  MDAL_MeshH m = openMeshWithNewGroup( file, &g );
  ASSERT_NE( m, nullptr );
  ASSERT_NE( g, nullptr );
  ASSERT_EQ( 0, MDAL_G_datasetCount( g ) );

  double min = 42.0, max = 42.0;
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_TRUE( std::isnan( min ) );
  EXPECT_TRUE( std::isnan( max ) );

  double minA = 42.0, maxA = 42.0;
  MDAL_G_minimumMaximumApprox( g, 3, &minA, &maxA );
  EXPECT_TRUE( std::isnan( minA ) );
  EXPECT_TRUE( std::isnan( maxA ) );

  MDAL_G_closeEditMode( g );
  double minC = 42.0, maxC = 42.0;
  MDAL_G_minimumMaximum( g, &minC, &maxC );
  EXPECT_TRUE( std::isnan( minC ) );
  EXPECT_TRUE( std::isnan( maxC ) );

  MDAL_CloseMesh( m );
}

namespace
{
  // Loads \a meshFile and \a datasetFile with and without
  // MDAL_LF_SkipStatistics, and checks that the lazily computed ranges are
  // the ones the eager load reports.
  void checkLazyRangesMatchEagerOnes( const std::string &meshFile, const std::string &datasetFile )
  {
    MDAL_MeshH eager = MDAL_LoadMesh( meshFile.c_str() );
    ASSERT_NE( eager, nullptr );
    MDAL_M_LoadDatasets( eager, datasetFile.c_str() );
    ASSERT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    const int groupCount = MDAL_M_datasetGroupCount( eager );
    ASSERT_GT( groupCount, 1 ) << "the dataset file added no group";
    const std::vector<double> expected = allRanges( eager );
    MDAL_CloseMesh( eager );

    MDAL_MeshH lazy = MDAL_LoadMeshWithFlags( meshFile.c_str(), MDAL_LF_SkipStatistics );
    ASSERT_NE( lazy, nullptr );
    MDAL_M_LoadDatasetsWithFlags( lazy, datasetFile.c_str(), MDAL_LF_SkipStatistics );
    ASSERT_EQ( MDAL_LastStatus(), MDAL_Status::None );
    ASSERT_EQ( groupCount, MDAL_M_datasetGroupCount( lazy ) );
    const std::vector<double> lazyRanges = allRanges( lazy );
    ASSERT_EQ( expected.size(), lazyRanges.size() );
    for ( size_t i = 0; i < expected.size(); ++i )
      EXPECT_DOUBLE_EQ( expected[i], lazyRanges[i] ) << "value " << i;
    MDAL_CloseMesh( lazy );
  }
}

TEST( MeshLoadFlagsTest, LazyRangesMatchEagerOnesForAsciiDat )
{
  checkLazyRangesMatchEagerOnes( test_file( "/2dm/quad_and_triangle.2dm" ),
                                 test_file( "/ascii_dat/quad_and_triangle_vertex_scalar.dat" ) );
}

TEST( MeshLoadFlagsTest, LazyRangesMatchEagerOnesForBinaryDat )
{
  checkLazyRangesMatchEagerOnes( test_file( "/2dm/quad_and_triangle.2dm" ),
                                 test_file( "/binary_dat/quad_and_triangle_binary.dat" ) );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret = RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
