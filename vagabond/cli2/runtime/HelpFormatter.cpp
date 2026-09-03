#include "HelpFormatter.h"

#include <CLI/CLI.hpp>

#include <string>
#include <utility>

namespace rope::cli::detail
{
namespace
{
[[nodiscard]] std::string remove_one_trailing_newline(std::string text)
{
    if (!text.empty() && text.back() == '\n')
    {
        text.pop_back();
    }
    return text;
}

class compact_help_formatter final : public CLI::Formatter
{
public:
    [[nodiscard]] std::string make_description(
        const CLI::App* app) const override
    {
        return remove_one_trailing_newline(
            CLI::Formatter::make_description(app));
    }

    [[nodiscard]] std::string make_usage(
        const CLI::App* app,
        std::string name) const override
    {
        return remove_one_trailing_newline(
            CLI::Formatter::make_usage(app, std::move(name)));
    }
};
} // namespace

std::shared_ptr<CLI::FormatterBase> make_help_formatter()
{
    auto formatter = std::make_shared<compact_help_formatter>();
    formatter->label("SUBCOMMAND", "COMMAND");
    formatter->label("SUBCOMMANDS", "COMMANDS");
    return formatter;
}
} // namespace rope::cli::detail

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

TEST_CASE("help formatter uses one blank line between sections")
{
    CLI::App app{"Description", "rope"};
    app.add_flag("--plain", "Print plain output");
    app.formatter(rope::cli::detail::make_help_formatter());

    const std::string help = app.help();

    CHECK(help.find("Description\n\nrope [OPTIONS]\n\nOPTIONS:\n") !=
          std::string::npos);
    CHECK(help.find("\n\n\n") == std::string::npos);
}

TEST_CASE("help formatter calls child applications commands")
{
    CLI::App app{"Description", "rope"};
    app.add_subcommand("add", "Add two integers")->group("Commands");
    app.add_subcommand("read", "Read a value")->group("Commands");
    app.formatter(rope::cli::detail::make_help_formatter());

    const std::string help = app.help();

    CHECK(help.find("rope [OPTIONS] [COMMANDS]") != std::string::npos);
    CHECK(help.find("Commands:\n  add") != std::string::npos);
    CHECK(help.find("SUBCOMMAND") == std::string::npos);
}

#endif // ROPE_INLINE_TESTS
