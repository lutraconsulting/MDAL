/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_MEMORY_DATA_MODEL_HPP
#define MDAL_MEMORY_DATA_MODEL_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "mdal.h"
#include "mdal_data_model.hpp"

namespace MDAL
{
  typedef struct
  {
    double x;
    double y;
    double z; // Bed elevation
  } Vertex;

  typedef std::vector<size_t> Face;
  typedef std::vector<Vertex> Vertices;
  typedef std::vector<Face> Faces;

  /**
   * The MemoryDataset stores all the data in the memory
   */
  class MemoryDataset: public Dataset
  {
    public:
      ~MemoryDataset() override;

      size_t scalarData( size_t indexStart, size_t count, double *buffer ) override;
      size_t vectorData( size_t indexStart, size_t count, double *buffer ) override;
      size_t activeData( size_t indexStart, size_t count, int *buffer ) override;

      /**
       * size - face count if !isOnVertices
       * size - vertex count if isOnVertices
       */
      std::vector<Value> values;
      std::vector<int> active; // size - face count. Whether the output for this is active...
  };

  class MemoryMesh: public Mesh
  {
    public:
      MemoryMesh( size_t verticesCount,
                  size_t facesCount,
                  size_t faceVerticesMaximumCount,
                  BBox extent,
                  const std::string &uri );
      ~MemoryMesh() override;

      std::unique_ptr<MDAL::MeshVertexIterator> readVertices() override;
      std::unique_ptr<MDAL::MeshFaceIterator> readFaces() override;

      void addBedElevationDataset( const Vertices &vertices, const Faces &faces );

      Vertices vertices;
      Faces faces;
  };

  class MemoryMeshVertexIterator: public MeshVertexIterator
  {
    public:
      MemoryMeshVertexIterator( const MemoryMesh *mesh );
      ~MemoryMeshVertexIterator() override;

      size_t next( size_t vertexCount, double *coordinates ) override;

      const MemoryMesh *mMemoryMesh;
      size_t mLastVertexIndex = 0;

  };

  class MemoryMeshFaceIterator: public MeshFaceIterator
  {
    public:
      MemoryMeshFaceIterator( const MemoryMesh *mesh );
      ~MemoryMeshFaceIterator() override;

      size_t next( size_t faceOffsetsBufferLen,
                   int *faceOffsetsBuffer,
                   size_t vertexIndicesBufferLen,
                   int *vertexIndicesBuffer ) override;

      const MemoryMesh *mMemoryMesh;
      size_t mLastFaceIndex = 0;

  };
} // namespace MDAL
#endif //MDAL_MEMORY_DATA_MODEL_HPP
