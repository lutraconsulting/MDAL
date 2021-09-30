/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_dhi_dfsu.hpp"

#include <cassert>

Mesh::~Mesh()
{
  close();
}

bool Mesh::canRead( const std::string &uri )
{
  LPFILE      Fp;
  LPHEAD      pdfs;
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

std::unique_ptr<Mesh> Mesh::loadMesh( const std::string &uri )
{
  LPFILE      Fp;
  LPHEAD      pdfs;
  LPCTSTR fileName = uri.c_str();

  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  if ( rc != F_NO_ERROR )
    return nullptr;

  int totalNodeCount;
  int elementCount;
  int dimension;
  int maxNumberOfLayer;
  int numberOfSigmaLayer;

  if ( !fileInfo( pdfs, totalNodeCount, elementCount, dimension, maxNumberOfLayer, numberOfSigmaLayer ) )
    return nullptr;

  std::unique_ptr<Mesh> mesh( new Mesh );
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

  if ( !mesh->populateMeshFrame() )
    return nullptr;

  if ( !mesh->populateDatasetGroups() )
    return nullptr;

  return mesh;
}

void Mesh::close()
{
  dfsFileClose( mPdfs, &mFp );
  dfsFileClose( mPdfs, &mFp );
}

int Mesh::verticesCount() const
{
  return static_cast<int>( mVertexCoordinates.size() / 3 );
}

int Mesh::facesCount() const
{
  return static_cast<int>( mFaceNodeCount.size() );
}

//! return a pointer to the vertices coordinates for \a index

double *Mesh::vertexCoordinates( int index )
{
  return &mVertexCoordinates[static_cast<size_t>( index * 3 )];
}

int Mesh::connectivity( int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBuffer )
{
  int maxFaceCount = std::max( 0, std::min( facesCount() - startFaceIndex, faceCount ) );
  size_t conPos = connectivityPosition( startFaceIndex );
  size_t conCount = 0;
  std::vector<int> faceOffset( static_cast<size_t>( maxFaceCount ) );
  size_t effectiveFaceCount = 0;
  for ( size_t i = 0; i < maxFaceCount; ++i )
  {
    if ( conCount + mFaceNodeCount[i + startFaceIndex] > vertexIndicesBufferLen )
      break;
    conCount += mFaceNodeCount[i + startFaceIndex];
    faceOffset[i] = static_cast<int>( conCount );
    effectiveFaceCount++;
  }

  if ( startFaceIndex + effectiveFaceCount < mFaceNodeCount.size() )
  {
    mNextFaceIndexForConnectivity = startFaceIndex + effectiveFaceCount;
    mNextConnectivityPosition = conPos + conCount;
  }

  memcpy( faceOffsetsBuffer, faceOffset.data(), static_cast<size_t>( effectiveFaceCount ) * sizeof( int ) );
  memcpy( vertexIndicesBuffer, &mConnectivity[conPos], conCount * sizeof( int ) );

  return static_cast<int>( effectiveFaceCount );
}

//! Returns mesh extent

void Mesh::extent( double *xMin, double *xMax, double *yMin, double *yMax ) const
{
  *xMin = mXmin;
  *xMax = mXmax;
  *yMin = mYmin;
  *yMax = mYmax;
}

int Mesh::datasetGroupsCount() const
{
  return static_cast<int>( mDatasetGroups.size() );
}

//! Returns reference time string (ISO8601)

const std::string &Mesh::referenceTime() const
{
  return mReferenceTime;
}

int Mesh::timeStepCount() const
{
  return mTimeStepCount;
}

DatasetGroup *Mesh::datasetgroup( int i ) const
{
  if ( i >= 0 && static_cast<size_t>( i ) < mDatasetGroups.size() )
    return mDatasetGroups.at( i ).get();
  else
    return nullptr;
}

double Mesh::time( int index ) const
{
  if ( index < 0 )
    return 0;
  size_t i = static_cast<int>( index );
  if ( i < mTimes.size() )
    return mTimes.at( static_cast<int>( index ) );
  else
    return 0;
}

bool Mesh::is3D() const
{
  return mIs3D;
}

size_t Mesh::vertexIdToIndex( int id ) const
{
  if ( !mNodeId2VertexIndex.empty() )
  {
    std::map<int, size_t>::const_iterator it = mNodeId2VertexIndex.find( id );
    if ( it != mNodeId2VertexIndex.end() )
      return it->second;
  }

  return static_cast<size_t>( id - mGapFromVertexToNode );
}

size_t Mesh::connectivityPosition( int faceIndex ) const
{
  size_t fi = static_cast<size_t>( faceIndex );
  if ( fi == mNextFaceIndexForConnectivity )
    return mNextConnectivityPosition;
  size_t conPos = 0;
  for ( int i = 0; i < fi; i++ )
    conPos += mFaceNodeCount[i];

  return conPos;
}

bool Mesh::fileInfo( LPHEAD pdfs, int &totalNodeCount, int &elementCount, int &dimension, int &maxNumberOfLayer, int &numberOfSigmaLayer )
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
        SimpleType dataType;
        LPCTSTR name;
        LONG size;
        void *customblockData = nullptr;
        error = dfsGetCustomBlock( customBlock, &dataType, &name, &size, &customblockData, &customBlock );
        if ( error != F_NO_ERROR )
          break;

        if ( 0 == strcmp( name, "MIKE_FM" ) && dataType == UFS_INT )
        {
          int *intData = static_cast<int *>( customblockData );

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

bool Mesh::populateMeshFrame()
{
  mVertexCoordinates.resize( mTotalNodeCount * 3 );
  mFaceNodeCount.resize( mTotalElementCount );

  if ( mIs3D )
    return populate3DMeshFrame();
  else
    return populate2DMeshFrame();

  return false;
}

bool Mesh::populate2DMeshFrame()
{
  LPVECTOR pvec;
  LONG error;
  LONG          itemType;
  LPCTSTR       itemName;
  SimpleType    itemDataType;

  bool fail = false;

  while ( ( pvec = dfsStaticRead( mFp, &error ) ) && !fail )
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

bool Mesh::populate3DMeshFrame()
{
  //load all the element connectivity and vertices
  if ( !populate2DMeshFrame() )
    return false;
  //now, we have all the data, we can build the real 2D mesh

  std::vector<int> connecticity2D;
  std::vector<int> faceNodeCount2D;
  std::unordered_map<int, int> vertices3DToVertices2D;

  size_t connectivityPosition = 0;
  size_t maxColumneSize = 0;
  size_t currentColumnSize = 0;
  std::vector<size_t> bottomVerticesCount;
  std::vector<double> bottomVerticesZValueSum;
  std::vector<double> currentBottomZvalue;
  size_t maxPos = 0;
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
      faceNodeCount2D.push_back( faceSize );

      for ( size_t n = 0; n < faceSize; ++n )
      {
        int vertexIndex3D = mConnectivity.at( connectivityPositionTop + n );
        lastLevel.push_back( vertexIndex3D );

        std::unordered_map<int, int>::const_iterator it = vertices3DToVertices2D.find( vertexIndex3D );

        int vertexIndex2D;
        if ( it == vertices3DToVertices2D.end() )
        {
          vertexIndex2D = vertices3DToVertices2D.size();
          bottomVerticesZValueSum.push_back( 0.0 );
          bottomVerticesCount.push_back( 0 );
          vertices3DToVertices2D[vertexIndex3D] = vertexIndex2D;
        }
        else
        {
          vertexIndex2D = it->second;
        }

        connecticity2D.push_back( vertexIndex2D );
        mFaceToVolume.push_back( fi );
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

bool Mesh::setCoordinate( LPVECTOR pvec, LPITEM staticItem, SimpleType itemDatatype, size_t offset, double &min, double &max )
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

static std::vector<std::string> split( const std::string &str,
                                       const char delimiter )
{
  std::vector<std::string> list;
  std::string::const_iterator start = str.begin();
  std::string::const_iterator end = str.end();
  std::string::const_iterator next;
  std::string token;
  do
  {
    next = std::find( start, end, delimiter );
    token = std::string( start, next );
    if ( !token.empty() )
      list.push_back( token );

    if ( next == end )
      break;
    else
      start = next + 1;
  }
  while ( true );
  return list;
}


static bool isVector( const std::string &rawName, std::string &name, bool &isX )
{
  std::vector<std::string> splitName = split( rawName, ' ' );
  if ( splitName.empty() )
    return false;

  size_t vectorIndicatorPos = 0;
  bool isVector = false;

  while ( vectorIndicatorPos < splitName.size() && !isVector )
  {
    if ( splitName.at( vectorIndicatorPos ) == "U" )
    {
      isX = true;
      isVector = true;
    }
    else if ( splitName.at( vectorIndicatorPos ) == "V" )
    {
      isX = false;
      isVector = true;
    }
    else
      vectorIndicatorPos++;
  }

  if ( isVector )
  {
    splitName.erase( splitName.begin() + vectorIndicatorPos );
    if ( splitName.empty() )
      name.clear();
    else
      name = splitName.front();

    for ( size_t i = 1; i < splitName.size(); ++i )
      name += " " + splitName.at( i );

    std::transform( name.begin(), name.begin() + 1, name.begin(), toupper );
  }

  return isVector;
}

static bool isVelocity(LONG itemType, std::string& name, bool& isX, bool &isVerticalComponent)
{
    LPCTSTR eumIdent;
    if (!eumGetItemTypeIdent(itemType, &eumIdent))
        return false;

    std::string ident(eumIdent);

    if (std::strcmp(eumIdent, "eumIuVelocity") == 0)
    {
        isX = true;
        isVerticalComponent = false;
        name = "Velocity";
        return true;
    }

    if (std::strcmp(eumIdent, "eumIvVelocity") == 0)
    {
        isX = false;
        isVerticalComponent = false;
        name = "Velocity";
        return true;
    }

    if (std::strcmp(eumIdent, "eumIwVelocity") == 0)
    {
        isX = false;
        isVerticalComponent = true;
        name = "Vertical velocity";
        return true;
    }

    isX = false;
    isVerticalComponent = false;
    return false;
}


static double convertTimeToHours( double time, LONG timeUnit )
{
  LONG idHour = 0;
  double result = time;
  LPCTSTR ident = "hour";
  LONG count = eumGetItemTypeCount();
  if ( eumGetUnitTag( ident, &idHour ) )
    eumConvertUnit( timeUnit, time, idHour, &result );

  return result;
}

typedef std::map<std::string, std::pair<LONG, bool>> VectorGroups; // used to temporarily store component of vector datasetgroup until the second component is found

bool Mesh::populateDatasetGroups()
{

  TimeAxisType timeType = dfsGetTimeAxisType( mPdfs );
  assert( timeType == F_CAL_EQ_AXIS );

  LONG dynamicItemCount = dfsGetNoOfItems( mPdfs );

  LPCTSTR stringStartDate, stringStartTime;
  LONG nTimeUnit;
  LPCTSTR timeUnit;
  double start = 0;
  double step = 0;
  LONG timeStepCount = 0;

  long error = dfsGetEqCalendarAxis( mPdfs, &stringStartDate, &stringStartTime, &nTimeUnit, &timeUnit, &start, &step, &timeStepCount, nullptr );
  if ( error != F_NO_ERROR )
    return false;

  double startTime = convertTimeToHours( start, nTimeUnit );
  double timeStep = convertTimeToHours( step, nTimeUnit );
  mTimeStepCount = static_cast<int>( timeStepCount );
  mReferenceTime = std::string( stringStartDate ) + std::string( "T" ) + std::string( stringStartTime );
  mTimes.resize( static_cast<size_t>( timeStepCount ) );
  for ( size_t i = 0; i < mTimes.size(); ++i )
    mTimes[i] = startTime + i * timeStep;

  float floatDelete = dfsGetDeleteValFloat( mPdfs );
  double doubleDelete = dfsGetDeleteValDouble( mPdfs );

  VectorGroups vectorGroups;

  LONG startItemIndex;

  if ( mIs3D )
  {
    LPITEM dynItem = dfsItemD( mPdfs, 1 );
    LONG          itemType;
    LPCTSTR       itemName;
    LPCTSTR       itemUnit;
    SimpleType    itemDataType;

    error = dfsGetItemInfo_( dynItem, &itemType, &itemName, &itemUnit, &itemDataType );
    if ( error != F_NO_ERROR )
      return false;

    bool doublePrecision = itemDataType == UFS_DOUBLE;
    mLevelGenerator->initializeTimeStep( timeStepCount, doublePrecision, doubleDelete, floatDelete );

    for ( int i = 0; i < 9; ++i )
      mLevelGenerator->totalVolumesCount( i );

    startItemIndex = 2;
  }
  else
  {
    startItemIndex = 1;
  }

  for ( LONG i = startItemIndex; i <= dynamicItemCount; ++i )
  {
    LPITEM dynItem = dfsItemD( mPdfs, i );

    LONG          itemType;
    LPCTSTR       itemName;
    LPCTSTR       itemUnit;
    SimpleType    itemDataType;

    // populate groups
    error = dfsGetItemInfo_( dynItem, &itemType, &itemName, &itemUnit, &itemDataType );
    if ( error != F_NO_ERROR )
      return false;

    bool doublePrecision = itemDataType == UFS_DOUBLE;
    if ( !doublePrecision )
      assert( itemDataType == UFS_FLOAT );

    std::string name = itemName;
    std::string groupName;
    bool isX;

    bool isVectorDataset = false;
    bool isVerticalVelocity=false;

    isVectorDataset = isVelocity(itemType, groupName, isX, isVerticalVelocity);

    if (!isVectorDataset)
        isVectorDataset = isVector(itemName, groupName, isX);

    if (isVectorDataset && !isVerticalVelocity)
    {
      VectorGroups::const_iterator it = vectorGroups.find( groupName );
      if ( it == vectorGroups.end() )
        vectorGroups[groupName] = { i, isX };
      else
      {
        LONG idx = i;
        LONG idy = it->second.first;
        if ( it->second.second ) //the recorded group is X, so the new one is Y
        {
          idx = idy;
          idy = i;
        }
        mDatasetGroups.emplace_back( new DatasetGroup( groupName, itemUnit, idx, idy, doublePrecision, mFp, mPdfs ) );
        mDatasetGroups.back()->setLevelValueGenerator( mLevelGenerator.get() );
      }
    }
    else
    {
        if (isVerticalVelocity)
            name = groupName;
      mDatasetGroups.emplace_back( new DatasetGroup( name, itemUnit, i, doublePrecision, mFp, mPdfs ) );
      mDatasetGroups.back()->setLevelValueGenerator( mLevelGenerator.get() );
    }
  }

  // fill dataset group
  for ( std::unique_ptr < DatasetGroup> &group : mDatasetGroups )
    group->init( mTimeStepCount, mTotalElementCount, doubleDelete, floatDelete );

  return true;
}

Dataset::Dataset( LPFILE Fp, LPHEAD Pdfs, LONG timeStepNo, size_t size, bool doublePrecision, double doubleDeleteValue, float floatDeleteValue ) :
  mFp( Fp ),
  mPdfs( Pdfs ),
  mTimeStepNo( timeStepNo ),
  mSize( size ),
  mIsDoublePrecision( doublePrecision ),
  mDoubleDeleteValue(doubleDeleteValue),
  mFloatDeleteValue(floatDeleteValue)
{}

int Dataset::getActive( int indexStart, int count, int *buffer )
{
  if ( !mLoaded )
    getData( 0, 0, nullptr ); //too load the data

  if ( buffer == nullptr )
    return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mActive.size() ) - indexStart ) );
  int *dataStart = &mActive[indexStart];

  memcpy( buffer, dataStart, sizeof( int )*effectiveCount );

  return effectiveCount;
}

void Dataset::unload()
{
  mData.clear();
  mData.shrink_to_fit();
  mActive.clear();
  mActive.shrink_to_fit();
  mLoaded = false;
}

// read all data and put them in pointed array

bool Dataset::readData( LONG itemNo, void *ptr ) const
{
  double time;
  LONG err = dfsFindItemDynamic( mPdfs, mFp, mTimeStepNo, itemNo );
  if ( err != F_NO_ERROR )
    return false;

  err = dfsReadItemTimeStep( mPdfs, mFp, &time, ptr );
  if ( err != F_NO_ERROR )
    return false;

  return true;
}

ScalarDataset::ScalarDataset( LPFILE Fp,
                              LPHEAD Pdfs,
                              LONG timeStepNo,
                              LONG itemNo,
                              size_t size,
                              bool doublePrecision,
                              double deleteDoubleValue,
                              float deleteFloatValue ) :
  Dataset( Fp, Pdfs, timeStepNo, size, doublePrecision, deleteDoubleValue, deleteFloatValue),
  mItemNo( itemNo )
{}
int ScalarDataset::getData( int indexStart, int count, double *buffer )
{
  if ( !mLoaded )
  {
    mData.resize( mSize );
    mActive.resize( mSize );
    if ( mIsDoublePrecision )
    {
      if ( !readData( mItemNo, mData.data() ) )
        return 0;

      for ( size_t i = 0; i < mSize; ++i )
        if ( mData.at( i ) == mDoubleDeleteValue )
          mActive[i] = 0;
        else
          mActive[i] = 1;
    }
    else
    {
      std::vector<float> floatData( mSize, -1 );
      if ( !readData( mItemNo, floatData.data() ) )
        return 0;

      for ( size_t i = 0; i < mSize; ++i )
        if ( floatData.at( i ) == mFloatDeleteValue )
          mActive[i] = 0;
        else
          mActive[i] = 1;

      for ( size_t i = 0; i < mSize; ++i )
        mData[i] = floatData[i];
    }
    mLoaded = true;
  }

  if (buffer == nullptr)
      return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) - indexStart ) );
  double *dataStart = &mData[indexStart];

  memcpy( buffer, dataStart, sizeof( double )*effectiveCount );

  return effectiveCount;
}

