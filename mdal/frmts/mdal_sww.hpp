/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_SWW_HPP
#define MDAL_SWW_HPP

#include <string>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"

namespace MDAL
{
  // AnuGA format with extension .SWW
  class LoaderSWW
  {
    public:
      LoaderSWW( const std::string &resultsFile );
      std::unique_ptr< Mesh > load( MDAL_Status *status );
    private:
      std::string mFileName;
  };
} // namespace MDAL
#endif //MDAL_SWW_HPP
