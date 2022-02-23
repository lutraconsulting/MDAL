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
  //! Helper structure used to sent H2i links information to the dataset for reading file and interpreting H2i data to MDAL data
  struct LinkH2i
  {
    size_t cell0; //!< index of the first related cell
    size_t cell1; //!< index of the second related cell
    unsigned char ori; //!< orientation of the link, 0: following X, 1: folowing Y
    double ratioSize0; //!< Ratio of the size of the cell 1 with the cell 0, or 1 if ratio is greater than 1
    double ratioSize1; //!< Ratio of the size of the cell 0 with the cell 1, or 1 if ratio is greater than 1
    double minSize; //!< minimum cell size betxeen cell0 and cell 1
  };

  typedef std::shared_ptr<std::vector<LinkH2i>> LinksH2i;

  /**
   * Driver of H2i files format.
   *
   * H2i mesh structure is a quad tree structure. The structure is defined by nodes a the center of quad and by links
   * that, at each side of quads, represents the interface between quad.
   *
   * Files of H2i format are:
   * - a json file with:
   *    - some information about the mesh (mesh name, crs, reference time and time steps)
   *    - relative path of the files containing the nodes definition and the links definition
   *    - relative path to the file containing the quantity dataset groups.
   * - text file containing the nodes information
   * - text file containing the links information
   * - text file containing the time step
   * - one binary file per dataset group, dataset group can be relied to nodes or links.
   *
   * Dataset groups on nodes have scalar values (water depth, water level) that are interpreted by MDAL as scalar value on faces.
   *
   * Dataset groups on links have also scalar values but related to quantities that are vectors. Theses values as interpreted by MDAL as vector value on
   * faces using direction of the link and converting value like this:
   * for each face (node), X or Y vector value is average of value of link in X or Y direction weighted by the side lenght of the node on the other side of the link.
   *
   * For the particular case of discharge, H2i format store this quantity in links as full discharge (m3/s), that is not compatible with a 2D planar representation.
   * So MDAL converts discharge dataset group as "unit discharge"(m2/s), dividing the full discharge value per the side of faces to retrieve the unit discharge.
   *
   */
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
        bool isScalar;
      };

      struct MetadataH2i
      {
        std::string metadataFilePath;
        std::string dirPath;
        std::string meshName;
        std::string gridFile;
        std::string gridlayer;
        std::string referenceTime;
        std::string timeStepFile;
        std::string crs;

        std::vector<MetadataH2iDataset> datasetGroups;
      };

      std::unique_ptr<Mesh> createMeshFrame( const MetadataH2i &metadata );

      bool parseJsonFile( const std::string filePath, MetadataH2i &metadata );

      void parseTime( const MetadataH2i &metadata, MDAL::DateTime &referenceTime, std::vector<MDAL::RelativeTimestamp> &timeSteps );
  };

  class DatasetH2i: public Dataset2D
  {
    public:

      DatasetH2i( DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex );

      void clear();

    protected:

      std::shared_ptr<std::ifstream> mIn;
      bool mDataLoaded = false;
      std::vector<double> mValues;
      size_t mDatasetIndex = 0;
  };

  /**
   * Dataset group of H2i format for scalor
   *
   */
  class DatasetH2iScalar: public DatasetH2i
  {
    public:

      DatasetH2iScalar( DatasetGroup *grp, std::shared_ptr<std::ifstream> in, size_t datasetIndex );

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t, size_t, double * ) override {return 0;}

    private:

      void loadData();
      std::streampos beginingInFile() const;
  };

  class DatasetH2iVector: public DatasetH2i
  {
    public:

      DatasetH2iVector( DatasetGroup *grp,
                        std::shared_ptr<std::ifstream> in,
                        size_t datasetIndex );

      size_t scalarData( size_t, size_t, double * ) override {return 0;}
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;

    private:

      void loadData();
      std::streampos beginingInFile() const;
  };

}

#endif // MDAL_H2I_HPP
