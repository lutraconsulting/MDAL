.. _download:

================================================================================
Download
================================================================================

.. only:: html

    .. contents::
       :depth: 3
       :backlinks: none

Current Releases
------------------------------------------------------------------------------

https://github.com/lutraconsulting/MDAL/releases


Past Releases
------------------------------------------------------------------------------

https://github.com/lutraconsulting/MDAL/releases


Conda Installation
------------------------------------------------------------------------------

MDAL can be installed as a stand-alone package (i.e. outside of QGIS) using `conda <https://anaconda.org/conda-forge/mdal>`__.

The package can installed by running :

::

    conda install -c conda-forge mdal


This package provides the MDAL ABI through the mdal shared object( i.e. mdal.dll, libmdal.dylib or libmdal.so) and the mdalinfo CLI.

.. note:: A friendly note about versions. The conda package is usually targetted at the latest version of GDAL on conda-forge. This is usually a later version than used by QGIS. Therefore, there may be some subtle differences in behaviour when loading e.g. GRIB files.

UPM Installation
++++++++++++++++

There is also a community supported Unity Package Manager (UPM) package for MDAL to allow MDAL to be used in Unity based projects. This builds on top of the Conda package.

https://openupm.com/packages/com.virgis.mdal/


Development Source
------------------------------------------------------------------------------

The main repository for MDAL is located on github at
https://github.com/lutraconsulting/MDAL.

You can obtain a copy of the active source code by issuing the following
command

::

    git clone https://github.com/lutraconsulting/MDAL.git


