#include "CliRuntime.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include "../commands/general/General.h"

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

static_assert(!rope::cli::detail::child_names_are_unique<
              rope::cli::help_group<"First", rope::cli::commands::Version>,
              rope::cli::help_group<"Second", rope::cli::commands::Version>>());

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
    rope::cli::option<bool, rope::cli::argument_meta{
        .name = "enabled",
        .description = "Set enabled explicitly",
        .short_name = 'e',
    }>>;

using ArgumentRoot = rope::cli::group<
    "rope.cli2",
    "CLI argument-kind test",
    ObserveArguments>;

void incompatible_flag_handler(int)
{}

static_assert(rope::cli::detail::handler_matches<
              &observe_arguments,
              rope::cli::positional<int, rope::cli::argument_meta{
                  .name = "count",
                  .description = "",
              }>,
              rope::cli::flag<rope::cli::argument_meta{
                  .name = "verbose",
                  .description = "",
              }>,
              rope::cli::option<bool, rope::cli::argument_meta{
                  .name = "enabled",
                  .description = "",
              }>>());
static_assert(!rope::cli::detail::handler_matches<
              &incompatible_flag_handler,
              rope::cli::flag<rope::cli::argument_meta{
                  .name = "verbose",
                  .description = "",
              }>>());

struct Store
{
    int value = 0;
};

const Store* read_store = nullptr;
Store* mutated_store = nullptr;
int mutation_calls = 0;

int read_value(const Store& store)
{
    read_store = &store;
    return store.value;
}

void set_value(Store& store, int value)
{
    mutated_store = &store;
    ++mutation_calls;
    store.value = value;
}

using ReadValue = rope::cli::command<
    rope::cli::command_meta{
        .name = "read",
        .description = "Read the stored value",
        .handler = &read_value,
    },
    rope::cli::read_state<Store>>;

using SetValue = rope::cli::command<
    rope::cli::command_meta{
        .name = "set",
        .description = "Set the stored value",
        .handler = &set_value,
    },
    rope::cli::mutate_state<Store>,
    rope::cli::positional<int, rope::cli::argument_meta{
        .name = "value",
        .description = "New value",
    }>>;

using ReadOnlyRoot = rope::cli::group<
    "rope.cli2",
    "Read-only state test",
    ReadValue>;

using StateRoot = rope::cli::group<
    "rope.cli2",
    "State capability test",
    ReadValue,
    SetValue>;

template <typename Root, typename... States>
int run_cli_with_input(std::vector<std::string> arguments,
                       std::string input_text,
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

    std::istringstream input{std::move(input_text)};
    return rope::cli::run_with_streams<Root>(
        static_cast<int>(argv.size()),
        argv.data(),
        input,
        output,
        error_output,
        states...);
}

template <typename Root, typename... States>
int run_cli(std::vector<std::string> arguments,
            std::ostringstream& output,
            std::ostringstream& error_output,
            States&... states)
{
    return run_cli_with_input<Root>(
        std::move(arguments), "", output, error_output, states...);
}

