/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#ifndef MDAL_LOGGER_H
#define MDAL_LOGGER_H

#include <string>

#include "mdal.h"

namespace MDAL
{
  namespace Log
  {
    void error( MDAL_Status status, std::string mssg );

    void warning( MDAL_Status status, std::string mssg );

    MDAL_Status getLastStatus();

    void resetLastStatus();

    void setLoggerCallback( MDAL_LoggerCallback callback );
  }
}

#endif // MDAL_LOGGER_H
