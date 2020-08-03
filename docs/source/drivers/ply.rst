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

.. _PLY: https://en.wikipedia.org/wiki/PLY_(file_format)
