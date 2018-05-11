#define xstr(a) str(a)
#define str(a) #a

#include "mdal_testutils.hpp"

const char *data_path()
{
  return TESTDATA;
}


std::string test_file( std::string basename )
{
  std::string path( data_path() );
  path += basename;
  return path;
}
