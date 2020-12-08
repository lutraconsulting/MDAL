/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_DHI_HPP
#define MDAL_DHI_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "eum.h"
#include "dfsio.h"

#undef min
#undef max

class Dataset
{
  public:
    Dataset( LPFILE Fp, LPHEAD  Pdfs, LONG timeStepNo, size_t size, bool doublePrecision );

    //! Fills the buffer with data
    virtual int getData( int indexStart, int count, double *buffer ) = 0;

    //! Fills the buffer with active flags
    int getActive( int indexStart, int count, int *buffer );

    //! Clears the data stored in memory
    void unload();

  protected:
    LPFILE mFp;
    LPHEAD  mPdfs;
    bool mLoaded = false;
    std::vector<double> mData;
    std::vector<int> mActive;
    LONG mTimeStepNo = 0;
    size_t mSize = 0;
    bool mIsDoublePrecision = false;

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
    double mDoubleDeleteValue = 0.0;
    double mFloatDeleteValue = 0.0;

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
    double mDoubleDeleteValue = 0.0;
    double mFloatDeleteValue = 0.0;

};

typedef std::pair<std::string, std::string> Metadata;

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

  private:
    LPFILE mFp;
    LPHEAD  mPdfs;
    std::string mName;
    std::vector<Metadata> mMetadata;
    LONG mIdX = 0;
    LONG mIdY = 0;
    bool mIsDoublePrecision = false;

    std::vector < std::unique_ptr<Dataset>> mDatasets;
};


class Mesh
{
  public:
    ~Mesh();
    static bool canRead( const std::string &uri );
    static std::unique_ptr<Mesh> loadMesh( const std::string &uri );
    void close();

    //**************** Mesh frame *************
    int verticesCount() const;
    int facesCount() const;

    //! Returqn a pointer to the vertices coordinates for \a index
    double *vertexCoordinates( int index );

    //! Returns connectivty informations
    int connectivity( int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBuffer );

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

  private:
    Mesh() = default;
    LPFILE mFp;
    LPHEAD  mPdfs;
    std::string mWktProjection = "projection";
    std::vector<double> mVertexCoordinates;
    std::map<int, size_t> mNodeId2VertexIndex;
    int mGapFromVertexToNode = 0;
    double mXmin = std::numeric_limits<double>::max();
    double mXmax = -std::numeric_limits<double>::max();
    double mYmin = std::numeric_limits<double>::max();
    double mYmax = -std::numeric_limits<double>::max();

    std::vector<std::unique_ptr<DatasetGroup>> mDatasetGroups;
    int mTimeStepCount;
    std::string mReferenceTime;
    std::vector<double> mTimes;

    size_t vertexIdToIndex( int id ) const;

    std::vector<int> mFaceNodeCount;
    std::map<int, size_t> mElemId2faceIndex;
    int mGapFromFaceToElement = 0;

    size_t faceIdToIndex( int id ) const;

    std::vector<int> mConnectivity;
    size_t connectivityPosition( int faceIndex ) const;;
    size_t mNextFaceIndexForConnectivity = 0; //cache to speed up acces to connectivity
    size_t mNextConnectivityPosition = 0; //cache to speed up acces to connectivity

    bool populateMeshFrame();
    bool setCoordinate( LPVECTOR pvec, LPITEM staticItem, SimpleType    itemDatatype, size_t offset, double &min, double &max );

    bool populateDatasetGroups();
};

#endif //MDAL_DHI_HPP