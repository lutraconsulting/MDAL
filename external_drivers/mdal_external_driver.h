/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_EXTERNAL_DRIVER_H
#define MDAL_EXTERNAL_DRIVER_H

#define mdal_EXPORTS

#  if defined _WIN32 || defined __CYGWIN__
#    ifdef mdal_EXPORTS
#      ifdef __GNUC__
#        define MDAL_LIB_EXPORT __attribute__ ((dllexport))
#      else
#        define MDAL_LIB_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define MDAL_LIB_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define MDAL_LIB_EXPORT
#    endif
#  endif //_WIN32 || defined __CYGWIN__


#ifdef __cplusplus
extern "C" {
#endif

//! Returns the driver name
MDAL_LIB_EXPORT const char *MDAL_DRIVER_driverName();

//! Returns the driver long name
MDAL_LIB_EXPORT const char *MDAL_DRIVER_driverLongName();

//! Returns the driver suffix files filters
MDAL_LIB_EXPORT const char *MDAL_DRIVER_filters();

//! Returns the driver capability flags
MDAL_LIB_EXPORT int MDAL_DRIVER_capabilities();

//! Returns the driver capability flags
MDAL_LIB_EXPORT int MDAL_DRIVER_maxVertexPerFace();

//! Returns whether the driver can read the file
MDAL_LIB_EXPORT bool MDAL_DRIVER_canReadMesh( const char * );

//! Opens mesh and return the id of the mesh. This id is used to access to the mesh later.
MDAL_LIB_EXPORT int MDAL_DRIVER_openMesh( const char *uri, const char *name );

//! Closes the mesh
MDAL_LIB_EXPORT void MDAL_DRIVER_closeMesh( int meshId );

//! Returns the vertices count
MDAL_LIB_EXPORT int MDAL_DRIVER_M_vertexCount( int meshId );

//! Returns the faces count
MDAL_LIB_EXPORT int MDAL_DRIVER_M_faceCount( int meshId );

//! Returns the edges count
MDAL_LIB_EXPORT int MDAL_DRIVER_M_edgeCount( int meshId );

/**
 * Returns mesh extent in native projection
 */
MDAL_LIB_EXPORT void MDAL_DRIVER_M_extent( int meshId, double *xMin, double *xMax, double *yMin, double *yMax );

/**
 * Returns mesh projection
 */
MDAL_LIB_EXPORT const char *MDAL_DRIVER_M_projection( int meshId );

/**
 * Populates the \a buffer with vertices coordinates with \a count vertices from the \a startIndex.
 * \returns number of vertices effectivly written in the buffer
 */
MDAL_LIB_EXPORT int MDAL_DRIVER_M_vertices( int meshId, int startIndex, int count, double *buffer );

/**
 * Populates the \a vertexIndicesBufer with vertex indexes corresponding to face with \a count vertices from the \a startIndex.
 * Returns the effectivly count of faces returned.
 * \param meshId id of the requested mesh
 * \param faceCount requested faces count
 * \param faceOffsetsBuffer allocated array to store face offset in vertexIndicesBuffer for given face.
 *                          To find number of vertices of face i, calculate faceOffsetsBuffer[i] - faceOffsetsBuffer[i-1]
 * \param vertexIndicesBufferLen size of vertexIndicesBuffer, minimum is MDAL_M_faceVerticesMaximumCount()
 * \param vertexIndicesBuffer writes vertex indexes for faces
 *                            faceOffsetsBuffer[i-1] is index where the vertices for face i begins,
 * \returns number of faces effectivly written in the buffer
 */
MDAL_LIB_EXPORT int MDAL_DRIVER_M_faces( int meshId, int startFaceIndex, int faceCount, int *faceOffsetsBuffer, int vertexIndicesBufferLen, int *vertexIndicesBufer );

/**
 * Returns edges from iterator for the mesh
 * \param meshId id of the requested mesh
 * \param edgeCount maximum number or edges to be written to buffer
 * \param startVertexIndices must be allocated to edgesCount items to store start vertex indices for edges
 * \param endVertexIndices must be allocated to edgesCount items to store end vertex indices for edges
 * \returns number of vertices effectivly written in the buffer
 */
MDAL_LIB_EXPORT int MDAL_DRIVER_M_edges( int meshId, int startEdgeIndex, int edgeCount, int *startVertexIndices, int *endVertexIndices );

//! Returns the the dataset group count for the mesh with id \a meshId
MDAL_LIB_EXPORT int MDAL_DRIVER_M_datasetGroupCount( int meshId );

//! Returns the name of the group with index \a groupIndex of the mesh with id \a meshId
MDAL_LIB_EXPORT const char *MDAL_DRIVER_G_groupName( int meshId, int groupIndex );

//! Returns the reference time (ISO 8601) of the group with index \a groupIndex of the mesh with id \a meshId
MDAL_LIB_EXPORT const char *MDAL_DRIVER_G_referenceTime( int meshId, int groupIndex );

//! Returns the metadata count for the dataset group
MDAL_LIB_EXPORT int MDAL_DRIVER_G_metadataCount( int meshId, int groupIndex );

//! Returns the metadata key
MDAL_LIB_EXPORT const char *MDAL_DRIVER_G_metadataKey( int meshId, int groupIndex, int metaDataIndex );

//! Returns the metadata value
MDAL_LIB_EXPORT const char *MDAL_DRIVER_G_metadataValue( int meshId, int groupIndex, int metaDataIndex );

//! Returns dataset description, that is attributes that define the data
MDAL_LIB_EXPORT bool MDAL_DRIVER_G_datasetsDescription( int meshId, int groupIndex, bool *isScalar, int *dataLocation, int *datasetCount );

//! Returns the dataset time value
MDAL_LIB_EXPORT double MDAL_DRIVER_D_time( int meshId, int groupIndex, int datasetIndex, bool *ok );

//! Returns the dataset value, for value on vector, the buffer contains \a count pair of values : x0,y0,
MDAL_LIB_EXPORT int MDAL_DRIVER_D_data( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double *buffer );

//! Returns whether the dataset has active flag capability
MDAL_LIB_EXPORT bool MDAL_DRIVER_D_hasActiveFlagCapability( int meshId, int groupIndex, int datasetIndex );

//! Returns the dataset active flags
MDAL_LIB_EXPORT int MDAL_DRIVER_D_activeFlags( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer );

//! Returns maximum number of vertical levels (for 3D meshes)
MDAL_LIB_EXPORT int MDAL_DRIVER_D_maximumVerticalLevelCount( int meshId, int groupIndex, int datasetIndex );

//! Returns volumes count for the mesh (for 3D meshes)
MDAL_LIB_EXPORT int MDAL_DRIVER_D_volumeCount( int meshId, int groupIndex, int datasetIndex );

//! Returns the vertical levels count data (for 3D meshes)
MDAL_LIB_EXPORT int MDAL_DRIVER_D_verticalLevelCountData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer );

//! Returns the vertical levels data (for 3D meshes)
MDAL_LIB_EXPORT int MDAL_DRIVER_D_verticalLevelData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, double *buffer );

//! Returns the face to volume data (for 3D meshes)
MDAL_LIB_EXPORT int MDAL_DRIVER_D_faceToVolumeData( int meshId, int groupIndex, int datasetIndex, int indexStart, int count, int *buffer );

//! Unload data store in memory (for driver that support lazy loading, data are unloaded after statistic calculation)
MDAL_LIB_EXPORT void MDAL_DRIVER_D_unload( int meshId, int groupIndex, int datasetIndex );

#ifdef __cplusplus
}
#endif

#endif //MDAL_EXTERNAL_DRIVER_H
