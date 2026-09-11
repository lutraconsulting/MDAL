/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/
#include "gtest/gtest.h"
#include <cmath>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_utils.hpp"

TEST( MeshDynamicDriverTest, hasMinimalExternalDriver )
{
  MDAL_DriverH driver = MDAL_driverFromName( "Dynamic_driver_test" );
  ASSERT_TRUE( driver );
}

TEST( MeshDynamicDriverTest, openMesh )
{
  std::string path = test_file( "/dynamic_driver/mesh_1.msh" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( m );

  // Vertices
  int v_count = MDAL_M_vertexCount( m );
  EXPECT_EQ( v_count, 5 );

  EXPECT_EQ( getVertexXCoordinatesAt( m, 0 ), 1000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 0 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 0 ), 0.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 1 ), 2000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 1 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 1 ), 1.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 2 ), 3000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 2 ), 2000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 2 ), 2.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 3 ), 2000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 3 ), 3000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 3 ), 3.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( m, 4 ), 1000.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( m, 4 ), 3000.0 );
  EXPECT_EQ( getVertexZCoordinatesAt( m, 4 ), 4.0 );

  // Faces
  int f_count = MDAL_M_faceCount( m );
  EXPECT_EQ( f_count, 2 );
  EXPECT_EQ( getFaceVerticesCountAt( m, 0 ), 4 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 0 ), 0 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 1 ), 1 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 2 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 0, 3 ), 4 );

  EXPECT_EQ( getFaceVerticesCountAt( m, 1 ), 3 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 0 ), 1 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 1 ), 2 );
  EXPECT_EQ( getFaceVerticesIndexAt( m, 1, 2 ), 3 );

  // Edges
  int e_count = MDAL_M_edgeCount( m );
  EXPECT_EQ( e_count, 3 );
  std::vector<int> start;
  std::vector<int> end;
  getEdgeVertexIndices( m, e_count, start, end );
  EXPECT_EQ( start.at( 0 ), 0 );
  EXPECT_EQ( end.at( 0 ), 1 );
  EXPECT_EQ( start.at( 1 ), 3 );
  EXPECT_EQ( end.at( 1 ), 4 );
  EXPECT_EQ( start.at( 2 ), 4 );
  EXPECT_EQ( end.at( 2 ), 2 );

  double xMin, xMax, yMin, yMax;
  MDAL_M_extent( m, &xMin, &xMax, &yMin, &yMax );

  EXPECT_EQ( xMin, 1000 );
  EXPECT_EQ( xMax, 3000 );
  EXPECT_EQ( yMin, 2000 );
  EXPECT_EQ( yMax, 3000 );

  std::string crs = MDAL_M_projection( m );
  EXPECT_EQ( crs, "EPSG::32620" );


  // Dataset
  ASSERT_EQ( MDAL_M_datasetGroupCount( m ), 8 );

  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );

  EXPECT_TRUE( compareReferenceTime( g, "1990-02-03T01:02:00" ) );

  int meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 3, meta_count );

  const char *name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "DatasetGroup_1" ), std::string( name ) );

  const char *metaKey = MDAL_G_metadataKey( g, 1 );
  EXPECT_EQ( std::string( "unit" ), std::string( metaKey ) );
  const char *metaValue = MDAL_G_metadataValue( g, 1 );
  EXPECT_EQ( std::string( "m" ), std::string( metaValue ) );

  bool scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( true, scalar );

  MDAL_DataLocation dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnVertices );

  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );
  EXPECT_FALSE( MDAL_D_hasActiveFlagCapability( ds ) );

  bool valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  int count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 5, count );

  double value = getValue( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.0, value );
  value = getValue( ds, 1 );
  EXPECT_DOUBLE_EQ( 2.0, value );
  value = getValue( ds, 2 );
  EXPECT_DOUBLE_EQ( 3.0, value );
  value = getValue( ds, 3 );
  EXPECT_DOUBLE_EQ( 4.0, value );
  value = getValue( ds, 4 );
  EXPECT_DOUBLE_EQ( 5.0, value );

  double min, max;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 1.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );

  /////////
  g = MDAL_M_datasetGroup( m, 5 );
  ASSERT_NE( g, nullptr );

  EXPECT_TRUE( compareReferenceTime( g, "1990-02-03T01:05:00" ) );

  meta_count = MDAL_G_metadataCount( g );
  ASSERT_EQ( 3, meta_count );

  name = MDAL_G_name( g );
  EXPECT_EQ( std::string( "DatasetGroup_6" ), std::string( name ) );

  metaKey = MDAL_G_metadataKey( g, 2 );
  EXPECT_EQ( std::string( "long unit" ), std::string( metaKey ) );
  metaValue = MDAL_G_metadataValue( g, 2 );
  EXPECT_EQ( std::string( "square meter per second" ), std::string( metaValue ) );

  scalar = MDAL_G_hasScalarData( g );
  EXPECT_EQ( false, scalar );

  dataLocation = MDAL_G_dataLocation( g );
  EXPECT_EQ( dataLocation, MDAL_DataLocation::DataOnFaces );

  ASSERT_EQ( 3, MDAL_G_datasetCount( g ) );
  ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );
  ASSERT_TRUE( MDAL_D_hasActiveFlagCapability( ds ) );

  valid = MDAL_D_isValid( ds );
  EXPECT_EQ( true, valid );

  count = MDAL_D_valueCount( ds );
  ASSERT_EQ( 2, count );

  value = getValueX( ds, 0 );
  EXPECT_DOUBLE_EQ( 1.0, value );
  value = getValueY( ds, 0 );
  EXPECT_DOUBLE_EQ( 2.0, value );
  value = getValueX( ds, 1 );
  EXPECT_DOUBLE_EQ( 3.0, value );
  value = getValueY( ds, 1 );
  EXPECT_DOUBLE_EQ( 2.0, value );

  EXPECT_EQ( getActive( ds, 0 ), 1 );
  EXPECT_EQ( getActive( ds, 1 ), 0 );

  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( MDAL::equals( 2.236, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 2.236, max, 0.001 ) );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_TRUE( MDAL::equals( 1.4142, min, 0.001 ) );
  EXPECT_TRUE( MDAL::equals( 2.236, max, 0.001 ) );

  // Dataset on volume
  g = MDAL_M_datasetGroup( m, 6 );
  ASSERT_NE( g, nullptr );

  EXPECT_EQ( 5, MDAL_G_maximumVerticalLevelCount( g ) );
  EXPECT_TRUE( MDAL_G_hasScalarData( g ) );
  EXPECT_EQ( 2, MDAL_G_datasetCount( g ) );

  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 5, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 6, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 5, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 1, getLevelsCount3D( ds, 1 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 5, get3DFrom2D( ds, 1 ) );

  int faceIndex = 0;
  EXPECT_EQ( -0.1, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ) );
  EXPECT_EQ( -0.2, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ) );
  EXPECT_EQ( 2, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ) );

  faceIndex = 1;
  EXPECT_EQ( 0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex ) );
  EXPECT_EQ( -0.2, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ) );
  EXPECT_EQ( 8, getValue3D( ds, get3DFrom2D( ds, faceIndex ) ) );

  ds = MDAL_G_dataset( g, 1 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 3, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 5, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 3, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 2, getLevelsCount3D( ds, 1 ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 3, get3DFrom2D( ds, 1 ) );

  faceIndex = 0;
  EXPECT_EQ( -0.1, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ) );
  EXPECT_EQ( -0.2, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ) );
  EXPECT_EQ( 4, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ) );

  faceIndex = 1;
  EXPECT_EQ( 0.5, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex ) );
  EXPECT_EQ( 0.4, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ) );
  EXPECT_EQ( 1, getValue3D( ds, get3DFrom2D( ds, faceIndex ) + 1 ) );

  g = MDAL_M_datasetGroup( m, 7 );
  ASSERT_NE( g, nullptr );

  EXPECT_EQ( 5, MDAL_G_maximumVerticalLevelCount( g ) );
  EXPECT_FALSE( MDAL_G_hasScalarData( g ) );
  EXPECT_EQ( 2, MDAL_G_datasetCount( g ) );

  ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );
  EXPECT_EQ( 5, MDAL_D_maximumVerticalLevelCount( ds ) );
  EXPECT_EQ( 6, MDAL_D_volumesCount( ds ) );
  EXPECT_EQ( 5, getLevelsCount3D( ds, 0 ) );
  EXPECT_EQ( 1, getLevelsCount3D( ds, 1 ) );

  faceIndex = 0;
  EXPECT_EQ( -0.2, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 2 ) );
  EXPECT_EQ( -0.3, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 3 ) );
  EXPECT_EQ( 3, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) + 2 ) );
  EXPECT_EQ( 4.2, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) + 2 ) );

  faceIndex = 1;
  EXPECT_EQ( 0, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex ) );
  EXPECT_EQ( -0.2, getLevelZ3D( ds, get3DFrom2D( ds, faceIndex ) + faceIndex + 1 ) );
  EXPECT_EQ( 8, getValue3DX( ds, get3DFrom2D( ds, faceIndex ) ) );
  EXPECT_EQ( 3, getValue3DY( ds, get3DFrom2D( ds, faceIndex ) ) );
  EXPECT_EQ( 0, get3DFrom2D( ds, 0 ) );
  EXPECT_EQ( 5, get3DFrom2D( ds, 1 ) );

  MDAL_CloseMesh( m );
}

