#include "Console.h"

#include <cstdio>

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
void write_line(std::ostream& output, std::string_view text)
{
    output << text;
    if (text.empty() || text.back() != '\n')
    {
        output << '\n';
    }
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

void console::result(std::string_view text)
{
    standard_ << text << '\n';
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

    console.result("value");
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
    CHECK(console.context().standard_is_terminal);
    CHECK_FALSE(console.context().error_is_terminal);
    CHECK(console.context().width == 113);

    console.set_mode(rope::cli::detail::console_mode::chained);
    CHECK(console.context().mode == rope::cli::detail::console_mode::chained);
}

#endif // ROPE_INLINE_TESTS
