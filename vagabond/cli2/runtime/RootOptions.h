#pragma once

#include <CLI/CLI.hpp>

#include <concepts>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "CommandSpec.h"
#include "Console.h"

namespace rope::cli
{
template <std::size_t NamesSize, std::size_t DescriptionSize>
struct root_help_option
{
    fixed_string<NamesSize> names;
    fixed_string<DescriptionSize> description;
};

template <std::size_t NamesSize, std::size_t DescriptionSize>
root_help_option(const char (&)[NamesSize],
                 const char (&)[DescriptionSize])
    -> root_help_option<NamesSize, DescriptionSize>;

enum class root_effect
{
    plain_output,
};

template <std::size_t NamesSize, std::size_t DescriptionSize>
struct root_flag_option
{
    fixed_string<NamesSize> names;
    fixed_string<DescriptionSize> description;
    root_effect effect;
};

template <std::size_t NamesSize, std::size_t DescriptionSize>
root_flag_option(const char (&)[NamesSize],
                 const char (&)[DescriptionSize],
                 root_effect)
    -> root_flag_option<NamesSize, DescriptionSize>;

template <std::size_t NamesSize,
          std::size_t DescriptionSize,
          typename Handler>
struct root_action_option
{
    fixed_string<NamesSize> names;
    fixed_string<DescriptionSize> description;
    Handler handler;
};

template <std::size_t NamesSize,
          std::size_t DescriptionSize,
          typename Handler>
root_action_option(const char (&)[NamesSize],
                   const char (&)[DescriptionSize],
                   Handler)
    -> root_action_option<NamesSize, DescriptionSize, Handler>;

namespace detail
{
template <typename T>
struct is_root_help_option : std::false_type
{};

template <std::size_t NamesSize, std::size_t DescriptionSize>
struct is_root_help_option<
    root_help_option<NamesSize, DescriptionSize>> : std::true_type
{};

template <typename T>
struct is_root_flag_option : std::false_type
{};

template <std::size_t NamesSize, std::size_t DescriptionSize>
struct is_root_flag_option<
    root_flag_option<NamesSize, DescriptionSize>> : std::true_type
{};

template <typename T>
struct is_root_action_option : std::false_type
{};

template <std::size_t NamesSize,
          std::size_t DescriptionSize,
          typename Handler>
struct is_root_action_option<
    root_action_option<NamesSize, DescriptionSize, Handler>> : std::true_type
{};

template <typename T>
inline constexpr bool is_root_option_v =
    is_root_help_option<T>::value ||
    is_root_flag_option<T>::value ||
    is_root_action_option<T>::value;

[[nodiscard]] bool root_option_has_name(std::string_view names,
                                        std::string_view argument);

void apply_root_effect(root_effect effect, console& output);

template <auto Option>
void configure_root_option(CLI::App& app)
{
    using option_type = std::remove_cvref_t<decltype(Option)>;

    if constexpr (is_root_help_option<option_type>::value)
    {
        app.set_help_flag(
            std::string(Option.names.view()),
            std::string(Option.description.view()));
    }
    else if constexpr (is_root_flag_option<option_type>::value)
    {
        app.add_flag(
            std::string(Option.names.view()),
            std::string(Option.description.view()));
    }
    else
    {
        using handler_type = decltype(Option.handler);
        static_assert(std::invocable<handler_type>,
                      "a root action handler must take no arguments");
        static_assert(
            std::same_as<std::invoke_result_t<handler_type>, std::string>,
            "a root action handler must return std::string");
        static_assert(Option.handler != nullptr,
                      "a root action handler must not be null");

        app.add_flag_callback(
               std::string(Option.names.view()),
               []()
               {
                   throw CLI::CallForVersion(
                       std::invoke(Option.handler), 0);
               },
               std::string(Option.description.view()))
            ->configurable(false)
            ->callback_priority(CLI::CallbackPriority::First);
    }
}

template <auto Option>
[[nodiscard]] bool consume_root_flag(std::string_view argument,
                                     console& output)
{
    using option_type = std::remove_cvref_t<decltype(Option)>;
    if constexpr (is_root_flag_option<option_type>::value)
    {
        if (root_option_has_name(Option.names.view(), argument))
        {
            apply_root_effect(Option.effect, output);
            return true;
        }
    }
    return false;
}

struct root_arguments
{
    std::vector<char*> values;
};
} // namespace detail

template <auto... Options>
struct root_options
{
    static_assert(
        (detail::is_root_option_v<
             std::remove_cvref_t<decltype(Options)>> && ...),
        "root_options accepts only root option definitions");

    static void configure(CLI::App& app)
    {
        (detail::configure_root_option<Options>(app), ...);
    }

    [[nodiscard]] static bool consume_flag(std::string_view argument,
                                           detail::console& output)
    {
        return (detail::consume_root_flag<Options>(argument, output) || ...);
    }
};

namespace detail
{
template <typename T>
struct is_root_options : std::false_type
{};

template <auto... Options>
struct is_root_options<root_options<Options...>> : std::true_type
{};

template <typename T>
inline constexpr bool is_root_options_v = is_root_options<T>::value;

template <typename RootOptions>
    requires is_root_options_v<RootOptions>
[[nodiscard]] root_arguments extract_root_arguments(
    int argc,
    char** argv,
    console& output)
{
    root_arguments arguments;
    if (argc > 0)
    {
        arguments.values.reserve(static_cast<std::size_t>(argc));
        arguments.values.push_back(argv[0]);
    }

    bool options_enabled = true;
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument{argv[index]};
        if (options_enabled && RootOptions::consume_flag(argument, output))
        {
            continue;
        }

        arguments.values.push_back(argv[index]);
        if (argument == "--")
        {
            options_enabled = false;
        }
    }
    return arguments;
}
} // namespace detail
} // namespace rope::cli
