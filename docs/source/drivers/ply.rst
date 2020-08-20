.. _driver.ply:

================================================================================
PLY -- ASCII STAMFORD POLYGON FORMAT
================================================================================

.. shortname:: PLY

.. built_in_by_default::

MDAL supports reading of `PLY`_ format in ASCII format only. This format is used to store triangulated irregular network (TIN) in ASCII format with arbitrary scalar data on the vertices, faces and edges.

Note that:

- This driver does NOT support binary formats,
- This driver does NOT support user-defined properties that are lists,
- The `vertex-indices` property MUST be the first property defined on the face data.
- The driver implements one additional feature without breaking the standard. If it finds a line in the header starting "comment crs " it will use the rest of the line as the string to set the mesh projection.

The PLy format aallows data to be attached tp both edges and faces in the same data set and this driver will successfully load that data.
However, most host applications (like QGIS) will expect the dataset to be either a 1d mesh - with edges - or a 2D mwesh - with faces. There is no quarantee how the host application will process a dataset with both.

.. _PLY: https://en.wikipedia.org/wiki/PLY_(file_format)
