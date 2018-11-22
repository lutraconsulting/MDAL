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
  return val[1];
}
