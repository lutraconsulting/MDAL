:orphan:

.. _unity_api:


================================================================================
MDAL Unity API
================================================================================

The MDAL Unity integration allows you to access and manipulation geospatial mesh data sets in C# amd
includes integration to access them as Unity objects (thought the Geometry3sharp to Unity integration)

Currently, this integration can:

#. read all MDAL compatible file formats,
#. access the metadata for the source,
#. access the vertex, face and edge data,
#. access 'double' datasets (both scalar and vector), and
#. convert the MDAL source mesh into a `Geometry3sharp`_ DMesh3 mesh object.

This version does not currently allow the MDAL source mesh to be written or ammended.

.. _Geometry3sharp: https://github.com/gradientspace/geometry3Sharp

Installation
------------

Available as a Unity Package Manager package from `OpenUPM`_ :

https://openupm.com/packages/com.virgis.mdal/

.. _OpenUPM: https://openupm.com/

Documentation
-------------

https://virgis-team.github.io/mdal-upm/html/annotated.html

Development
___________

Submit issues and PRs to :

https://github.com/ViRGIS-Team/mdal-upm

Example Usage
-------------

.. code-block:: C#

    using Mdal;
    using g3;

    List<DMesh3> features = new List<DMesh3>();

    // for MDAL files - load the mesh directly
    ds = Datasource.Load("...SourceFileName");

    for (int i = 0; i < ds.meshes.Length; i++) {
        DMesh3 mesh = ds.GetMesh(i);
        mesh.RemoveMetadata("properties");
        mesh.AttachMetadata("properties", new Dictionary<string, object>{
        { "Name", ds.meshes[i] }
    });
        // set the CRS based on what is known
        if (proj != null) {
            mesh.RemoveMetadata("CRS");
            mesh.AttachMetadata("CRS", proj);
        }
        if (layer.ContainsKey("Crs") && layer.Crs != null) {
            mesh.RemoveMetadata("CRS");
            mesh.AttachMetadata("CRS", layer.Crs);
        };
        features.Add(mesh);
    }