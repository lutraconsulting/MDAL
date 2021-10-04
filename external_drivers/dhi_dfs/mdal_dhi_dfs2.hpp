/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/


// compile with: /clr

#ifndef MDAL_DHI_DFS2_HPP
#define MDAL_DHI_DFS2_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <assert.h>

#include "mdal_dhi_dfs.hpp"


class MeshDfs2: public Mesh
{
  public:
    static bool canRead( const std::string &uri );
    static std::unique_ptr<MeshDfs2> loadMesh( const std::string &uri );

    //**************** Mesh frame *************
    int verticesCount() const override { return int( mVertexCoordinates.size() / 3 ); }
    int facesCount() const override { return int( mConnectivity.size() / 4 ); }

  private:
    MeshDfs2() = default;

    void buildMesh( double vertexX0, double vertexY0, double dIx, double dIy, double dJx, double dJy, size_t countI, size_t countJ );

    static bool fileInfo( LPHEAD pdfs, double &originX, double &originY, size_t &countX, size_t &countY, double &sizeX, double &sizeY );

    size_t connectivityPosition( int faceIndex ) const override { return static_cast<size_t>( faceIndex * 4 ); }
    int nodeCount( size_t faceIndex ) const override;
};

#endif //MDAL_DHI_DFS2_HPP;
