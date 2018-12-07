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

#include "crayfish_dataset.h"
#include "crayfish_output.h"
#include "crayfish_mesh.h"

#include "frmts/crayfish_hdf5.h"
#include <algorithm>

static HdfFile openHdfFile(const QString& fileName)
{
    HdfFile file(fileName);
    if (!file.isValid())
    {
      throw LoadStatus::Err_UnknownFormat;
    }
    return file;
}

static HdfGroup openHdfGroup(const HdfFile& hdfFile, const QString& name)
{
    HdfGroup grp = hdfFile.group(name);
    if (!grp.isValid())
    {
      throw LoadStatus::Err_UnknownFormat;
    }
    return grp;
}

static HdfGroup openHdfGroup(const HdfGroup& hdfGroup, const QString& name)
{
    HdfGroup grp = hdfGroup.group(name);
    if (!grp.isValid())
    {
      throw LoadStatus::Err_UnknownFormat;
    }
    return grp;
}

static HdfDataset openHdfDataset(const HdfGroup& hdfGroup, const QString& name)
{
    HdfDataset dsFileType = hdfGroup.dataset(name);
    if (!dsFileType.isValid())
    {
      throw LoadStatus::Err_UnknownFormat;
    }
    return dsFileType;
}


static QString openHdfAttribute(const HdfFile& hdfFile, const QString& name)
{
    HdfAttribute attr = hdfFile.attribute(name);
    if (!attr.isValid())
    {
      throw LoadStatus::Err_UnknownFormat;
    }
    return attr.readString();
}

static ElementOutput* getOutput(DataSet* dataset, const int time)
{

    ElementOutput* eo = dataset->elemOutput(time);
    if (!eo)
    {
        throw LoadStatus::Err_InvalidData;
    }
    return eo;
}

static HdfGroup getBaseOutputGroup(const HdfFile& hdfFile) {
    HdfGroup gResults = openHdfGroup(hdfFile, "Results");
    HdfGroup gUnsteady = openHdfGroup(gResults, "Unsteady");
    HdfGroup gOutput = openHdfGroup(gUnsteady, "Output");
    HdfGroup gOBlocks = openHdfGroup(gOutput, "Output Blocks");
    HdfGroup gBaseO = openHdfGroup(gOBlocks, "Base Output");
    return gBaseO;
}

static HdfGroup get2DFlowAreasGroup(const HdfFile& hdfFile, const QString loc) {
    HdfGroup gBaseO = getBaseOutputGroup(hdfFile);
    HdfGroup gLoc = openHdfGroup(gBaseO, loc);
    HdfGroup g2DFlowRes = openHdfGroup(gLoc, "2D Flow Areas");
    return g2DFlowRes;
}

static QVector<float> readTimes(const HdfFile& hdfFile) {
    HdfGroup gBaseO = getBaseOutputGroup(hdfFile);
    HdfGroup gUnsteadTS = openHdfGroup(gBaseO, "Unsteady Time Series");
    HdfDataset dsTimes = openHdfDataset(gUnsteadTS, "Time");
    QVector<float> times = dsTimes.readArray();
    return times;
}

static QVector<int> readFace2Cells(const HdfFile& hdfFile, const QString& flowAreaName, int* nFaces)
{
    // First read face to node mapping
    HdfGroup gGeom = openHdfGroup(hdfFile, "Geometry");
    HdfGroup gGeom2DFlowAreas = openHdfGroup(gGeom, "2D Flow Areas");
    HdfGroup gArea = openHdfGroup(gGeom2DFlowAreas, flowAreaName);
    HdfDataset dsFace2Cells = openHdfDataset(gArea, "Faces Cell Indexes");

    QVector<hsize_t> fdims = dsFace2Cells.dims();
    QVector<int> face2Cells = dsFace2Cells.readArrayInt(); //2x nFaces

    *nFaces = fdims[0];
    return face2Cells;
}

