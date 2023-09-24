/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2021 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_dhi_dfs.hpp"

#include <cassert>

Mesh::~Mesh()
{
  close();
}

void Mesh::close()
{
  dfsFileClose( mPdfs, &mFp );
  dfsHeaderDestroy( &mPdfs );
}

double *Mesh::vertexCoordinates( int index )
{
  return &mVertexCoordinates[static_cast<size_t>( index * 3 )];
}

int Mesh::connectivity( int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBuffer ) const
{
  int maxFaceCount = std::max( 0, std::min( facesCount() - startFaceIndex, faceCount ) );
  size_t conPos = connectivityPosition( startFaceIndex );
  size_t conCount = 0;
  std::vector<int> faceOffset( static_cast<size_t>( maxFaceCount ) );
  size_t effectiveFaceCount = 0;
  for ( size_t i = 0; i < maxFaceCount; ++i )
  {
    int nc = nodeCount( i + startFaceIndex );
    if ( conCount + nc > vertexIndicesBufferLen )
      break;
    conCount += nc;
    faceOffset[i] = static_cast<int>( conCount );
    effectiveFaceCount++;
  }

  if ( startFaceIndex + effectiveFaceCount < facesCount() )
  {
    mNextFaceIndexForConnectivity = startFaceIndex + effectiveFaceCount;
    mNextConnectivityPosition = conPos + conCount;
  }

  memcpy( faceOffsetsBuffer, faceOffset.data(), static_cast<size_t>( effectiveFaceCount ) * sizeof( int ) );
  memcpy( vertexIndicesBuffer, &mConnectivity[conPos], conCount * sizeof( int ) );

  return static_cast<int>( effectiveFaceCount );
}

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


static bool isVector( const std::string &rawName, std::string &name, LONG type, bool &isX )
{
  std::vector<std::string> splitName = split( rawName, ' ' );
  if ( splitName.empty() )
    return false;

  size_t vectorIndicatorPos = 0;
  bool isVector = false;

  while ( vectorIndicatorPos < splitName.size() && !isVector )
  {
    std::string vectorIndicator = splitName.at( vectorIndicatorPos );

    if ( vectorIndicator == "U" ||
         ( type == 100080 && vectorIndicator == "P" ) )
    {
      isX = true;
      isVector = true;
    }
    else if ( vectorIndicator == "V" ||
              ( type == 100080 && vectorIndicator == "Q" ) )
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

    std::transform( name.begin(), name.begin() + 1, name.begin(), []( char c )-> char { return static_cast<char>( ( int( c ) ) ); } );
  }

  return isVector;
}