VectorDataset::VectorDataset( LPFILE Fp,
                              LPHEAD Pdfs,
                              LONG timeStepNo,
                              LONG itemNoX,
                              LONG itemNoY,
                              size_t size,
                              bool doublePrecision,
                              double deleteDoubleValue,
                              float deleteFloatValue ) :
  Dataset( Fp, Pdfs, timeStepNo, size, doublePrecision, deleteDoubleValue, deleteFloatValue ),
  mItemNoX( itemNoX ),
  mItemNoY( itemNoY )
{}

int VectorDataset::getData( int indexStart, int count, double *buffer )
{
  if ( !mLoaded )
  {
    mData.resize( mSize * 2 );
    mActive.resize( mSize );
    if ( mIsDoublePrecision )
    {
      std::vector<double> xData( mSize );
      std::vector<double> yData( mSize );
      if ( !readData( mItemNoX, xData.data() ) )
        return 0;
      if ( !readData( mItemNoY, yData.data() ) )
        return 0;

      for ( size_t i = 0; i < mSize; ++i )
      {
        mData[2 * i] = xData.at( i );
        mData[2 * i + 1] = yData.at( i );

        if ( xData.at( i ) == mDoubleDeleteValue )
          mActive[i] = 0;
        else
          mActive[i] = 1;
      }
    }
    else
    {
      std::vector<float> xData( mSize );
      std::vector<float> yData( mSize );
      if ( !readData( mItemNoX, xData.data() ) )
        return 0;
      if ( !readData( mItemNoY, yData.data() ) )
        return 0;

      for ( size_t i = 0; i < mSize; ++i )
      {
        mData[2 * i] = xData.at( i );
        mData[2 * i + 1] = yData.at( i );

        if ( xData.at( i ) == mFloatDeleteValue )
          mActive[i] = 0;
        else
          mActive[i] = 1;
      }
    }
    mLoaded = true;
  }

  if (buffer == nullptr)
      return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) / 2 - indexStart ) );
  double *dataStart = &mData[indexStart * 2];

  memcpy( buffer, dataStart, sizeof( double )*effectiveCount * 2 );

  return effectiveCount;
}

