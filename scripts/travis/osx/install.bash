#!/usr/bin/env bash

brew tap osgeo/osgeo4mac --verbose
brew install hdf5 -y
brew install netcdf -y
brew install osgeo/osgeo4mac/gdal2 --with-complete --with-libkml -y

/usr/local/opt/gdal2/bin/gdalinfo --formats
/usr/local/opt/gdal2/bin/ogrinfo --formats