# test data "c02_classmap_ucmagdir"

*File:*           simplebox_clm.nc
*Source:*         D-Flow Flexible Mesh
*References:*     https://oss.deltares.nl/web/delft3dfm
*Original input:* https://repos.deltares.nl/repos/DSCTestbench/trunk/cases/e02_dflowfm/f012_inout/c094_class_map_both/
*Author:*         arthur.vandam@deltares.nl

## Description:
*Domain:* 2D Cartesian grid, 100x40m extent. 10x4 square grid cells.
Coordinate reference system is not relevant here. 

*Data:*
* "mesh2d_s1": Water level in cell centres, as classes.
* "mesh2d_waterdepth": Water depth in cell centres, as classes.

*Time:*   24 snapshots in time.

NOTE: replaced on 6th April 2020, now it has new “:flag_bounds” attribute that specifies the connection 
