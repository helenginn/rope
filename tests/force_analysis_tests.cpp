#include <doctest/doctest.h>
#include <vagabond/core/forces/ForceAnalysis.h>
#include <filesystem>

TEST_CASE("DOES IT COMPILE?" * doctest::test_suite("force-e2e"))
{
  const char *directory = std::getenv("ROPE_FORCES_FIXTURES");
  REQUIRE(directory != nullptr);

  const std::filesystem::path fixtures{directory};
  
  CHECK(5 == 6);

}
