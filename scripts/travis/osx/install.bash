#!/usr/bin/env bash

brew tap osgeo/osgeo4mac
brew install hdf5 -y
brew install netcdf -y
brew install osgeo/osgeo4mac/gdal2 -y #TODO --with-complete timeouts travis

/usr/local/opt/gdal2/bin/gdalinfo --formats
/usr/local/opt/gdal2/bin/ogrinfo --formats