#!/usr/bin/env bash
set -e

choco install qgis --version 3.2.3 -y
cp -R "C:/Program Files/QGIS 3.2" "C:/OSGeo4W64"