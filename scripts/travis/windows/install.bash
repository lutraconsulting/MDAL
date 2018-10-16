#!/usr/bin/env bash
set -e

choco install cmake --installargs 'ADD_CMAKE_TO_PATH=User'

# TODO install GDAL, NetCDF and HDF5
wget http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe
# ./osgeo4w-setup-x86_64.exe --autoaccept --arch x86_64 --packages pkg-gdal
