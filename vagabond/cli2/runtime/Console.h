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

struct console_context
{
    console_mode mode = console_mode::one_shot;
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

    void result(std::string_view text);
    void error(std::string_view text);
    void help(std::string_view text);
    void parser_output(std::string_view standard,
                       std::string_view error);
    void prompt(std::string_view name);

private:
    std::ostream& standard_;
    std::ostream& error_;
    console_context context_;
};

[[nodiscard]] console_context standard_console_context(
    console_mode mode = console_mode::one_shot);
} // namespace rope::cli::detail
