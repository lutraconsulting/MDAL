/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"
#include <limits>
#include <cmath>

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"
#include "mdal_config.hpp"

TEST( ApiTest, GlobalApi )
{
  EXPECT_NE( MDAL_Version(), std::string( "" ) );
}

TEST( ApiTest, DriversApi )
{
  int driversCount = MDAL_driverCount();
  ASSERT_TRUE( driversCount > 2 ); // variable based on the available drivers on system
  MDAL_DriverH dr = MDAL_driverFromIndex( 0 );
  ASSERT_TRUE( dr );

  std::string name = MDAL_DR_name( dr );
  ASSERT_EQ( name, "2DM" );

  std::string longName = MDAL_DR_longName( dr );
  ASSERT_EQ( longName, "2DM Mesh File" );

  bool meshLoad = MDAL_DR_meshLoadCapability( dr );
  ASSERT_TRUE( meshLoad );

  std::string filters = MDAL_DR_filters( dr );
  ASSERT_EQ( filters, "*.2dm" );

  // Some wrong calls tests
  EXPECT_EQ( MDAL_driverFromIndex( -1 ), nullptr );
  EXPECT_EQ( MDAL_driverFromIndex( MDAL_driverCount() ), nullptr );
  EXPECT_EQ( MDAL_driverFromName( "invaliddrivername" ), nullptr );
  EXPECT_FALSE( MDAL_DR_meshLoadCapability( nullptr ) );
  EXPECT_FALSE( MDAL_DR_writeDatasetsCapability( nullptr, MDAL_DataLocation::DataOnVertices ) );
  EXPECT_EQ( MDAL_DR_longName( nullptr ), std::string( "" ) );
  EXPECT_EQ( MDAL_DR_name( nullptr ), std::string( "" ) );
  EXPECT_EQ( MDAL_DR_filters( nullptr ), std::string( "" ) );
}

TEST( ApiTest, MeshApi )
{
  EXPECT_EQ( MDAL_LoadMesh( nullptr ), nullptr );
  EXPECT_EQ( MDAL_M_projection( nullptr ), std::string( "" ) );
  double a, b, c, d;
  MDAL_M_extent( nullptr, &a, &b, &c, &d );
  EXPECT_TRUE( std::isnan( a ) );

  EXPECT_EQ( MDAL_M_vertexCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_M_faceCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( nullptr ), 0 );
  MDAL_M_LoadDatasets( nullptr, nullptr );
  EXPECT_EQ( MDAL_M_datasetGroupCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_M_datasetGroup( nullptr, 0 ), nullptr );
  EXPECT_EQ( MDAL_M_addDatasetGroup( nullptr, nullptr, MDAL_DataLocation::DataOnVertices, true, nullptr, nullptr ), nullptr );
  EXPECT_EQ( MDAL_M_addDatasetGroup( nullptr, nullptr, MDAL_DataLocation::DataOnVolumes, true, nullptr, nullptr ), nullptr );
  EXPECT_EQ( MDAL_M_driverName( nullptr ), nullptr );
  EXPECT_EQ( MDAL_M_metadataCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_M_metadataKey( nullptr, 0 ), std::string( "" ) );
  EXPECT_EQ( MDAL_M_metadataValue( nullptr, 0 ), std::string( "" ) );
  MDAL_M_setMetadata( nullptr, nullptr, nullptr );
  EXPECT_EQ( MDAL_LastStatus(), Err_IncompatibleMesh );
}

void _populateFaces( MDAL_MeshH m, std::vector<int> &ret, size_t faceOffsetsBufferLen, size_t vertexIndicesBufferLen )
{
  int facesCount = MDAL_M_faceCount( m );
  ret.resize( 0 );
  std::vector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  std::vector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  MDAL_MeshFaceIteratorH it = MDAL_M_faceIterator( m );
  int faceIndex = 0;
  while ( faceIndex < facesCount )
  {
    int facesRead = MDAL_FI_next( it,
                                  static_cast<int>( faceOffsetsBufferLen ),
                                  faceOffsetsBuffer.data(),
                                  static_cast<int>( vertexIndicesBufferLen ),
                                  vertexIndicesBuffer.data() );
    if ( facesRead == 0 )
      break;

    ASSERT_TRUE( facesRead <= static_cast<int>( faceOffsetsBufferLen ) );
    int nVertices = faceOffsetsBuffer[static_cast<size_t>( facesRead - 1 )];
    ASSERT_TRUE( nVertices <= static_cast<int>( vertexIndicesBufferLen ) );

    ret.insert( ret.end(),
                vertexIndicesBuffer.begin(),
                vertexIndicesBuffer.begin() + nVertices );

    faceIndex += facesRead;
  }
  MDAL_FI_close( it );
}

