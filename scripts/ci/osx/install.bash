#!/usr/bin/env bash

export THIS_DIR=`pwd`
wget https://qgis.org/downloads/macos/deps/qgis-deps-${QGIS_DEPS_VERSION}.tar.gz

sudo mkdir -p /opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/
cd /opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/
sudo tar -xvzf $THIS_DIR/qgis-deps-${QGIS_DEPS_VERSION}.tar.gz

/opt/QGIS/qgis-deps-${QGIS_DEPS_VERSION}/stage/bin/gdalinfo --formats
