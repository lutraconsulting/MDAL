/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2019 Vincent Cloarec (vcloarec at gmail dot com)
*/

#ifndef MDAL_ESRI_TIN_H
#define MDAL_ESRI_TIN_H

#include <string>
#include <vector>
#include <list>
#include <memory>
#include <iosfwd>
#include <iostream>
#include <fstream>

#include "mdal_data_model.hpp"
#include "mdal.h"
#include "mdal_driver.hpp"
#include "mdal_utils.hpp"

namespace MDAL
{

  /** *********************************************
   *                   Structure of files
   * https://en.wikipedia.org/wiki/Esri_TIN
   ************************************************
   * thul.adf
   *
   * file containing only int (big-endianness)
   * first, list of superpoint indexes (infinity point to North, South East and West)
   * used by ArcGIS but unused here
   * This list finished with value -1
   * Then indexes of the outer boundary indexes or inner boundary indexes (holes)
   * Those list are separated with value 0
   *
   * ***************************************
   * tnxy.adf
   *
   * vertices position X,Y : for each value 8 bytes : double with big-endianness
   *
   * ***************************************
   * tnz.adf : z value of each vertices
   *
   * vertices Z value: for each value 4 bytes : float with big endianness
   *
   * ***************************************
   * tnod.adf : faces (triangles) of the TIN
   *
   * indexes of the 3 vertices for each faces (int32 big-endianness)
   *
   * ***************************************
   * tmsk.adf and tmsx.adf
   *
   * files used to store information about masked faces. Those faces are not displayed
   *
   *****************************************
   * prj.adf
   *
   * text file containing the WKT CRS
   *
  */


  class DriverEsriTin: public Driver
  {
    public:
      DriverEsriTin();
      ~DriverEsriTin() override {}

      Driver *create() override;

      virtual std::unique_ptr< Mesh > load( const std::string &uri, const std::string &meshName = "" ) override;

      bool canReadMesh( const std::string &uri ) override;

    private:
      std::string xyFile( const std::string &uri ) const;
      std::string zFile( const std::string &uri ) const;
      std::string faceFile( const std::string &uri ) const;
      std::string mskFile( const std::string &uri ) const;
      std::string msxFile( const std::string &uri ) const;
      std::string hullFile( const std::string &uri ) const;
      std::string denvFile( const std::string &uri ) const;
      std::string denv9File( const std::string &uri ) const;
      std::string crsFile( const std::string &uri ) const;
      std::string getCrsWkt( const std::string &uri ) const;
  };
}

#endif // MDAL_ESRI_TIN_H