TEST( ApiTest, FacesApi )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // reference buffer where taken in one go
  std::vector<int> refIndices;
  _populateFaces( m,
                  refIndices,
                  static_cast<size_t>( MDAL_M_faceCount( m ) ),
                  static_cast<size_t>( MDAL_M_faceCount( m ) * MDAL_M_faceVerticesMaximumCount( m ) )
                );


  {
    std::vector<int> indices;
    _populateFaces( m,
                    indices,
                    10,
                    static_cast<size_t>( MDAL_M_faceVerticesMaximumCount( m ) )
                  );

    EXPECT_TRUE( compareVectors( refIndices, indices ) );
  }

  {
    std::vector<int> indices;
    _populateFaces( m,
                    indices,
                    13,
                    4 * 13
                  );

    EXPECT_TRUE( compareVectors( refIndices, indices ) );
  }

  {
    std::vector<int> indices;
    _populateFaces( m,
                    indices,
                    3,
                    1000
                  );

    EXPECT_TRUE( compareVectors( refIndices, indices ) );
  }
  MDAL_CloseMesh( m );

  // Some wrong calls tests
  EXPECT_EQ( MDAL_M_vertexIterator( nullptr ), nullptr );
  EXPECT_EQ( MDAL_VI_next( nullptr, 0, nullptr ), 0 );
}

void _populateVertices( MDAL_MeshH m, std::vector<double> &ret, size_t itemsLen )
{
  int verticesCount = MDAL_M_vertexCount( m );
  ret.resize( 0 );
  std::vector<double> coordsBuffer( itemsLen * 3 );

  MDAL_MeshVertexIteratorH it = MDAL_M_vertexIterator( m );
  int vertexIndex = 0;
  while ( vertexIndex < verticesCount )
  {
    int verticesRead = MDAL_VI_next( it,
                                     static_cast<int>( itemsLen ),
                                     coordsBuffer.data() );
    if ( verticesRead == 0 )
      break;

    ASSERT_TRUE( verticesRead <= static_cast<int>( itemsLen ) );

    ret.insert( ret.end(),
                coordsBuffer.begin(),
                coordsBuffer.begin() + verticesRead * 3 );

    vertexIndex += verticesRead;
  }
  MDAL_VI_close( it );
}

TEST( ApiTest, VerticesApi )
{
  std::string path = test_file( "/2dm/regular_grid.2dm" );
  MDAL_MeshH m = MDAL_LoadMesh( path.c_str() );
  EXPECT_NE( m, nullptr );
  MDAL_Status s = MDAL_LastStatus();
  ASSERT_EQ( MDAL_Status::None, s );

  // reference buffer where taken in one go
  std::vector<double> refCoors;
  _populateVertices(
    m,
    refCoors,
    static_cast<size_t>( MDAL_M_vertexCount( m ) )
  );


  {
    std::vector<double> coords;
    _populateVertices( m,
                       coords,
                       13
                     );

    EXPECT_TRUE( compareVectors( refCoors, coords ) );
  }

  {
    std::vector<double> coords;
    _populateVertices( m,
                       coords,
                       10000
                     );

    EXPECT_TRUE( compareVectors( refCoors, coords ) );
  }
  MDAL_CloseMesh( m );

  // Some wrong calls tests
  EXPECT_EQ( MDAL_M_faceIterator( nullptr ), nullptr );
  EXPECT_EQ( MDAL_FI_next( nullptr, 0, nullptr, 0, nullptr ), 0 );
}

