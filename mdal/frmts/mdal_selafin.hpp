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
#include "mdal_utils.hpp"

namespace MDAL
{
  class SerafinStreamReader
  {
    public:
      SerafinStreamReader();
      bool initialize( const std::string &fileName );

      std::string read_string( size_t len );
      std::vector<double> read_double_arr( size_t len );
      std::vector<int> read_int_arr( size_t len );
      std::vector<size_t> read_size_t_arr( size_t len );

      double read_double( );
      int read_int( );
      size_t read_sizet( );

      size_t remainingBytes();

      //! Return true if is float precision
      bool streamInFloatPrecision() const;

    private:
      void ignore_array_length( );
      std::string read_string_without_length( size_t len );
      void ignore( int len );
      bool getStreamPrecision();

      std::string mFileName;
      bool mStreamInFloatPrecision = true;
      bool mChangeEndianness = true;
      long long mFileSize = -1;
      std::ifstream mIn;

      bool readHeader();
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

      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mFileName;
      SerafinStreamReader mReader;
  };

} // namespace MDAL
#endif //MDAL_SELAFIN_HPP
