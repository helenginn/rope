#include "AreaMeasurer.h"
#include "BondCalculator.h"
#include "BondSequenceHandler.h"
#include "BaseTask.h"
#include "Task.h"
#include "Tasks.h"
#include "Result.h"
#include "vagabond/core/Environment.h"
#include "gemmi/model.hpp"
#include "vagabond/core/files/PdbFile.h"
class SurfaceAreaValidator
{
public:
  SurfaceAreaValidator();
  ~SurfaceAreaValidator();

  void loadEnv(std::string environmentFilePath);
  
  void validatePaths();

private:
  Environment &_env;
};

