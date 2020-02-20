/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#include <iostream>

#include "mdal_logger.hpp"
#include "mdal_utils.hpp"

static MDAL_Status sLastStatus;
static MDAL_LoggerCallback sLoggerCallback = MDAL::setDefaultLoggerCallback();

void _log(MDAL_LogLevel logLevel, MDAL_Status status, std::string mssg)
{
  if ( sLoggerCallback )
  {
    sLoggerCallback( logLevel, status, mssg.c_str() );
  }
}

void MDAL::Log::error( MDAL_Status status, std::string mssg )
{
  sLastStatus = status;
  _log(MDAL_LogLevel::Error, status, mssg);
}

void MDAL::Log::warning( MDAL_Status status, std::string mssg )
{
  sLastStatus = status;
  _log( MDAL_LogLevel::Warn, status, mssg );
}

MDAL_Status MDAL::Log::getLastStatus()
{
  return sLastStatus;
}

void MDAL::Log::resetLastStatus()
{
  sLastStatus = MDAL_Status::None;
}

void MDAL::Log::setLoggerCallback(MDAL_LoggerCallback callback)
{
  sLoggerCallback = callback;
}
