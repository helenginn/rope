#include "SurfaceAreaValidator.h"
#include "PathManager.h"
#include "EntityManager.h"
#include "ModelManager.h"
#include <iomanip>

SurfaceAreaValidator::SurfaceAreaValidator()
{
}

SurfaceAreaValidator::~SurfaceAreaValidator()
{
}

void SurfaceAreaValidator::loadEnv(std::string environmentFilePath)
{
  _env = Environment();
  _env.load(environmentFilePath);
}

void SurfaceAreaValidator::validatePaths()
{
  std::cout << "Entity count: " << _env.entityCount() << std::endl;
  std::cout << "Model count: " << _env.modelManager()->objects().size() << std::endl;
  std::cout << "Path count: " << _env.pathManager()->objectCount() << std::endl;
}