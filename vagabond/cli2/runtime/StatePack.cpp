#include "StatePack.h"

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include "CliRuntime.h"

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

#endif // ROPE_INLINE_TESTS
