#define xstr(a) str(a)
#define str(a) #a

#include "mdal_testutils.hpp"
#include <vector>
#include <math.h>
#include <assert.h>

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

bool getActive(DatasetH dataset, int index)
{
  char active;
  int nValuesRead = MDAL_D_data(dataset, index, 1, MDAL_DataType::ACTIVE_BOOL, &active);
  assert (nValuesRead == 1);
  return static_cast<bool>(active);
}

double getValue(DatasetH dataset, int index)
{
  double val;
  int nValuesRead = MDAL_D_data(dataset, index, 1, MDAL_DataType::SCALAR_DOUBLE, &val);
  assert (nValuesRead == 1);
  return val;
}

double getValueX(DatasetH dataset, int index)
{
  double val[2];
  int nValuesRead = MDAL_D_data(dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val);
  assert (nValuesRead == 1);
  return val[0];
}

double getValueY(DatasetH dataset, int index)
{
  double val[2];
  int nValuesRead = MDAL_D_data(dataset, index, 1, MDAL_DataType::VECTOR_2D_DOUBLE, &val);
  assert (nValuesRead == 1);
  return val[1];
}

bool compareVectors(const std::vector<double>& a, const std::vector<double>& b)
{
  double eps = 1e-4;
  if (a.size() != b.size())
     return false;

  for (size_t i=0; i<a.size(); ++i)
    if (fabs(a[i] - b[i]) > eps)
      return false;

  return true;
}
