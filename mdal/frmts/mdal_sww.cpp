/*
Crayfish - A collection of tools for TUFLOW and other hydraulic modelling packages
Copyright (C) 2016 Lutra Consulting

info at lutraconsulting dot co dot uk
Lutra Consulting
23 Chestnut Close
Burgess Hill
West Sussex
RH15 8HN

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "crayfish.h"
#include "crayfish_mesh.h"
#include "crayfish_dataset.h"
#include "crayfish_output.h"

#include <netcdf.h>

// threshold for determining whether an element is active (wet)
// the format does not explicitly store that information so we
// determine that when loading data
#define DEPTH_THRESHOLD   0.0001   // in meters


Mesh* Crayfish::loadSWW(const QString& fileName, LoadStatus* status)
{
  if (status) status->clear();

  int ncid;
  int res;

  res = nc_open(fileName.toUtf8().constData(), NC_NOWRITE, &ncid);
  if (res != NC_NOERR)
  {
    qDebug("error: %s", nc_strerror(res));
    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  // get dimensions
  int nVolumesId, nVerticesId, nPointsId, nTimestepsId;
  size_t nVolumes, nVertices, nPoints, nTimesteps;
  if (nc_inq_dimid(ncid, "number_of_volumes", &nVolumesId) != NC_NOERR ||
      nc_inq_dimid(ncid, "number_of_vertices", &nVerticesId) != NC_NOERR ||
      nc_inq_dimid(ncid, "number_of_points", &nPointsId) != NC_NOERR ||
      nc_inq_dimid(ncid, "number_of_timesteps", &nTimestepsId) != NC_NOERR)
  {
    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }
  if (nc_inq_dimlen(ncid, nVolumesId, &nVolumes) != NC_NOERR ||
      nc_inq_dimlen(ncid, nVerticesId, &nVertices) != NC_NOERR ||
      nc_inq_dimlen(ncid, nPointsId, &nPoints) != NC_NOERR ||
      nc_inq_dimlen(ncid, nTimestepsId, &nTimesteps) != NC_NOERR)
  {
    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  if (nVertices != 3)
  {
    qDebug("Expecting triangular elements!");
    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  int xid, yid, zid, volumesid, timeid, stageid;
  if (nc_inq_varid(ncid, "x", &xid) != NC_NOERR ||
      nc_inq_varid(ncid, "y", &yid) != NC_NOERR ||
      nc_inq_varid(ncid, "volumes", &volumesid) != NC_NOERR ||
      nc_inq_varid(ncid, "time", &timeid) != NC_NOERR ||
      nc_inq_varid(ncid, "stage", &stageid) != NC_NOERR)
  {
    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  // load mesh data
  QVector<float> px(nPoints), py(nPoints), pz(nPoints);
  unsigned int* pvolumes = new unsigned int[nVertices * nVolumes];
  if (nc_get_var_float (ncid, xid, px.data()) != NC_NOERR ||
      nc_get_var_float (ncid, yid, py.data()) != NC_NOERR ||
      nc_get_var_int (ncid, volumesid, (int *) pvolumes) != NC_NOERR)
  {
    delete [] pvolumes;

    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  // we may need to apply a shift to the X,Y coordinates
  float xLLcorner = 0, yLLcorner = 0;
  nc_get_att_float(ncid, NC_GLOBAL, "xllcorner", &xLLcorner);
  nc_get_att_float(ncid, NC_GLOBAL, "yllcorner", &yLLcorner);

  Mesh::Nodes nodes(nPoints);
  Node* nodesPtr = nodes.data();
  for (size_t i = 0; i < nPoints; ++i, ++nodesPtr)
  {
    nodesPtr->setId(i);
    nodesPtr->x = px[i] + xLLcorner;
    nodesPtr->y = py[i] + yLLcorner;
  }

  QVector<float> times(nTimesteps);
  nc_get_var_float(ncid, timeid, times.data());

  int zDims = 0;
  if ( nc_inq_varid(ncid, "z", &zid) == NC_NOERR &&
       nc_get_var_float (ncid, zid, pz.data()) == NC_NOERR )
  {
    // older SWW format: elevation is constant over time

    zDims = 1;
  }
  else if ( nc_inq_varid(ncid, "elevation", &zid) == NC_NOERR &&
            nc_inq_varndims(ncid, zid, &zDims) == NC_NOERR &&
            ((zDims == 1 && nc_get_var_float (ncid, zid, pz.data()) == NC_NOERR) || zDims == 2) )
  {
    // we're good
  }
  else
  {
    // neither "z" nor "elevation" are present -> something is going wrong

    delete [] pvolumes;

    nc_close(ncid);
    if (status) status->mLastError = LoadStatus::Err_UnknownFormat;
    return 0;
  }

  // read bed elevations
  QList<NodeOutput*> elevationOutputs;
  if (zDims == 1)
  {
    // either "z" or "elevation" with 1 dimension
    NodeOutput* o = new NodeOutput;
    o->init(nPoints, nVolumes, false);
    o->time = 0.0;
    memset(o->getActive().data(), 1, nVolumes); // All cells active
    for (size_t i = 0; i < nPoints; ++i)
      o->getValues()[i] = pz[i];
    elevationOutputs << o;
  }
  else if (zDims == 2)
  {
    // newer SWW format: elevation may change over time
    for (size_t t = 0; t < nTimesteps; ++t)
    {
      NodeOutput* toe = new NodeOutput;
      toe->init(nPoints, nVolumes, false);
      toe->time = times[t] / 3600.;
      memset(toe->getActive().data(), 1, nVolumes); // All cells active
      float* elev = toe->getValues().data();

      // fetching "elevation" data for one timestep
      size_t start[2], count[2];
      const ptrdiff_t stride[2] = {1,1};
      start[0] = t;
      start[1] = 0;
      count[0] = 1;
      count[1] = nPoints;
      nc_get_vars_float(ncid, zid, start, count, stride, elev);

      elevationOutputs << toe;
    }
  }

  Mesh::Elements elements(nVolumes);
  Element* elementsPtr = elements.data();

  for (size_t i = 0; i < nVolumes; ++i, ++elementsPtr)
  {
    elementsPtr->setId(i);
    elementsPtr->setEType(Element::E3T);
    elementsPtr->setP(&pvolumes[3*i]);
  }

  delete [] pvolumes;

  Mesh* mesh = new Mesh(nodes, elements);

  // Create a dataset for the bed elevation
  DataSet* bedDs = new DataSet(fileName);
  bedDs->setType(DataSet::Bed);
  bedDs->setName("Bed Elevation");
  if (elevationOutputs.count() == 1)
  {
    bedDs->setIsTimeVarying(false);
    bedDs->addOutput(elevationOutputs.at(0));  // takes ownership of the Output
  }
  else
  {
    bedDs->setIsTimeVarying(true);
    foreach (NodeOutput* o, elevationOutputs)
      bedDs->addOutput(o);
  }
  bedDs->updateZRange();
  mesh->addDataSet(bedDs);

  // load results

  DataSet* dss = new DataSet(fileName);
  dss->setType(DataSet::Scalar);
  dss->setName("Stage");
  dss->setIsTimeVarying(true);

  DataSet* dsd = new DataSet(fileName);
  dsd->setType(DataSet::Scalar);
  dsd->setName("Depth");
  dsd->setIsTimeVarying(true);

  for (size_t t = 0; t < nTimesteps; ++t)
  {
    const NodeOutput* elevO = bedDs->isTimeVarying() ? bedDs->nodeOutput(t) : bedDs->nodeOutput(0);
    const float* elev = elevO->loadedValues().constData();

    NodeOutput* tos = new NodeOutput;
    tos->init(nPoints, nVolumes, false);
    tos->time = times[t] / 3600.;
    float* values = tos->getValues().data();

    // fetching "stage" data for one timestep
    size_t start[2], count[2];
    const ptrdiff_t stride[2] = {1,1};
    start[0] = t;
    start[1] = 0;
    count[0] = 1;
    count[1] = nPoints;
    nc_get_vars_float(ncid, stageid, start, count, stride, values);

    // derived data: depth = stage - elevation
    NodeOutput* tod = new NodeOutput;
    tod->init(nPoints, nVolumes, false);
    tod->time = tos->time;
    float* depths = tod->getValues().data();
    for (size_t j = 0; j < nPoints; ++j)
      depths[j] = values[j] - elev[j];

    // determine which elements are active (wet)
    for (size_t elemidx = 0; elemidx < nVolumes; ++elemidx)
    {
      const Element& elem = mesh->elements()[elemidx];
      float v0 = depths[elem.p(0)];
      float v1 = depths[elem.p(1)];
      float v2 = depths[elem.p(2)];
      tos->getActive()[elemidx] = v0 > DEPTH_THRESHOLD && v1 > DEPTH_THRESHOLD && v2 > DEPTH_THRESHOLD;
    }
    tod->getActive() = tos->getActive();

    dss->addOutput(tos);
    dsd->addOutput(tod);
  }

  dss->updateZRange();
  mesh->addDataSet(dss);
  dsd->updateZRange();
  mesh->addDataSet(dsd);

  int momentumxid, momentumyid;
  if (nc_inq_varid(ncid, "xmomentum", &momentumxid) == NC_NOERR &&
      nc_inq_varid(ncid, "ymomentum", &momentumyid) == NC_NOERR)
  {
    DataSet* mds = new DataSet(fileName);
    mds->setType(DataSet::Vector);
    mds->setName("Momentum");
    mds->setIsTimeVarying(true);

    QVector<float> valuesX(nPoints), valuesY(nPoints);
    for (size_t t = 0; t < nTimesteps; ++t)
    {
      NodeOutput* mto = new NodeOutput;
      mto->init(nPoints, nVolumes, true);
      mto->time = times[t] / 3600.;
      mto->getActive() = dsd->nodeOutput(t)->getActive();

      // fetching "stage" data for one timestep
      size_t start[2], count[2];
      const ptrdiff_t stride[2] = {1,1};
      start[0] = t;
      start[1] = 0;
      count[0] = 1;
      count[1] = nPoints;
      nc_get_vars_float(ncid, momentumxid, start, count, stride, valuesX.data());
      nc_get_vars_float(ncid, momentumyid, start, count, stride, valuesY.data());

      NodeOutput::float2D* mtoValuesV = mto->getValuesV().data();
      float* mtoValues = mto->getValues().data();
      for (size_t i = 0; i < nPoints; ++i)
      {
        mtoValuesV[i].x = valuesX[i];
        mtoValuesV[i].y = valuesY[i];
        mtoValues[i] = mtoValuesV[i].length();
      }

      mds->addOutput(mto);
    }


    mds->updateZRange();
    mesh->addDataSet(mds);
  }

  nc_close(ncid);

  return mesh;
}