//! Constructor for a scalar dataset group

DatasetGroup::DatasetGroup( std::string name, std::string unit, LONG idNumber, bool isDoublePrecision, LPFILE fp, LPHEAD pdfs ) :
  mFp( fp ),
  mPdfs( pdfs ),
  mIdX( idNumber ),
  mIsDoublePrecision( isDoublePrecision )
{
  mMetadata.push_back( { "name", name } );
  mMetadata.push_back( { "unit", unit } );
}

//! Constructor for a vector dataset group
DatasetGroup::DatasetGroup( std::string name, std::string unit, LONG idNumberX, LONG idNumberY, bool isDoublePrecision, LPFILE fp, LPHEAD pdfs ) :
  mFp( fp ),
  mPdfs( pdfs ),
  mIdX( idNumberX ),
  mIdY( idNumberY ),
  mIsDoublePrecision( isDoublePrecision )
{
  mMetadata.push_back( { "name", name } );
  mMetadata.push_back( { "unit", unit } );
}

const std::string &DatasetGroup::name() const
{
  return mName;
}
bool DatasetGroup::isScalar() const
{
  return mIdY == 0;
}

const std::vector<Metadata> &DatasetGroup::metadata() const
{
  return mMetadata;
}

int DatasetGroup::datasetCount() const
{
  return static_cast<int>( mDatasets.size() );
}