static bool isVelocity( LONG itemType, std::string &name, bool &isX, bool &isVerticalComponent )
{
  LPCTSTR eumIdent;
  if ( !eumGetItemTypeIdent( itemType, &eumIdent ) )
    return false;

  std::string ident( eumIdent );

  if ( std::strcmp( eumIdent, "eumIuVelocity" ) == 0 )
  {
    isX = true;
    isVerticalComponent = false;
    name = "Velocity";
    return true;
  }

  if ( std::strcmp( eumIdent, "eumIvVelocity" ) == 0 )
  {
    isX = false;
    isVerticalComponent = false;
    name = "Velocity";
    return true;
  }

  if ( std::strcmp( eumIdent, "eumIwVelocity" ) == 0 )
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
    bool isVerticalVelocity = false;

    isVectorDataset = isVelocity( itemType, groupName, isX, isVerticalVelocity );

    if ( !isVectorDataset )
      isVectorDataset = isVector( itemName, groupName, itemType, isX );

    if ( isVectorDataset && !isVerticalVelocity )
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
      if ( isVerticalVelocity )
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
  mDoubleDeleteValue( doubleDeleteValue ),
  mFloatDeleteValue( floatDeleteValue )
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
  Dataset( Fp, Pdfs, timeStepNo, size, doublePrecision, deleteDoubleValue, deleteFloatValue ),
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
        {
          mActive[i] = 0;
          mData[i] = std::numeric_limits<double>::quiet_NaN();
        }
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
        {
          mActive[i] = 0;
          mData[i] = std::numeric_limits<double>::quiet_NaN();
        }
        else
        {
          mActive[i] = 1;
          mData[i] = floatData[i];
        }
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
        {
          mActive[i] = 0;
          mData[2 * i] = std::numeric_limits<double>::quiet_NaN();
          mData[2 * i + 1] = std::numeric_limits<double>::quiet_NaN();
        }
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
        if ( xData.at( i ) == mFloatDeleteValue )
        {
          mData[2 * i] = std::numeric_limits<double>::quiet_NaN();
          mData[2 * i + 1] = std::numeric_limits<double>::quiet_NaN();
          mActive[i] = 0;
        }
        else
        {
          mData[2 * i] = xData.at( i );
          mData[2 * i + 1] = yData.at( i );
          mActive[i] = 1;
        }
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
    if ( mLevelValueGenerator ) //3D stacked mesh
    {
      if ( mIdY == 0 ) //scalar dataset group
        mDatasets.emplace_back( new ScalarDatasetOnVolumes( mFp, mPdfs, index, mIdX, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue, mLevelValueGenerator ) );
      else
        mDatasets.emplace_back( new VectorDatasetOnVolumes( mFp, mPdfs, index, mIdX, mIdY, elementsCount, mIsDoublePrecision, deleteDoubleValue, deleteFloatValue, mLevelValueGenerator ) );
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

  mVolumeCountPerTimeStep = std::vector<int>( timeStepCount, 0 );
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

int LevelValuesGenerator::totalVolumesCount( LONG timeStepNo )
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

void LevelValuesGenerator::unload( LONG timeStep )
{
  mFaceToStartVolumePositionPerTimeStep.at( timeStep ).clear();
  mFaceToStartVolumePositionPerTimeStep.at( timeStep ).shrink_to_fit();
  mFaceLevelCountPerTimeStep.at( timeStep ).clear();
  mFaceLevelCountPerTimeStep.at( timeStep ).shrink_to_fit();
  mFaceLevelsDataPerTimeStep.at( timeStep ).clear();
  mFaceLevelsDataPerTimeStep.at( timeStep ).shrink_to_fit();
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

void LevelValuesGenerator::buildVolumeForTimeStep( LONG timeStepNo )
{
  LONG err = dfsFindItemDynamic( mPdfs, mFp, timeStepNo, 1 );
  if ( err != F_NO_ERROR )
    return;

  double time;
  err = dfsReadItemTimeStep( mPdfs, mFp, &time, rawDataPointerForRead( m3DVertexCount ) );
  if ( err != F_NO_ERROR )
    return;

  std::vector<int> levelCounts;
  std::vector<double> levelValues;
  std::vector<int> faceToStartVolumePosition;

  int &volumeCount = mVolumeCountPerTimeStep.at( timeStepNo );
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
    int effectiveLevelCount = 0;

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

    levelCounts[faceIndex] = effectiveLevelCount - 1;

    zValues.resize( static_cast<size_t>( effectiveLevelCount ) );
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

  rawDataPointerForRead( 0 ); //clear the temporary raw data container
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
  : DatasetOnVolumes( Fp, Pdfs, timeStepNo, maxSize, doublePrecision, deleteDoubleValue, deleteFloatValue, levelValueGenerator )
  , mItemNo( itemNo )
{
}

int ScalarDatasetOnVolumes::getData( int indexStart, int count, double *buffer )
{
  if ( !mLoaded )
  {
    mActive.resize( mLevelValueGenerator->levelCounts( mTimeStepNo ).size() );
    for ( size_t i = 0; i < mActive.size(); ++i )
      mActive[i] = false;

    mData.resize( mSize );
    if ( mIsDoublePrecision )
    {
      if ( !readData( mItemNo, mData.data() ) )
        return 0;

      mData.resize( mLevelValueGenerator->totalVolumesCount( mTimeStepNo ) );
      reverseAndActiveData( mData, mDoubleDeleteValue );
    }
    else
    {
      std::vector<float> floatData( mSize, -1 );

      if ( !readData( mItemNo, floatData.data() ) )
        return 0;

      floatData.resize( mLevelValueGenerator->totalVolumesCount( mTimeStepNo ) );
      reverseAndActiveData( floatData, mFloatDeleteValue );

      mData.resize( mLevelValueGenerator->totalVolumesCount( mTimeStepNo ) );
      for ( size_t i = 0; i < mSize; ++i )
        mData[i] = floatData[i];
    }

    mData.resize( mLevelValueGenerator->totalVolumesCount( mTimeStepNo ) );

    mLoaded = true;
  }

  if ( buffer == nullptr )
    return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) - indexStart ) );
  double *dataStart = &mData[indexStart];

  memcpy( buffer, dataStart, sizeof( double ) * effectiveCount );

  return effectiveCount;
}


VectorDatasetOnVolumes::VectorDatasetOnVolumes( LPFILE Fp,
    LPHEAD Pdfs,
    LONG timeStepNo,
    LONG itemNoX,
    LONG itemNoY,
    size_t maxSize,
    bool doublePrecision,
    double deleteDoubleValue,
    float deleteFloatValue,
    LevelValuesGenerator *levelValueGenerator )
  : DatasetOnVolumes( Fp, Pdfs, timeStepNo, maxSize, doublePrecision, deleteDoubleValue, deleteFloatValue, levelValueGenerator )
  , mItemNoX( itemNoX )
  , mItemNoY( itemNoY )
{
}

int VectorDatasetOnVolumes::getData( int indexStart, int count, double *buffer )
{
  if ( !mLoaded )
  {
    mActive.resize( mLevelValueGenerator->levelCounts( mTimeStepNo ).size() );
    for ( size_t i = 0; i < mActive.size(); ++i )
      mActive[i] = false;

    mData.resize( mSize * 2 );

    if ( mIsDoublePrecision )
    {
      std::vector<double> xData( mSize );
      std::vector<double> yData( mSize );
      if ( !readData( mItemNoX, xData.data() ) )
        return 0;
      if ( !readData( mItemNoY, yData.data() ) )
        return 0;

      reverseAndActiveData( xData, mDoubleDeleteValue );
      reverseAndActiveData( yData, mDoubleDeleteValue );

      for ( size_t i = 0; i < mSize; ++i )
      {
        mData[2 * i] = xData.at( i );
        mData[2 * i + 1] = yData.at( i );
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

      reverseAndActiveData( xData, mFloatDeleteValue );
      reverseAndActiveData( yData, mFloatDeleteValue );

      for ( size_t i = 0; i < mSize; ++i )
      {
        mData[2 * i] = xData.at( i );
        mData[2 * i + 1] = yData.at( i );
      }
    }
    mLoaded = true;
  }

  if ( buffer == nullptr )
    return 0;

  int effectiveCount = std::max( 0, std::min( count, static_cast<int>( mData.size() ) / 2 - indexStart ) );
  double *dataStart = &mData[indexStart * 2];

  memcpy( buffer, dataStart, sizeof( double ) * effectiveCount * 2 );

  return effectiveCount;
}

int DatasetOnVolumes::volumeCount() { return mLevelValueGenerator->totalVolumesCount( mTimeStepNo ); }

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
