![Linux Tests](https://github.com/lutraconsulting/MDAL/workflows/Linux%20Tests/badge.svg?branch=master)
![MemCheck Tests](https://github.com/lutraconsulting/MDAL/workflows/MemCheck%20Tests/badge.svg)
![Coverage Tests](https://github.com/lutraconsulting/MDAL/workflows/Coverage%20Tests/badge.svg)
![OSX Tests](https://github.com/lutraconsulting/MDAL/workflows/OSX%20Tests/badge.svg)
[![WIN Tests](https://api.travis-ci.org/lutraconsulting/MDAL.svg?branch=master)](https://www.travis-ci.org/lutraconsulting/MDAL)

![Code Style](https://github.com/lutraconsulting/MDAL/workflows/Code%20Style/badge.svg)
[![Coverage Status](https://img.shields.io/coveralls/lutraconsulting/MDAL.svg)](https://coveralls.io/github/lutraconsulting/MDAL?branch=master)
[![Docs Build](https://dev.azure.com/lutraconsulting/MDAL/_apis/build/status/lutraconsulting.MDAL?branchName=master)](https://dev.azure.com/lutraconsulting/MDAL/_build/latest?definitionId=1&branchName=master)

# MDAL
Mesh Data Abstraction Library

- see [Unstructured Mesh Layers](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119)
- see [3D layered meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/158)
- see [1D meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/164)

## Supported Formats

You can use MDAL to load the following file formats:

- [2DM](https://www.xmswiki.com/wiki/SMS:2D_Mesh_Files_*.2dm): Mesh representation of various various hydrodynamic modelling packages (e.g. BASEMENT, TUFLOW)
- [XMS TIN](https://www.xmswiki.com/wiki/TIN_Files): Mesh TIN representation in ASCII format
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
- [PLY](https://en.wikipedia.org/wiki/PLY_(file_format)): Stamford Polygon Format also useful for mesh created from PointClouds by [PDAL](https://pdal.io)

\* Data lazy loaded

\*\* Formats can be preprocessed using QGIS [Crayfish](https://plugins.qgis.org/plugins/crayfish/) Mesh processing algorithm to one of supported formats

\*\*\* Results should pre pre-processed to become [UGRID compliant](https://github.com/lutraconsulting/MDAL/issues/155#issuecomment-530853839)

| Format  | Mesh Frame Support | Mesh Lazy Loaded | 1D Data Support | 2D Data Support | 3D Data Support | Data Lazy Loaded |
| ------- | ------- | ------- | ------- | ------- | ------- | ------- | 
| 2DM   |  READ-WRITE | N/A | N/A | N/A | N/A | N/A |
| XMS TIN   | READ-ONLY | N/A | N/A | N/A | N/A | N/A |
| Esri TIN   |  READ-ONLY | N/A | N/A | N/A | N/A | N/A |
| NetCDF   |  READ-ONLY | NO | NO | READ-ONLY | N/A | NO | 
| GRIB   |  READ-ONLY | NO | NO | READ-ONLY | N/A | NO | 
| XMDF   |  N/A | N/A | NO | READ-ONLY | READ-ONLY | YES |
| XDMF   |  N/A | N/A | NO | READ-ONLY | NO | YES |
| DAT   |  N/A | N/A | READ-ONLY | READ-WRITE | N/A | NO | 
| 3Di   |  READ-ONLY | NO |  READ-ONLY | READ-ONLY | NO | NO | 
| UGRID   |  READ-WRITE | NO | READ-ONLY | READ-ONLY | NO | NO | 
| FLO-2D   |  READ-ONLY  | NO | READ-ONLY | READ-WRITE | NO | NO  | 
| Selafin   |  READ-ONLY | NO | NO | READ-ONLY | NO | NO | 
| SWW   |  READ-ONLY | NO | NO | READ-ONLY | NO | NO | 
| PLY   | READ-ONLY | N/A | READ-ONLY | READ-ONLY | N/A | N/A |

# Versioning and integration in QGIS

QGIS contains internal copy of MDAL library in following versions:

| QGIS    | MDAL    | Features | 
| ------- | ------- | -------- |
| 3.0.2   | N/A     |  [2D meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/119) |
| 3.2.3   | 0.0.3   |          |
| 3.4.14  | 0.0.10  |          |
| 3.6.3   | 0.3.2   | Many new formats supported |
| 3.8.3   | 0.3.3   | |
| 3.10.0  | 0.3.3   | |
| 3.10.1  | 0.4.0   | Save datasets for some formats |
| 3.10.2  | 0.4.1   | |
| 3.10.3  | 0.4.2   | |
| 3.12.0  | 0.5.1   |  [3D layered meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/158) |
| 3.14.0  | 0.6.1   |  [1D meshes](https://github.com/qgis/QGIS-Enhancement-Proposals/issues/164) | 

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
./check_all.bash
```

or use git pre-commit hook
```
cd MDAL
ln -s ./scripts/mdal_astyle.bash .git/hooks/pre-commit
```

