#!/usr/bin/env bash
set -e

# MDAL
sudo add-apt-repository ppa:ubuntugis/ppa -y # for gdal 2.x
sudo apt-get -qq update
sudo apt-get install -y --allow-unauthenticated libgdal-dev
sudo apt-get install -y libhdf5-dev libnetcdf-dev
sudo apt-get install -y libxml2-dev

# MinGW
if [ "x${LINUX_MINGW}" = "xtrue" ]; then
  sudo apt-get install mingw-w64
fi

# Valgrind
if [ "x${LINUX_MEMCHECK}" = "xtrue" ]; then
  sudo apt-get install libc6-dbg gdb valgrind
fi
 
# Code coverage
if [ "x${LINUX_COVERAGE}" = "xtrue" ]; then
  sudo apt-get install -y ruby
  sudo apt-get install -y lcov
  sudo gem install coveralls-lcov
fi
