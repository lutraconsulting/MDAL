/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_dhi_dfsu.hpp"

#include <cassert>

bool MeshDfsu::canRead( const std::string &uri )
{
  LPFILE      Fp = nullptr;;
  LPHEAD      pdfs = nullptr;;
  LPCTSTR fileName = uri.c_str();

  bool ok = false;
  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  if ( rc == F_NO_ERROR )
  {
    int totalNodeCount;
    int elementCount;
    int dimension;
    int maxNumberOfLayer;
    int numberOfSigmaLayer;

    if ( fileInfo( pdfs, totalNodeCount, elementCount, dimension, maxNumberOfLayer, numberOfSigmaLayer ) )
      ok = dimension == 2 || dimension == 3;
  }

  dfsFileClose( pdfs, &Fp );
  dfsHeaderDestroy( &pdfs );

  return ok;

}

std::unique_ptr<MeshDfsu> MeshDfsu::loadMesh( const std::string &uri )
{
  LPFILE      Fp = nullptr;
  LPHEAD      pdfs = nullptr;;
  LPCTSTR fileName = uri.c_str();

  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  bool ok = false;

  if ( rc == F_NO_ERROR )
  {
    int totalNodeCount;
    int elementCount;
    int dimension;
    int maxNumberOfLayer;
    int numberOfSigmaLayer;

    if ( fileInfo( pdfs, totalNodeCount, elementCount, dimension, maxNumberOfLayer, numberOfSigmaLayer ) )
    {
      std::unique_ptr<MeshDfsu> mesh( new MeshDfsu );
      mesh->mFp = Fp;
      mesh->mPdfs = pdfs;
      mesh->mIs3D = dimension == 3;
      mesh->mMaxNumberOfLayer = maxNumberOfLayer;
      mesh->mTotalNodeCount = size_t( totalNodeCount );
      mesh->mTotalElementCount = size_t( elementCount );

      GeoInfoType geoInfoType = dfsGetGeoInfoType( pdfs );
      LPCTSTR projectionWkt;
      if ( geoInfoType == F_UTM_PROJECTION )
      {
        dfsGetGeoInfoUTMProj( pdfs, &projectionWkt, nullptr, nullptr, nullptr );
        mesh->mWktProjection = projectionWkt;
      }

      if ( mesh->populateMeshFrame() && mesh->populateDatasetGroups() )
        return mesh;
    }
  }

  dfsFileClose( pdfs, &Fp );
  dfsHeaderDestroy( &pdfs );
  return nullptr;
}

int MeshDfsu::verticesCount() const
{
  return static_cast<int>( mVertexCoordinates.size() / 3 );
}

int MeshDfsu::facesCount() const
{
  return static_cast<int>( mFaceNodeCount.size() );
}

size_t MeshDfsu::vertexIdToIndex( int id ) const
{
  if ( !mNodeId2VertexIndex.empty() )
  {
    std::map<int, size_t>::const_iterator it = mNodeId2VertexIndex.find( id );
    if ( it != mNodeId2VertexIndex.end() )
      return it->second;
  }

  return static_cast<size_t>( id - mGapFromVertexToNode );
}

int MeshDfsu::nodeCount( size_t faceIndex ) const
{
  return static_cast<size_t>( mFaceNodeCount.at( faceIndex ) );
}

size_t MeshDfsu::connectivityPosition( int faceIndex ) const
{
  size_t fi = static_cast<size_t>( faceIndex );
  if ( fi == mNextFaceIndexForConnectivity )
    return mNextConnectivityPosition;
  size_t conPos = 0;
  for ( int i = 0; i < fi; i++ )
    conPos += mFaceNodeCount[i];

  return conPos;
}

