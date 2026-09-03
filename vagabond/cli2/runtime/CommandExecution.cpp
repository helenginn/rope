#include "CommandExecution.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include "CliRuntime.h"

#include <optional>
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

using StateRoot = rope::cli::group<
    "rope.cli2",
    "State capability test",
    ReadValue,
    SetValue>;

int result_value_calls = 0;
int result_quiet_calls = 0;
int result_failure_calls = 0;

rope::cli::command_result<int> produce_result_value()
{
    ++result_value_calls;
    return rust_type::Ok(42);
}

rope::cli::command_result<void> succeed_quietly()
{
    ++result_quiet_calls;
    return rust_type::Ok();
}

rope::cli::command_result<void> fail_command(Store&)
{
    ++result_failure_calls;
    return rust_type::Err(
        rope::cli::command_error{"Deliberate command failure"});
}

rope::cli::command_result<int> report_optional(std::optional<int> limit)
{
    if (!limit.has_value())
    {
        return rust_type::Err(
            rope::cli::command_error{"No limit supplied"});
    }
    return rust_type::Ok(*limit);
}

using ResultValue = rope::cli::command<
    rope::cli::command_meta{
        .name = "result",
        .description = "Return a successful value",
        .handler = &produce_result_value,
    }>;

using ResultQuiet = rope::cli::command<
    rope::cli::command_meta{
        .name = "quiet",
        .description = "Succeed without a value",
        .handler = &succeed_quietly,
    }>;

using ResultFailure = rope::cli::command<
    rope::cli::command_meta{
        .name = "fail",
        .description = "Return a command failure",
        .handler = &fail_command,
    },
    rope::cli::mutate_state<Store>>;

using ResultOptional = rope::cli::command<
    rope::cli::command_meta{
        .name = "result-optional",
        .description = "Return an optional input",
        .handler = &report_optional,
    },
    rope::cli::option<int, rope::cli::argument_meta{
        .name = "limit",
        .description = "Optional limit",
        .short_name = 'l',
    }>>;

using ResultRoot = rope::cli::group<
    "rope.cli2",
    "Command result test",
    ReadValue,
    SetValue,
    ResultValue,
    ResultQuiet,
    ResultFailure,
    ResultOptional>;

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

TEST_CASE("a successful command Result prints its value")
{
    Store store;
    result_value_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ResultRoot>(
        {"rope.cli2", "result"}, output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(result_value_calls == 1);
    CHECK(output.str() == "42\n");
    CHECK(error_output.str().empty());
}

TEST_CASE("a successful void command Result is silent")
{
    Store store;
    result_quiet_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ResultRoot>(
        {"rope.cli2", "quiet"}, output, error_output, store);

    CHECK(exit_code == 0);
    CHECK(result_quiet_calls == 1);
    CHECK(output.str().empty());
    CHECK(error_output.str().empty());
}

TEST_CASE("a failed command Result writes to the error stream")
{
    Store store;
    result_failure_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ResultRoot>(
        {"rope.cli2", "fail"}, output, error_output, store);

    CHECK(exit_code != 0);
    CHECK(result_failure_calls == 1);
    CHECK(output.str().empty());
    CHECK(error_output.str() == "Deliberate command failure\n");
}

TEST_CASE("a failed command Result stops the remaining chain")
{
    Store store{0};
    mutation_calls = 0;
    result_failure_calls = 0;
    std::ostringstream output;
    std::ostringstream error_output;

    const int exit_code = run_cli<ResultRoot>(
        {"rope.cli2", "set", "4", "fail", "set", "8", "read"},
        output,
        error_output,
        store);

    CHECK(exit_code != 0);
    CHECK(mutation_calls == 1);
    CHECK(result_failure_calls == 1);
    CHECK(store.value == 4);
    CHECK(output.str().empty());
    CHECK(error_output.str() == "Deliberate command failure\n");
}

TEST_CASE("optional input remains distinct from command failure")
{
    Store store;

    SUBCASE("a supplied value succeeds")
    {
        std::ostringstream output;
        std::ostringstream error_output;

        const int exit_code = run_cli<ResultRoot>(
            {"rope.cli2", "result-optional", "--limit", "7"},
            output,
            error_output,
            store);

        CHECK(exit_code == 0);
        CHECK(output.str() == "7\n");
        CHECK(error_output.str().empty());
    }

    SUBCASE("an absent value can produce a command failure")
    {
        std::ostringstream output;
        std::ostringstream error_output;

        const int exit_code = run_cli<ResultRoot>(
            {"rope.cli2", "result-optional"},
            output,
            error_output,
            store);

        CHECK(exit_code != 0);
        CHECK(output.str().empty());
        CHECK(error_output.str() == "No limit supplied\n");
    }
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

#endif // ROPE_INLINE_TESTS
