#include <vagabond/utils/include_boost.h>
#include <vagabond/core/engine/SurfaceAreaValidator.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(validate_paths_1)
{
  std::cout << std::endl << "validate_paths_1" << std::endl << std::endl;
  SurfaceAreaValidator SAV;
  SAV.loadEnv("rope.json");
  SAV.validatePaths(100);
}

BOOST_AUTO_TEST_CASE(validate_paths_2)
{
  std::cout << std::endl << "validate_paths_2" << std::endl << std::endl;
  SurfaceAreaValidator SAV;
  SAV.loadEnv("rope_unrefined_paths.json");
  SAV.validatePaths(100);
}