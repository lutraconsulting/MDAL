/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SELAFIN_HPP
#define MDAL_SELAFIN_HPP

#include <string>
#include <memory>
#include <map>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"

namespace MDAL
{
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

      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, MDAL_Status *status ) override;

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
      void addData( const std::vector<std::string> &var_names, const std::vector<timestep_map> &data, size_t nPoints, size_t nElems );
      void parseFile( std::vector<std::string> &var_names,
                      double *xOrigin,
                      double *yOrigin,
                      size_t *nElem,
                      size_t *nPoint,
                      size_t *nPointsPerElem,
                      std::vector<size_t> &ikle,
                      std::vector<double> &x,
                      std::vector<double> &y,
                      std::vector<timestep_map> &data );

      bool getStreamPrecision( std::ifstream &in );

      std::unique_ptr< MDAL::MemoryMesh > mMesh;
      std::string mFileName;
  };

} // namespace MDAL
#endif //MDAL_SELAFIN_HPP
