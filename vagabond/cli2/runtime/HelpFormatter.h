#pragma once

#include <memory>

namespace CLI
{
class FormatterBase;
}

namespace rope::cli::detail
{
[[nodiscard]] std::shared_ptr<CLI::FormatterBase> make_help_formatter();
} // namespace rope::cli::detail
