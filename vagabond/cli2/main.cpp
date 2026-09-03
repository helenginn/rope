#include "commands/general/General.h"
#include "commands/test_commands/add.h"
#include "runtime/CliRuntime.h"

#ifdef ROPE_INLINE_TESTS
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#endif

namespace
{
using Root = rope::cli::group<
    "rope.cli2",
    "Representation of Protein Entities (RoPE)",
    rope::cli::commands::General,
    rope::cli::commands::Add>;
} // namespace

int main(int argc, char** argv)
{
    return rope::cli::run<Root>(argc, argv);
}
