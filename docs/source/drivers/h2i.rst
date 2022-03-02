.. _driver.h2i:

================================================================================
H2I -- H2i format
================================================================================

.. shortname:: H2I

.. built_in_by_default::

MDAL supports reading of the H2i mesh format.

H2i mesh structure is a quad tree structure defined by nodes a the center of quad and by links that, at each side of quads, represents the interface between quad.
As quads can be cut by ridge edges, the structure can't be consideded as a real quad tree structure, some faces could not be quads and have more than 4 vertices.
The geometries of face are defined in a GPKG file and MDAL use only this file to build the mesh frame.

Files of H2i format are:
* a json file with some information about the mesh (mesh name, crs, reference time and time steps) and relative path of the other files defining the mesh and its dataset groups
* text file containing the nodes information
* text file containing the links information
* text file containing the time step
* one binary file per dataset group, dataset group can be relied to nodes or links.

.. _H2I: https://github.com/d2hydro/H2i_code_factory/blob/main/docs/data_format.md

