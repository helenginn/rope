#include "CommandSpec.h"

#ifdef ROPE_INLINE_TESTS

#include "../commands/general/General.h"

#include <optional>

namespace
{
void observe_arguments(int, bool, bool)
{}

void incompatible_flag_handler(int)
{}

using OptionalArgument = rope::cli::option<int, rope::cli::argument_meta{
    .name = "limit",
    .description = "Optional limit",
}>;

static_assert(std::same_as<OptionalArgument::value_type, std::optional<int>>);

static_assert(!rope::cli::detail::child_names_are_unique<
              rope::cli::help_group<
                  "First", rope::cli::commands::Version>,
              rope::cli::help_group<
                  "Second", rope::cli::commands::Version>>());

static_assert(rope::cli::detail::handler_matches<
              &observe_arguments,
              rope::cli::positional<int, rope::cli::argument_meta{
                  .name = "count",
                  .description = "",
              }>,
              rope::cli::flag<rope::cli::argument_meta{
                  .name = "verbose",
                  .description = "",
              }>,
              rope::cli::required_option<bool, rope::cli::argument_meta{
                  .name = "enabled",
                  .description = "",
              }>>());

static_assert(!rope::cli::detail::handler_matches<
              &incompatible_flag_handler,
              rope::cli::flag<rope::cli::argument_meta{
                  .name = "verbose",
                  .description = "",
              }>>());
} // namespace

#endif // ROPE_INLINE_TESTS
