#pragma once

#include <cstddef>
#include <ostream>
#include <string_view>

namespace rope::cli::detail
{
enum class console_mode
{
    one_shot,
    chained,
    shell,
    batch,
};

enum class console_style
{
    automatic,
    plain,
};

struct console_context
{
    console_mode mode = console_mode::one_shot;
    console_style style = console_style::automatic;
    bool standard_is_terminal = false;
    bool error_is_terminal = false;
    std::size_t width = 80;
};

class console
{
public:
    console(std::ostream& standard,
            std::ostream& error,
            console_context context = {});

    [[nodiscard]] const console_context& context() const;
    void set_mode(console_mode mode);
    void set_style(console_style style);

    void command_result(std::string_view command,
                        std::string_view text);
    void error(std::string_view text);
    void help(std::string_view text);
    void parser_output(std::string_view standard,
                       std::string_view error);
    void prompt(std::string_view name);

private:
    [[nodiscard]] bool formats_command_results() const;

    std::ostream& standard_;
    std::ostream& error_;
    console_context context_;
};

[[nodiscard]] console_context standard_console_context(
    console_mode mode = console_mode::one_shot);
} // namespace rope::cli::detail
