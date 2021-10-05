/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_DHI_DFSU_HPP
#define MDAL_DHI_DFSU_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <assert.h>

#include "mdal_dhi_dfs.hpp"

class MeshDfsu: public Mesh
{
  public:
    static bool canRead( const std::string &uri );
    static std::unique_ptr<MeshDfsu> loadMesh( const std::string &uri );

    //**************** Mesh frame *************
    int verticesCount() const override;
    int facesCount() const override;

  private:
    MeshDfsu() = default;

    int mMaxNumberOfLayer = 0;
    size_t mTotalNodeCount = 0;
    std::string mWktProjection = "projection";
    std::map<int, size_t> mNodeId2VertexIndex;
    int mGapFromVertexToNode = 0;

    size_t vertexIdToIndex( int id ) const;

    std::vector<int> mFaceNodeCount;
    std::map<int, size_t> mElemId2faceIndex;
    int mGapFromFaceToElement = 0;

    int nodeCount( size_t faceIndex ) const override;
    size_t connectivityPosition( int faceIndex ) const override;

    // for 3D stacked mesh
    std::vector<int> mFaceToVolume;
    VertexIndexesOfLevelsOnMesh mLevels;

    static bool fileInfo( LPHEAD  pdfs, int &totalNodeCount, int &elementCount, int &dimension, int &maxNumberOfLayer, int &numberOfSigmaLayer );

    bool populateMeshFrame();
    bool populate2DMeshFrame();
    bool populate3DMeshFrame();
    bool setCoordinate( LPVECTOR pvec, LPITEM staticItem, SimpleType    itemDatatype, size_t offset, double &min, double &max );
};

#endif //MDAL_DHI_DFSU_HPP;