Dataset *DatasetGroup::dataset( int index ) const
{
  if ( index >= 0 && static_cast<size_t>( index ) < mDatasets.size() )
    return mDatasets.at( static_cast<size_t>( index ) ).get();
  else
    return nullptr;
}

//! Initialize the group by filling it with \a mTimeStepCount datasets ready to load data when needed (lazy loading)

void DatasetGroup::init( LONG timeStepCount, size_t elementsCount, double deleteDoubleValue, float deleteFloatValue )
{
  mDatasets.clear();
  for ( LONG index = 0; index < timeStepCount; ++index )
  {
    if ( mLevelValueGenerator ) //3D stacked mesh
    {
        if (mIdY == 0) //scalar dataset group
            mDatasets.emplace_back(new ScalarDatasetOnVolumes(mFp, mPdfs, index, mIdX, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue, mLevelValueGenerator));
        else
            mDatasets.emplace_back(new VectorDatasetOnVolumes(mFp, mPdfs, index, mIdX, mIdY, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue, mLevelValueGenerator));
    }
    else
    {
      if ( mIdY == 0 ) //scalar dataset group
        mDatasets.emplace_back( new ScalarDataset( mFp, mPdfs, index, mIdX, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue ) );
      else
        mDatasets.emplace_back( new VectorDataset( mFp, mPdfs, index, mIdX, mIdY, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue ) );
    }
  }
}

