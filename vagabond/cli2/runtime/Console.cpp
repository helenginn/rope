#include "Console.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include <vagabond/utils/os.h>

#ifdef OS_WINDOWS
#include <io.h>
#include <windows.h>
#endif

#ifdef OS_UNIX
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace rope::cli::detail
{
namespace
{
constexpr std::size_t command_label_width = 12;

void write_line(std::ostream& output, std::string_view text)
{
    output << text;
    if (text.empty() || text.back() != '\n')
    {
        output << '\n';
    }
}

[[nodiscard]] std::string command_label(std::string_view command)
{
    const std::size_t leaf = command.find_last_of(' ');
    std::string label{command.substr(
        leaf == std::string_view::npos ? 0 : leaf + 1)};
    if (!label.empty() && label.front() >= 'a' && label.front() <= 'z')
    {
        label.front() = static_cast<char>(label.front() - 'a' + 'A');
    }
    return label;
}

[[nodiscard]] std::string_view without_trailing_line_breaks(
    std::string_view text)
{
    while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
    {
        text.remove_suffix(1);
    }
    return text;
}

[[nodiscard]] std::string plain_result(std::string_view text)
{
    text = without_trailing_line_breaks(text);
    std::string rendered{text};
    rendered += '\n';
    return rendered;
}

[[nodiscard]] std::string labelled_result(std::string_view command,
                                          std::string_view text)
{
    const std::string label = command_label(command);
    const std::size_t width = std::max(command_label_width, label.size());
    const std::string first_prefix =
        std::string(width - label.size(), ' ') + label + ':';
    const std::string continuation_prefix(width + 2, ' ');

    text = without_trailing_line_breaks(text);
    if (text.empty())
    {
        return first_prefix + '\n';
    }

    std::string rendered;
    std::size_t start = 0;
    bool first_line = true;
    while (start <= text.size())
    {
        const std::size_t end = text.find('\n', start);
        const std::string_view line = text.substr(
            start,
            end == std::string_view::npos
                ? std::string_view::npos
                : end - start);

        if (first_line)
        {
            rendered += first_prefix;
            if (!line.empty())
            {
                rendered += ' ';
            }
            first_line = false;
        }
        else if (!line.empty())
        {
            rendered += continuation_prefix;
        }
        rendered += line;
        rendered += '\n';

        if (end == std::string_view::npos)
        {
            break;
        }
        start = end + 1;
    }
    return rendered;
}
} // namespace

console::console(std::ostream& standard,
                 std::ostream& error,
                 console_context context)
    : standard_{standard}, error_{error}, context_{context}
{}

const console_context& console::context() const
{
    return context_;
}

void console::set_mode(console_mode mode)
{
    context_.mode = mode;
}

void console::set_style(console_style style)
{
    context_.style = style;
}

bool console::formats_command_results() const
{
    return context_.style == console_style::automatic &&
           context_.standard_is_terminal &&
           context_.mode == console_mode::chained;
}

void console::command_result(std::string_view command,
                             std::string_view text)
{
    standard_ << (formats_command_results()
                      ? labelled_result(command, text)
                      : plain_result(text));
}

void console::error(std::string_view text)
{
    error_ << text << '\n';
}

void console::help(std::string_view text)
{
    write_line(standard_, text);
}

void console::parser_output(std::string_view standard,
                            std::string_view error)
{
    standard_ << standard;
    error_ << error;
}

void console::prompt(std::string_view name)
{
    standard_ << name << "> " << std::flush;
}

console_context standard_console_context(console_mode mode)
{
    console_context context{.mode = mode};

#ifdef OS_WINDOWS
    context.standard_is_terminal = _isatty(_fileno(stdout)) != 0;
    context.error_is_terminal = _isatty(_fileno(stderr)) != 0;

    CONSOLE_SCREEN_BUFFER_INFO info;
    if (context.standard_is_terminal &&
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
    {
        context.width = static_cast<std::size_t>(
            info.srWindow.Right - info.srWindow.Left + 1);
    }
#endif

#ifdef OS_UNIX
    context.standard_is_terminal = isatty(STDOUT_FILENO) != 0;
    context.error_is_terminal = isatty(STDERR_FILENO) != 0;

    struct winsize size {};
    if (context.standard_is_terminal &&
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == 0 &&
        size.ws_col > 0)
    {
        context.width = size.ws_col;
    }
#endif

    return context;
}
} // namespace rope::cli::detail

#ifdef ROPE_INLINE_TESTS

#include <doctest/doctest.h>

#include <sstream>

TEST_CASE("console routes complete messages to their intended streams")
{
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{output, error_output};

    console.command_result("rope.cli2 value", "value");
    console.help("help without newline");
    console.help("help with newline\n");
    console.error("failure");
    console.parser_output("parser standard\n", "parser error\n");
    console.prompt("rope.cli2");

    CHECK(output.str() ==
          "value\n"
          "help without newline\n"
          "help with newline\n"
          "parser standard\n"
          "rope.cli2> ");
    CHECK(error_output.str() == "failure\nparser error\n");
}

TEST_CASE("console carries explicit presentation context")
{
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{
        output,
        error_output,
        {
            .mode = rope::cli::detail::console_mode::shell,
            .standard_is_terminal = true,
            .error_is_terminal = false,
            .width = 113,
        }};

    CHECK(console.context().mode == rope::cli::detail::console_mode::shell);
    CHECK(console.context().style ==
          rope::cli::detail::console_style::automatic);
    CHECK(console.context().standard_is_terminal);
    CHECK_FALSE(console.context().error_is_terminal);
    CHECK(console.context().width == 113);

    console.set_mode(rope::cli::detail::console_mode::chained);
    CHECK(console.context().mode == rope::cli::detail::console_mode::chained);

    console.set_style(rope::cli::detail::console_style::plain);
    CHECK(console.context().style == rope::cli::detail::console_style::plain);
}

TEST_CASE("console labels terminal command results like Cargo status lines")
{
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{
        output,
        error_output,
        {
            .mode = rope::cli::detail::console_mode::chained,
            .standard_is_terminal = true,
        }};

    console.command_result("rope.cli2 add", "4");
    console.command_result("rope.cli2 version", "RoPE 1.8.0");

    CHECK(output.str() ==
          "         Add: 4\n"
          "     Version: RoPE 1.8.0\n");
}

TEST_CASE("console aligns multiline command results")
{
    std::ostringstream output;
    std::ostringstream error_output;
    rope::cli::detail::console console{
        output,
        error_output,
        {
            .mode = rope::cli::detail::console_mode::chained,
            .standard_is_terminal = true,
        }};

    console.command_result(
        "rope.cli2 report", "Model A\nResidues: 143\n\nChains: 2\n");

    CHECK(output.str() ==
          "      Report: Model A\n"
          "              Residues: 143\n"
          "\n"
          "              Chains: 2\n");
}

TEST_CASE("console leaves command results plain when presentation is disabled")
{
    SUBCASE("standard output is not a terminal")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        rope::cli::detail::console console{output, error_output};

        console.command_result("rope.cli2 version", "RoPE 1.8.0\n");

        CHECK(output.str() == "RoPE 1.8.0\n");
    }

    SUBCASE("plain style is requested")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        rope::cli::detail::console console{
            output,
            error_output,
            {
                .style = rope::cli::detail::console_style::plain,
                .standard_is_terminal = true,
            }};

        console.command_result("rope.cli2 version", "RoPE 1.8.0");

        CHECK(output.str() == "RoPE 1.8.0\n");
    }

    SUBCASE("batch mode is always plain")
    {
        std::ostringstream output;
        std::ostringstream error_output;
        rope::cli::detail::console console{
            output,
            error_output,
            {
                .mode = rope::cli::detail::console_mode::batch,
                .standard_is_terminal = true,
            }};

        console.command_result("rope.cli2 version", "RoPE 1.8.0");

        CHECK(output.str() == "RoPE 1.8.0\n");
    }
}

#endif // ROPE_INLINE_TESTS
