#include "Version.h"

#include "config/config.h"

namespace rope::cli::commands
{
std::string version()
{
    return std::string{"RoPE "} + ROPE_VERSION;
}
} // namespace rope::cli::commands

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

TEST_CASE("version reports the configured RoPE version")
{
    CHECK(rope::cli::commands::version() ==
          std::string{"RoPE "} + ROPE_VERSION);
}

#endif // ROPE_INLINE_TESTS
