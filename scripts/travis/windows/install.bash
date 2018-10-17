#!/usr/bin/env bash
set -e

choco install qgis --version 3.2.3 -y
mkdir -p C:/OSGeo4W64/lib

# fixes LINK : fatal error LNK1104: cannot open file 'C:\OSGeo4W64\lib\zlib.lib'
# [C:\Users\travis\build\lutraconsulting\MDAL\build_win\mdal\mdal.vcxproj]
cp -v "C:/Program Files/QGIS 3.2/lib/zlib.lib" "C:/OSGeo4W64/lib/zlib.lib"
# LINK : fatal error LNK1104: cannot open file 'C:\OSGeo4W64\lib\szip.lib'
# [C:\Users\travis\build\lutraconsulting\MDAL\build_win\mdal\mdal.vcxproj]
cp -v "C:/Program Files/QGIS 3.2/lib/szip.lib" "C:/OSGeo4W64/lib/szip.lib"