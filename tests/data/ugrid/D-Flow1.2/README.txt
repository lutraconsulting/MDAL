
File 1: bw_11_zonder_riviergrid_met_1dwtg_map.nc
* NetCDF 4 with classic format
* 2D model
* several data variables, to be identified by :mesh and :location attributes.
* all data variables contain standard_name and long_name. Use for GUI display.
* Scalar variable 'Mesh2D_s1' contains water levels.
  Interesting feature: it points to :ancillary_variables = 'Mesh2D_waterdepth',
  to enable masking the water level plot based on water *depths* (up to the user).
* Mesh2D_flowelem_bl is also a data variable, containing bed levels, but is
  time *independent*. Need to be visualized as well.
* Variable Mesh2D and the underlying variables contain all UGRID-info, including
  :start_index = 1.
* When plotting variables on faces (i.e., grid cells), try and fill the face
  polygons in two ways.
** The file contains 'Mesh2D_face_x_bnd', with the counterclockwise contour of each
   face. Variable (name) should be found via:
   data variable (e.g. .._s1,), --> that variable's :coordinates attribute -->
   in the x/y coordinate variable, find their :bounds attribute.
** alternatively, since the bounds is optional, also support deriving the face
   contour coordinates via:
   Mesh2d --> :face_node_connectivity for corner points --> :node_coordinates for
   x/y of corners.
** grid mapping RD New/Amersfoort is included.

File 2: bw_11_zonder_riviergrid_met_1dwtg_clm.nc
* NetCDF4 with HDF5 format
* Similar to file 1, but now data variables are in integer classes, instead of
  double precision values.
* meaning of the value classes via, e.g., Mesh2D_s1:flag_meanings + :flag_values

