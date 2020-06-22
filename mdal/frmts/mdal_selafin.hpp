/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SELAFIN_HPP
#define MDAL_SELAFIN_HPP

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{
  /**
   * This class is used to read the selafin file format.
   * The file is opened with initialize() and stay opened until this object is destructed
   *
   * \note SerafinStreamReader object is shared between different datasets, with the mesh and its iterators.
   *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
   *
   * Each record of the stream has a int (4 bytes) at the beginning and the end of the record, this int is the size of the record
  */
  class SerafinStreamReader
  {
    public:
      //! Constructs the reader
      SerafinStreamReader();
      //! Initializes and open the file file with the \a fileName
      void initialize( const std::string &fileName );

      //! Reads a string record with a size \a len from current position in the stream, throws an exception if the size in not compaitble
      std::string readString( size_t len );

      /**
       * Reads a double array record with a size \a len from current position in the stream,
       * throws an exception if the size in not compatible
       */
      std::vector<double> readDoubleArr( size_t len );

      /**
       * Reads some values in a double array record. The values count is \a len,
       * the reading begin at the stream \a position with the \a offset
       */
      std::vector<double> readDoubleArr( const std::streampos &position, size_t offset, size_t len );

      /**
       * Reads a int array record with a size \a len from current position in the stream,
       * throws an exception if the size in not compatible
       */
      std::vector<int> readIntArr( size_t len );

      /**
       * Reads some values in a int array record. The values count is \a len,
       * the reading begin at the stream \a position with the \a offset
       */
      std::vector<int> readIntArr( const std::streampos &position, size_t offset, size_t len );

      /**
       * Reads a size_t array record with a size \a len from current position in the stream,
       * throws an exception if the size in not compatible
       */
      std::vector<size_t> readSizeTArr( size_t len );

      //! Returns whether there is a int array with size \a len at the current position in the stream
      bool checkIntArraySize( size_t len );

      //! Returns whether there is a double array with size \a len at the current position in the stream
      bool checkDoubleArraySize( size_t len );

      //! Returns the remaining bytes in the stream from current position until the end
      size_t remainingBytes();

      /**
       * Set the position in the stream just after the int array with \a size, returns position of the beginning of the array
       * The presence of int array can be check with checkIntArraySize()
       */
      std::streampos passThroughIntArray( size_t size );

      /**
       * Set the position in the stream just after the double array with \a size, returns position of the beginning of the array
       * The presence of double array can be check with checkDoubleArraySize()
       */
      std::streampos passThroughDoubleArray( size_t size );

    private:
      double readDouble( );
      int readInt( );
      size_t readSizet( );

      void ignoreArrayLength( );
      std::string readStringWithoutLength( size_t len );
      void ignore( int len );
      bool getStreamPrecision();

      std::string mFileName;
      bool mStreamInFloatPrecision = true;
      bool mIsNativeLittleEndian = true;
      long long mFileSize = -1;
      std::ifstream mIn;
  };

  class SelafinDataset : public Dataset2D
  {
    public:
      /**
       * Contructs a dataset with a SerafinStreamReader object, \a reader, and the \a size of the array(s)
       *
       * \note SerafinStreamReader object is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
       *
       * Position of array(s) in the stream has to be set after construction (default = begin of the stream),
       * see setXStreamPosition() and setYStreamPosition()  (X for scalar dataset, X and Y for vector dataset)
      */
      SelafinDataset( DatasetGroup *parent,
                      std::shared_ptr<SerafinStreamReader> reader,
                      size_t size );

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

      //! Sets the position of the X array in the stream
      void setXStreamPosition( const std::streampos &xStreamPosition );
      //! Sets the position of the Y array in the stream
      void setYStreamPosition( const std::streampos &yStreamPosition );

    private:
      std::shared_ptr<SerafinStreamReader> mReader;
      const size_t mSize;

      std::streampos mXStreamPosition = 0;
      std::streampos mYStreamPosition = 0;

      std::vector<double> mValues;

      bool mLoaded = false;
  };

  class MeshSelafinVertexIterator: public MeshVertexIterator
  {
    public:
      /**
       * Contructs a vertex iterator with:
       * - SerafinStreamReader object \a reader,
       * - the position \a startX of the abscisses array in the stream,
       * - the position \a startY of the ordinates array in the stream,
       * - the vertices count \a verticesCount,
       * - \a xOrigin and \a yOrigin, the position of the origin (coordinate are relative to this point).
       *
       * \note SerafinStreamReader object is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
       */
      MeshSelafinVertexIterator( std::shared_ptr<SerafinStreamReader> reader,
                                 std::streampos startX,
                                 std::streampos startY,
                                 size_t verticesCount,
                                 double xOrigin,
                                 double yOrigin );

      size_t next( size_t vertexCount, double *coordinates ) override;

    private:
      std::shared_ptr<SerafinStreamReader> mReader;
      std::streampos mStartX;
      std::streampos mStartY;
      size_t mPosition = 0;
      size_t mTotalVerticesCount;
      double mXOrigin;
      double mYOrigin;
  };

  class MeshSelafinFaceIterator: public MeshFaceIterator
  {
    public:
      /**
       * Contructs a face iterator with:
       * - a SerafinStreamReader object \a reader,
       * - the position \a start of connectivity table in the stream,
       * - the vertices count \a verticesCount,
       * - the faces count \a facesCount,
       * - the number of vertices per face \a verticesPerFace.
       *
       * \note SerafinStreamReader object is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
       */
      MeshSelafinFaceIterator( std::shared_ptr<SerafinStreamReader> reader,
                               std::streampos start,
                               size_t verticesCount,
                               size_t facesCount,
                               size_t verticesPerFace );

      size_t next( size_t faceOffsetsBufferLen, int *faceOffsetsBuffer, size_t vertexIndicesBufferLen, int *vertexIndicesBuffer ) override;

    private:
      std::shared_ptr<SerafinStreamReader> mReader;
      std::streampos mStart;
      size_t mPosition = 0;
      size_t mTotalVerticesCount;
      size_t mTotalFacesCount;
      size_t mVerticesPerFace;
  };

  class MeshSelafin: public Mesh
  {
    public:
      /**
       * Contructs a dataset with:
       * - a SerafinStreamReader object \a reader,
       * - the position \a startX of the abscisses array in the stream,
       * - the position \a startY of the ordinates array in the stream,
       * - the position \a start of connectivity table in the stream,
       * - the vertices count \a verticesCount,
       * - the faces count \a facesCount,
       * - the number of vertices per face \a verticesPerFace,
       * - \a xOrigin and \a yOrigin, the position of the origin (coordinate are relative to this point).
       *
       * \note SerafinStreamReader object is shared between different dataset, with the mesh and its iterators.
       *       As SerafinStreamReader is not thread safe, it has to be shared in the same thread.
      */
      MeshSelafin( const std::string &uri,
                   std::shared_ptr<SerafinStreamReader> reader,
                   const std::streampos &verticesXStart,
                   const std::streampos &verticesYStart,
                   const std::streampos &ikleTableStart,
                   size_t verticesCount,
                   size_t facesCount,
                   size_t verticesPerFace,
                   double xOrigin,
                   double yOrigin
                 );

      std::unique_ptr<MeshVertexIterator> readVertices() override;

      //! Selafin format doesn't support edges in MDAL, returns a void unique_ptr
      std::unique_ptr<MeshEdgeIterator> readEdges() override;

      std::unique_ptr<MeshFaceIterator> readFaces() override;

      size_t verticesCount() const override {return mVerticesCount;}
      size_t edgesCount() const override {return 0;}
      size_t facesCount() const override {return mFacesCount;}
      BBox extent() const override;

    private:
      mutable bool mIsExtentUpToDate = false;
      mutable BBox mExtent;

      std::shared_ptr<SerafinStreamReader> mReader;
      std::streampos mXVerticesStart;
      std::streampos mYVerticesStart;
      std::streampos mIkleTableStart;
      size_t mVerticesCount = 0;
      size_t mFacesCount = 0;
      size_t mVerticesPerFace = 0;
      double mXOrigin;
      double mYOrigin;

      void calculateExtent() const;
  };

  /**
   * Serafin format (also called Selafin)
   *
   * Binary format for triangular mesh with datasets defined on vertices
   * Source of this doc come from :
   * http://www.opentelemac.org/downloads/MANUALS/TELEMAC-2D/telemac-2d_user_manual_en_v7p0.pdf Appendix 3
   * https://www.gdal.org/drv_selafin.html
   *
   * The Selafin file records are listed below:
   * - 1 record containing the title of the study (72 characters) and a 8 characters string indicating the type
   *    of format (SERAFIN or SERAFIND)
   * - record containing the two integers NBV(1)and NBV(2)(number of linear and quadratic variables, NBV(2)with the value of 0 for Telemac,
   * cas quadratic values are not saved so far),
   * - NBV(1)records containing the names and units of each variable (over 32 characters),
   * - 1 record containing the integers table IPARAM(10 integers, of which only the 6are currently being used),
   *        - if IPARAM (3)!=0: the value corresponds to the x-coordinate of the origin of the mesh,
   *        - if IPARAM (4)!=0: the value corresponds to the y-coordinate of the origin of the mesh,
   *        - if IPARAM (7): the value corresponds to the number of  planes on the vertical (3D computation),
   *        - if IPARAM (8)!=0: the value corresponds to the number of boundary points (in parallel),
   *        - if IPARAM (9)!=0: the value corresponds to the number of interface points (in parallel),
   *        - if IPARAM(8 )or IPARAM(9) !=0: the array IPOBO below is replaced by the array KNOLG(total initial number of points).
   *            All the other numbers are local to the sub-domain, including IKLE.
   *
   * - if IPARAM(10)= 1: a record containing the computation starting date,
   * - 1 record containing the integers NELEM,NPOIN,NDP,1(number of elements, number of points, number of points per element and the value 1),
   * - 1 record containing table IKLE(integer array of dimension (NDP,NELEM) which is the connectivity table.
   *    N.B.: in TELEMAC-2D, the dimensions of this array are (NELEM,NDP)),
   * - 1 record containing table IPOBO(integer array of dimension NPOIN);
   *    the value of one element is 0 for an internal point, and gives the numbering of boundary points for the others,
   * - 1 record containing table X(real array of dimension NPOINcontaining the abscissae of the points),
   * - 1 record containing table Y(real array of dimension NPOINcontaining the ordinates of the points),
   *
   * Next, for each time step, the following are found:
   * - 1 record containing time T(real),
   * - NBV(1)+NBV(2)records containing the results tables for each variable at time T.
   */
  class DriverSelafin: public Driver
  {
    public:
      DriverSelafin();
      ~DriverSelafin() override;
      DriverSelafin *create() override;

      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;

    private:
      typedef std::map<double, std::streampos > timestep_map; //TIME (sorted), position in the stream file

      void addData( const std::vector<std::string> &var_names,
                    const std::vector<timestep_map> &data,
                    size_t nPoints,
                    const DateTime &referenceTime );

      void parseFile( std::vector<std::string> &var_names,
                      std::map<std::string,
                      std::streampos> &streamPositions,
                      double *xOrigin,
                      double *yOrigin,
                      size_t *nElem,
                      size_t *nPoint,
                      size_t *nPointsPerElem,
                      std::vector<timestep_map> &data,
                      DateTime &referenceTime );

      bool getStreamPrecision( std::ifstream &in );

      std::unique_ptr< MDAL::Mesh > mMesh;
      std::string mFileName;
      std::shared_ptr<SerafinStreamReader> mReader;
  };

} // namespace MDAL
#endif //MDAL_SELAFIN_HPP
