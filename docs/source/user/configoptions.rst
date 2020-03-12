.. configoptions:

================================================================================
Configuration options
================================================================================

This page discussed runtime configuration options for MDAL, and is distinct from
options to the build-time configure script. Runtime configuration options apply
on all platforms, and are evaluated at runtime. They can be set programmatically,
by commandline switches or in the environment by the user.

Configuration options are normally used to alter the default behavior of MDAL
drivers and in some cases the MDAL core. They are essentially global
variables the user can set.

How to set configuration options ?
----------------------------------

TODO

List of configuration options and where they apply
--------------------------------------------------

.. note::
    This list is known to be incomplete. It depends on proper annotation of configuration
    options where they are mentionned elsewhere in the documentation.
    If you want to help to extend it, use the ``:decl_configoption:`NAME```
    syntax in places where a configuration option is mentionned.


.. include:: configoptions_index_generated.rst
