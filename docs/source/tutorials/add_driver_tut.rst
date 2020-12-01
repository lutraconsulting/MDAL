.. _add_driver_tut:

================================================================================
MDAL driver implementation tutorial
================================================================================

.. highlight:: cpp

Overall Approach
----------------

In general new formats are added to MDAL by implementing format specific drivers. Drivers 
need to be derived class of `MDAL::Driver()` and be registered in `MDAL::DriverManager::DriverManager()`.
Drivers can have different types, they can read (or write) the mesh frame or mesh datasets (or both).
Some formats have separate file for mesh (frame) definition and data definition, some formats have multiple files per dataset,
some formats use single for to store all the data in single file. All possibilities are supported. 

Driver structure
-----------------

There are two types of drivers in MDAL

A) Regular drivers: they are compiled into the MDAL library at compile time
B) Dynamic drivers: they are compiled separately, without linkage to MDAL library.
   MDAL loads the on runtime from folder defined by environment variable `MDAL_DRIVER_PATH`

Regular drivers
---------------

Each regular MDAL driver consists of files in

1. MDAL/docs/source/drivers/<driver_name>.rst where the driver documentation is preserved
2. MDAL/mdal/frmts/mdal_<driver_name>.cpp where the driver implementation is coded
3. MDAL/tests/test_<driver_name>.cpp where the test is implemented.
   Test coverage is aimed for more than 90% of the code base.
   Each driver should come with the driver test with size < 3MB that is stored under the same folder.

The new driver needs to also be registered in `mdal_driver_manager.cpp` file.

The driver can be implemented with lazy-loaded fashion,
so the data is gathered only when requested (preferred) or in brute-force way,
where everything is loaded in the memory. The latter options is easier to
implement as is a usual practice to implement the all-in-memory way by usage of
classes `MDAL::MemoryMesh` and `MDAL::MemoryDataset` first.

Dynamic drivers
---------------

Implementation of the dynamic drivers is a lot more challenging than the regular drivers. To be able to
guarantee the binary stability, dynamic drivers needs to expose plain C API with set of exported functions.

The example implementation of the dynamic driver is in `MDAL/tools/externaldriver/`. Basically you need to
implement and export all functions defined in `MDAL/tools/externaldriver/mdal_external_driver.h`.
Please consult the functions description.

The dynamic driver should not link MDAL library function, so it is not dependent on the version of the library.
Also, with the new release of MDAL you do not need to recompile your drive to be able to use it.

On runtime, MDAL scans the folder defined by environment variable `MDAL_DRIVER_PATH`
and scans all libraries found in the folder. If any library contains the required `MDAL_DRIVER_*`
symbols, it is registered in runtime to mdal driver registry.

When the mesh is opened in MDAL, it tries `MDAL_DRIVER_canReadMesh()` function of the driver to
find out if the driver shall be used to open the file.

The reading follows by calling `MDAL_DRIVER_M_*` functions to get the information and structure of
the mesh frame.

Afterwards, the dataset groups are read by `MDAL_DRIVER_G_*` functions.

When client requests particular data, they are served through `MDAL_DRIVER_D_data`. Data could be lazy-loaded.
