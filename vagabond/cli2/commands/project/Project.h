#pragma once

#include <string>

#include "../../runtime/CommandResult.h"
#include "../../runtime/CommandSpec.h"
#include "../../state/ProjectState.h"

namespace rope::cli::commands
{
command_result<std::string> load_project(ProjectState& project,
                                         std::string filename);
std::string project_status(const ProjectState& project);
command_result<std::string> report_project(const ProjectState& project);

inline constexpr auto ProjectFilenameMeta = argument_meta{
    .name = "filename",
    .description = "Path to a RoPE project JSON file",
};
using ProjectFilename = positional<std::string, ProjectFilenameMeta>;

using LoadProject = command<
    command_meta{
        .name = "load_env",
        .description = "Load a RoPE project environment",
        .handler = &load_project,
    },
    mutate_state<ProjectState>,
    ProjectFilename>;

using ProjectStatus = command<
    command_meta{
        .name = "status",
        .description = "Show the loaded project",
        .handler = &project_status,
    },
    read_state<ProjectState>>;

using ProjectReport = command<
    command_meta{
        .name = "report",
        .description = "Report project entities and models",
        .handler = &report_project,
    },
    read_state<ProjectState>>;

using Project = help_group<
    "Project",
    LoadProject,
    ProjectStatus,
    ProjectReport>;
} // namespace rope::cli::commands
