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
      AtomGroup *atomGroup = pr->instance()->currentAtoms();
      pr->setup();

      gemmi::Structure st;
      int steps = 30;
      float step = 1 / (float)steps;
      for (size_t i = 0; i < steps; i++)
      {
        float frac = i * step;
        pr->submitJobAndRetrieve(frac, true);

        std::string model_name = std::to_string(i + 1);
        PdbFile::writeAtomsToStructure(atomGroup, st, model_name);
        
        BondCalculator *calculator = new BondCalculator();

        const int resources = 1;
        const int threads = 1;
        BondSequenceHandler *sequences = new BondSequenceHandler(resources);
        sequences->setTotalSamples(1);
        sequences->addAnchorExtension(atomGroup->chosenAnchor());

        sequences->setup();
        sequences->prepareSequences();

        Tasks * tasks = new Tasks();
        tasks->run(threads);

        BaseTask *first_hook = nullptr;
        CalcTask *final_hook = nullptr;

        Task<Result, void *> *submit_result = calculator->submitResult(0);

        Flag::Calc calc_flags = Flag::Calc(Flag::DoTorsions);
	      Flag::Extract gets = Flag::Extract(Flag::AtomMap);

        Task<BondSequence *, void *> *letgo = nullptr;

        Task<BondSequence *, AtomPosMap *> *extract_map;

        sequences->calculate(calc_flags, {}, &first_hook, &final_hook);
        letgo = sequences->extract(gets, nullptr, final_hook,	nullptr, nullptr, &extract_map);

        auto map_to_surface_job = [](AtomPosMap *map) -> SurfaceAreaValue
        {
          AreaMeasurer am;
          am.copyAtomMap(*map);
          float area = am.surfaceArea(*map);
          return SurfaceAreaValue{area};
        };

        auto *map_to_surface = new Task<AtomPosMap *, SurfaceAreaValue>(map_to_surface_job, "map to surface");

        extract_map->follow_with(map_to_surface);
        map_to_surface->follow_with(submit_result);

        /* first task to be initiated by tasks list */
        tasks->addTask(first_hook);

        Result *r = calculator->acquireResult();
        
        float area = r->surface_area;
        std::cout << frac << " area: " << area << std::endl;
        
        delete calculator;
        delete sequences;
      }

    }
  }
}
