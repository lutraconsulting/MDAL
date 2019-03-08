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
   * - Currently we do not support stride other than 1 (every element)
   * - Vector datasets are not supported
   * - Assumes BASEMENT 3.x format where the array is nFaces x 1
   */
  struct HyperSlab
  {
    size_t startX = 0; // offset X
    size_t startY = 0; // offset Y
    size_t count = 0; // number of cells/vertices
    bool countInFirstColumn = true;
    bool isScalar;
  };

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
      std::vector<hsize_t> selections( size_t copyValues );

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
      /**
       Parse XML File with this structure, where data is specified as pointers to HDF in Attribute tags

       <?xml version="1.0" ?>
       <!DOCTYPE Xdmf SYSTEM "Xdmf.dtd" []>
       <Xdmf Version="2.0">
        <Domain>
            <Topology> ... </Topology>
            <Geometry> ... </Geometry>
            <Grid GridType="Collection" Name="..." CollectionType="Temporal">
                <Grid GridType="Uniform" Name="Timestep">
                    <Time TimeType="Single" Name="time = 0.000000" Value="0.000000"> </Time>
                    <Topology></Topology>
                    <Geometry GeometryType="XY" Reference="/Xdmf/Domain/Geometry[1]"></Geometry>
                <Attribute> ... </Attribute>
                </Grid>
            </Grid>
        <Domain>
       </Xdmf>
      */
      DatasetGroups parseXdmfXml( );

      /**
       * Parse hyperslab specification from matrix in DataItem [Dimension] tag

         <DataItem ItemType="Uniform" Dimensions="3 3" Format="XML">
           0 0 0 1 1 1 18497 1 1
         </DataItem>
       */
      HyperSlab parseHyperSlab( const std::string &str, size_t dimB );

      //! Extract HDF5 filename and HDF5 Path from XML file dirname and fileName:hdfPath syntax
      void hdf5NamePath( const std::string &dataItemPath, std::string &filePath, std::string &hdf5Path );

      //! Parse 2d matrix from text, e.g. 3 2 -> [3, 2].
      //! Verify that it has 2 item
      std::vector<size_t> parseDimensions2D( const std::string &data );

      MDAL::Mesh *mMesh = nullptr;
      std::string mDatFile;

  };

} // namespace MDAL
#endif //MDAL_XDMF_HPP