LevelValuesGenerator::LevelValuesGenerator( LPFILE fp, LPHEAD pdfs, const VertexIndexesOfLevelsOnMesh &levels, size_t vertex3DCount )
  : mFp( fp )
  , mPdfs( pdfs )
  , mVertexIndexesOfLevelsOnMesh( levels )
  , m3DVertexCount( vertex3DCount )
{}

void LevelValuesGenerator::initializeTimeStep( size_t timeStepCount, bool doublePrecision, double deleteDoubleValue, float deleteFloatValue )
{
  mIsDoublePrecision = doublePrecision;
  mDoubleDeleteValue = deleteDoubleValue;
  mFloatDeleteValue = deleteFloatValue;

  mFaceToStartVolumePositionPerTimeStep = std::vector<std::vector<int>>( timeStepCount, std::vector<int>() );
  mFaceLevelCountPerTimeStep = std::vector<std::vector<int>>( timeStepCount, std::vector<int>() );
  mFaceLevelsDataPerTimeStep = std::vector<std::vector<double>>( timeStepCount, std::vector<double>() );

  mVolumeCountPerTimeStep = std::vector<size_t>( timeStepCount, 0 );
  mMaximumeLevelCountPerTimeStep = std::vector<int>( timeStepCount, -1 );
}

int LevelValuesGenerator::verticalLevelCountData( LONG timeStepNo, int indexStart, int count, int *buffer )
{
  if ( buffer == nullptr )
    return 0;

  if ( mFaceLevelCountPerTimeStep.at( timeStepNo ).empty() )
    buildVolumeForTimeStep( timeStepNo );

  const std::vector<int> &faceLevelCount = mFaceLevelCountPerTimeStep.at( timeStepNo );

  if ( indexStart >= faceLevelCount.size() )
    return 0;

  const int effectiveCount = std::max( 0, std::min( count, static_cast<int>( faceLevelCount.size() ) - indexStart ) );
  const int *dataStart = &faceLevelCount[indexStart];

  memcpy( buffer, dataStart, sizeof( int ) * effectiveCount );

  return effectiveCount;
}

