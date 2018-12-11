/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DRIVER_MANAGER_HPP
#define MDAL_DRIVER_MANAGER_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>

#include "mdal.h"
#include "mdal_data_model.hpp"
#include "frmts/mdal_driver.hpp"

namespace MDAL
{

  class DriverManager
  {
    public:
      static std::unique_ptr< Mesh > load( const std::string &meshFile, MDAL_Status *status );
      static void loadDatasets( Mesh *mesh, const std::string &datasetFile, MDAL_Status *status );

      static std::vector<std::shared_ptr<MDAL::Driver>> drivers();
      static std::shared_ptr<Driver> driver( const std::string &driverName );
  };

} // namespace MDAL
#endif //MDAL_DRIVER_MANAGER_HPP
