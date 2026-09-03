#include "RootOptions.h"

namespace rope::cli::detail
{
namespace
{
[[nodiscard]] std::string_view trim(std::string_view text)
{
    const std::size_t first = text.find_first_not_of(" \t");
    if (first == std::string_view::npos)
    {
        return {};
    }
    const std::size_t last = text.find_last_not_of(" \t");
    return text.substr(first, last - first + 1);
}
} // namespace

bool root_option_has_name(std::string_view names,
                          std::string_view argument)
{
    std::size_t start = 0;
    while (start <= names.size())
    {
        const std::size_t end = names.find(',', start);
        const std::string_view name = trim(names.substr(
            start,
            end == std::string_view::npos
                ? std::string_view::npos
                : end - start));
        if (name == argument)
        {
            return true;
        }
        if (end == std::string_view::npos)
        {
            break;
        }
        start = end + 1;
    }
    return false;
}

void apply_root_effect(root_effect effect, console& output)
{
    switch (effect)
    {
        case root_effect::plain_output:
            output.set_style(console_style::plain);
            break;
    }
}
} // namespace rope::cli::detail

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include <sstream>
#include <string>

namespace
{
std::string root_options_test_version()
{
    return "RoPE test-version";
}

inline constexpr auto TestHelpOption = rope::cli::root_help_option{
    .names = "-h,--help",
    .description = "Print help",
};

inline constexpr auto TestPlainOption = rope::cli::root_flag_option{
    .names = "--plain",
    .description = "Print plain output",
    .effect = rope::cli::root_effect::plain_output,
};

inline constexpr auto TestVersionOption = rope::cli::root_action_option{
    .names = "-V,--version",
    .description = "Print version info",
    .handler = &root_options_test_version,
};

using TestRootOptions = rope::cli::root_options<
    TestHelpOption,
    TestPlainOption,
    TestVersionOption>;
} // namespace

TEST_CASE("root options configure their CLI11 options from one definition")
{
    CLI::App app{"Description", "rope"};
    TestRootOptions::configure(app);

    const std::string help = app.help();

    CHECK(help.find("-h") != std::string::npos);
    CHECK(help.find("--help") != std::string::npos);
    CHECK(help.find("--plain") != std::string::npos);
    CHECK(help.find("-V") != std::string::npos);
    CHECK(help.find("--version") != std::string::npos);
}

TEST_CASE("root flags are consumed before command parsing")
{
    std::ostringstream standard;
    std::ostringstream error;
    rope::cli::detail::console output{
        standard,
        error,
        {.standard_is_terminal = true}};
    char executable[] = "rope";
    char command[] = "read";
    char plain[] = "--plain";
    char* argv[]{executable, command, plain};

    const auto arguments =
        rope::cli::detail::extract_root_arguments<TestRootOptions>(
            3, argv, output);

    REQUIRE(arguments.values.size() == 2);
    CHECK(std::string_view{arguments.values[0]} == "rope");
    CHECK(std::string_view{arguments.values[1]} == "read");
    CHECK(output.context().style ==
          rope::cli::detail::console_style::plain);
}

#endif // ROPE_INLINE_TESTS
