#!/usr/bin/env bash
set -e

wget http://download.osgeo.org/osgeo4w/osgeo4w-setup-x86_64.exe
./osgeo4w-setup-x86_64.exe --autoaccept --arch x86_64 --quiet-mode  --packages pkg-gdal