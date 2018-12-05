#!/usr/bin/env bash
set -e

# MDAL
sudo add-apt-repository ppa:ubuntugis/ppa -y # for gdal 2.x
sudo apt-get -qq update
sudo apt-get install -y --allow-unauthenticated libgdal-dev
sudo apt-get install -y libhdf5-dev libnetcdf-dev

# MinGW
sudo apt-get install mingw-w64

# Valgrind
sudo apt-get install libc6-dbg gdb valgrind