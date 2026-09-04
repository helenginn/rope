#include "commands/general/General.h"
#include "commands/project/Project.h"
#include "commands/test_commands/add.h"
#include "runtime/CliRuntime.h"
#include "state/ProjectState.h"

#ifdef ROPE_INLINE_TESTS
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#endif

namespace
{
inline constexpr auto RootHelpOption = rope::cli::root_help_option{
    .names = "-h,--help",
    .description = "Print this help message and exit",
};

inline constexpr auto RootVersionOption = rope::cli::root_action_option{
    .names = "-V,--version",
    .description = "Print version info",
    .handler = &rope::cli::commands::version,
};

inline constexpr auto RootPlainOption = rope::cli::root_flag_option{
    .names = "--plain",
    .description = "Print command results without terminal formatting",
    .effect = rope::cli::root_effect::plain_output,
};

using RootOptions = rope::cli::root_options<
    RootHelpOption,
    RootVersionOption,
    RootPlainOption>;

using Root = rope::cli::group<
    "rope.cli2",
    "Representation of Protein Entities (RoPE)",
    rope::cli::commands::General,
    rope::cli::commands::Project,
    rope::cli::commands::Add>;
} // namespace

int main(int argc, char** argv)
{
    rope::cli::ProjectState project;
    return rope::cli::run<Root, RootOptions>(argc, argv, project);
}
