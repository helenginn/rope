#include "SurfaceAreaValidator.h"
#include <iomanip>

SurfaceAreaValidator::SurfaceAreaValidator()
{
}

SurfaceAreaValidator::~SurfaceAreaValidator()
{
  // delete &_env;
}

void SurfaceAreaValidator::loadEnv(std::string environmentFilePath)
{
  _env = Environment();
  _env.load(environmentFilePath);
  std::cout << _env.entityCount() << std::endl;
}