#!/usr/bin/env bash

brew update
brew tap osgeo/osgeo4mac
brew install hdf5
brew install netcdf
brew install osgeo/osgeo4mac/gdal2 --with-complete --with-libkml