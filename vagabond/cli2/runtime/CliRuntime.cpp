#include "CliRuntime.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include <sstream>
#include <string>
#include <vector>

namespace
{
struct Store
{
    int value = 0;
};

const Store* read_store = nullptr;
int mutation_calls = 0;
int result_failure_calls = 0;
int version_calls = 0;

std::string test_version()
{
    ++version_calls;
    return "RoPE test-version";
}

int read_value(const Store& store)
{
    read_store = &store;
    return store.value;
}

void set_value(Store& store, int value)
{
    ++mutation_calls;
    store.value = value;
}

rope::cli::command_result<void> fail_command(Store&)
{
    ++result_failure_calls;
    return rust_type::Err(
        rope::cli::command_error{"Deliberate command failure"});
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

using ResultFailure = rope::cli::command<
    rope::cli::command_meta{
        .name = "fail",
        .description = "Return a command failure",
        .handler = &fail_command,
    },
    rope::cli::mutate_state<Store>>;

using StateRoot = rope::cli::group<
    "rope.cli2",
    "State capability test",
    ReadValue,
    SetValue>;

using ResultRoot = rope::cli::group<
    "rope.cli2",
    "Command result test",
    ReadValue,
    SetValue,
    ResultFailure>;

inline constexpr auto TestHelpOption = rope::cli::root_help_option{
    .names = "-h,--help",
    .description = "Print this help message and exit",
};

inline constexpr auto TestVersionOption = rope::cli::root_action_option{
    .names = "-V,--version",
    .description = "Print version info",
    .handler = &test_version,
};

inline constexpr auto TestPlainOption = rope::cli::root_flag_option{
    .names = "--plain",
    .description = "Print command results without terminal formatting",
    .effect = rope::cli::root_effect::plain_output,
};

using TestRootOptions = rope::cli::root_options<
    TestHelpOption,
    TestVersionOption,
    TestPlainOption>;

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
    return rope::cli::run_with_streams<Root, TestRootOptions>(
        static_cast<int>(argv.size()),
        argv.data(),
        input,
        output,
        error_output,
        states...);
}

template <typename Root, typename... States>
int run_cli_with_console(std::vector<std::string> arguments,
                         std::string input_text,
                         rope::cli::detail::console& output,
                         States&... states)
{
    std::vector<char*> argv;
    argv.reserve(arguments.size());
    for (std::string& argument : arguments)
    {
        argv.push_back(argument.data());
    }

    std::istringstream input{std::move(input_text)};
    return rope::cli::detail::run_with_console<Root, TestRootOptions>(
        static_cast<int>(argv.size()),
        argv.data(),
        input,
        output,
        states...);
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

TEST_CASE("root version options use the configured version handler")
{
    Store store{0};
    version_calls = 0;

    for (std::string option : {"-V", "--version"})
    {
        std::ostringstream output;
        std::ostringstream error_output;

        const int exit_code = run_cli_with_input<StateRoot>(
            {"rope.cli2", std::move(option)},
            "",
            output,
            error_output,
            store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "RoPE test-version\n");
        CHECK(error_output.str().empty());
    }

    CHECK(version_calls == 2);
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

TEST_CASE("runtime selects the console presentation mode")
{
    Store store{0};
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{output, error_output};

    SUBCASE("one-shot command")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "read"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(console.context().mode ==
              rope::cli::detail::console_mode::one_shot);
    }

    SUBCASE("chained commands")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "set", "4", "read"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(console.context().mode ==
              rope::cli::detail::console_mode::chained);
    }

    SUBCASE("interactive shell")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "shell"}, "exit\n", console, store);

        CHECK(exit_code == 0);
        CHECK(console.context().mode ==
              rope::cli::detail::console_mode::shell);
    }

    SUBCASE("standard-input batch")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "-"}, "read\n", console, store);

        CHECK(exit_code == 0);
        CHECK(console.context().mode ==
              rope::cli::detail::console_mode::batch);
    }
}

TEST_CASE("runtime labels only chained terminal command results")
{
    Store store{3};
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{
        output,
        error_output,
        {.standard_is_terminal = true}};

    SUBCASE("one-shot command")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "read"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "3\n");
    }

    SUBCASE("chained commands")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "read", "read"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() ==
              "        Read: 3\n"
              "        Read: 3\n");
    }

    SUBCASE("interactive shell")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "shell"}, "read\nexit\n", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() ==
              "rope.cli2> 3\n"
              "rope.cli2> ");
    }

    SUBCASE("chained commands in the interactive shell")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "shell"},
            "read read\nexit\n",
            console,
            store);

        CHECK(exit_code == 0);
        CHECK(output.str() ==
              "rope.cli2>         Read: 3\n"
              "        Read: 3\n"
              "rope.cli2> ");
    }

    SUBCASE("standard-input batch")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "-"}, "read\n", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "3\n");
    }
}

TEST_CASE("plain is a global command-line presentation option")
{
    Store store{3};
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{
        output,
        error_output,
        {.standard_is_terminal = true}};

    SUBCASE("before a command")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "--plain", "read"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "3\n");
    }

    SUBCASE("after a command")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "read", "--plain"}, "", console, store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "3\n");
    }

    SUBCASE("before shell mode")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "--plain", "shell"},
            "read\nexit\n",
            console,
            store);

        CHECK(exit_code == 0);
        CHECK(output.str() ==
              "rope.cli2> 3\n"
              "rope.cli2> ");
    }

    SUBCASE("after shell mode")
    {
        const int exit_code = run_cli_with_console<StateRoot>(
            {"rope.cli2", "shell", "--plain"},
            "read\nexit\n",
            console,
            store);

        CHECK(exit_code == 0);
        CHECK(output.str() ==
              "rope.cli2> 3\n"
              "rope.cli2> ");
    }
}

TEST_CASE("root help describes plain output")
{
    Store store{0};
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<StateRoot>(
        {"rope.cli2"}, "", output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(output.str().find("--plain") != std::string::npos);
    CHECK(output.str().find("without terminal formatting") !=
          std::string::npos);
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

TEST_CASE("a command Result failure stops standard-input mode")
{
    Store store{0};
    mutation_calls = 0;
    result_failure_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<ResultRoot>(
        {"rope.cli2", "-"},
        "fail\nset 8\n",
        output,
        error_output,
        store);

    CHECK(exit_code != 0);
    CHECK(result_failure_calls == 1);
    CHECK(mutation_calls == 0);
    CHECK(store.value == 0);
    CHECK(output.str().empty());
    CHECK(error_output.str() == "Deliberate command failure\n");
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

TEST_CASE("shell continues after a command Result failure")
{
    Store store{0};
    mutation_calls = 0;
    result_failure_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli_with_input<ResultRoot>(
        {"rope.cli2", "shell"},
        "fail\nset 8\nread\nexit\n",
        output,
        error_output,
        store);

    CHECK(exit_code == 0);
    CHECK(result_failure_calls == 1);
    CHECK(mutation_calls == 1);
    CHECK(store.value == 8);
    CHECK(output.str().find("rope.cli2> ") != std::string::npos);
    CHECK(output.str().find("8\n") != std::string::npos);
    CHECK(error_output.str() == "Deliberate command failure\n");
}

#endif // ROPE_INLINE_TESTS