bool MeshDfsu::fileInfo( LPHEAD pdfs, int &totalNodeCount, int &elementCount, int &dimension, int &maxNumberOfLayer, int &numberOfSigmaLayer )
{
  bool ok = false;
  LONG dataType = dfsGetDataType( pdfs );
  if ( dataType == 2001 || dataType != 2000 ) //dfsu file
  {
    LPBLOCK customBlock;
    LONG error = dfsGetCustomBlockRef( pdfs, &customBlock );
    if ( error == F_NO_ERROR )
    {
      // Search for "MIKE_FM" custom block containing int data
      while ( customBlock )
      {
        SimpleType blocDataType;
        LPCTSTR name;
        LONG size;
        void *customBlockData = nullptr;
        error = dfsGetCustomBlock( customBlock, &blocDataType, &name, &size, &customBlockData, &customBlock );
        if ( error != F_NO_ERROR )
          break;

        if ( 0 == strcmp( name, "MIKE_FM" ) && blocDataType == UFS_INT )
        {
          int *intData = static_cast<int *>( customBlockData );

          totalNodeCount = intData[0];
          elementCount = intData[1];
          dimension = intData[2];
          maxNumberOfLayer = intData[3];
          numberOfSigmaLayer = intData[4];
          ok = true;
          break;
        }
      }
    }
  }

  return ok;
}

bool MeshDfsu::populateMeshFrame()
{
  mVertexCoordinates.resize( mTotalNodeCount * 3 );
  mFaceNodeCount.resize( mTotalElementCount );

  if ( mIs3D )
    return populate3DMeshFrame();
  else
    return populate2DMeshFrame();
}

bool MeshDfsu::populate2DMeshFrame()
{
  LPVECTOR pvec;
  LONG error;
  LONG          itemType;
  LPCTSTR       itemName;
  SimpleType    itemDataType;

  bool fail = false;

  while ( ( pvec = dfsStaticRead( mFp, &error ) ) != nullptr && !fail )
  {
    LPITEM staticItem;
    staticItem = dfsItemS( pvec );
    dfsGetItemInfo_( staticItem, &itemType, &itemName, nullptr, &itemDataType );

    if ( lstrcmp( itemName, "Node id" ) == 0 )
    {
      size_t nodeIdCount = static_cast<size_t>( dfsGetItemElements( staticItem ) );
      size_t size = dfsGetItemBytes( staticItem );
      if ( nodeIdCount * sizeof( int ) == size  && nodeIdCount == mTotalNodeCount )
      {
        if ( nodeIdCount == mVertexCoordinates.size() / 3 )
        {
          std::vector<int> vertexToNode( nodeIdCount );
          dfsStaticGetData( pvec, vertexToNode.data() );

          if ( !vertexToNode.empty() )
          {
            mGapFromVertexToNode = vertexToNode.at( 0 );
            for ( size_t i = 1; i < vertexToNode.size(); ++i )
              if ( vertexToNode.at( i ) - i != mGapFromVertexToNode )
                mNodeId2VertexIndex[vertexToNode.at( i )] = i;
          }
        }
        else
          fail = true;
      }
      else
        fail = true;
    }
    else if ( lstrcmp( itemName, "X-coord" ) == 0 )
    {
      fail = !setCoordinate( pvec, staticItem, itemDataType, 0, mXmin, mXmax );
    }
    else if ( lstrcmp( itemName, "Y-coord" ) == 0 )
    {
      fail = !setCoordinate( pvec, staticItem, itemDataType, 1, mYmin, mYmax );
    }
    else if ( lstrcmp( itemName, "Z-coord" ) == 0 )
    {
      double zMin, zMax;
      fail = !setCoordinate( pvec, staticItem, itemDataType, 2, zMin, zMax );
    }
    else if ( lstrcmp( itemName, "Element id" ) == 0 )
    {
      // not needed, in any cases, the element ad associated data are always stored in the same order
    }
    else if ( lstrcmp( itemName, "Element type" ) == 0 )
    {
      // not needed, use directly "No of nodes"
    }
    else if ( lstrcmp( itemName, "No of nodes" ) == 0 )
    {
      size_t elemCount = static_cast<size_t>( dfsGetItemElements( staticItem ) );
      size_t size = dfsGetItemBytes( staticItem );
      if ( elemCount * sizeof( int ) == size )
      {
        if ( mFaceNodeCount.empty() )
          mFaceNodeCount.resize( elemCount );

        if ( elemCount == mFaceNodeCount.size() )
        {
          error = dfsStaticGetData( pvec, mFaceNodeCount.data() );
          if ( error != F_NO_ERROR )
            fail = true;
        }
        else
          fail = true;
      }
      else
        fail = true;
    }
    else if ( lstrcmp( itemName, "Connectivity" ) == 0 )
    {
      size_t valueCount = static_cast<size_t>( dfsGetItemElements( staticItem ) );
      size_t size = dfsGetItemBytes( staticItem );
      if ( valueCount * sizeof( int ) == size )
      {
        mConnectivity.resize( valueCount );
        error = dfsStaticGetData( pvec, mConnectivity.data() );
        if ( error != F_NO_ERROR )
          fail = true;
        //transform node id to vertex indexes
        if ( !fail )
          for ( size_t i = 0; i < mConnectivity.size(); ++i )
            mConnectivity[i] = static_cast<int>( vertexIdToIndex( mConnectivity.at( i ) ) );
      }
      else
        fail = true;
    }

    dfsStaticDestroy( &pvec );
  }

  return !fail;
}