TEST( ApiTest, GroupsApi )
{
  EXPECT_EQ( MDAL_G_mesh( nullptr ), nullptr );
  EXPECT_EQ( MDAL_G_datasetCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_G_dataset( nullptr, 0 ), nullptr );
  EXPECT_EQ( MDAL_G_metadataCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_G_metadataKey( nullptr, 0 ), std::string( "" ) );
  EXPECT_EQ( MDAL_G_metadataValue( nullptr, 0 ), std::string( "" ) );
  EXPECT_EQ( MDAL_G_name( nullptr ), std::string( "" ) );
  EXPECT_EQ( MDAL_G_hasScalarData( nullptr ), true );
  EXPECT_EQ( 0, MDAL_G_maximumVerticalLevelCount( nullptr ) );
  EXPECT_EQ( MDAL_G_dataLocation( nullptr ), MDAL_DataLocation::DataInvalidLocation );
  double a, b;
  MDAL_G_minimumMaximum( nullptr, &a, &b );
  EXPECT_TRUE( std::isnan( a ) );

  EXPECT_EQ( MDAL_G_addDataset( nullptr, 0, nullptr, nullptr ), nullptr );
  EXPECT_EQ( MDAL_G_isInEditMode( nullptr ), true );
  MDAL_G_closeEditMode( nullptr );
  MDAL_G_setMetadata( nullptr, nullptr, nullptr );
  EXPECT_EQ( MDAL_G_driverName( nullptr ), std::string( "" ) );
}

TEST( ApiTest, DatasetsApi )
{
  EXPECT_EQ( MDAL_D_group( nullptr ), nullptr );
  EXPECT_TRUE( std::isnan( MDAL_D_time( nullptr ) ) );
  EXPECT_EQ( MDAL_D_valueCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_D_volumesCount( nullptr ), 0 );
  EXPECT_EQ( MDAL_D_isValid( nullptr ), false );
  EXPECT_EQ( MDAL_D_data( nullptr, 0, 0, MDAL_DataType::SCALAR_DOUBLE, nullptr ), 0 );
  double a, b;
  MDAL_D_minimumMaximum( nullptr, &a, &b );
  EXPECT_TRUE( std::isnan( a ) );
  // do not crash is enough for this
  MDAL_D_minimumMaximum( nullptr, &a, nullptr );
  MDAL_D_minimumMaximum( nullptr, nullptr, &b );
}

static std::string receivedLogMessage;
static MDAL_LogLevel receivedLogLevel;

void _testLoggerCallback( MDAL_LogLevel logLevel, MDAL_Status, const char *mssg )
{
  receivedLogMessage = mssg;
  receivedLogLevel = logLevel;
}

TEST( ApiTest, LoggerApi )
{
  MDAL_SetLoggerCallback( &_testLoggerCallback );
  MDAL_SetLogVerbosity( MDAL_LogLevel::Debug );

  // obvious silly call to test logger call
  auto dr = MDAL_driverFromIndex( -1 );

  EXPECT_EQ( dr, nullptr );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::Err_MissingDriver );
  EXPECT_EQ( receivedLogLevel, MDAL_LogLevel::Error );
  EXPECT_EQ( receivedLogMessage, "No driver with index: -1" );

  // test reset status
  MDAL_ResetStatus();
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );

  // test set Error
  MDAL_SetStatus( MDAL_LogLevel::Error, MDAL_Status::Err_NotEnoughMemory, "Test" );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::Err_NotEnoughMemory );
  EXPECT_EQ( receivedLogLevel, MDAL_LogLevel::Error );
  EXPECT_EQ( receivedLogMessage, "Test" );

  // test set Warning
  MDAL_SetStatus( MDAL_LogLevel::Warn, MDAL_Status::Warn_InvalidElements, "Test1" );
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::Warn_InvalidElements );
  EXPECT_EQ( receivedLogLevel, MDAL_LogLevel::Warn );
  EXPECT_EQ( receivedLogMessage, "Test1" );

  // test set Info
  MDAL_SetStatus( MDAL_LogLevel::Info, MDAL_Status::Warn_InvalidElements, "Test2" );
  // Note - the status should be actually used
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
  EXPECT_EQ( receivedLogLevel, MDAL_LogLevel::Info );
  EXPECT_EQ( receivedLogMessage, "Test2" );

  // test set Debug
  MDAL_SetStatus( MDAL_LogLevel::Debug, MDAL_Status::Warn_InvalidElements, "Test3" );
  // Note - the status should be actually used
  EXPECT_EQ( MDAL_LastStatus(), MDAL_Status::None );
  EXPECT_EQ( receivedLogLevel, MDAL_LogLevel::Debug );
  EXPECT_EQ( receivedLogMessage, "Test3" );
}

