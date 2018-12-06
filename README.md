[![Build Status](https://travis-ci.org/lutraconsulting/MDAL.svg?branch=master)](https://travis-ci.org/lutraconsulting/MDAL)
[![CircleCI](https://circleci.com/gh/lutraconsulting/MDAL.svg?style=svg)](https://circleci.com/gh/lutraconsulting/MDAL)
[<img src="https://my.cdash.org/images/cdash.gif" alt="cdash" width="20"/>](https://my.cdash.org/index.php?project=MDAL)

# MDAL
Mesh Data Abstraction Library

see [Unstructured Mesh Layers](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119#issuecomment-380018557)

## Supported Formats

You can use MDAL to load the following file formats:

- [NetCDF](https://en.wikipedia.org/wiki/NetCDF): Generic format for scientific data. Examples can be found [here](http://apps.ecmwf.int/datasets/data/interim-full-daily/levtype=sfc/)
- [GRIB](https://en.wikipedia.org/wiki/GRIB): Format commonly used in meteorology. Examples can be found [here](http://apps.ecmwf.int/datasets/data/interim-full-daily/levtype=sfc/)
- [XMDF](https://en.wikipedia.org/wiki/XMDF): **Lazy Loading** As an example, hydraulic outputs from TUFLOW modelling package
- [DAT](http://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat): Outputs of various hydrodynamic modelling packages (e.g. BASEMENT, HYDRO_AS-2D, TUFLOW)
- [3Di](http://www.3di.nu): 3Di modelling package formate based on [CF Conventions](http://cfconventions.org)

Some formats are loaded in-memory, some formats support lazy loading.

# Build 

## Windows 

For 64-bit version:

* Install Microsoft Visual Studio 2017
* Install OSGeo4W (64bit) to C:\OSGeo4W64
* see [win build script](scripts/ci/windows/build.bash)

## Linux

install dependencies for drivers

```
sudo apt-get install libgdal-dev libhdf5-dev libnetcdf-dev
```

and use cmake to generate build system

```
mkdir build;cd build
cmake -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
```

## MacOS

First you need to install homebrew and osgeo4mac dependencies, 
see [osx install script](scripts/ci/osx/install.bash)

To build, create build system with cmake and make sure you
use dependencies from homebrew and not system ones

```
mkdir build;cd build
export CMAKE_PREFIX_PATH=/usr/local/opt/gdal2
cmake -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
```

# Tests

run ctest command in build directory `ctest -VV`

# Contribution

format code:
```
cd scripts
./mdal_astyle.sh `find .. -name \*.h* -print -o -name \*.c* -print`
```

or use git pre-commit hook
```
cd MDAL
ln -s ./scripts/mdal_astyle.sh .git/hooks/pre-commit
```
