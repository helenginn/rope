#include <vagabond/utils/include_boost.h>
#include <vagabond/core/engine/SurfaceAreaValidator.h>

namespace tt = boost::test_tools;

BOOST_AUTO_TEST_CASE(test_SAV_load)
{
  SurfaceAreaValidator SAV;
  SAV.loadEnv("/home/iko/UNI/BA-BSC/ROPE/quick_bundle/rope.json");
}