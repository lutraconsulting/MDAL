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
  class SerafinStreamReader
  {
    public:
      SerafinStreamReader();
      void initialize( const std::string &fileName );

      std::string read_string( size_t len );
      std::vector<double> read_double_arr( size_t len );
      std::vector<double> read_double_arr( const std::streampos &position, size_t offset, size_t len );
      std::vector<int> read_int_arr( size_t len );
      std::vector<int> read_int_arr( const std::streampos &position, size_t offset, size_t len );
      std::vector<size_t> read_size_t_arr( size_t len );

      double read_double( );
      int read_int( );
      size_t read_sizet( );

      bool checkIntArraySize( size_t len );
      bool checkDoubleArraySize( size_t len );

      size_t remainingBytes();

      std::streampos posistion()
      {
        return mIn.tellg();
      }

      std::streampos passThroughIntArray( size_t size )
      {
        std::streampos pos = mIn.tellg();
        mIn.seekg( size * 4, std::ios_base::cur );
        ignore_array_length();
        return pos;
      }

      std::streampos passThroughDoubleArray( size_t size )
      {
        std::streampos pos = mIn.tellg();
        if ( mStreamInFloatPrecision )
          size *= 4;
        else
          size *= 8;

        mIn.seekg( size, std::ios_base::cur );
        ignore_array_length();
        return pos;
      }

    private:
      void ignore_array_length( );
      std::string read_string_without_length( size_t len );
      void ignore( int len );
      bool getStreamPrecision();

      std::string mFileName;
      bool mStreamInFloatPrecision = true;
      bool mIsNativeLittleEndian = true;
      long long mFileSize = -1;
      std::ifstream mIn;
  };

  class MeshSelafinVertexIterator: public MeshVertexIterator
  {
    public:

      MeshSelafinVertexIterator( std::shared_ptr<SerafinStreamReader> reader,
                                 std::streampos startX,
                                 std::streampos startY,
                                 size_t verticesCount,
                                 double xOrigin,
                                 double yOrigin ):
        mReader( reader )
        , mStartX( startX )
        , mStartY( startY )
        , mTotalVerticesCount( verticesCount )
        , mXOrigin( xOrigin )
        , mYOrigin( yOrigin )
      {}

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

  class MeshSelafinEdgeIterator: public MeshEdgeIterator
  {
    public:
      size_t next( size_t edgeCount,
                   int *startVertexIndices,
                   int *endVertexIndices ) override;
  };

  class MeshSelafinFaceIterator: public MeshFaceIterator
  {
    public:
      MeshSelafinFaceIterator( std::shared_ptr<SerafinStreamReader> reader,
                               std::streampos start,
                               size_t facesCount ):
        mReader( reader )
        , mStart( start )
        , mTotalFacesCount( facesCount )
      {}

      size_t next( size_t faceOffsetsBufferLen, int *faceOffsetsBuffer, size_t vertexIndicesBufferLen, int *vertexIndicesBuffer );

    private:
      std::shared_ptr<SerafinStreamReader> mReader;
      std::streampos mStart;
      size_t mPosition = 0;
      size_t mTotalFacesCount;
  };

  class MeshSelafin: public Mesh
  {
    public:
      MeshSelafin( const std::string &uri,
                   std::shared_ptr<SerafinStreamReader> reader,
                   const std::streampos &verticesXStart,
                   const std::streampos &verticesYStart,
                   const std::streampos &ikleTableStart,
                   size_t verticesCount,
                   size_t facesCount,
                   double xOrigin,
                   double yOrigin
                 ):
        Mesh( "SELAFIN", 3, uri )
        , mReader( reader )
        , mXVerticesStart( verticesXStart )
        , mYVerticesStart( verticesYStart )
        , mIkleTableStart( ikleTableStart )
        , mVerticesCount( verticesCount )
        , mFacesCount( facesCount )
        , mXOrigin( xOrigin )
        , mYOrigin( yOrigin )
      {}

      std::unique_ptr<MeshVertexIterator> readVertices();
      std::unique_ptr<MeshEdgeIterator> readEdges();
      std::unique_ptr<MeshFaceIterator> readFaces();
      size_t verticesCount() const {return mVerticesCount;}
      size_t edgesCount() const {return 0;}
      size_t facesCount() const {return mFacesCount;}
      BBox extent() const;

    private:
      mutable bool mIsExtentUpToDate = false;
      mutable BBox mExtent;

      std::shared_ptr<SerafinStreamReader> mReader;
      std::streampos mXVerticesStart;
      std::streampos mYVerticesStart;
      std::streampos mIkleTableStart;
      size_t mVerticesCount = 0;
      size_t mFacesCount = 0;
      double mXOrigin;
      double mYOrigin;

      void calculateExtent() const;
  };

  /**
   * Serafin format (also called Selafin)
   *
   * Binary format for triangular mesh with datasets defined on vertices
   * http://www.opentelemac.org/downloads/Archive/v6p0/telemac2d_user_manual_v6p0.pdf Appendix 3
   * https://www.gdal.org/drv_selafin.html
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
      typedef std::map<double, std::vector<double> > timestep_map; //TIME (sorted), nodeVal

      void createMesh( double xOrigin,
                       double yOrigin,
                       size_t nElems,
                       size_t nPoints,
                       size_t nPointsPerElem,
                       std::vector<size_t> &ikle,
                       std::vector<double> &x,
                       std::vector<double> &y );
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
                      std::vector<size_t> &ikle,
                      std::vector<double> &x,
                      std::vector<double> &y,
                      std::vector<timestep_map> &data,
                      DateTime &referenceTime );

      bool getStreamPrecision( std::ifstream &in );

      std::unique_ptr< MDAL::Mesh > mMesh;
      std::string mFileName;
      std::shared_ptr<SerafinStreamReader> mReader;
  };

} // namespace MDAL
#endif //MDAL_SELAFIN_HPP
