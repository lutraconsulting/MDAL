/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Tomas Mizera (tomas.mizera2 at gmail dot com)
*/

#include "mdal_logger.hpp"

#include <iostream>

void _standardStdout( MDAL_Status status, const char* mssg )
{
  if ( status > None && status < Warn_UnsupportedElement)
    std::cerr << "Status " << +status << ":" << mssg << std::endl;
  else
    std::cout << "Status " << +status << ":" << mssg << std::endl;
}

void MDAL::Log::error( MDAL_Status status )
{
  std::string message;

  switch ( status ) {
    case Err_MissingDriver:
      message = "Driver is missing!";
      break;
    default:
      break;
  }

  sLastStatus = status;
  Logger::getInstance().log( status, message );
}

void MDAL::Log::warning( MDAL_Status status )
{
  std::string message;

  switch ( status ) {
    case Warn_UnsupportedElement:
      message = "Unsupported element!";
      break;
    default:
      break;
  }

  sLastStatus = status;
  Logger::getInstance().log( status, message );
}


MDAL::Log::Logger::Logger()
{
  init();
}

void MDAL::Log::Logger::init()
{
  if ( getenv( "MDAL_DEBUG" ) )
  {
    setCallback( &_standardStdout );
  }
}

void MDAL::Log::Logger::log( MDAL_Status status, std::string message )
{
  mLoggerCallback( status, message.c_str() );
}

void MDAL::Log::Logger::setCallback(MDAL_LoggerCallback callback)
{
  mLoggerCallback = callback;
}
