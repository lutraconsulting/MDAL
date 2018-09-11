#!/usr/bin/env bash
set -e

sudo add-apt-repository ppa:cs50/ppa -y # for astyle 3.x
sudo add-apt-repository ppa:ubuntugis/ppa -y # for gdal 2.x
sudo apt-get -qq update
sudo apt-get install -y --allow-unauthenticated libgdal-dev astyle
sudo apt-get install -y libhdf5-dev libnetcdf-dev
sudo apt-get install mingw-w64
