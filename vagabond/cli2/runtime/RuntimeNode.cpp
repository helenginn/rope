#include "RuntimeNode.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include "../commands/general/General.h"
#include "CliRuntime.h"

#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace
{
using VersionRoot = rope::cli::group<
    "rope.cli2",
    "CLI vertical-slice test",
    rope::cli::commands::Version>;

using NestedGroup = rope::cli::group<
    "nested",
    "Nested command group",
    rope::cli::commands::Version,
    rope::cli::commands::Help>;

using NestedRoot = rope::cli::group<
    "rope.cli2",
    "Nested group test",
    NestedGroup>;

using HelpGroupedRoot = rope::cli::group<
    "rope.cli2",
    "Help group test",
    rope::cli::commands::General>;

int argument_calls = 0;
int observed_count = 0;
bool observed_verbose = false;
bool observed_enabled = false;

void observe_arguments(int count, bool verbose, bool enabled)
{
    ++argument_calls;
    observed_count = count;
    observed_verbose = verbose;
    observed_enabled = enabled;
}

using ObserveArguments = rope::cli::command<
    rope::cli::command_meta{
        .name = "observe",
        .description = "Observe positional, flag, and option values",
        .handler = &observe_arguments,
    },
    rope::cli::positional<int, rope::cli::argument_meta{
        .name = "count",
        .description = "Number to observe",
    }>,
    rope::cli::flag<rope::cli::argument_meta{
        .name = "verbose",
        .description = "Enable verbose output",
        .short_name = 'v',
    }>,
    rope::cli::required_option<bool, rope::cli::argument_meta{
        .name = "enabled",
        .description = "Set enabled explicitly",
        .short_name = 'e',
    }>>;

using ArgumentRoot = rope::cli::group<
    "rope.cli2",
    "CLI argument-kind test",
    ObserveArguments>;

std::vector<std::optional<int>> observed_limits;

void observe_optional(std::optional<int> limit)
{
    observed_limits.push_back(limit);
}

using ObserveOptional = rope::cli::command<
    rope::cli::command_meta{
        .name = "optional",
        .description = "Observe an optional value",
        .handler = &observe_optional,
    },
    rope::cli::option<int, rope::cli::argument_meta{
        .name = "limit",
        .description = "Optional limit",
        .short_name = 'l',
    }>>;

using OptionalRoot = rope::cli::group<
    "rope.cli2",
    "Optional argument test",
    ObserveOptional>;

template <typename Root, typename... States>
int run_cli(std::vector<std::string> arguments,
            std::ostringstream& output,
            std::ostringstream& error_output,
            States&... states)
{
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (std::string& argument : arguments)
    {
        argv.push_back(argument.data());
    }

    std::istringstream input;
    return rope::cli::run_with_streams<Root, rope::cli::root_options<>>(
        static_cast<int>(argv.size()),
        argv.data(),
        input,
        output,
        error_output,
        states...);
}
} // namespace