static void readFaceOutput(Mesh* mesh, const QString fileName, const HdfFile& hdfFile, const HdfGroup& rootGroup,
                           const QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames,
                           const QString rawDatasetName, const QString datasetName, const QVector<float>& times) {

    int nElems = mesh->elements().size();
    double eps = std::numeric_limits<double>::min();

    DataSet* dsd = new DataSet(fileName);
    dsd->setName(datasetName, false);
    dsd->setType(DataSet::Scalar);
    dsd->setIsTimeVarying(times.size()>1);

    for (int tidx=0; tidx<times.size(); ++tidx)
    {
        ElementOutput* tos = new ElementOutput;
        tos->init(nElems, false);
        tos->time = times[tidx];
        std::fill(tos->getValues().begin(),tos->getValues().end(),-9999.0f);
        dsd->addOutput(tos);
    }

    for (int nArea=0; nArea < flowAreaNames.size(); ++nArea)
    {
        QString flowAreaName = flowAreaNames[nArea];

        int nFaces;
        QVector<int> face2Cells = readFace2Cells(hdfFile, flowAreaName, &nFaces);

        HdfGroup gFlowAreaRes = openHdfGroup(rootGroup, flowAreaName);
        HdfDataset dsVals = openHdfDataset(gFlowAreaRes, rawDatasetName);
        QVector<float> vals = dsVals.readArray();

        for (int tidx=0; tidx<times.size(); ++tidx)
        {
            ElementOutput* tos = getOutput(dsd, tidx);

            for (int i = 0; i < nFaces; ++i) {
                int idx = tidx*nFaces + i;
                float val = vals[idx]; // This is value on face!

                if (val == val && fabs(val) > eps) { //not nan and not 0
                    for (int c = 0; c < 2; ++c) {
                        int cell_idx = face2Cells[2*i + c] + areaElemStartIndex[nArea];
                        // Take just maximum
                        if (tos->getValues()[cell_idx] < val ) {
                            tos->getValues()[cell_idx] = val;
                        }
                    }
                }
            }
        }
    }
    dsd->updateZRange();
    mesh->addDataSet(dsd);
}

