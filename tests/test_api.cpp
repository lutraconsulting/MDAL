/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/
#include "gtest/gtest.h"

//mdal
#include "mdal.h"
#include "mdal_testutils.hpp"

TEST( ApiTest, DriversApi )
{
  int driversCount = MDAL_driverCount();
  ASSERT_TRUE( driversCount > 2 ); // variable based on the available drivers on system
  DriverH dr = MDAL_driverFromIndex( 0 );
  ASSERT_TRUE( dr );

  std::string name = MDAL_DR_name( dr );
  ASSERT_EQ( name, "2DM" );

  std::string longName = MDAL_DR_longName( dr );
  ASSERT_EQ( longName, "2DM Mesh File" );

  bool meshLoad = MDAL_DR_meshLoadCapability( dr );
  ASSERT_TRUE( meshLoad );

  std::string filters = MDAL_DR_filters( dr );
  ASSERT_EQ( filters, "*.2dm" );
}

void _populateFaces( MeshH m, std::vector<int> &ret, size_t faceOffsetsBufferLen, size_t vertexIndicesBufferLen )
{
  int facesCount = MDAL_M_faceCount( m );
  ret.resize( 0 );
  std::vector<int> faceOffsetsBuffer( faceOffsetsBufferLen );
  std::vector<int> vertexIndicesBuffer( vertexIndicesBufferLen );

  MeshFaceIteratorH it = MDAL_M_faceIterator( m );
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
  MeshH m = MDAL_LoadMesh( path.c_str() );
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

    compareVectors( refIndices, indices );
  }

  {
    std::vector<int> indices;
    _populateFaces( m,
                    indices,
                    13,
                    4 * 13
                  );

    compareVectors( refIndices, indices );
  }

  {
    std::vector<int> indices;
    _populateFaces( m,
                    indices,
                    3,
                    1000
                  );

    compareVectors( refIndices, indices );
  }
  MDAL_CloseMesh( m );
}

void _populateVertices( MeshH m, std::vector<double> &ret, size_t itemsLen )
{
  int verticesCount = MDAL_M_vertexCount( m );
  ret.resize( 0 );
  std::vector<double> coordsBuffer( itemsLen * 3 );

  MeshVertexIteratorH it = MDAL_M_vertexIterator( m );
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
  MeshH m = MDAL_LoadMesh( path.c_str() );
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

    compareVectors( refCoors, coords );
  }

  {
    std::vector<double> coords;
    _populateVertices( m,
                       coords,
                       10000
                     );

    compareVectors( refCoors, coords );
  }
  MDAL_CloseMesh( m );
}

int main( int argc, char **argv )
{
  testing::InitGoogleTest( &argc, argv );
  init_test();
  int ret =  RUN_ALL_TESTS();
  finalize_test();
  return ret;
}