int LevelValuesGenerator::verticalLevelData( LONG timeStepNo, int indexStart, int count, double *buffer )
{
  if ( buffer == nullptr )
    return 0;

  if ( mFaceLevelsDataPerTimeStep.at( timeStepNo ).empty() )
    buildVolumeForTimeStep( timeStepNo );

  const std::vector<double> &faceLevelData = mFaceLevelsDataPerTimeStep.at( timeStepNo );

  if ( indexStart >= faceLevelData.size() )
    return 0;

  const int effectiveCount = std::max( 0, std::min( count, static_cast<int>( faceLevelData.size() ) - indexStart ) );
  const double *dataStart = &faceLevelData[indexStart];

  memcpy( buffer, dataStart, sizeof( double ) * effectiveCount );

  return effectiveCount;
}

int LevelValuesGenerator::faceToVolume( LONG timeStepNo, int indexStart, int count, int *buffer )
{
  if ( buffer == nullptr )
    return 0;

  if ( mFaceToStartVolumePositionPerTimeStep.at( timeStepNo ).empty() )
    buildVolumeForTimeStep( timeStepNo );

  const std::vector<int> &faceToVolume = mFaceToStartVolumePositionPerTimeStep.at( timeStepNo );

  if ( indexStart >= faceToVolume.size() )
    return 0;

  const int effectiveCount = std::max( 0, std::min( count, static_cast<int>( faceToVolume.size() ) - indexStart ) );
  const int *dataStart = &faceToVolume[indexStart];

  memcpy( buffer, dataStart, sizeof( int ) * effectiveCount );

  return effectiveCount;
}

size_t LevelValuesGenerator::totalVolumesCount( LONG timeStepNo )
{
  if ( timeStepNo >= mVolumeCountPerTimeStep.size() )
    return 0;

  if ( mVolumeCountPerTimeStep.at( timeStepNo ) == 0 )
    buildVolumeForTimeStep( timeStepNo );

  return mVolumeCountPerTimeStep.at( timeStepNo );
}

int LevelValuesGenerator::maximumLevelCount( LONG timeStepNo )
{
  if ( timeStepNo >= mVolumeCountPerTimeStep.size() )
    return 0;

  if ( mMaximumeLevelCountPerTimeStep.at( timeStepNo ) == -1 )
    buildVolumeForTimeStep( timeStepNo );

  return mMaximumeLevelCountPerTimeStep.at( timeStepNo );
}

const std::vector<int> &LevelValuesGenerator::faceTostartVolumePosition( LONG timeStepNo )
{
  if ( mFaceToStartVolumePositionPerTimeStep.at( timeStepNo ).empty() )
    buildVolumeForTimeStep( timeStepNo );

  return mFaceToStartVolumePositionPerTimeStep.at( timeStepNo );
}

const std::vector<int> &LevelValuesGenerator::levelCounts( LONG timeStepNo )
{
  if ( mFaceLevelCountPerTimeStep.at( timeStepNo ).empty() )
    buildVolumeForTimeStep( timeStepNo );

  return mFaceLevelCountPerTimeStep.at( timeStepNo );
}

void LevelValuesGenerator::unload(LONG timeStep)
{
    mFaceToStartVolumePositionPerTimeStep.at(timeStep).clear();
    mFaceToStartVolumePositionPerTimeStep.at(timeStep).shrink_to_fit();
    mFaceLevelCountPerTimeStep.at(timeStep).clear();
    mFaceLevelCountPerTimeStep.at(timeStep).shrink_to_fit();
    mFaceLevelsDataPerTimeStep.at(timeStep).clear();
    mFaceLevelsDataPerTimeStep.at(timeStep).shrink_to_fit();
}

void *LevelValuesGenerator::rawDataPointerForRead( size_t size )
{
  mRawDataDouble.clear();
  mRawDataFloat.clear();
  if ( mIsDoublePrecision )
  {
    mRawDataDouble.resize( size );
    return mRawDataDouble.data();
  }
  else
  {
    mRawDataFloat.resize( size );
    return mRawDataFloat.data();
  }
}