bool MeshDfsu::populate3DMeshFrame()
{
  //load all the element connectivity and vertices
  if ( !populate2DMeshFrame() )
    return false;
  //now, we have all the data, we can build the real 2D mesh

  std::vector<int> connecticity2D;
  std::vector<int> faceNodeCount2D;
  std::unordered_map<int, int> vertices3DToVertices2D;

  size_t connectivityPosition = 0;
  size_t currentColumnSize = 0;
  std::vector<size_t> bottomVerticesCount;
  std::vector<double> bottomVerticesZValueSum;
  std::vector<double> currentBottomZvalue;
  VertexIndexesOfLevelsOnFace currentLevels;
  size_t totalVolumeCount = 0;

  size_t elementCount = mFaceNodeCount.size();

  for ( size_t fi = 0; fi < elementCount; ++fi )
  {
    size_t elementSize = mFaceNodeCount.at( fi );
    size_t faceSize = elementSize / 2;
    size_t connectivityPositionTop = connectivityPosition + faceSize;

    VertexIndexesOfLevel level;
    for ( size_t n = 0; n < faceSize; ++n )
      level.push_back( mConnectivity.at( connectivityPosition + n ) );
    currentLevels.push_back( std::move( level ) );

    bool topExist = true;
    if ( fi < elementCount - 1 )
    {
      for ( size_t n = 0; n < faceSize; ++n )
      {
        topExist = mConnectivity.at( connectivityPositionTop + n ) == mConnectivity.at( connectivityPositionTop + n + faceSize );
        if ( !topExist )
          break;
      }
    }
    else
      topExist = false;

    if ( currentColumnSize == 0 )
    {
      currentBottomZvalue.clear();
      for ( size_t n = 0; n < faceSize; ++n )
      {
        int vertexIndex3D = mConnectivity.at( connectivityPosition + n );
        currentBottomZvalue.push_back( mVertexCoordinates.at( vertexIndex3D * 3 + 2 ) );
      }
    }

    if ( ! topExist )
    {
      VertexIndexesOfLevel lastLevel;
      faceNodeCount2D.push_back( int( faceSize ) );

      for ( size_t n = 0; n < faceSize; ++n )
      {
        int vertexIndex3D = mConnectivity.at( connectivityPositionTop + n );
        lastLevel.push_back( vertexIndex3D );

        std::unordered_map<int, int>::const_iterator it = vertices3DToVertices2D.find( vertexIndex3D );

        int vertexIndex2D;
        if ( it == vertices3DToVertices2D.end() )
        {
          vertexIndex2D = int( vertices3DToVertices2D.size() );
          bottomVerticesZValueSum.push_back( 0.0 );
          bottomVerticesCount.push_back( 0 );
          vertices3DToVertices2D[vertexIndex3D] = vertexIndex2D;
        }
        else
        {
          vertexIndex2D = it->second;
        }

        connecticity2D.push_back( vertexIndex2D );
        mFaceToVolume.push_back( int( fi ) );
        bottomVerticesZValueSum[vertexIndex2D] += currentBottomZvalue.at( n );
        bottomVerticesCount[vertexIndex2D] += 1;
      }

      currentLevels.push_back( std::move( lastLevel ) );

      mLevels.push_back( std::move( currentLevels ) );
      currentLevels = std::vector<std::vector<int>>();

      totalVolumeCount += currentColumnSize;
      currentColumnSize = 0;
    }
    else
      currentColumnSize++;

    connectivityPosition += elementSize;
  }

  size_t vertex3DCount = mVertexCoordinates.size() / 3;


  std::vector<double> vertexCoordinates2D( vertices3DToVertices2D.size() * 3 );
  for ( std::unordered_map<int, int>::const_iterator it = vertices3DToVertices2D.cbegin(); it != vertices3DToVertices2D.cend(); ++it )
  {
    int vertexIndex3D = it->first;
    int vertexIndex2D = it->second;
    for ( size_t vc = 0; vc < 2; ++vc )
      vertexCoordinates2D[vertexIndex2D * 3 + vc] = mVertexCoordinates.at( vertexIndex3D * 3 + vc );
  }

  // now vertices of the 2D mesh are on the top levels, to be representative of the terrain elevation,
  // we need to take the vertices of the bottom level. But with non Sigma Z layer,
  // there could be several vertices on the bottom (staircase bottom). We take the average value
  for ( size_t vi = 0; vi < vertexCoordinates2D.size() / 3; ++vi )
    vertexCoordinates2D[vi * 3 + 2] = bottomVerticesZValueSum.at( vi ) / bottomVerticesCount.at( vi );

  mVertexCoordinates = std::move( vertexCoordinates2D );
  mConnectivity = std::move( connecticity2D );
  mFaceNodeCount = std::move( faceNodeCount2D );

  mLevelGenerator = std::make_unique<LevelValuesGenerator>( mFp, mPdfs, mLevels, vertex3DCount );

  return true;
}

