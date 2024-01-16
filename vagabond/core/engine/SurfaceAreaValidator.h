#include "AreaMeasurer.h"
#include "vagabond/core/Environment.h"

class SurfaceAreaValidator
{

public:
  SurfaceAreaValidator();
  ~SurfaceAreaValidator();

  void loadEnv(std::string environmentFilePath);
  
private:
  Environment _env;

};

