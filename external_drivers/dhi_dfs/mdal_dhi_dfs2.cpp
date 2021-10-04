#include "mdal_dhi_dfs.hpp"
/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Vincent Cloarec (vcloarec at gmail dot com)
*/

#include "mdal_dhi_dfs2.hpp"

#include <cassert>

#define M_PI 3.14159265358979323846264338327

using namespace DHI;
using namespace Projections;

bool MeshDfs2::canRead( const std::string &uri )
{
  LPFILE      Fp;
  LPHEAD      pdfs;
  LPCTSTR fileName = uri.c_str();

  bool ok = false;
  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  if ( rc == F_NO_ERROR )
  {
    size_t countX;
    size_t countY;
    double originX;
    double originY;
    double sizeX;
    double sizeY;
    ok = fileInfo( pdfs, originX, originY, countX, countY, sizeX, sizeY );
  }

  dfsFileClose( pdfs, &Fp );
  dfsHeaderDestroy( &pdfs );

  return ok;
}

std::unique_ptr<MeshDfs2> MeshDfs2::loadMesh( const std::string &uri )
{
  LPFILE      Fp;
  LPHEAD      pdfs;
  LPCTSTR fileName = uri.c_str();

  ufsErrors rc = static_cast<ufsErrors>( dfsFileRead( fileName, &pdfs, &Fp ) );

  std::unique_ptr<MeshDfs2> mesh;
  bool ok = false;

  if ( rc == F_NO_ERROR )
  {
    size_t countX;
    size_t countY;
    double originX;
    double originY;
    double sizeX;
    double sizeY;
    LONG error;

    if ( fileInfo( pdfs, originX, originY, countX, countY, sizeX, sizeY ) )
    {
      mesh.reset( new MeshDfs2 );
      mesh->mFp = Fp;
      mesh->mPdfs = pdfs;

      GeoInfoType geoInfoType = dfsGetGeoInfoType( pdfs );
      LPCTSTR projectionWkt;
      if ( geoInfoType == F_UTM_PROJECTION )
      {
        double lon, lat, orientation;
        error = dfsGetGeoInfoUTMProj( pdfs, &projectionWkt, &lon, &lat, &orientation );

        if ( error == NO_ERROR )
        {
          mesh->mWktProjection = projectionWkt;
        }

        System::String ^stringProjection = gcnew System::String( projectionWkt );
        MapProjection ^mapProj = gcnew MapProjection( stringProjection, false );
        double N, E;
        mapProj->Geo2Proj( lon, lat, E, N );
        double projOri = mapProj->Geo2ProjRotation( lon, lat, orientation );

        // projOri: clockwise rotation of grid
        // I direction = X direction if projOri=0;
        // J direction = Y direction if projOri=0;
        double dIx = sizeX * cos( projOri * M_PI / 180 ); //offset of X coordinate for each element in I direction
        double dIy = -sizeX * sin( projOri * M_PI / 180 ); //offset of Y coordinate for each element in I direction
        double dJx = sizeY * sin( projOri * M_PI / 180 ); //offset of X coordinate for each element in J direction
        double dJy = sizeY * cos( projOri * M_PI / 180 ); //offset of Y coordinate for each element in J direction

        double vertexX0;
        double vertexY0;
        if ( projectionWkt == "NON-UTM" )
        {
          vertexX0 = E + originX;
          vertexY0 = N + originY;
        }
        else
        {
          vertexX0 = E + originX - ( dIx + dJx ) / 2;
          vertexY0 = N + originY - ( dIy + dJy ) / 2;
        }

        mesh->buildMesh( vertexX0, vertexY0, dIx, dIy, dJx, dJy, countX, countY );

        ok = mesh->populateDatasetGroups();
      }
    }
  }

  if ( ok )
    return mesh;
  else
  {
    dfsFileClose( pdfs, &Fp );
    return nullptr;
  }
}

