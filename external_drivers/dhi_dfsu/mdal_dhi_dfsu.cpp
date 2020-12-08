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

  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );
  if ( rc != F_NO_ERROR )
    return false;

  LONG dataType = dfsGetDataType( pdfs );

  if ( dataType != 2001 && dataType != 2000 )
    return false;

  return true;

}

std::unique_ptr<Mesh> Mesh::loadMesh( const std::string &uri )
{
  LPFILE      Fp;
  LPHEAD      pdfs;
  LPCTSTR fileName = uri.c_str();

  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  if ( rc != F_NO_ERROR )
    return nullptr;

  std::unique_ptr<Mesh> mesh( new Mesh );
  mesh->mFp = Fp;
  mesh->mPdfs = pdfs;

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

size_t Mesh::faceIdToIndex( int id ) const
{
  if ( !mElemId2faceIndex.empty() )
  {
    std::map<int, size_t>::const_iterator it = mElemId2faceIndex.find( id );
    if ( it != mElemId2faceIndex.end() )
      return it->second;
  }

  return static_cast<size_t>( id - mGapFromFaceToElement );
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

bool Mesh::populateMeshFrame()
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
      if ( nodeIdCount * sizeof( int ) == size )
      {
        if ( mVertexCoordinates.size() == 0 )
        {
          mVertexCoordinates.resize( nodeIdCount * 3 );
        }

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
      size_t elemIdCount = static_cast<size_t>( dfsGetItemElements( staticItem ) );
      size_t size = dfsGetItemBytes( staticItem );
      if ( elemIdCount * sizeof( int ) == size )
      {
        if ( mFaceNodeCount.empty() )
          mFaceNodeCount.resize( elemIdCount );

        if ( elemIdCount == mFaceNodeCount.size() )
        {
          std::vector<int> faceToElement( elemIdCount );
          error = dfsStaticGetData( pvec, faceToElement.data() );
          if ( error != F_NO_ERROR )
            fail = true;

          if ( !faceToElement.empty() && !fail )
          {
            mGapFromFaceToElement = faceToElement.at( 0 );
            for ( size_t i = 1; i < faceToElement.size(); ++i )
              if ( faceToElement.at( i ) - i != mGapFromFaceToElement )
                mElemId2faceIndex[faceToElement.at( i )] = i;
          }
        }
        else fail = true;
      }
      else
        fail = true;
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

bool Mesh::setCoordinate( LPVECTOR pvec, LPITEM staticItem, SimpleType itemDatatype, size_t offset, double &min, double &max )
{
  LONG valueCount = dfsGetItemElements( staticItem );
  size_t size = dfsGetItemBytes( staticItem );
  bool coordinateIsDouble = itemDatatype == UFS_DOUBLE;
  if ( valueCount * ( coordinateIsDouble ? sizeof( double ) : sizeof( float ) ) == size )
  {
    if ( mVertexCoordinates.size() == 0 )
    {
      mVertexCoordinates.resize( valueCount * 3 );
    }

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

typedef std::map<std::string, std::pair<LONG, bool>> VectorGroups;


static double convertTimeToHours( double time, LONG timeUnit )
{
  //timeUnit value from EUM.xml
  switch ( timeUnit )
  {
    case 1400: //second
      return time / 3600;
      break;
    case 1401: //minute
      return time / 60;
      break;
    case 1402: //hour
      return time;
      break;
    case 1403: //day
      return time * 24;
      break;
    case 1404: //year
      return time * 24 * 365;
      break;
    case 1405: //month
      return time * 24 * 30;
      break;
    case 1406: //millisecond
      return time / 3600 / 1000;
      break;
    default:
      return 0;
      break;
  }
}

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

  for ( LONG i = 1; i <= dynamicItemCount; ++i )
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
    if ( isVector( itemName, groupName, isX ) )
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
      }
    }
    else
    {
      mDatasetGroups.emplace_back( new DatasetGroup( name, itemUnit, i, doublePrecision, mFp, mPdfs ) );
    }

  }

  // fill dataset group

  for ( std::unique_ptr < DatasetGroup> &group : mDatasetGroups )
    group->init( mTimeStepCount, mFaceNodeCount.size(), doubleDelete, floatDelete );

  return true;
}

Dataset::Dataset( LPFILE Fp, LPHEAD Pdfs, LONG timeStepNo, size_t size, bool doublePrecision ) :
  mFp( Fp ),
  mPdfs( Pdfs ),
  mTimeStepNo( timeStepNo ),
  mSize( size ),
  mIsDoublePrecision( doublePrecision )
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
  Dataset( Fp, Pdfs, timeStepNo, size, doublePrecision ),
  mItemNo( itemNo ),
  mDoubleDeleteValue( deleteDoubleValue ),
  mFloatDeleteValue( deleteFloatValue )
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
      std::vector<float> floatData( mSize );
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

  if ( buffer == nullptr )
    return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) - indexStart ) );
  double *dataStart = &mData[indexStart];

  memcpy( buffer, dataStart, sizeof( double )*effectiveCount );

  return effectiveCount;
}

inline VectorDataset::VectorDataset( LPFILE Fp,
                                     LPHEAD Pdfs,
                                     LONG timeStepNo,
                                     LONG itemNoX,
                                     LONG itemNoY,
                                     size_t size,
                                     bool doublePrecision,
                                     double deleteDoubleValue,
                                     float deleteFloatValue ) :
  Dataset( Fp, Pdfs, timeStepNo, size, doublePrecision ),
  mItemNoX( itemNoX ),
  mItemNoY( itemNoY ),
  mDoubleDeleteValue( deleteDoubleValue ),
  mFloatDeleteValue( deleteFloatValue )
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

  if ( buffer == nullptr )
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
    if ( mIdY == 0 ) //scalar dataset group
      mDatasets.emplace_back( new ScalarDataset( mFp, mPdfs, index, mIdX, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue ) );
    else
      mDatasets.emplace_back( new VectorDataset( mFp, mPdfs, index, mIdX, mIdY, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue ) );
  }
}
