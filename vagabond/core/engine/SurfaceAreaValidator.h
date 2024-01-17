#include "AreaMeasurer.h"
#include "vagabond/core/Environment.h"

class SurfaceAreaValidator
{
public:
  SurfaceAreaValidator();
  ~SurfaceAreaValidator();

  void loadEnv(std::string environmentFilePath);
  
  void validatePaths();
  
private:
  Environment _env;
};