void MeshDfs2::buildMesh( double vertexX0, double vertexY0, double dIx, double dIy, double dJx, double dJy, size_t countI, size_t countJ )
{
  // build vertices
  mVertexCoordinates = std::vector<double>( ( countI + 1 ) * ( countJ + 1 ) * 3, 0 );
  for ( size_t j = 0; j <= countJ; ++j )
    for ( size_t i = 0; i <= countI; ++i )
    {
      mVertexCoordinates[3 * ( i + j * ( countI + 1 ) )] = vertexX0 + dIx * i + dJx * j;
      mVertexCoordinates[3 * ( i + j * ( countI + 1 ) ) + 1] = vertexY0 + dIy * i + dJy * j;
    }

  std::vector<double> xBounding( 4, 0 );
  std::vector<double> yBounding( 4, 0 );

  xBounding[0] = mVertexCoordinates.at( 0 );
  yBounding[0] = mVertexCoordinates.at( 1 );
  xBounding[1] = mVertexCoordinates.at( 3 * countI );
  yBounding[1] = mVertexCoordinates.at( 3 * countI + 1 );
  xBounding[2] = mVertexCoordinates.at( 3 * ( countI + 1 ) * ( countJ ) );
  yBounding[2] = mVertexCoordinates.at( 3 * ( countI + 1 ) * ( countJ ) + 1 );
  xBounding[3] = mVertexCoordinates.at( mVertexCoordinates.size() - 3 );
  yBounding[3] = mVertexCoordinates.at( mVertexCoordinates.size() - 2 );

  for ( size_t i = 0; i < 4; ++i )
  {
    if ( mXmin > xBounding.at( i ) )
      mXmin = xBounding.at( i );
    if ( mYmin > yBounding.at( i ) )
      mYmin = yBounding.at( i );
    if ( mXmax < xBounding.at( i ) )
      mXmax = xBounding.at( i );
    if ( mYmax < yBounding.at( i ) )
      mYmax = yBounding.at( i );
  }

  // build faces
  mTotalElementCount = ( countI ) * ( countJ );
  mConnectivity = std::vector<int>( mTotalElementCount * 4, 0 );
  for ( size_t j = 0; j < countJ; ++j )
    for ( size_t i = 0; i < countI; ++i )
    {
      int p1 = int( i );
      int p2 = int( j * ( countI + 1 ) );
      int p3 = int( ( j + 1 ) * ( countI + 1 ) );
      mConnectivity[4 * ( i + j * countI )] = p1 + p2;
      mConnectivity[4 * ( i + j * countI ) + 1] = p1 + 1 + p2;
      mConnectivity[4 * ( i + j * countI ) + 2] = p1 + 1 + p3;
      mConnectivity[4 * ( i + j * countI ) + 3] = p1 + p3;
    }
}

bool MeshDfs2::fileInfo( LPHEAD pdfs, double &originX, double &originY, size_t &countX, size_t &countY, double &sizeX, double &sizeY )
{
  LONG dynamicItemCount = dfsGetNoOfItems( pdfs );

  if ( dynamicItemCount < 1 )
    return false;

  LPITEM firstItem = dfsItemD( pdfs, 1 );

  if ( !firstItem )
    return false;

  SpaceAxisType axisType = dfsGetItemAxisType( firstItem );

  if ( axisType != F_EQ_AXIS_D2 )
    return false;

  LONG eumUnit;
  LONG xGridPointsCount;
  LONG yGridPointsCount;
  float x0, y0, dx, dy;

  LONG error = dfsGetItemAxisEqD2( firstItem, &eumUnit, nullptr, &xGridPointsCount, &yGridPointsCount, &x0, &y0, &dx, &dy );
  if ( error != NO_ERROR )
    return false;

  for ( LONG itemNo = 2; itemNo <= dynamicItemCount; ++itemNo )
  {
    LPITEM item = dfsItemD( pdfs, itemNo );
    if ( !itemNo )
      return false;

    SpaceAxisType itemAxisType = dfsGetItemAxisType( firstItem );

    if ( axisType != itemAxisType )
      return false;

    LONG itemEumUnit;
    LONG itemXGridPointsCount;
    LONG itemYGridPointsCount;
    float itemX0, itemY0, itemDx, itemDy;

    error = dfsGetItemAxisEqD2( item, &itemEumUnit, nullptr, &itemXGridPointsCount, &itemYGridPointsCount, &itemX0, &itemY0, &itemDx, &itemDy );
    if ( error != NO_ERROR )
      return false;

    if ( itemEumUnit != eumUnit ||
         xGridPointsCount != itemXGridPointsCount || yGridPointsCount != itemYGridPointsCount ||
         itemX0 != x0 || itemY0 != y0 || itemDx != dx || itemDy != dy )
      return false;
  }

  originX = x0;
  originY = y0;
  sizeX = dx;
  sizeY = dy;
  countX = xGridPointsCount;
  countY = yGridPointsCount;

  return true;
}

int MeshDfs2::nodeCount( size_t ) const
{
  return 4;
}
