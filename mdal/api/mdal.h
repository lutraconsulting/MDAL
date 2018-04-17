/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2018 Peter Petrik (zilolv at gmail dot com)
*/

#ifndef MDAL_H
#define MDAL_H

#ifdef MDAL_STATIC
#  define MDAL_EXPORT
#else
#  if defined _WIN32 || defined __CYGWIN__
#    ifdef mdal_EXPORTS
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllexport))
#      else
#        define MDAL_EXPORT __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    else
#      ifdef __GNUC__
#        define MDAL_EXPORT __attribute__ ((dllimport))
#      else
#        define MDAL_EXPORT __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
#      endif
#    endif
#  else
#    if __GNUC__ >= 4
#      define MDAL_EXPORT __attribute__ ((visibility ("default")))
#    else
#      define MDAL_EXPORT
#    endif
#  endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Statuses */
enum Status
{
  None,
  // Errors
  Err_NotEnoughMemory,
  Err_FileNotFound,
  Err_UnknownFormat,
  Err_IncompatibleMesh,
  Err_InvalidData,
  Err_MissingDriver,
  // Warnings
  Warn_UnsupportedElement,
  Warn_InvalidElements,
  Warn_ElementWithInvalidNode,
  Warn_ElementNotUnique,
  Warn_NodeNotUnique
};

/* Mesh */
typedef void *MeshH;

MDAL_EXPORT const char *MDAL_Version();

MDAL_EXPORT Status MDAL_LastStatus();

MDAL_EXPORT MeshH MDAL_LoadMesh( const char *meshFile );
MDAL_EXPORT void MDAL_CloseMesh( MeshH mesh );

MDAL_EXPORT int MDAL_M_vertexCount( MeshH mesh );
MDAL_EXPORT double MDAL_M_vertexXCoordinatesAt( MeshH mesh, int index );
MDAL_EXPORT double MDAL_M_vertexYCoordinatesAt( MeshH mesh, int index );

MDAL_EXPORT int MDAL_M_faceCount( MeshH mesh );
MDAL_EXPORT int MDAL_M_faceVerticesCountAt( MeshH mesh, int index );
MDAL_EXPORT int MDAL_M_faceVerticesIndexAt( MeshH mesh, int face_index, int vertex_index );

#ifdef __cplusplus
}
#endif

#endif //MDAL_H