template <typename Root, typename... States>
int run_session(std::string input_text,
                std::ostringstream& output,
                std::ostringstream& error_output,
                bool interactive,
                States&... states)
{
    std::istringstream input{std::move(input_text)};
    return rope::cli::run_session_with_streams<Root>(
        input, output, error_output, interactive, states...);
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

TEST_CASE("no command prints help without reading standard input")
{
    Store store{0};
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<StateRoot>(
        {"rope.cli2"}, "set 9\n", output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(mutation_calls == 0);
    CHECK(store.value == 0);
    CHECK(output.str().find("State capability test") != std::string::npos);
    CHECK(output.str().find("rope.cli2 shell") != std::string::npos);
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

TEST_CASE("read_state passes through the exact object as const")
{
    const Store store{7};
    read_store = nullptr;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ReadOnlyRoot>(
        {"rope.cli2", "read"}, output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(read_store == &store);
    CHECK(output.str() == "7\n");
    CHECK(error_output.str().empty());
}

TEST_CASE("mutate_state passes through the exact object and preserves mutation")
{
    Store store{3};
    mutated_store = nullptr;
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<StateRoot>(
        {"rope.cli2", "set", "9"}, output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(mutated_store == &store);
    CHECK(mutation_calls == 1);
    CHECK(store.value == 9);
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("parse errors and help do not invoke stateful handlers")
{
    Store store{3};
    read_store = nullptr;
    mutated_store = nullptr;
    mutation_calls = 0;

    SUBCASE("conversion failure")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli<StateRoot>(
            {"rope.cli2", "set", "not-an-integer"},
            output,
            error_output,
            store);

        CHECK(exit_code != 0);
        CHECK_FALSE(error_output.str().empty());
    }

    SUBCASE("help")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli<StateRoot>(
            {"rope.cli2", "--help"}, output, error_output, store);

        CHECK(exit_code == 0);
        CHECK(output.str().find("read") != std::string::npos);
        CHECK(error_output.str().empty());
    }

    CHECK(read_store == nullptr);
    CHECK(mutated_store == nullptr);
    CHECK(mutation_calls == 0);
    CHECK(store.value == 3);
}

TEST_CASE("sibling commands chain implicitly in parsing order")
{
    Store store{3};
    mutation_calls = 0;
    read_store = nullptr;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<StateRoot>(
        {"rope.cli2", "set", "9", "read"},
        output,
        error_output,
        store);

    CHECK(exit_code == 0);
    CHECK(mutation_calls == 1);
    CHECK(read_store == &store);
    CHECK(store.value == 9);
    CHECK(output.str() == "9\n");
    CHECK(error_output.str().empty());
}

TEST_CASE("the explicit terminator chains repeated commands")
{
    Store store{0};
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<StateRoot>(
        {"rope.cli2", "set", "4", "++", "set", "8", "++", "read"},
        output,
        error_output,
        store);

    CHECK(exit_code == 0);
    CHECK(mutation_calls == 2);
    CHECK(store.value == 8);
    CHECK(output.str() == "8\n");
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

TEST_CASE("a later chain parse error prevents every handler invocation")
{
    Store store{0};
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<StateRoot>(
        {"rope.cli2", "set", "1", "++",
         "set", "not-an-integer"},
        output,
        error_output,
        store);

    CHECK(exit_code != 0);
    CHECK(mutation_calls == 0);
    CHECK(store.value == 0);
    CHECK(output.str().empty());
    CHECK_FALSE(error_output.str().empty());
}

TEST_CASE("entry modes must be used alone")
{
    Store store{0};
    mutation_calls = 0;

    SUBCASE("shell")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli_with_input<StateRoot>(
            {"rope.cli2", "shell", "read"},
            "set 9\n",
            output,
            error_output,
            store);

        CHECK(exit_code != 0);
        CHECK(output.str().empty());
        CHECK(error_output.str().find("must be used alone") != std::string::npos);
    }

    SUBCASE("standard input")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        const int exit_code = run_cli_with_input<StateRoot>(
            {"rope.cli2", "-", "read"},
            "set 9\n",
            output,
            error_output,
            store);

        CHECK(exit_code != 0);
        CHECK(output.str().empty());
        CHECK(error_output.str().find("must be used alone") != std::string::npos);
    }

    CHECK(mutation_calls == 0);
    CHECK(store.value == 0);
}

TEST_CASE("a later shell token rejects the whole command chain")
{
    Store store{0};
    read_store = nullptr;
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<StateRoot>(
        {"rope.cli2", "set", "1", "read", "shell"},
        "set 9\n",
        output,
        error_output,
        store);

    CHECK(exit_code != 0);
    CHECK(mutation_calls == 0);
    CHECK(read_store == nullptr);
    CHECK(store.value == 0);
    CHECK(output.str().empty());
    CHECK_FALSE(error_output.str().empty());
}

TEST_CASE("dash reads commands from standard input without prompts")
{
    Store store{0};
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<StateRoot>(
        {"rope.cli2", "-"},
        "# here-doc style input\n\nset 6\nread\n",
        output,
        error_output,
        store);

    CHECK(exit_code == 0);
    CHECK(mutation_calls == 1);
    CHECK(store.value == 6);
    CHECK(output.str() == "6\n");
    CHECK(error_output.str().empty());
}

TEST_CASE("exit stops a session and ignores the remaining chain")
{
    Store store{0};
    mutation_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_session<StateRoot>(
        "set 4 ++ exit ++ set 9\nset 12\n",
        output,
        error_output,
        false,
        store);

    CHECK(exit_code == 0);
    CHECK(mutation_calls == 1);
    CHECK(store.value == 4);
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("shell prints prompts and survives parse errors")
{
    Store store{5};
    read_store = nullptr;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<StateRoot>(
        {"rope.cli2", "shell"},
        "set not-an-integer\nread\nexit\n",
        output,
        error_output,
        store);

    CHECK(exit_code == 0);
    CHECK(read_store == &store);
    CHECK(store.value == 5);
    CHECK(output.str().find("rope.cli2> ") != std::string::npos);
    CHECK(output.str().find("5\n") != std::string::npos);
    CHECK_FALSE(error_output.str().empty());
}

#endif // ROPE_INLINE_TESTS
