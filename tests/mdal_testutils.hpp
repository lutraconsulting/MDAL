/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_TESTUTILS_HPP
#define MDAL_TESTUTILS_HPP

#include <string>
#include "mdal.h"

const char *data_path();

std::string test_file( std::string basename );

//! Convinient function to get one value for active flag
bool getActive( DatasetH dataset, int index );
//! Convinient function to get one scalar
double getValue( DatasetH dataset, int index );
//! Convinient function to get one data X value
double getValueX( DatasetH dataset, int index );
//! Convinient function to get one data Y value
double getValueY( DatasetH dataset, int index );

//! Compare two vectors
bool compareVectors( const std::vector<double> &a, const std::vector<double> &b );

#endif // MDAL_TESTUTILS_HPP
