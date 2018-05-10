/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_DEFINES_HPP
#define MDAL_DEFINES_HPP

#include <stddef.h>
#include <vector>
#include <memory>
#include <map>

namespace MDAL
{

  typedef struct
  {
    double x;
    double y;
  } Vertex;

  typedef std::vector<size_t> Face;




  typedef struct
  {
    double x;
    double y;

    bool noData = false;
  } Value; //Dataset Value

  class Dataset
  {
  public:
    void setMetadata(const std::string& key, const std::string& val) {
      // TODO check if there is no such pair already
      metadata.push_back(std::make_pair(key, val));
    }
    void setName(const std::string& name) {
      setMetadata("name", name);
    }

    std::vector< std::pair< std::string, std::string > > metadata;
    std::vector<Value> values; // size - vertex count if isOnVertices
                               // size - face count if !isOnVertices
    std::vector<bool> active; // size - face count. Whether the output for this is active...

    bool isScalar = true;
    bool isOnVertices = true;
    bool isValid = true;

    std::string uri; // file/uri from where it came


    void free() {
      values.clear();
      metadata.clear();
      isValid = false;
    }

    bool isActive(size_t faceIndex) {
      if (isOnVertices) {
        if (active.size() > faceIndex)
          return active[faceIndex];
        else
          return false;
      } else {
        return true;
      }
    }
  };

  struct Mesh
  {
    std::string uri; // file/uri from where it came

    std::vector<Vertex> vertices;
    std::map<size_t, size_t> vertexIDtoIndex; // only for 2DM and DAT files

    std::vector<Face> faces;
    std::map<size_t, size_t> faceIDtoIndex; // only for 2DM and DAT files

    std::vector<std::shared_ptr<Dataset>> datasets;
  };
} // namespace MDAL
#endif //MDAL_DEFINES_HPP

