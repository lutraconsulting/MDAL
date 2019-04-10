#!/usr/bin/env bash


brew uninstall --ignore-dependencies gdal
brew uninstall --ignore-dependencies proj
brew uninstall --ignore-dependencies libgeotiff
brew uninstall --ignore-dependencies libspatialite
brew uninstall --ignore-dependencies postgresql

brew tap osgeo/osgeo4mac
brew install hdf5
brew install libxml2
brew install osgeo-netcdf
brew install osgeo-gdal
brew install osgeo-gdal

ls -la /usr/local/opt
ls -la /usr/local/Cellar
