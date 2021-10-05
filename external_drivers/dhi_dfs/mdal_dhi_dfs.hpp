/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2021 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_DHI_DFS_HPP
#define MDAL_DHI_DFS_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <assert.h>

#include "eum.h"
#include "dfsio.h"

#undef min
#undef max

typedef std::pair<std::string, std::string> Metadata;
typedef std::vector<int> VertexIndexesOfLevel;
typedef std::vector<VertexIndexesOfLevel> VertexIndexesOfLevelsOnFace;
typedef std::vector<VertexIndexesOfLevelsOnFace> VertexIndexesOfLevelsOnMesh;

class Dataset
{
  public:
    Dataset( LPFILE Fp, LPHEAD  Pdfs, LONG timeStepNo, size_t size, bool doublePrecision, double doubleDeleteValue, float floatDeleteValue );
    virtual ~Dataset() = default;

    //! Fills the buffer with data
    virtual int getData( int indexStart, int count, double *buffer ) = 0;

    //! Fills the buffer with active flags
    int getActive( int indexStart, int count, int *buffer );

    //! Clears the data stored in memory
    virtual void unload();

    virtual int maximum3DLevelCount() { return  -1; }
    virtual int volumeCount() { return -1; }
    virtual int verticalLevelCountData( int, int, int * ) { return -1; }
    virtual int verticalLevelData( int, int, double * ) { return -1; }
    virtual int faceToVolume( int, int, int * ) { return -1; }

  protected:
    LPFILE mFp;
    LPHEAD  mPdfs;
    bool mLoaded = false;
    std::vector<double> mData;
    std::vector<int> mActive;
    LONG mTimeStepNo = 0;
    size_t mSize = 0;
    bool mIsDoublePrecision = false;
    double mDoubleDeleteValue = 0.0;
    float mFloatDeleteValue = 0.0;

    // read all data and put them in pointed array
    bool readData( LONG itemNo, void *ptr ) const;
};

class ScalarDataset: public Dataset
{
  public:
    ScalarDataset( LPFILE Fp,
                   LPHEAD  Pdfs,
                   LONG timeStepNo,
                   LONG itemNo,
                   size_t size,
                   bool doublePrecision,
                   double deleteDoubleValue,
                   float deleteFloatValue );

    int getData( int indexStart, int count, double *buffer ) override;

  private:
    LONG mItemNo = 0;

};

class VectorDataset : public Dataset
{
  public:
    VectorDataset( LPFILE Fp,
                   LPHEAD  Pdfs,
                   LONG timeStepNo,
                   LONG itemNoX,
                   LONG itemNoY,
                   size_t size,
                   bool doublePrecision,
                   double deleteDoubleValue,
                   float deleteFloatValue );

    int getData( int indexStart, int count, double *buffer ) override;

  private:
    LONG mItemNoX = 0;
    LONG mItemNoY = 0;
};


class LevelValuesGenerator
{
  public:
    LevelValuesGenerator( LPFILE fp, LPHEAD pdfs, const VertexIndexesOfLevelsOnMesh &levels, size_t vertex3DCount );

    void initializeTimeStep( size_t timeStepCount, bool doublePrecision, double deleteDoubleValue, float deleteFloatValue );
    int verticalLevelCountData( LONG timeStepNo, int indexStart, int count, int *buffer );
    int verticalLevelData( LONG timeStepNo, int indexStart, int count, double *buffer );
    int faceToVolume( LONG timeStepNo, int indexStart, int count, int *buffer );
    int totalVolumesCount( LONG timeStepNo );
    int maximumLevelCount( LONG timeStepNo );


    const std::vector<int> &faceTostartVolumePosition( LONG timeStepNo );
    const std::vector<int> &levelCounts( LONG timeStepNo );

    void unload( LONG timeStep );

  private:
    LPFILE mFp = nullptr;
    LPHEAD  mPdfs = nullptr;
    const VertexIndexesOfLevelsOnMesh &mVertexIndexesOfLevelsOnMesh; //contains node index for each levels
    size_t m3DVertexCount = 0;
    int mTotalVolumeCount = 0;
    bool mIsDoublePrecision = false;
    double mDoubleDeleteValue = 0.0;
    float mFloatDeleteValue = 0.0;

