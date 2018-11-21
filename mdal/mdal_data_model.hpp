/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DEFINES_HPP
#define MDAL_DEFINES_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>
#include <string>

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
    double z; // Bed elevation
  } Vertex;

  typedef std::vector<size_t> Face;

  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Face> Faces;

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
      virtual ~Dataset() = 0;
      double time;

      size_t valuesCount() const;
      virtual Value value(size_t index) = 0;
      virtual bool isActive( size_t faceIndex ) = 0;

      bool isValid = true;
      DatasetGroup *parent = nullptr;
  };

  class MemoryDataset: public Dataset
  {
    public:
      ~MemoryDataset() = default;

      Value value(size_t index);
      bool isActive( size_t faceIndex );

      /**
       * size - face count if !isOnVertices
       * size - vertex count if isOnVertices
       */
      std::vector<Value> values;
      std::vector<bool> active; // size - face count. Whether the output for this is active...
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
      void setIsScalar(bool isScalar);

      bool isOnVertices() const;
      void setIsOnVertices(bool isOnVertices);

      std::string uri() const;
      void setUri(const std::string& uri);

    private:
      bool mIsScalar = true;
      bool mIsOnVertices = true;
      std::string mUri; // file/uri from where it came
  };

  typedef std::vector<std::shared_ptr<DatasetGroup>> DatasetGroups;

  class Mesh
  {
  public:
    std::string crs() const;
    void setSourceCrs( const std::string &str );
    void setSourceCrsFromWKT( const std::string &wkt );
    void setSourceCrsFromEPSG( int code );
    void addBedElevationDataset();

    DatasetGroups datasetGroups;

    Vertices vertices;
    std::map<size_t, size_t> vertexIDtoIndex; // only for 2DM and DAT files

    Faces faces;
    std::map<size_t, size_t> faceIDtoIndex; // only for 2DM and DAT files

    size_t verticesCount() const;
    size_t facesCount() const;

    std::string uri() const;
    void setUri(const std::string& uri);

    private:
    std::string mUri; // file/uri from where it came
    std::string mCrs;
  };

} // namespace MDAL
#endif //MDAL_DEFINES_HPP