double LevelValuesGenerator::rawDataValue( size_t i )
{
  if ( mIsDoublePrecision )
  {
    double value = mRawDataDouble.at( i );
    if ( value == mDoubleDeleteValue )
      return std::numeric_limits<double>::quiet_NaN();
    else
      return value;
  }
  else
  {
    float value = mRawDataFloat.at( i );
    if ( value == mFloatDeleteValue )
      return std::numeric_limits<double>::quiet_NaN();
    else
      return value;
  }
}

bool LevelValuesGenerator::buildVolumeForTimeStep( LONG timeStepNo )
{
  LONG err = dfsFindItemDynamic( mPdfs, mFp, timeStepNo, 1 );
  if ( err != F_NO_ERROR )
    return false;

  double time;
  err = dfsReadItemTimeStep( mPdfs, mFp, &time, rawDataPointerForRead( m3DVertexCount ) );
  if ( err != F_NO_ERROR )
    return false;

  std::vector<int> levelCounts;
  std::vector<double> levelValues;
  std::vector<int> faceToStartVolumePosition;

  size_t &volumeCount = mVolumeCountPerTimeStep.at( timeStepNo );
  int &maxLevelCount = mMaximumeLevelCountPerTimeStep.at( timeStepNo );
  volumeCount = 0;
  maxLevelCount = 0;

  size_t faceIndexCount = mVertexIndexesOfLevelsOnMesh.size(); //this is faces count
  levelCounts.resize( faceIndexCount );
  faceToStartVolumePosition.resize( faceIndexCount );
  for ( size_t faceIndex = 0; faceIndex < faceIndexCount; ++faceIndex )
  {
    const VertexIndexesOfLevelsOnFace &vertexIndexOfLevelsOnFace = mVertexIndexesOfLevelsOnMesh.at( faceIndex );
    size_t totalFaceLevelsCount = vertexIndexOfLevelsOnFace.size();
    size_t effectiveLevelCount = 0;

    faceToStartVolumePosition[faceIndex] = volumeCount;

    std::vector<double> zValues( totalFaceLevelsCount );

    bool columnEnd = false;
    for ( size_t levelIndex = 0; levelIndex < totalFaceLevelsCount; ++levelIndex )
    {
      const VertexIndexesOfLevel &vertexIndexOfLevel = vertexIndexOfLevelsOnFace.at( levelIndex );

      double levelValue = 0;
      for ( size_t i = 0; i < vertexIndexOfLevel.size(); ++i )
      {
        double z = rawDataValue( vertexIndexOfLevel.at( i ) );
        if ( std::isnan( z ) )
        {
          columnEnd = true;
          break;
        }

        levelValue += z;
      }

      if ( columnEnd )
        break;
      levelValue /= vertexIndexOfLevel.size();
      zValues[levelIndex] = levelValue;
      effectiveLevelCount++;
    }

    if (faceIndex == 1500)
        int a = 1;

    levelCounts[faceIndex] = effectiveLevelCount - 1;

    zValues.resize( effectiveLevelCount );
    size_t firstlevelPos = levelValues.size();
    levelValues.resize( firstlevelPos + zValues.size() );
    //as MDAL consider volume with Z decreasing, we need to reverse values
    for ( size_t i = 0; i < zValues.size(); ++i )
    {
      levelValues[levelValues.size() - 1 - i] = zValues.at( i );
    }
    volumeCount += effectiveLevelCount - 1;
    if ( effectiveLevelCount > maxLevelCount )
      maxLevelCount = effectiveLevelCount;
  }

  mFaceLevelCountPerTimeStep[timeStepNo] = std::move( levelCounts );
  mFaceToStartVolumePositionPerTimeStep[timeStepNo] = std::move( faceToStartVolumePosition );
  mFaceLevelsDataPerTimeStep[timeStepNo] = std::move( levelValues );

  rawDataPointerForRead(0); //clear the temporary raw data container
}

ScalarDatasetOnVolumes::ScalarDatasetOnVolumes( LPFILE Fp,
    LPHEAD Pdfs, 
    LONG timeStepNo, 
    LONG itemNo, 
    size_t maxSize,
    bool doublePrecision, 
    double deleteDoubleValue,
    float deleteFloatValue, 
    LevelValuesGenerator *levelValueGenerator )
  : DatasetOnVolumes( Fp, Pdfs, timeStepNo, maxSize, doublePrecision, deleteDoubleValue, deleteFloatValue, levelValueGenerator)
  , mItemNo( itemNo )
{
}