    //LevelValues mLevelValues;
    std::vector<int> mVolumeCountPerTimeStep;
    std::vector<int> mMaximumeLevelCountPerTimeStep;
    std::vector<std::vector<int>> mFaceToStartVolumePositionPerTimeStep;
    std::vector<std::vector<int>> mFaceLevelCountPerTimeStep;
    std::vector<std::vector<double>> mFaceLevelsDataPerTimeStep;

    std::vector<double> mRawDataDouble;
    std::vector<float> mRawDataFloat;

    void *rawDataPointerForRead( size_t size );
    double rawDataValue( size_t i );
    void buildVolumeForTimeStep( LONG timeStepNo );
};



class DatasetOnVolumes : public Dataset
{
  public:
    DatasetOnVolumes( LPFILE Fp,
                      LPHEAD  Pdfs,
                      LONG timeStepNo,
                      size_t maxSize,
                      bool doublePrecision,
                      double deleteDoubleValue,
                      float deleteFloatValue,
                      LevelValuesGenerator *levelValueGenerator )
      : Dataset( Fp, Pdfs, timeStepNo, maxSize, doublePrecision, deleteDoubleValue, deleteFloatValue )
      , mLevelValueGenerator( levelValueGenerator ) {}

    virtual int volumeCount() override;
    virtual int verticalLevelCountData( int indexStart, int count, int *buffer ) override;
    virtual int verticalLevelData( int indexStart, int count, double *buffer ) override;
    virtual int maximum3DLevelCount() override;
    int faceToVolume( int indexStart, int count, int *buffer );
    void unload() override;

    template<typename T>
    void reverseAndActiveData( std::vector<T> &dataArray, T deleteValue )
    {
      // need to reorder the data, MDAL need to have level data from top to bottom
      const std::vector<int> &faceToStartVolumePosition = mLevelValueGenerator->faceTostartVolumePosition( mTimeStepNo );
      const std::vector<int> &levelCounts = mLevelValueGenerator->levelCounts( mTimeStepNo );

      assert( levelCounts.size() == faceToStartVolumePosition.size() );

      mActive.resize( levelCounts.size() );

      for ( size_t faceIndex = 0; faceIndex < faceToStartVolumePosition.size(); ++faceIndex )
      {
        size_t volumeStartIndex = faceToStartVolumePosition.at( faceIndex );
        if ( levelCounts.at( faceIndex ) > 1 )
        {
          size_t volumeCount = levelCounts.at( faceIndex );
          std::vector<T>::iterator it = dataArray.begin() + volumeStartIndex;
          std::reverse( it, it + volumeCount );

          for ( std::vector<T>::iterator itv = dataArray.begin() + volumeStartIndex; itv != dataArray.begin() + volumeStartIndex + volumeCount; ++itv )
          {
            bool isActive = ( *itv ) != deleteValue;
            if ( !isActive )
              ( *itv ) = std::numeric_limits<T>::quiet_NaN();
            if ( mActive.at( faceIndex ) == 1 || isActive )
              mActive[faceIndex] = 1;
          }
        }
        else
        {
          mActive[faceIndex] = false;
        }
      }
    }

  protected:
    LevelValuesGenerator *mLevelValueGenerator = nullptr;
};

class ScalarDatasetOnVolumes : public DatasetOnVolumes
{
  public:
    ScalarDatasetOnVolumes( LPFILE Fp,
                            LPHEAD  Pdfs,
                            LONG timeStepNo,
                            LONG itemNo,
                            size_t maxSize,
                            bool doublePrecision,
                            double deleteDoubleValue,
                            float deleteFloatValue,
                            LevelValuesGenerator *levelValueGenerator );

    int getData( int indexStart, int count, double *buffer ) override;

  private:
    LONG mItemNo = 0;
};


