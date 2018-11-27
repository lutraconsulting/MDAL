/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DATA_MODEL_HPP
#define MDAL_DATA_MODEL_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "mdal.h"

namespace MDAL
{
  class DatasetGroup;
  class Mesh;

  struct BBox
  {
    BBox() {}
    BBox( double lx, double ux, double ly, double uy ): minX( lx ), maxX( ux ), minY( ly ), maxY( uy ) {}

    double minX;
    double maxX;
    double minY;
    double maxY;
  };

  typedef struct
  {
    double x;
    double y;

    bool noData = false;
  } Value; //Dataset Value

  typedef std::vector< std::pair< std::string, std::string > > Metadata;

  class Dataset
  {
    public:
      virtual ~Dataset();
      double time;

      size_t valuesCount() const;
      virtual size_t scalarData( size_t indexStart, size_t count, double *buffer ) = 0;
      virtual size_t vectorData( size_t indexStart, size_t count, double *buffer ) = 0;
      virtual size_t activeData( size_t indexStart, size_t count, int *buffer ) = 0;

      bool isValid = true;
      DatasetGroup *parent = nullptr;
  };

  typedef std::vector<std::shared_ptr<Dataset>> Datasets;

  class DatasetGroup
  {
    public:
      std::string getMetadata( const std::string &key );

      void setMetadata( const std::string &key, const std::string &val );

      std::string name();
      void setName( const std::string &name );

      Metadata metadata;
      Datasets datasets;
      Mesh *parent = nullptr;

      bool isScalar() const;
      void setIsScalar( bool isScalar );

      bool isOnVertices() const;
      void setIsOnVertices( bool isOnVertices );

      std::string uri() const;
      void setUri( const std::string &uri );

    private:
      bool mIsScalar = true;
      bool mIsOnVertices = true;
      std::string mUri; // file/uri from where it came
  };

  typedef std::vector<std::shared_ptr<DatasetGroup>> DatasetGroups;

  class MeshVertexIterator
  {
    public:
      virtual ~MeshVertexIterator();

      virtual size_t next( size_t vertexCount, double *coordinates ) = 0;
  };

  class MeshFaceIterator
  {
    public:
      virtual ~MeshFaceIterator();

      virtual size_t next( size_t faceOffsetsBufferLen,
                           int *faceOffsetsBuffer,
                           size_t vertexIndicesBufferLen,
                           int *vertexIndicesBuffer ) = 0;
  };

  class Mesh
  {
    public:
      Mesh( size_t verticesCount,
            size_t facesCount,
            size_t faceVerticesMaximumCount,
            BBox extent,
            const std::string &uri );
      virtual ~Mesh();

      void setSourceCrs( const std::string &str );
      void setSourceCrsFromWKT( const std::string &wkt );
      void setSourceCrsFromEPSG( int code );

      void setVerticesCount( const size_t &verticesCount );
      void setFacesCount( const size_t &facesCount );
      void setFaceVerticesMaximumCount( const size_t &faceVerticesMaximumCount );
      void setExtent( const BBox &extent );

      virtual std::unique_ptr<MDAL::MeshVertexIterator> readVertices() = 0;
      virtual std::unique_ptr<MDAL::MeshFaceIterator> readFaces() = 0;

      DatasetGroups datasetGroups;

      size_t verticesCount() const;
      size_t facesCount() const;
      std::string uri() const;
      BBox extent() const;
      std::string crs() const;
      size_t faceVerticesMaximumCount() const;

    private:
      size_t mVerticesCount = 0;
      size_t mFacesCount = 0;
      size_t mFaceVerticesMaximumCount = 0; //typically 3 or 4, sometimes up to 9
      BBox mExtent;
      const std::string mUri; // file/uri from where it came
      std::string mCrs;
  };
} // namespace MDAL
#endif //MDAL_DATA_MODEL_HPP