namespace
{
  // The test driver exposes a few hooks beside the driver API. They live in
  // the driver library, which MDAL has already loaded; loading it again
  // returns the same module, so the hooks read the very state the driver
  // uses. MDAL's own loader is not exported from the shared library, hence
  // the platform calls.
  std::string testDriverLibraryPath()
  {
    const std::string dirPath = std::string( drivers_path() ) + "/minimal_example/";
#ifdef _WIN32
    return dirPath + "mdal_dummy_driver.dll";
#elif defined( __APPLE__ )
    return dirPath + "libmdal_dummy_driver.dylib";
#else
    return dirPath + "libmdal_dummy_driver.so";
#endif
  }

  template<typename F>
  F testDriverSymbol( const char *symbolName )
  {
    const std::string path = testDriverLibraryPath();
#ifdef _WIN32
    HMODULE module = LoadLibraryA( path.c_str() );
    if ( !module )
      return nullptr;
    return reinterpret_cast<F>( GetProcAddress( module, symbolName ) );
#else
    void *handle = dlopen( path.c_str(), RTLD_NOW );
    if ( !handle )
      return nullptr;
    return reinterpret_cast<F>( dlsym( handle, symbolName ) );
#endif
  }

  // Counts the MDAL_DRIVER_D_unload() calls the driver received for a dataset
  // whose MDAL_DRIVER_D_data() was never called. -1 if the hook is missing.
  int unpairedUnloadCount()
  {
    typedef int ( *Counter )();
    Counter counter = testDriverSymbol<Counter>( "MDAL_DRIVER_TEST_unpairedUnloadCount" );
    return counter ? counter() : -1;
  }