bool MeshDfsu::setCoordinate( LPVECTOR pvec, LPITEM staticItem, SimpleType itemDatatype, size_t offset, double &min, double &max )
{
  LONG valueCount = dfsGetItemElements( staticItem );
  size_t size = dfsGetItemBytes( staticItem );
  bool coordinateIsDouble = itemDatatype == UFS_DOUBLE;
  if ( valueCount * ( coordinateIsDouble ? sizeof( double ) : sizeof( float ) ) == size )
  {
    if ( valueCount * 3 == mVertexCoordinates.size() )
    {
      if ( coordinateIsDouble )
      {
        std::vector<double> values( valueCount );
        LONG error = dfsStaticGetData( pvec, values.data() );
        if ( error != F_NO_ERROR )
          return false;
        for ( int i = 0; i < valueCount; ++i )
        {
          mVertexCoordinates[i * 3 + offset] = values.at( i );
          if ( values.at( i ) < min )
            min = values.at( i );
          if ( values.at( i ) > max )
            max = values.at( i );
        }
      }
      else
      {
        std::vector<float> values( valueCount );
        LONG error = dfsStaticGetData( pvec, values.data() );
        if ( error != F_NO_ERROR )
          return false;
        for ( int i = 0; i < valueCount; ++i )
        {
          mVertexCoordinates[i * 3 + offset] = values.at( i );
          if ( values.at( i ) < min )
            min = values.at( i );
          if ( values.at( i ) > max )
            max = values.at( i );
        }
      }

      return true;
    }
  }

  return false;
}
