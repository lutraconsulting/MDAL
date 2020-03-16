/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TESTUTILS_HPP
#define MDAL_TESTUTILS_HPP

#include <string>
#include <cstring>
#include <vector>

#include "mdal.h"

const char *data_path();

void init_test();
void finalize_test();

std::string test_file( std::string basename );
std::string tmp_file( std::string basename );
void copy( const std::string &src, const std::string &dest );
void deleteFile( const std::string &path );
bool fileExists( const std::string &filename );

// Mesh
std::vector<double> getCoordinates( MeshH mesh, int verticesCount );
double getVertexXCoordinatesAt( MeshH mesh, int index );
double getVertexYCoordinatesAt( MeshH mesh, int index );
double getVertexZCoordinatesAt( MeshH mesh, int index );
int getFaceVerticesCountAt( MeshH mesh, int faceIndex );
int getFaceVerticesIndexAt( MeshH mesh, int faceIndex, int index );
std::vector<int> faceVertexIndices( MeshH mesh, int faceCount );
void getEdgeVertexIndices( MeshH mesh, int edgesCount, std::vector<int> &start, std::vector<int> &end );

// Datasets 2D

// < 0 invalid (does not support flag), 0 false, 1 true
int getActive( DatasetH dataset, int index );
double getValue( DatasetH dataset, int index );
double getValueX( DatasetH dataset, int index );
double getValueY( DatasetH dataset, int index );
int get3DFrom2D( DatasetH dataset, int index );

// Datasets 3D
int getLevelsCount3D( DatasetH dataset, int index );
double getLevelZ3D( DatasetH dataset, int index );
bool getActive3D( DatasetH dataset, int index );
double getValue3D( DatasetH dataset, int index );
double getValue3DX( DatasetH dataset, int index );
double getValue3DY( DatasetH dataset, int index );


//! Compare two vectors
bool compareVectors( const std::vector<double> &a, const std::vector<double> &b );
bool compareVectors( const std::vector<int> &a, const std::vector<int> &b );

//! Same vertices (coords), faces, edges and connectivity between them
void compareMeshFrames( MeshH meshA, MeshH meshB );
void saveAndCompareMesh( const std::string &filename, const std::string &savedFile, const std::string &driver, const std::string &meshName = "" );

//! Compare duration with millisecond precision
bool compareDurationInHours( double h1, double h2 );

//! test if reference time is defined
bool hasReferenceTime( DatasetGroupH group );

//! Compare the reference time with the string
bool compareReferenceTime( DatasetGroupH group, const char *referenceTime );

#endif // MDAL_TESTUTILS_HPP
