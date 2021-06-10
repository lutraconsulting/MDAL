:orphan:

.. _python_api:

================================================================================
MDAL Python API
================================================================================

MDAL Python integration allows you to access and manipulation geospatial mesh data sets in Python.

Currently, this integration can:

#. read all MDAL compatible file formats,
#. access the metadata for the source,
#. access the vertex, face and edge data as numpy arrays,
#. access 'double' datasets (both scalar and vector) as numpy arrays, and
#. convert the MDAL source mesh into a `meshio`_ mesh object (with some restrictions currently).

This version does not currently allow the MDAL source mesh to be written or ammended.

.. _meshio: https://github.com/nschloe/meshio

Installation
------------

.. code-block::

    conda -c conda-forge install mdal-python

Documentation
-------------

https://virgis-team.github.io/mdal-python/html/index.html

Development
-----------

https://github.com/ViRGIS-Team/mdal-python

Contributions and PRs are welcome

Example Usage
-------------

.. code-block:: python

    from mdal import Datasource, Info, last_status

    print(f"MDAL Version:  {Info.version}")
    print(f"MDAL Driver Count :{Info.driver_count}")
    print(last_status())

    for driver in Info.drivers:
        print(driver)


    ds = Datasource("data/ply/test_mesh.ply")
    print(ds.meshes)

    with ds.load(0) as mesh:
        print(f"Driver : {mesh.driver_name}")
        print(f"Vertex Count : {mesh.vertex_count}")
        print(f"Face Count : {mesh.face_count}")
        print(f"Largest Face: {mesh.largest_face}")
        print(f"Edge Count : {mesh.edge_count}")
        print(f"CRS : {mesh.projection}")
        print(f"Mesh extent : {mesh.extent}")

        vertex = mesh.vertices
        print(f"Vertex Array Shape : {vertex.shape}")

        faces = mesh.faces
        print(f"Face Array Shape : {faces.shape}")

        edges = mesh.edges
        print(f"Edges Array Shape : {edges.shape}")

        print("")

        group = mesh.group(0)
        print(f"DatasetGroup Name : {group.name}")
        print(f"DatasetGroup Location : {group.location}")
        print(f"Dataset Count : {group.dataset_count}")
        print(f"Group has scalar values : {group.has_scalar}")
        print(f"Group has temporal values : {group.is_temporal}")
        print(f"Reference Time : {group.reference_time}")
        print(f"Maximum Vertical Level Count : {group.level_count}")
        print(f"Minimum / Maximum ; {group.minmax}")
        print(f"Metadata : {group.metadata}")

        print("")
        for i in range(0, group.dataset_count):
            data = group.data_as_double(i)
            time = group.dataset_time(i)
            print(f"Dataset Shape for time {time} : {data.shape}")

        print("")

        meshio = mesh.meshio()

    print(meshio)
    print(mesh)
    print(mesh.vertex_count)
