/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_XDMF_HPP
#define MDAL_XDMF_HPP

#include <string>
#include <vector>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_hdf5.hpp"
#include "mdal_driver.hpp"

namespace MDAL
{

  /**
   * The XdmfDataset reads the data directly from HDF5 file
   * by usage of hyperslabs retrieval
   *
   * http://xdmf.org/index.php/XDMF_Model_and_Format#HyperSlab
   * HyperSlab consists of 3 rows: start, stride, and count
   * or 2 rows: start and count
   * - Currently we do not support stride other than 1 (every element)
   * - Vector datasets are not supported
   * - Assumes BASEMENT 3.x format where the array is nFaces x 1
   */
  typedef std::vector<std::vector <int> > HyperSlab;

  class XdmfDataset: public Dataset
  {
    public:
      XdmfDataset( DatasetGroup *grp,
                   const HyperSlab &slab,
                   const HdfDataset &valuesDs,
                   double time
                 );
      ~XdmfDataset() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

    private:
      std::vector<hsize_t> offsets( size_t indexStart );

      HdfDataset mHdf5DatasetValues;
      HyperSlab mHyperSlab;
  };

  class DriverXdmf: public Driver
  {
    public:
      /**
       * Driver for XDMF Files
       *
       * XDMF is combination of XML file with dataset metadata and
       * HDF5 file with actual data for the datasets
       *
       * full file specification http://xdmf.org/index.php/XDMF_Model_and_Format
       *
       * XDMF file can have data (vectors) stored in different ways. Currently we
       * only support format for BASEMENET 3.x solver
       */
      DriverXdmf();
      ~DriverXdmf( ) override;
      DriverXdmf *create() override;

      bool canRead( const std::string &uri ) override;
      void load( const std::string &datFile, Mesh *mesh, MDAL_Status *status ) override;

    private:
      DatasetGroups parseXdmfXml( );

      MDAL::Mesh *mMesh = nullptr;
      std::string mDatFile;
  };

} // namespace MDAL
#endif //MDAL_XDMF_HPP
