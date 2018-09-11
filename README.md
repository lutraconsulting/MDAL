[![Build Status](https://travis-ci.org/lutraconsulting/MDAL.svg?branch=master)](https://travis-ci.org/lutraconsulting/MDAL)

# MDAL
Mesh Data Abstraction Library

see [Unstructured Mesh Layers](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119#issuecomment-380018557)

# Build 

## Windows 

TODO

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
see [osx install script](scripts/travis/osx/install.bash)

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
