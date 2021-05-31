.. _mdal_data_model:

================================================================================
MDAL Data Model
================================================================================

Data Model
-----------

In our context, a mesh is a collection of vertices, edges and faces in 2D or 3D space:

#. Vertices - XY(Z) points (in the layer’s coordinate reference system).
#. Edges - connect pairs of vertices.
#. Faces - sets of edges forming a closed shape - typically triangles or quadrilaterals (quads), rarely polygons with higher number of vertices.

Mesh-examples
-------------

Mesh gives us information about the spatial structure. In addition to the mesh we have datasets that assign a value to every vertex. For example, ice cap thickness at particular moment of time. A single file may contain multiple datasets - typically multiple quantities (e.g. water depth, water flow) that may be varying in time (time being represented in discrete timesteps, so each quantity may have N arrays, one for each timestep). Datasets do not have to vary in time (e.g. maximum water depth over the whole simulation).

Here is an example of a triangular mesh with numbered vertices:

We can visualize the data by assigning colors to values (similarly to how it is done with “Singleband pseudocolor” raster rendering) and interpolating data between vertices according to the mesh topology. It is common that some quantities are 2D vectors rather than being simple scalar values (e.g. wind direction). For such quantities it is very desired to display arrows indicating vector direction.