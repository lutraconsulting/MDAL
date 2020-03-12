.. _mdal_data_model:

================================================================================
MDAL Data Model
================================================================================

This document attempts to describe the MDAL data model. That is the types of information that a MDAL data store can contain, and their semantics.

n our context, a mesh is a collection of vertices, edges and faces in 2D or 3D space:

vertices - XY(Z) points (in the layer’s coordinate reference system)
edges - connect pairs of vertices
faces - sets of edges forming a closed shape - typically triangles or quadrilaterals (quads), rarely polygons with higher number of vertices
mesh-examples

Mesh gives us information about the spatial structure. In addition to the mesh we have datasets that assign a value to every vertex. For example, ice cap thickness at particular moment of time. A single file may contain multiple datasets - typically multiple quantities (e.g. water depth, water flow) that may be varying in time (time being represented in discrete timesteps, so each quantity may have N arrays, one for each timestep). Datasets do not have to vary in time (e.g. maximum water depth over the whole simulation).

Here is an example of a triangular mesh with numbered vertices:

mesh-vertex-numbers

The following table gives an idea what information is stored in datasets. Table columns represent indices of mesh vertices, each row represents one dataset. The first two are scalar datasets, the latter two are datasets with 2D vectors.

 	1	2	3	…	13
Water depth at time=0s	5	5	5	…	2
Water depth at time=60s	6	5	3	…	4
Water flow at time=0s	[1,2]	[2,2]	[3,2]	…	[1,2]
Water flow at time=60s	[3,2]	[3,2]	[2,2]	…	[4,2]
In some cases datasets assign values to faces or edges instead of assigning values to vertices.

We can visualize the data by assigning colors to values (similarly to how it is done with “Singleband pseudocolor” raster rendering) and interpolating data between vertices according to the mesh topology. It is common that some quantities are 2D vectors rather than being simple scalar values (e.g. wind direction). For such quantities it is very desired to display arrows indicating vector direction.