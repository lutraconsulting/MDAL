/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_LOADER_HPP
#define MDAL_LOADER_HPP

#include <string>
#include <memory>

#include "mdal.h"
#include "mdal_defines.hpp"

namespace MDAL
{

  class Loader
  {
    public:
      static std::unique_ptr< Mesh > load( const std::string &meshFile, MDAL_Status *status );
  };

} // namespace MDAL
#endif //MDAL_LOADER_HPP
