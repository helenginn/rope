#include "SurfaceAreaValidator.h"
#include "PathManager.h"
#include "EntityManager.h"
#include "ModelManager.h"
#include <iomanip>

SurfaceAreaValidator::SurfaceAreaValidator(): _env(Environment::env())
{
}

SurfaceAreaValidator::~SurfaceAreaValidator()
{
}

void SurfaceAreaValidator::loadEnv(std::string environmentFilePath)
{
  _env.load(environmentFilePath);
}

void SurfaceAreaValidator::validatePaths()
{
  std::cout << "Entity count: " << _env.entityCount() << std::endl;
  std::cout << "Model count: " << _env.modelManager()->objects().size() << std::endl;
  std::cout << "Path count: " << _env.pathManager()->objectCount() << std::endl;

  std::vector<Entity *> entities = _env.entityManager()->entities();

  for (Entity *e : entities)
  {
    for (Path *p : _env.pathManager()->pathsForEntity(e))
    {
      std::cout << "Path id: " << p->id() << "\n" << std::endl;

      PlausibleRoute *pr = p->toRoute();
      // AtomGroup *atomGroup = pr->instance()->currentAtoms();
      // pr->setup();
    }
  }
}