TEST( ApiTest, MeshNamesApi )
{
  MDAL_SetLoggerCallback( &_testLoggerCallback );
  MDAL_SetLogVerbosity( MDAL_LogLevel::Debug );

  std::vector<std::pair<std::string, std::string>> testScenarios
  {
    {
      test_file( "/2dm/regular_grid.2dm" ),
      "2DM:\"" + test_file( "/2dm/regular_grid.2dm" ) + "\""
    },
    {
      test_file( "/ugrid/1dtest/dflow1d_map.nc" ),
      "Ugrid:\"" + test_file( "/ugrid/1dtest/dflow1d_map.nc" ) + "\":" + "mesh1d"
    },
    {
      "Invalid_File_Path",
      ""
    },
    {
      test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ),
      "Ugrid:\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\":" + "mesh1d" + ";;"
      + "Ugrid:\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\":" + "mesh2d"
    },
    {
      "\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\"",
      "Ugrid:\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\":" + "mesh1d" + ";;"
      + "Ugrid:\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\":" + "mesh2d"
    },
    {
      "nonExistingDriver:\"" + test_file( "/ugrid/D-Flow1.1/manzese_1d2d_small_map.nc" ) + "\"",
      ""
    },
    {
      "Ugrid:\"" + test_file( "/2dm/regular_grid.2dm" ) + "\"",
      ""
    },
    {
      "",
      ""
    },
  };
#ifdef HAVE_HDF5
  testScenarios.push_back(
  {
    test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ),
    "FLO2D:\"" + test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ) + "\":" + "mesh2d"
  }
  );
  testScenarios.push_back(
  {
    test_file( "/flo2d/BarnyHDF5/TIMP.HDF5" ),
    ""
  }
  );
  testScenarios.push_back(
  {
    "\"\"" + test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ) + "\"\"",
    "FLO2D:\"" + test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ) + "\":" + "mesh2d"
  }
  );
  testScenarios.push_back(
  {
    "2DM:\"" + test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ) + "\"",
    "2DM:\"" + test_file( "/flo2d/BarnHDF5/TIMDEP.HDF5" ) + "\""
  }
  );
#endif
  for ( const std::pair<std::string, std::string> &test : testScenarios )
  {
    EXPECT_EQ( MDAL_MeshNames( test.first.c_str() ), test.second );
  }
  EXPECT_EQ( MDAL_MeshNames( nullptr ), nullptr );
}