static void readFaceResults(Mesh* mesh, const QString fileName, const HdfFile& hdfFile, const QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames)
{
    // UNSTEADY
    HdfGroup flowGroup = get2DFlowAreasGroup(hdfFile, "Unsteady Time Series");
    QVector<float> times = readTimes(hdfFile);

    readFaceOutput(mesh, fileName, hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Shear Stress", "Face Shear Stress", times);
    readFaceOutput(mesh, fileName, hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Face Velocity", "Face Velocity", times);

    // SUMMARY
    flowGroup = get2DFlowAreasGroup(hdfFile, "Summary Output");
    times.clear();
    times.push_back(0.0f);

    readFaceOutput(mesh, fileName, hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Shear Stress", "Face Shear Stress/Maximums", times);
    readFaceOutput(mesh, fileName, hdfFile, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Face Velocity", "Face Velocity/Maximums", times);
}


static ElementOutput* readElemOutput(Mesh* mesh, const QString fileName, const HdfGroup& rootGroup,
                                     const QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames,
                                     const QString rawDatasetName, const QString datasetName,
                                     const QVector<float>& times,
                                     ElementOutput* bed_elevation = 0)
{
    bool is_bed_output = (bed_elevation == 0);
    int nElems = mesh->elements().size();
    double eps = std::numeric_limits<double>::min();

    DataSet* dsd = new DataSet(fileName);
    if (is_bed_output) {
        dsd->setType(DataSet::Bed);
    } else {
        dsd->setType(DataSet::Scalar);
    }
    dsd->setName(datasetName, false);

    dsd->setIsTimeVarying(times.size()>1);
    for (int tidx=0; tidx<times.size(); ++tidx)
    {
        ElementOutput* tos = new ElementOutput;
        tos->init(nElems, false);
        tos->time = times[tidx];
        dsd->addOutput(tos);
    }

    for (int nArea=0; nArea < flowAreaNames.size(); ++nArea)
    {
        int nAreaElements = areaElemStartIndex[nArea + 1] - areaElemStartIndex[nArea];
        QString flowAreaName = flowAreaNames[nArea];
        HdfGroup gFlowAreaRes = openHdfGroup(rootGroup, flowAreaName);

        HdfDataset dsVals = openHdfDataset(gFlowAreaRes, rawDatasetName);
        QVector<float> vals = dsVals.readArray();

        for (int tidx=0; tidx<times.size(); ++tidx)
        {
            ElementOutput* tos = getOutput(dsd, tidx);

            for (int i = 0; i < nAreaElements; ++i) {
              int idx = tidx*nAreaElements + i;
              int eInx = areaElemStartIndex[nArea] + i;
              float val = vals[idx];
              if (val != val) { //NaN
                tos->getValues()[eInx] = -9999;
              } else {
                if (is_bed_output) {
                    tos->getValues()[eInx] = val;
                } else
                {
                    if (datasetName == "Depth") {
                        if (fabs(val) < eps) {
                            tos->getValues()[eInx] = -9999; // 0 Depth is no-data
                        } else {
                            tos->getValues()[eInx] = val;
                        }
                    } else { //Water surface
                        float bed_elev = bed_elevation->getValues()[eInx];
                        if (fabs(bed_elev - val) < eps) {
                            tos->getValues()[eInx] = -9999; // no change from bed elevation
                        } else {
                            tos->getValues()[eInx] = val;
                        }
                    }
                }
              }
            }
        }
    }

    dsd->updateZRange();
    mesh->addDataSet(dsd);

    return getOutput(dsd, 0);
}

static ElementOutput* readBedElevation(Mesh* mesh, const QString fileName, const HdfGroup& gGeom2DFlowAreas, const QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames)
{
    QVector<float> times(1, 0.0f);
    return readElemOutput(mesh, fileName, gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames, "Cells Minimum Elevation", "Bed Elevation", times);
}

static void readElemResults(Mesh* mesh, const QString fileName, const HdfFile& hdfFile, ElementOutput* bed_elevation,
                            const QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames)
{
    // UNSTEADY
    HdfGroup flowGroup = get2DFlowAreasGroup(hdfFile, "Unsteady Time Series");
    QVector<float> times = readTimes(hdfFile);

    readElemOutput(mesh, fileName, flowGroup, areaElemStartIndex, flowAreaNames, "Water Surface", "Water Surface", times, bed_elevation);
    readElemOutput(mesh, fileName, flowGroup, areaElemStartIndex, flowAreaNames, "Depth", "Depth", times, bed_elevation);

    // SUMMARY
    flowGroup = get2DFlowAreasGroup(hdfFile, "Summary Output");
    times.clear();
    times.push_back(0.0f);

    readElemOutput(mesh, fileName, flowGroup, areaElemStartIndex, flowAreaNames, "Maximum Water Surface", "Water Surface/Maximums", times, bed_elevation);
}

QStringList read2DFlowAreasNames(HdfGroup gGeom2DFlowAreas) {
    HdfDataset dsNames = openHdfDataset(gGeom2DFlowAreas, "Names");
    QStringList names = dsNames.readArrayString();
    if (names.isEmpty()) {
        throw LoadStatus::Err_InvalidData;
    }
    return names;
}

static void setProjection(Mesh* mesh, HdfFile hdfFile) {
    try {
        QString proj_wkt = openHdfAttribute(hdfFile, "Projection");
        mesh->setSourceCrsFromWKT(proj_wkt);
    }
    catch (LoadStatus::Error) { /* projection not set */}
}

static Mesh* parseMesh(HdfGroup gGeom2DFlowAreas, QVector<int>& areaElemStartIndex, const QStringList& flowAreaNames)
{
    Mesh::Nodes nodes;
    Mesh::Elements elements;

    for (int nArea=0; nArea < flowAreaNames.size(); ++nArea)
    {
        QString flowAreaName = flowAreaNames[nArea];

        HdfGroup gArea = openHdfGroup(gGeom2DFlowAreas, flowAreaName);

        HdfDataset dsCoords = openHdfDataset(gArea, "FacePoints Coordinate");
        QVector<hsize_t> cdims = dsCoords.dims();
        QVector<double> coords = dsCoords.readArrayDouble(); //2xnNodes matrix in array
        int nNodes = cdims[0];
        int areaNodeStartIndex = nodes.size();
        nodes.resize(areaNodeStartIndex + nNodes);
        for (int n = 0; n < nNodes; ++n)
        {
            int nIdx = areaNodeStartIndex + n;
            nodes[nIdx].setId(nIdx);
            nodes[nIdx].x = coords[cdims[1]*n];
            nodes[nIdx].y = coords[cdims[1]*n+1];
        }

        HdfDataset dsElems = openHdfDataset(gArea, "Cells FacePoint Indexes");
        QVector<hsize_t> edims = dsElems.dims();
        int nElems = edims[0];
        int maxFaces = edims[1]; // elems have up to 8 faces, but sometimes the table has less than 8 columns
        QVector<int> elem_nodes = dsElems.readArrayInt(); //maxFacesxnElements matrix in array
        areaElemStartIndex[nArea] = elements.size();
        elements.resize(elements.size() + nElems);
        for (int e = 0; e < nElems; ++e)
        {
            int eIdx = areaElemStartIndex[nArea] + e;
            elements[eIdx].setId(eIdx);
            QVector<uint> idx(maxFaces);
            int nValidVertexes = maxFaces;
            for (int fi=0; fi<maxFaces; ++fi)
            {
                int elem_node_idx = elem_nodes[edims[1]*e + fi];

                if (elem_node_idx == -1) {
                    nValidVertexes = fi;
                    break;
                } else {
                    idx[fi] = areaNodeStartIndex + elem_node_idx; // shift by this area start node index
                }
            }

            if (nValidVertexes == 2) { // Line
                elements[eIdx].setEType(Element::E2L);
                elements[eIdx].setP(idx.data());
            } else if (nValidVertexes == 3) { // TRIANGLE
                elements[eIdx].setEType(Element::E3T);
                elements[eIdx].setP(idx.data());
            }
            else if (nValidVertexes == 4) { // RECTANGLE
                elements[eIdx].setEType(Element::E4Q);
                elements[eIdx].setP(idx.data());
            }
            else {
                elements[eIdx].setEType(Element::ENP, nValidVertexes);
                elements[eIdx].setP(idx.data());
            }
        }
    }
    areaElemStartIndex[flowAreaNames.size()] = elements.size();

    return new Mesh(nodes, elements);
}

Mesh* Crayfish::loadHec2D(const QString& fileName, LoadStatus* status)
{
    if (status) status->clear();
    Mesh* mesh = 0;

    try
    {
        HdfFile hdfFile = openHdfFile(fileName);

        // Verify it is correct file
        QString fileType = openHdfAttribute(hdfFile, "File Type");
        if (fileType != "HEC-RAS Results") {
            throw LoadStatus::Err_UnknownFormat;
        }

        HdfGroup gGeom = openHdfGroup(hdfFile, "Geometry");
        HdfGroup gGeom2DFlowAreas = openHdfGroup(gGeom, "2D Flow Areas");

        QStringList flowAreaNames = read2DFlowAreasNames(gGeom2DFlowAreas);
        QVector<int> areaElemStartIndex(flowAreaNames.size() + 1);

        mesh = parseMesh(gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames);
        setProjection(mesh, hdfFile);

        //Elevation
        ElementOutput* bed_elevation = readBedElevation(mesh, fileName, gGeom2DFlowAreas, areaElemStartIndex, flowAreaNames);

        // Element centered Values
        readElemResults(mesh, fileName, hdfFile, bed_elevation, areaElemStartIndex, flowAreaNames);

        // Face centered Values
        readFaceResults(mesh, fileName, hdfFile, areaElemStartIndex, flowAreaNames);
    }

    catch (LoadStatus::Error error)
    {
        if (status) status->mLastError = (error);
        if (mesh) delete mesh;
        mesh = 0;
    }

    return mesh;
}
