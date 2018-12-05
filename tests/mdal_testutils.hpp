/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TESTUTILS_HPP
#define MDAL_TESTUTILS_HPP

#include <string>
#include <vector>

#include "mdal.h"

const char *data_path();

std::string test_file( std::string basename );

// Mesh
std::vector<double> getCoordinates( MeshH mesh, int verticesCount );
double getVertexXCoordinatesAt( MeshH mesh, int index );
double getVertexYCoordinatesAt( MeshH mesh, int index );
double getVertexZCoordinatesAt( MeshH mesh, int index );
int getFaceVerticesCountAt( MeshH mesh, int faceIndex );
int getFaceVerticesIndexAt( MeshH mesh, int faceIndex, int index );

// Datasets
bool getActive( DatasetH dataset, int index );
double getValue( DatasetH dataset, int index );
double getValueX( DatasetH dataset, int index );
double getValueY( DatasetH dataset, int index );

//! Compare two vectors
bool compareVectors( const std::vector<double> &a, const std::vector<double> &b );
bool compareVectors( const std::vector<int> &a, const std::vector<int> &b );

#endif // MDAL_TESTUTILS_HPP
