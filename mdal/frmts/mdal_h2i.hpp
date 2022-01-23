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
      };

      struct CellH2i
      {
        double x, y, zmin, zmax;
        double size;

        std::vector<int> neighborsCellCountperSide;
      };

      std::unique_ptr<Mesh> createMeshFrame( const MetadataH2i &metadata );
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

}

#endif // MDAL_H2I_HPP
