#!/usr/bin/env bash
set -e

echo "Installing qgis with choco"

choco install qgis --version 3.4.3 -y --verbose
mv  "C:/Program Files/QGIS 3.4" "C:/OSGeo4W64"
