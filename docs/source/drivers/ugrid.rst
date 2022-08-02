.. _driver.ugrid:

================================================================================
UGRID -- NetCDF with Climate and Forecast (CF) metadata
================================================================================

.. shortname:: UGRID

.. built_in_by_default::

MDAL supports reading of unstructured grid (mesh) data from NetCDF files based on UGRID_ format. The UGRID conventions are an extension to CF_ conventions to describe hybrid 1D-2D-3D meshes and unstructured mesh topology and geometry.
UGRID is used for example by the `Delft3D Flexible Mesh Suite <https://www.deltares.nl/en/software/delft3d-flexible-mesh-suite/>`_, `FVCom/VisIt <http://visitusers.org/index.php?title=Reading_NETCDF>`_, `ADCIRC <https://adcirc.org/>`_  and `BAW's unstructured grid tools <https://wiki.baw.de/en/index.php/NetCDF_unstructured_grid>`_. Additionally, MDAL supports detailed 1D network topologies and geometries.
MDAL supports writing mesh frame and saving dataset group located on vertices or faces for UGRID format. The dataset group added has to have the same time steps as the destination file. If the destination file does not exist, a new file is created containing the mesh frame definition and the presisted dataset group.

.. _UGRID: http://ugrid-conventions.github.io/ugrid-conventions/
.. _CF: http://cfconventions.org/

