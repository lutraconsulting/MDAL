/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

#include "mdal.h"

namespace MDAL
{
  namespace Log
  {
    static MDAL_Status sLastStatus;

    void error( MDAL_Status status );
    void warning( MDAL_Status status );

    MDAL_Status getLastStatus() { return sLastStatus; }
    void resetLastStatus() { sLastStatus = MDAL_Status::None; }

    class Logger
    {
    public:
      static Logger& getInstance() {
        static Logger instance;
        return instance;
      }

      void operator= ( const Logger & ) = delete;
      Logger( const Logger & ) = delete;

      void init();
      void log( MDAL_Status status, std::string message );
      void setCallback( MDAL_LoggerCallback callback );

    private:
      Logger();

      MDAL_LoggerCallback mLoggerCallback;
    };
  }
}

#endif // LOGGER_H