class VectorDatasetOnVolumes : public DatasetOnVolumes
{
  public:
    VectorDatasetOnVolumes( LPFILE Fp,
                            LPHEAD  Pdfs,
                            LONG timeStepNo,
                            LONG itemNoX,
                            LONG itemNoY,
                            size_t maxSize,
                            bool doublePrecision,
                            double deleteDoubleValue,
                            float deleteFloatValue,
                            LevelValuesGenerator *levelValueGenerator );

    int getData( int indexStart, int count, double *buffer ) override;

  private:
    LONG mItemNoX = 0;
    LONG mItemNoY = 0;
};



class DatasetGroup
{
  public:

    //! Constructor for a scalar dataset group
    DatasetGroup( std::string name,
                  std::string unit,
                  LONG idNumber,
                  bool isDoublePrecision,
                  LPFILE fp,
                  LPHEAD  pdfs );

    //! Constructor for a vector dataset group
    DatasetGroup( std::string name,
                  std::string unit,
                  LONG idNumberX,
                  LONG idNumberY,
                  bool isDoublePrecision,
                  LPFILE fp,
                  LPHEAD  pdfs );

    //! Initiliaze the group by filling it with \a mTimeStepCount datasets ready to load data when needed (lazy loading)
    void init( LONG timeStepCount, size_t elementsCount, double deleteDoubleValue, float deleteFloatValue );

    const std::string &name() const;
    bool isScalar() const;
    const std::vector<Metadata> &metadata() const;

    int datasetCount() const;
    Dataset *dataset( int index ) const;

    void setLevelValueGenerator( LevelValuesGenerator *levelGenerator )
    {
      mLevelValueGenerator = levelGenerator;
    }

  private:
    LPFILE mFp;
    LPHEAD  mPdfs;
    std::string mName;
    std::vector<Metadata> mMetadata;
    LONG mIdX = 0;
    LONG mIdY = 0;
    bool mIsDoublePrecision = false;
    LevelValuesGenerator *mLevelValueGenerator = nullptr;

    std::vector < std::unique_ptr<Dataset>> mDatasets;
};


class Mesh
{
  public:
    virtual ~Mesh();
    void close();

    //**************** Mesh frame *************
    virtual int verticesCount() const = 0;
    virtual int facesCount() const = 0;

    //! Returns a pointer to the vertices coordinates for \a index
    double *vertexCoordinates( int index );

    //! Returns connectivty informations
    int connectivity( int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBuffer ) const;

    //! Returns wkt projection
    const std::string &projection() const { return mWktProjection; }

    //! Returns mesh extent
    void extent( double *xMin, double *xMax, double *yMin, double *yMax ) const;

    //**************** datasets *************
    int datasetGroupsCount() const;

    DatasetGroup *datasetgroup( int i ) const;

    //! Returns reference time string (ISO8601)
    const std::string &referenceTime() const;

    int timeStepCount() const;
    double time( int index ) const;

    bool is3D() const;

  protected:
    Mesh() = default;
    LPFILE mFp = nullptr;
    LPHEAD  mPdfs = nullptr;
    bool mIs3D = false;
    std::string mWktProjection = "projection";
    size_t mTotalElementCount = 0;
    std::vector<double> mVertexCoordinates;
    std::vector<int> mConnectivity;
    mutable size_t mNextFaceIndexForConnectivity = 0; //cache to speed up acces to connectivity
    mutable size_t mNextConnectivityPosition = 0; //cache to speed up acces to connectivity

    double mXmin = std::numeric_limits<double>::max();
    double mXmax = -std::numeric_limits<double>::max();
    double mYmin = std::numeric_limits<double>::max();
    double mYmax = -std::numeric_limits<double>::max();

    std::vector<std::unique_ptr<DatasetGroup>> mDatasetGroups;
    int mTimeStepCount = 0;
    std::string mReferenceTime;
    std::vector<double> mTimes;

    std::unique_ptr<LevelValuesGenerator> mLevelGenerator;

    bool populateDatasetGroups();

    virtual size_t connectivityPosition( int faceIndex ) const = 0;
    virtual int nodeCount( size_t faceIndex ) const = 0;
};

#endif //MDAL_DHI_DFS_HPP;
