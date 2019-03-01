/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
 Christophe Coulet - Arteliagroup
*/

#ifndef MDAL_SLF_HPP
#define MDAL_SLF_HPP

#include <string>
#include <memory>

#include "mdal_data_model.hpp"
#include "mdal_memory_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"
#include "SerafinReader.h"

namespace MDAL
{
  class MeshSlf: public MemoryMesh
  {
    public:
      MeshSlf( size_t verticesCount,
               size_t facesCount,
               size_t faceVerticesMaximumCount,
               BBox extent,
               const std::string &uri);
      ~MeshSlf() override;

  };

  class DriverSlf: public Driver
  {
    public:
      DriverSlf();
      ~DriverSlf() override;
      DriverSlf *create() override;

      bool canRead( const std::string &uri ) override;
      std::unique_ptr< Mesh > load( const std::string &meshFile, MDAL_Status *status ) override;

	  char* title;
      ifstream *FileStream;  // Flux de lecture du fichier
      SerafinReader* Reader; /** /!\ Instance de lecture du fichier Serafin **/

    private:
      std::string mMeshFile;
  };

} // namespace MDAL
#endif //MDAL_SLF_HPP
