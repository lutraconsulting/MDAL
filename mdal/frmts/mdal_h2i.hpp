/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2022 Vincent Cloarec (vcloarec at gmail dot com)
*/
#ifndef MDAL_H2I_HPP
#define MDAL_H2I_HPP

#include "mdal_driver.hpp"
#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"

namespace MDAL
{

  class DriverH2i: public Driver
  {
    public:
      DriverH2i();

      DriverH2i *create() override;
      int faceVerticesMaximumCount() const override {return 4;}
      bool canReadMesh( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, const std::string &meshName = "" ) override;

    private:
      std::string buildUri( const std::string &meshFile ) override;

      struct MetadataH2iDataset
      {
        std::string layer;
        std::string file;
        std::string type;
        std::string units;
        std::string topology_file;
      };

      struct MetadataH2i
      {
        std::string metadataFilePath;
        std::string dirPath;
        std::string meshName;
        std::string nodesFile;
        std::string linksFile;
        std::string referenceTime;
        std::string timeStepFile;
        std::string crs;

        std::vector<MetadataH2iDataset> datasetGroups;
      };

      struct CellH2i
      {
        double x, y, z;
        double size;

        std::vector<int> neighborsCellCountperSide;
      };

      std::unique_ptr<Mesh> createMeshFrame( const MetadataH2i &metadata, std::vector<double> &topography );
      bool parseJsonFile( const std::string filePath, MetadataH2i &metadata );
      void parseNodeFile( std::vector<CellH2i> &cells,
                          const MetadataH2i &meta,
                          double &minSize,
                          double &maxSize,
                          double &xMin,
                          double &xMax,
                          double &yMin,
                          double &yMax ) const;
      void parseLinkFile( std::vector<CellH2i> &cells, const MetadataH2i &meta ) const;

      void parseTime( const MetadataH2i &metadata, MDAL::DateTime &referenceTime, std::vector<MDAL::RelativeTimestamp> &timeSteps );
  };

  class DatasetH2iOnNode: public Dataset2D
  {
    public:
      DatasetH2iOnNode( DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex );

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t, size_t, double * ) override {return 0;}

      void clear();

    private:
      std::shared_ptr<std::ifstream> mIn;
      bool mDataloaded = false;
      std::vector<double> mValues;
      size_t mDatasetIndex;

      void loadData();

      std::streampos beginingInFile() const;

  };

}

#endif // MDAL_H2I_HPP
