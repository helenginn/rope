#pragma once

#include "../../runtime/CommandSpec.h"

namespace rope::cli::commands
{
using Help = help_command<
    help_command_meta{
        .name = "help",
        .description = "Print help",
    }>;
} // namespace rope::cli::commands