int ScalarDatasetOnVolumes::getData( int indexStart, int count, double *buffer )
{
  if ( !mLoaded )
  {
    mActive.resize(mLevelValueGenerator->levelCounts(mTimeStepNo).size());
    for (size_t i = 0; i<mActive.size(); ++i)
        mActive[i] = false;

    mData.resize( mSize );
    if ( mIsDoublePrecision )
    {
      if ( !readData( mItemNo, mData.data() ) )
        return 0;

      mData.resize(mLevelValueGenerator->totalVolumesCount(mTimeStepNo));
      reverseAndActiveData(mData,mDoubleDeleteValue);
    }
    else
    {
      std::vector<float> floatData( mSize, -1 );

      if ( !readData( mItemNo, floatData.data() ) )
        return 0;

      floatData.resize(mLevelValueGenerator->totalVolumesCount(mTimeStepNo));
      reverseAndActiveData(floatData, mFloatDeleteValue);

      mData.resize(mLevelValueGenerator->totalVolumesCount(mTimeStepNo));
      for ( size_t i = 0; i < mSize; ++i )
        mData[i] = floatData[i];
    }

    mData.resize( mLevelValueGenerator->totalVolumesCount( mTimeStepNo ) );

    mLoaded = true;
  }

  if (buffer == nullptr)
      return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) - indexStart ) );
  double *dataStart = &mData[indexStart];

  memcpy( buffer, dataStart, sizeof( double ) * effectiveCount );

  return effectiveCount;
}


VectorDatasetOnVolumes::VectorDatasetOnVolumes(LPFILE Fp, 
    LPHEAD Pdfs,
    LONG timeStepNo,
    LONG itemNoX,
    LONG itemNoY,
    size_t maxSize,
    bool doublePrecision,
    double deleteDoubleValue,
    float deleteFloatValue,
    LevelValuesGenerator* levelValueGenerator)
    : DatasetOnVolumes(Fp, Pdfs, timeStepNo, maxSize, doublePrecision, deleteDoubleValue, deleteFloatValue, levelValueGenerator)
    , mItemNoX(itemNoX)
    , mItemNoY(itemNoY)
{
}

int VectorDatasetOnVolumes::getData(int indexStart, int count, double* buffer)
{
    if (!mLoaded)
    {
        mActive.resize(mLevelValueGenerator->levelCounts(mTimeStepNo).size());
        for (size_t i = 0; i<mActive.size(); ++i)
            mActive[i] = false;

        mData.resize(mSize * 2);

        if (mIsDoublePrecision)
        {
            std::vector<double> xData(mSize);
            std::vector<double> yData(mSize);
            if (!readData(mItemNoX, xData.data()))
                return 0;
            if (!readData(mItemNoY, yData.data()))
                return 0;

            reverseAndActiveData(xData,mDoubleDeleteValue);
            reverseAndActiveData(yData, mDoubleDeleteValue);

            for (size_t i = 0; i < mSize; ++i)
            {
                mData[2 * i] = xData.at(i);
                mData[2 * i + 1] = yData.at(i);
            }
        }
        else
        {
            std::vector<float> xData(mSize);
            std::vector<float> yData(mSize);
            if (!readData(mItemNoX, xData.data()))
                return 0;
            if (!readData(mItemNoY, yData.data()))
                return 0;

            reverseAndActiveData(xData,mFloatDeleteValue);
            reverseAndActiveData(yData,mFloatDeleteValue);

            for (size_t i = 0; i < mSize; ++i)
            {
                mData[2 * i] = xData.at(i);
                mData[2 * i + 1] = yData.at(i);
            }
        }
        mLoaded = true;
    }

    if (buffer == nullptr)
        return 0;

    int effectiveCount = std::max(0, std::min(count, static_cast<int>(mData.size()) / 2 - indexStart));
    double* dataStart = &mData[indexStart * 2];

    memcpy(buffer, dataStart, sizeof(double) * effectiveCount * 2);

    return effectiveCount;
}

inline int DatasetOnVolumes::volumeCount() { return mLevelValueGenerator->totalVolumesCount( mTimeStepNo ); }

int DatasetOnVolumes::verticalLevelCountData( int indexStart, int count, int *buffer )
{
  return mLevelValueGenerator->verticalLevelCountData( mTimeStepNo, indexStart, count, buffer );
}

int DatasetOnVolumes::verticalLevelData( int indexStart, int count, double *buffer )
{
  return mLevelValueGenerator->verticalLevelData( mTimeStepNo, indexStart, count, buffer );
}

int DatasetOnVolumes::maximum3DLevelCount()
{
  return  mLevelValueGenerator->maximumLevelCount( mTimeStepNo );
}

int DatasetOnVolumes::faceToVolume( int indexStart, int count, int *buffer )
{
  return mLevelValueGenerator->faceToVolume( mTimeStepNo, indexStart, count, buffer );
}

void DatasetOnVolumes::unload()
{
  Dataset::unload();
  if ( mLevelValueGenerator )
    mLevelValueGenerator->unload( mTimeStepNo );
}

