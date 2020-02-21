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
  /** Namespace including functions responsible for handling logs.
   * Use in code as: MDAL::Log::error/warning( MDAL_Status, logMessage ).
   * By default, output from logger is not being shown anywhere, but it is
   * possible to set custom logger output with function setLoggerCallback.
   * If environment variable MDAL_DEBUG is set, logger sends logs to standard stdout. */
  namespace Log
  {
    /** Logger function handling error logs */
    void error( MDAL_Status status, std::string mssg );

    /** Logger function handling warning logs */
    void warning( MDAL_Status status, std::string mssg );

    /** Function to get last set MDAL_Status */
    MDAL_Status getLastStatus();

    /** Function to reset MDAL_Status variable */
    void resetLastStatus();

    //! Function to set custom callback
    //! If nullptr is passed instead of callback, logger will stop outputing logs
    //! MDAL_LoggerCallback is a function accepting MDAL_LogLevel, MDAL_Status and const char* string
    //! Declaration can be found in mdal.h
    void setLoggerCallback( MDAL_LoggerCallback callback );
  }
}

#endif // MDAL_LOGGER_H
