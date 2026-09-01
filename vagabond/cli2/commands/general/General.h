#pragma once

#include "Help.h"
#include "Version.h"

namespace rope::cli::commands
{
using General = help_group<
    "General",
    Version,
    Help>;
} // namespace rope::cli::commands
