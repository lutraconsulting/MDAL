[![Build Status](https://travis-ci.org/lutraconsulting/MDAL.svg?branch=master)](https://travis-ci.org/lutraconsulting/MDAL)
[![CircleCI](https://circleci.com/gh/lutraconsulting/MDAL.svg?style=svg)](https://circleci.com/gh/lutraconsulting/MDAL)
[![Coverage Status](https://img.shields.io/coveralls/lutraconsulting/MDAL.svg)](https://coveralls.io/github/lutraconsulting/MDAL?branch=master)
<!-- [<img src="https://my.cdash.org/images/cdash.gif" alt="cdash" width="20"/>](https://my.cdash.org/index.php?project=MDAL) -->

# MDAL
Mesh Data Abstraction Library

- see [Unstructured Mesh Layers](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119)
- see [3D layered meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/158)
- see [1D meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/164)

## Supported Formats

You can use MDAL to load the following file formats:

- [2DM](https://www.xmswiki.com/wiki/SMS:2D_Mesh_Files_*.2dm): Mesh representation of various various hydrodynamic modelling packages (e.g. BASEMENT, TUFLOW)
- [NetCDF](https://en.wikipedia.org/wiki/NetCDF): Generic format for scientific data. Examples can be found [here](http://apps.ecmwf.int/datasets/data/interim-full-daily/levtype=sfc/)
- [GRIB](https://en.wikipedia.org/wiki/GRIB): Format commonly used in meteorology. Examples can be found [here](http://apps.ecmwf.int/datasets/data/interim-full-daily/levtype=sfc/)
- [XMDF*](https://en.wikipedia.org/wiki/XMDF): As an example, hydraulic outputs from TUFLOW modelling package
- [XDMF*](http://xdmf.org/index.php/Main_Page): As an example, hydraulic outputs from BASEMENT 3.x modelling package
- [DAT](http://www.xmswiki.com/wiki/SMS:ASCII_Dataset_Files_*.dat): Outputs of various hydrodynamic modelling packages (e.g. BASEMENT, HYDRO_AS-2D, TUFLOW)
- [3Di*](http://www.3di.nu): 3Di modelling package format based on [CF Conventions](http://cfconventions.org)
- [UGRID*](https://www.deltares.nl/en/): Unstructured Grid format based on [CF Conventions](http://cfconventions.org)
- [FLO-2D](http://www.flo-2d.com/): Outputs of the FLO-2D modelling package
- [Selafin/Serafin](https://www.gdal.org/drv_selafin.html): Outputs of the TELEMAC 2D hydrodynamic modelling package
- [HEC-RAS](http://www.hec.usace.army.mil/software/hec-ras/): Outputs of the HEC-RAS modelling package
- [SWW](http://anuga.anu.edu.au/): Outputs of the ANUGA modelling package
- [Esri TIN](https://en.wikipedia.org/wiki/Esri_TIN): Format for storing elevation data as a triangulated irregular network
- [SAGA FLOW**](https://gis.stackexchange.com/a/254942/59405): Rasters in the SAGA flow direction format
- [ADCIRC***](https://adcirc.org): ADCIRC hydrodynamic model results

\* Data lazy loaded

\*\* Formats can be preprocessed using QGIS [Crayfish](https://plugins.qgis.org/plugins/crayfish/)/Mesh processing algorithm to one of supported formats

\*\*\* Results should pre pre-processed to become [UGRID compliant](https://github.com/lutraconsulting/MDAL/issues/155#issuecomment-530853839)

# Versioning and integration in QGIS

QGIS contains internal copy of MDAL library in following versions:

| QGIS    | MDAL    |
| ------- | ------- |
| 3.0.2   | N/A     |
| 3.2.3   | 0.0.3   |
| 3.4.14  | 0.0.10  |
| 3.6.3   | 0.3.2   |
| 3.8.3   | 0.3.3   |
| 3.10.0  | 0.3.3   |
| 3.10.1  | 0.4.0   |
| 3.10.2  | 0.4.1   |
| 3.10.3  | 0.4.2   |
| 3.12.0  | 0.5.1   |
| 3.14.0  | 0.6.x   |

versions `X.Y.9Z` are development versions or alpha/beta releases (e.g. `0.4.90`, `0.4.91`, ...)

# Backporting 

We maintain the version of MDAL used in current QGIS LTR version. For any CRITICAL bugfixes (e.g. crashes, coredumps, regressions, data corruption) we want to backport the code to MDAL LTR version. To do this, label your pull request with label "backport release-XXX". Once pull request is merged, mdal-bot will automatically create a pull request to specified branch. Note that the pull request must be squashed to 1 commit for automatic backport.

# Development

## Coding standards & Contribution

MDAL is an open-source project and all contributions to either documentation, format support, testing or code are 
more than appreciated. Any change to the code must go through Pull Request, followed by review by one of the 
MDAL core developers. 

To be able to accept a pull request, please verify that:

- code follows [QGIS coding style](https://docs.qgis.org/testing/en/docs/developers_guide/codingstandards.html)
- code is properly unit-tested with a set of small test files under `tests/data/<format>` (code coverage > 90%)
- code is reasonably documented and easy to read
- code compiles without any compilation warnings
- no dead-code (e.g. unused functions) or commented out code
- all new code or new dependencies (e.g. libraries) have GPLv2 compatible license
- all tests pass

Please respect our [Contributor Covenant Code of Conduct](code_of_conduct.md)

## Build 

### Windows 

For 64-bit version:

* Install Microsoft Visual Studio 2017
* Install OSGeo4W (64bit) to C:\OSGeo4W64
* see [win build script](scripts/ci/windows/build.bash)

### Linux

install dependencies for drivers

```
sudo apt-get install libgdal-dev libhdf5-dev libnetcdf-dev libxml2-dev
```

and use cmake to generate build system

```
mkdir build;cd build
cmake -DCMAKE_BUILD_TYPE=Rel -DENABLE_TESTS=ON ..
make
```

### MacOS

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

## Tests

run ctest command in build directory `ctest -VV`

## Code syntax

format code:
```
cd scripts
./check_all.sh
```

or use git pre-commit hook
```
cd MDAL
ln -s ./scripts/mdal_astyle.sh .git/hooks/pre-commit
```

