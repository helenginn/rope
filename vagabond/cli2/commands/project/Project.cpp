#include "Project.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

#include <vagabond/core/Reporter.h>

namespace rope::cli::commands
{
namespace
{
class scoped_stream_redirect
{
public:
    scoped_stream_redirect(std::ostream& stream, std::streambuf* destination)
        : stream_{stream}, previous_{stream.rdbuf(destination)}
    {}

    ~scoped_stream_redirect()
    {
        stream_.rdbuf(previous_);
    }

    scoped_stream_redirect(const scoped_stream_redirect&) = delete;
    scoped_stream_redirect& operator=(const scoped_stream_redirect&) = delete;

private:
    std::ostream& stream_;
    std::streambuf* previous_;
};

command_result<void> load_project_state(ProjectState& project,
                                        const std::string& filename)
{
    std::ifstream input{filename};
    if (!input.good())
    {
        return rust_type::Err(command_error{
            "Could not load project '" + filename + "': file not found"});
    }

    try
    {
        std::ostringstream ignored_output;
        scoped_stream_redirect redirect{std::cout, ignored_output.rdbuf()};
        project.environment().load(filename);
    }
    catch (const std::exception& error)
    {
        return rust_type::Err(command_error{
            "Could not load project '" + filename + "': " + error.what()});
    }

    project.mark_loaded(filename);
    return rust_type::Ok();
}
} // namespace

command_result<std::string> load_project(ProjectState& project,
                                         std::string filename)
{
    command_result<void> result = load_project_state(project, filename);
    if (result.is_err())
    {
        return rust_type::Err(std::move(result).unwrap_err());
    }
    return rust_type::Ok("Loaded project: " + filename);
}

std::string project_status(const ProjectState& project)
{
    if (!project.loaded())
    {
        return "No project loaded";
    }
    return "Loaded project: " + *project.loaded_from();
}

command_result<std::string> report_project(const ProjectState& project)
{
    if (!project.loaded())
    {
        return rust_type::Err(command_error{
            "No project loaded. Run 'load_env <filename>' first"});
    }

    try
    {
        std::ostringstream output;
        scoped_stream_redirect redirect{std::cout, output.rdbuf()};
        Reporter reporter;
        reporter.report();
        return rust_type::Ok(output.str());
    }
    catch (const std::exception& error)
    {
        return rust_type::Err(command_error{
            "Could not report project: " + std::string{error.what()}});
    }
}
} // namespace rope::cli::commands

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "../../runtime/CliRuntime.h"

namespace
{
using ProjectRoot = rope::cli::group<
    "rope.cli2",
    "Project command test",
    rope::cli::commands::Project>;

std::string empty_project_json()
{
    return R"({
        "entity_manager": {
            "ligand_entity_manager": {"entities": []},
            "polymer_entity_manager": {"entities": []}
        },
        "file_manager": {"files": []},
        "metadata": {
            "data": [],
            "headers": [],
            "ruler": {"rules": []},
            "source": "master"
        },
        "model_manager": {"models": []},
        "path_manager": {"paths": []}
    })";
}

class temporary_project_file
{
public:
    temporary_project_file()
        : filename_{"rope_cli2_project_test.json"}
    {
        std::ofstream output{filename_};
        output << empty_project_json();
    }

    ~temporary_project_file()
    {
        std::remove(filename_.c_str());
    }

    [[nodiscard]] const std::string& filename() const
    {
        return filename_;
    }

private:
    std::string filename_;
};

int run_project_cli(std::vector<std::string> arguments,
                    std::ostringstream& output,
                    std::ostringstream& error_output,
                    rope::cli::ProjectState& project)
{
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (std::string& argument : arguments)
    {
        argv.push_back(argument.data());
    }

    std::istringstream input;
    return rope::cli::run_with_streams<
        ProjectRoot,
        rope::cli::root_options<>>(
        static_cast<int>(argv.size()),
        argv.data(),
        input,
        output,
        error_output,
        project);
}
} // namespace

TEST_CASE("project commands share loaded state in a command chain")
{
    temporary_project_file file;
    rope::cli::ProjectState project;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_project_cli(
        {"rope.cli2", "load_env", file.filename(), "status", "report"},
        output,
        error_output,
        project);

    CHECK(exit_code == 0);
    CHECK(project.loaded());
    CHECK(project.loaded_from() == file.filename());
    CHECK(output.str().find("Loaded project: " + file.filename()) !=
          std::string::npos);
    CHECK(output.str().find("No. of entities: 0") != std::string::npos);
    CHECK(output.str().find("No. of models: 0") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("project report requires a loaded project")
{
    rope::cli::ProjectState project;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_project_cli(
        {"rope.cli2", "report"},
        output,
        error_output,
        project);

    CHECK(exit_code != 0);
    CHECK_FALSE(project.loaded());
    CHECK(output.str().empty());
    CHECK(error_output.str().find("No project loaded") != std::string::npos);
}

TEST_CASE("a failed project load stops the command chain")
{
    rope::cli::ProjectState project;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_project_cli(
        {"rope.cli2", "load_env", "missing-project.json", "report"},
        output,
        error_output,
        project);

    CHECK(exit_code != 0);
    CHECK_FALSE(project.loaded());
    CHECK(output.str().empty());
    CHECK(error_output.str().find("file not found") != std::string::npos);
}

#endif // ROPE_INLINE_TESTS
