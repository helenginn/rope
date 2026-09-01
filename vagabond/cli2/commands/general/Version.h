#pragma once

#include <string>

#include "../../runtime/CommandSpec.h"

namespace rope::cli::commands
{
std::string version();

using Version = command<
    command_meta{
        .name = "version",
        .description = "Print version info",
        .handler = &version,
    }>;
} // namespace rope::cli::commands