  // Makes the driver report a failed read. Returns false if the hook is missing.
  bool setDriverShortRead( bool shortRead )
  {
    typedef void ( *Setter )( bool );
    Setter setter = testDriverSymbol<Setter>( "MDAL_DRIVER_TEST_setShortRead" );
    if ( !setter )
      return false;
    setter( shortRead );
    return true;
  }
}

TEST( MeshDynamicDriverTest, skipStatisticsDoesNotUnloadNeverLoadedDatasets )
{
  std::string path = test_file( "/dynamic_driver/mesh_1.msh" );

  const int before = unpairedUnloadCount();
  ASSERT_GE( before, 0 ) << "the test driver does not expose the unload counter";

  // without the flag, every dataset is read and then released: pairs only
  MDAL_MeshH eager = MDAL_LoadMesh( path.c_str() );
  ASSERT_TRUE( eager );
  EXPECT_EQ( before, unpairedUnloadCount() );
  MDAL_CloseMesh( eager );

  // with the flag, no data is requested at load, so nothing may be released
  MDAL_MeshH m = MDAL_LoadMeshWithFlags( path.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_TRUE( m );
  EXPECT_EQ( before, unpairedUnloadCount() );

  // the deferred statistics do read the data before releasing it
  ASSERT_GT( MDAL_M_datasetGroupCount( m ), 0 );
  double min = 0, max = 0;
  MDAL_G_minimumMaximum( MDAL_M_datasetGroup( m, 0 ), &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );
  EXPECT_EQ( before, unpairedUnloadCount() );

  MDAL_CloseMesh( m );
}

TEST( MeshDynamicDriverTest, incompleteReadIsNotCachedAsStatistics )
{
  std::string path = test_file( "/dynamic_driver/mesh_1.msh" );

  MDAL_MeshH m = MDAL_LoadMeshWithFlags( path.c_str(), MDAL_LF_SkipStatistics );
  ASSERT_TRUE( m );
  ASSERT_GT( MDAL_M_datasetGroupCount( m ), 0 );
  MDAL_DatasetGroupH g = MDAL_M_datasetGroup( m, 0 );
  ASSERT_NE( g, nullptr );
  MDAL_DatasetH ds = MDAL_G_dataset( g, 0 );
  ASSERT_NE( ds, nullptr );

  ASSERT_TRUE( setDriverShortRead( true ) ) << "the test driver does not expose the short read hook";

  // a range computed from values that could not be read is not a range
  MDAL_ResetStatus();
  double min = 0, max = 0;
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_TRUE( std::isnan( min ) );
  EXPECT_TRUE( std::isnan( max ) );
  EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None );

  MDAL_ResetStatus();
  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_TRUE( std::isnan( min ) );
  EXPECT_TRUE( std::isnan( max ) );
  EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None );

  MDAL_ResetStatus();
  MDAL_G_minimumMaximumApprox( g, 2, &min, &max );
  EXPECT_TRUE( std::isnan( min ) );
  EXPECT_TRUE( std::isnan( max ) );
  EXPECT_NE( MDAL_LastStatus(), MDAL_Status::None );

  // nothing was cached, so a later readable file gives the right answer
  ASSERT_TRUE( setDriverShortRead( false ) );
  MDAL_ResetStatus();
  MDAL_D_minimumMaximum( ds, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 4.0, max );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

  MDAL_G_minimumMaximum( g, &min, &max );
  EXPECT_DOUBLE_EQ( 0.0, min );
  EXPECT_DOUBLE_EQ( 5.0, max );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  set_mdal_driver_path( "minimal_example" );
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}