TEST( ApiTest, MeshCreationApi )
{
  std::vector<double> coordinates( {0.0, 0.0, 0.0,
                                    1.0, 1.0, 0.0,
                                    2.0, 0.0, 2.0,
                                    1.0, 2.0, 3.0,
                                    0.0, -2.0, 4.0,
                                    2.0, -2.0, 4.0
                                   } );

  std::vector<int> invalidVertexIndices( {0, 7, 3,
                                          1, 2, 3,
                                          4, 5, 2, 0,
                                          0, 2, 1
                                         } );

  std::vector<int> faceSizes( {3, 3, 4, 3} );
  std::vector<int> vertexIndices( {0, 1, 3,
                                   1, 2, 3,
                                   4, 5, 2, 0,
                                   0, 2, 1
                                  } );

  MDAL_MeshH mesh = nullptr;
  MDAL_M_addVertices( mesh, 6, coordinates.data() );
  EXPECT_EQ( MDAL_LastStatus(), Err_IncompatibleMesh );
  MDAL_M_addFaces( mesh, 4, faceSizes.data(), invalidVertexIndices.data() );
  EXPECT_EQ( MDAL_LastStatus(), Err_IncompatibleMesh );
  MDAL_M_setProjection( mesh, "EPSG:32620" );
  EXPECT_EQ( MDAL_LastStatus(), Err_IncompatibleMesh );

  mesh = MDAL_CreateMesh( nullptr );
  EXPECT_EQ( mesh, nullptr );
  EXPECT_EQ( MDAL_LastStatus(), Err_MissingDriver );

  MDAL_DriverH driver = MDAL_driverFromName( "Ugrid" );
  mesh = MDAL_CreateMesh( driver );
  EXPECT_NE( mesh, nullptr );

  EXPECT_EQ( MDAL_M_vertexCount( mesh ), 0 );
  EXPECT_EQ( MDAL_M_faceCount( mesh ), 0 );
  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( mesh ), 0 );

  std::string createdMeshFile = tmp_file( "/createdMeshVoid" );
  MDAL_SaveMesh( mesh, createdMeshFile.c_str(), "Ugrid" );
  EXPECT_EQ( MDAL_LastStatus(), None );

  MDAL_M_addVertices( mesh, 6, coordinates.data() );
  MDAL_M_addFaces( mesh, 4, faceSizes.data(), invalidVertexIndices.data() );

  EXPECT_EQ( MDAL_LastStatus(), Err_InvalidData );
  EXPECT_EQ( MDAL_M_faceCount( mesh ), 0 );

  MDAL_M_addFaces( mesh, 4, faceSizes.data(), vertexIndices.data() );
  EXPECT_EQ( MDAL_M_vertexCount( mesh ), 6 );
  EXPECT_EQ( MDAL_M_faceCount( mesh ), 4 );
  EXPECT_EQ( MDAL_M_faceVerticesMaximumCount( mesh ), 4 );

  createdMeshFile = tmp_file( "/createdMesh" );
  MDAL_SaveMesh( mesh, createdMeshFile.c_str(), "Ugrid" );
  EXPECT_EQ( MDAL_LastStatus(), None );

  MDAL_M_setProjection( mesh, "EPSG:32620" );
  EXPECT_EQ( MDAL_LastStatus(), None );

  MDAL_M_setMetadata( mesh, "test", "value" );
  EXPECT_EQ( MDAL_LastStatus(), None );
  EXPECT_EQ( MDAL_M_metadataCount( mesh ), 1 );
  EXPECT_EQ( std::strcmp( MDAL_M_metadataKey( mesh, 0 ), "test" ), 0 );
  EXPECT_EQ( std::strcmp( MDAL_M_metadataValue( mesh, 0 ), "value" ), 0 );
  EXPECT_EQ( std::strcmp( MDAL_M_metadataKey( mesh, 1 ), "" ), 0 );
  EXPECT_EQ( MDAL_LastStatus(), Err_IncompatibleMesh );
  MDAL_ResetStatus();
  MDAL_M_setMetadata( mesh, "test", "value2" );
  EXPECT_EQ( MDAL_LastStatus(), None );
  EXPECT_EQ( MDAL_M_metadataCount( mesh ), 1 );
  EXPECT_EQ( std::strcmp( MDAL_M_metadataKey( mesh, 0 ), "test" ), 0 );
  EXPECT_EQ( std::strcmp( MDAL_M_metadataValue( mesh, 0 ), "value2" ), 0 );
  MDAL_M_setMetadata( mesh, nullptr, nullptr );
  EXPECT_EQ( MDAL_LastStatus(), Err_InvalidData );

  MDAL_CloseMesh( mesh );

  mesh = MDAL_LoadMesh( createdMeshFile.c_str() );
  EXPECT_EQ( MDAL_M_vertexCount( mesh ), 6 );
  EXPECT_EQ( MDAL_M_faceCount( mesh ), 4 );

  EXPECT_EQ( getVertexXCoordinatesAt( mesh, 1 ), 1.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( mesh, 1 ), 1.0 );
  EXPECT_EQ( getVertexXCoordinatesAt( mesh, 3 ), 1.0 );
  EXPECT_EQ( getVertexYCoordinatesAt( mesh, 3 ), 2.0 );

  EXPECT_EQ( getFaceVerticesCountAt( mesh, 1 ), 3 );
  EXPECT_EQ( getFaceVerticesCountAt( mesh, 2 ), 4 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 2, 0 ), 4 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 2, 1 ), 5 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 2, 2 ), 2 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 2, 3 ), 0 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 3, 0 ), 0 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 3, 1 ), 2 );
  EXPECT_EQ( getFaceVerticesIndexAt( mesh, 3, 2 ), 1 );

  MDAL_CloseMesh( mesh );

  driver = MDAL_driverFromName( "2DM" );
  EXPECT_TRUE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( std::strcmp( MDAL_DR_saveMeshSuffix( driver ), "2dm" ), 0 );

  driver = MDAL_driverFromName( "Ugrid" );
  EXPECT_TRUE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( std::strcmp( MDAL_DR_saveMeshSuffix( driver ), "nc" ), 0 );

  driver = MDAL_driverFromName( "SELAFIN" );
  EXPECT_TRUE( MDAL_DR_saveMeshCapability( driver ) );
  EXPECT_EQ( std::strcmp( MDAL_DR_saveMeshSuffix( driver ), "slf" ), 0 );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

