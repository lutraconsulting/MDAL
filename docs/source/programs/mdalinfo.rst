.. _mdalinfo:

================================================================================
mdalinfo
================================================================================

.. only:: html

    Lists information about a MDAL dataset.

.. Index:: mdalinfo

Synopsis
--------

.. code-block::

    mdalinfo [--help]

Description
-----------

:program:`mdalinfo` program lists various information about a MDAL supported files

The following command line parameters can appear in any order

.. program:: mdalinfo

.. option:: -todo
    todo

Example
-------

.. code-block::

    ./mdalinfo ./2dm/quad_and_triangle.2dm ./ascii_dat/quad_and_triangle_vertex_vector.dat
    mdalinfo 0.5.90
    Mesh File: ./2dm/quad_and_triangle.2dm
    Mesh loaded: OK
      Driver: 2DM
      Vertex count: 5
      Edge count: 0
      Face count: 2
      Edge count: 0
      Projection: undefined
    Dataset File: ./ascii_dat/quad_and_triangle_vertex_vector.dat
    Datasets loaded: OK
      Groups count: 2
      Bed Elevation
      VertexVectorDataset ( Vector )