TEST_CASE("a declared command is parsed and dispatched once")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<VersionRoot>(
        {"rope.cli2", "version"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(output.str() == rope::cli::commands::version() + "\n");
    CHECK(error_output.str().empty());
}

TEST_CASE("a bare group prints its local help without dispatching")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<VersionRoot>(
        {"rope.cli2"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(output.str().find("version") != std::string::npos);
    CHECK(output.str().find("rope.cli2 shell") != std::string::npos);
    CHECK(output.str().find("rope.cli2 -") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("a bare nested group prints its own local help")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<NestedRoot>(
        {"rope.cli2", "nested"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(output.str().find("Nested command group") != std::string::npos);
    CHECK(output.str().find("version") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("a nested help command prints its parent group's help")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<NestedRoot>(
        {"rope.cli2", "nested", "help"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(output.str().find("Nested command group") != std::string::npos);
    CHECK(output.str().find("version") != std::string::npos);
    CHECK(output.str().find("help") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("a help group changes presentation without changing command syntax")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int help_exit_code = run_cli<HelpGroupedRoot>(
        {"rope.cli2"}, output, error_output);

    CHECK(help_exit_code == 0);
    CHECK(output.str().find("General") != std::string::npos);
    CHECK(output.str().find("version") != std::string::npos);
    CHECK(error_output.str().empty());

    output.str("");
    output.clear();
    const int command_exit_code = run_cli<HelpGroupedRoot>(
        {"rope.cli2", "version"}, output, error_output);

    CHECK(command_exit_code == 0);
    CHECK(output.str() == rope::cli::commands::version() + "\n");
}

TEST_CASE("a help command prints root help")
{
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<HelpGroupedRoot>(
        {"rope.cli2", "help"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(output.str().find("Help group test") != std::string::npos);
    CHECK(output.str().find("General") != std::string::npos);
    CHECK(output.str().find("version") != std::string::npos);
    CHECK(output.str().find("help") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("flags and value options map to handler parameters left to right")
{
    argument_calls = 0;
    observed_count = 0;
    observed_verbose = false;
    observed_enabled = false;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ArgumentRoot>(
        {"rope.cli2", "observe", "7", "-v", "--enabled", "false"},
        output,
        error_output);

    CHECK(exit_code == 0);
    CHECK(argument_calls == 1);
    CHECK(observed_count == 7);
    CHECK(observed_verbose);
    CHECK_FALSE(observed_enabled);
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("an absent flag maps to false")
{
    argument_calls = 0;
    observed_verbose = true;
    observed_enabled = false;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ArgumentRoot>(
        {"rope.cli2", "observe", "4", "--enabled", "true"},
        output,
        error_output);

    CHECK(exit_code == 0);
    CHECK(argument_calls == 1);
    CHECK_FALSE(observed_verbose);
    CHECK(observed_enabled);
    CHECK(error_output.str().empty());
}

TEST_CASE("long flags and short value options are accepted")
{
    argument_calls = 0;
    observed_verbose = false;
    observed_enabled = false;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ArgumentRoot>(
        {"rope.cli2", "observe", "5", "--verbose", "-e", "true"},
        output,
        error_output);

    CHECK(exit_code == 0);
    CHECK(argument_calls == 1);
    CHECK(observed_verbose);
    CHECK(observed_enabled);
    CHECK(error_output.str().empty());
}

TEST_CASE("a scalar option is required and consumes a value")
{
    argument_calls = 0;

    SUBCASE("the option is absent")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli<ArgumentRoot>(
            {"rope.cli2", "observe", "4"}, output, error_output);

        CHECK(exit_code != 0);
        CHECK_FALSE(error_output.str().empty());
    }

    SUBCASE("the option has no value")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli<ArgumentRoot>(
            {"rope.cli2", "observe", "4", "--enabled"},
            output,
            error_output);

        CHECK(exit_code != 0);
        CHECK_FALSE(error_output.str().empty());
    }

    CHECK(argument_calls == 0);
}

TEST_CASE("an option reports whether its value was supplied")
{
    observed_limits.clear();
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<OptionalRoot>(
        {"rope.cli2", "optional"}, output, error_output);

    REQUIRE(exit_code == 0);
    REQUIRE(observed_limits.size() == 1);
    CHECK_FALSE(observed_limits.front().has_value());
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("an option passes its supplied value")
{
    observed_limits.clear();
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<OptionalRoot>(
        {"rope.cli2", "optional", "--limit", "7"},
        output,
        error_output);

    REQUIRE(exit_code == 0);
    REQUIRE(observed_limits.size() == 1);
    CHECK(observed_limits.front() == 7);
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("an invalid option value does not invoke its handler")
{
    observed_limits.clear();
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<OptionalRoot>(
        {"rope.cli2", "optional", "--limit", "not-an-integer"},
        output,
        error_output);

    CHECK(exit_code != 0);
    CHECK(observed_limits.empty());
    CHECK_FALSE(error_output.str().empty());
}

TEST_CASE("an option resets between repeated root commands")
{
    observed_limits.clear();
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<OptionalRoot>(
        {"rope.cli2", "optional", "-l", "4", "optional"},
        output,
        error_output);

    REQUIRE(exit_code == 0);
    REQUIRE(observed_limits.size() == 2);
    CHECK(observed_limits[0] == 4);
    CHECK_FALSE(observed_limits[1].has_value());
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("argument help shows long and short option names")
{
    argument_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ArgumentRoot>(
        {"rope.cli2", "observe", "--help"}, output, error_output);

    CHECK(exit_code == 0);
    CHECK(argument_calls == 0);
    CHECK(output.str().find("-v") != std::string::npos);
    CHECK(output.str().find("--verbose") != std::string::npos);
    CHECK(output.str().find("-e") != std::string::npos);
    CHECK(output.str().find("--enabled") != std::string::npos);
    CHECK(error_output.str().empty());
}

TEST_CASE("repeated commands receive freshly reset argument storage")
{
    argument_calls = 0;
    observed_verbose = true;
    observed_enabled = true;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ArgumentRoot>(
        {"rope.cli2", "observe", "1", "-v", "--enabled", "true",
         "++", "observe", "2", "--enabled", "false"},
        output,
        error_output);

    CHECK(exit_code == 0);
    CHECK(argument_calls == 2);
    CHECK(observed_count == 2);
    CHECK_FALSE(observed_verbose);
    CHECK_FALSE(observed_enabled);
    CHECK(error_output.str().empty());
}

#endif // ROPE_INLINE_TESTS
