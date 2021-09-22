.. _driver.ply:

================================================================================
PLY -- ASCII Stanford Polygon Format
================================================================================

.. shortname:: PLY

.. built_in_by_default::

MDAL supports reading and writing of `PLY`_ format in ASCII and BINARY formats. This format is used to store triangulated irregular networks (TINs) in ASCII format with arbitrary scalar data on the vertices, faces and/or edges.

Note that:

- This driver can read and write Meshes containing any (consistent) combination of vertices, faces and edges and can access Scalar and Vector datasets on vertices, faces & edges and Scalar (only) datasets on volumes.
- All MDAL Scalar datasets are stored in PLY as doubles,
- Vector Datasets are stored as PLY double lists. When Reading a PLY file, a list data type is converted into an MDAL Vector dataset with only the first two values of the list used,
- Datasets on Volumes are stored as TWO equal sized lists on the face elements (one containing the values using the name of the MDAL DatasetGroup and one containing the extrusion values which has "__vol" appended to the name). This is a format that is totally consistent with the PLY format but is unique to this driver. 
- This driver implements an additional feature without breaking the standard. If it finds a line in the header starting "comment crs " it will use the rest of the line as the string to set the mesh projection.

The PLY format allows data to be attached both to edges and to faces in the same data set and this driver will successfully load that data.
However, most host applications (like QGIS) will expect the dataset to be either a 1d mesh - with edges - or a 2D mwesh - with faces. There is no guarantee how the host application will process a dataset with both.

.. _PLY: https://en.wikipedia.org/wiki/PLY_(file_format)
