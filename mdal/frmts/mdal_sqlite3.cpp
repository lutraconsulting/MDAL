/*
 MDAL - Mesh Data Abstraction Library (MIT License)
 Copyright (C) 2020 Lutra Consulting Limited
*/

#include "mdal_sqlite3.hpp"
#include <sqlite3.h>


Sqlite3Db::Sqlite3Db() = default;

Sqlite3Db::~Sqlite3Db()
{
  close();
}

bool Sqlite3Db::open( const std::string &fileName )
{
  close();
  int rc = sqlite3_open( fileName.c_str(), &mDb );
  if ( rc )
    return false;
  else
    return true;
}

void Sqlite3Db::close()
{
  if ( mDb )
  {
    sqlite3_close( mDb );
    mDb = nullptr;
  }
}

sqlite3 *Sqlite3Db::get()
{
  return mDb;
}
