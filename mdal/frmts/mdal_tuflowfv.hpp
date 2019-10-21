/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TUFLOWFV_HPP
#define MDAL_TUFLOWFV_HPP

#include <string>
#include <memory>
#include <map>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"
#include "mdal_cf.hpp"

namespace MDAL
{
  /**
   * TUFLOW FV format
   *
   * Binary NetCDF format with structure similar to UGRID stored as
   * 3D Layered Mesh (https://github.com/qgis/QGIS-Enhancement-Proposals/issues/158)
   *
   * Both mesh and dataset is stored in single file.
   */
  class DriverTuflowFV: public DriverCF
  {
    public:
      DriverTuflowFV();
      ~DriverTuflowFV() override;
      DriverTuflowFV *create() override;

    private:
      CFDimensions populateDimensions( ) override;
      void populateFacesAndVertices( Vertices &vertices, Faces &faces ) override;
      void addBedElevation( MemoryMesh *mesh ) override;
      std::string getCoordinateSystemVariableName() override;
      std::set<std::string> ignoreNetCDFVariables() override;
      void parseNetCDFVariableMetadata( int varid, const std::string &variableName,
                                        std::string &name, bool *is_vector, bool *is_x ) override;
      std::string getTimeVariableName() const override;

      //! Returns number of vertices
      size_t parse2DMesh();

      void addBedElevationDatasetOnFaces();
      void populateVertices( MDAL::Vertices &vertices );
      void populateFaces( MDAL::Faces &faces );
  };

} // namespace MDAL
#endif //MDAL_TUFLOWFV_HPP
