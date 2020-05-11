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
Some formats have separate file for mesh definion and data definition, some formats have multiple files per dataset, 
some formats use single for to store all the data in single file. All possibilities are supported. 

Driver structure
-----------------

Each driver consists of files in 

1. MDAL/docs/source/drivers/<driver_name>.rst where the driver documentation is preserved
2. MDAL/mdal/frmts/mdal_<driver_name>.cpp where the driver implementation is coded
3. MDAL/tests/test_<driver_name>.cpp where the test is implemented. Test coverage is aimed for more than 90% of the code base. Each driver should come with the driver test with size < 3MB that is stored under the same folder.

The driver can be implemented with lazy-loaded fashion, so the data is gathered only when requested (preferred) or in brute-force way, where everything is loaded in the memory. The latter options is easier to implement as is a usuall practice to implement the all-in-memory way by usage of classes `MDAL::MemoryMesh` and `MDAL::MemoryDataset` first. 





MDAL::Driver()
