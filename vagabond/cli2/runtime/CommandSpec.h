#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace rope::cli
{
template <std::size_t Size>
struct fixed_string
{
    char value[Size]{};

    constexpr fixed_string(const char (&text)[Size])
    {
        for (std::size_t i = 0; i < Size; ++i)
        {
            value[i] = text[i];
        }
    }

    [[nodiscard]] constexpr std::string_view view() const
    {
        return {value, Size - 1};
    }
};

template <std::size_t NameSize,
          std::size_t DescriptionSize,
          typename Handler>
struct command_meta
{
    fixed_string<NameSize> name;
    fixed_string<DescriptionSize> description;
    Handler handler;
};

template <std::size_t NameSize,
          std::size_t DescriptionSize,
          typename Handler>
command_meta(const char (&)[NameSize],
             const char (&)[DescriptionSize],
             Handler)
    -> command_meta<NameSize, DescriptionSize, Handler>;

template <std::size_t NameSize, std::size_t DescriptionSize>
struct help_command_meta
{
    fixed_string<NameSize> name;
    fixed_string<DescriptionSize> description;
};

template <std::size_t NameSize, std::size_t DescriptionSize>
help_command_meta(const char (&)[NameSize],
                  const char (&)[DescriptionSize])
    -> help_command_meta<NameSize, DescriptionSize>;

template <std::size_t NameSize, std::size_t DescriptionSize>
struct argument_meta
{
    fixed_string<NameSize> name;
    fixed_string<DescriptionSize> description;
    char short_name = '\0';
};

template <std::size_t NameSize, std::size_t DescriptionSize>
argument_meta(const char (&)[NameSize],
              const char (&)[DescriptionSize],
              char = '\0')
    -> argument_meta<NameSize, DescriptionSize>;

template <typename T, argument_meta Meta>
struct positional
{
    static_assert(Meta.short_name == '\0',
                  "positional arguments cannot have short names");

    using value_type = T;

    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
};

template <typename T, argument_meta Meta>
struct option
{
    using parsed_type = T;
    using value_type = std::optional<T>;

    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
    static constexpr char short_name = Meta.short_name;
};

template <typename T, argument_meta Meta>
struct required_option
{
    using parsed_type = T;
    using value_type = T;

    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
    static constexpr char short_name = Meta.short_name;
};

template <argument_meta Meta>
struct flag
{
    using value_type = bool;

    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
    static constexpr char short_name = Meta.short_name;
};

template <typename T>
struct read_state
{
    static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                  "state types must be unqualified object types");

    using state_type = T;
};

template <typename T>
struct mutate_state
{
    static_assert(std::same_as<T, std::remove_cvref_t<T>>,
                  "state types must be unqualified object types");

    using state_type = T;
};

template <fixed_string Title, typename... Children>
struct help_group
{
    static_assert(sizeof...(Children) > 0, "a help group cannot be empty");

    static constexpr auto title = Title;
    using children = std::tuple<Children...>;
};

namespace detail
{
template <typename T>
struct is_positional : std::false_type
{};

template <typename T, argument_meta Meta>
struct is_positional<positional<T, Meta>> : std::true_type
{};

template <typename T>
inline constexpr bool is_positional_v = is_positional<T>::value;

template <typename T>
struct is_option : std::false_type
{};

template <typename T, argument_meta Meta>
struct is_option<option<T, Meta>> : std::true_type
{};

template <typename T>
inline constexpr bool is_option_v = is_option<T>::value;

template <typename T>
struct is_required_option : std::false_type
{};

template <typename T, argument_meta Meta>
struct is_required_option<required_option<T, Meta>> : std::true_type
{};

template <typename T>
inline constexpr bool is_required_option_v = is_required_option<T>::value;

template <typename T>
struct is_flag : std::false_type
{};

template <argument_meta Meta>
struct is_flag<flag<Meta>> : std::true_type
{};

template <typename T>
inline constexpr bool is_flag_v = is_flag<T>::value;

template <typename T>
inline constexpr bool is_value_binding_v =
    is_positional_v<T> || is_option_v<T> ||
    is_required_option_v<T> || is_flag_v<T>;

template <typename T>
struct is_read_state : std::false_type
{};

template <typename T>
struct is_read_state<read_state<T>> : std::true_type
{};

template <typename T>
inline constexpr bool is_read_state_v = is_read_state<T>::value;

template <typename T>
struct is_mutate_state : std::false_type
{};

template <typename T>
struct is_mutate_state<mutate_state<T>> : std::true_type
{};

template <typename T>
inline constexpr bool is_mutate_state_v = is_mutate_state<T>::value;

template <typename T>
inline constexpr bool is_supported_binding_v =
    is_value_binding_v<T> || is_read_state_v<T> || is_mutate_state_v<T>;

template <typename T>
struct function_traits
{
    static constexpr bool valid = false;
};

template <typename Return, typename... Arguments>
struct function_traits<Return (*)(Arguments...)>
{
    static constexpr bool valid = true;
    static constexpr std::size_t arity = sizeof...(Arguments);

    using return_type = Return;
    using argument_types = std::tuple<Arguments...>;
};

template <typename Return, typename... Arguments>
struct function_traits<Return (*)(Arguments...) noexcept>
    : function_traits<Return (*)(Arguments...)>
{};

template <auto Handler>
consteval bool is_function_pointer()
{
    using handler_type = decltype(Handler);
    if constexpr (std::is_pointer_v<handler_type> &&
                  std::is_function_v<std::remove_pointer_t<handler_type>>)
    {
        return Handler != nullptr;
    }
    return false;
}

template <typename Binding, typename Parameter>
consteval bool binding_matches_parameter()
{
    if constexpr (is_value_binding_v<Binding>)
    {
        using value_type = typename Binding::value_type;
        return std::same_as<Parameter, value_type> ||
               std::same_as<Parameter, const value_type&> ||
               std::same_as<Parameter, value_type&&>;
    }
    else if constexpr (is_read_state_v<Binding>)
    {
        return std::same_as<Parameter, const typename Binding::state_type&>;
    }
    else if constexpr (is_mutate_state_v<Binding>)
    {
        return std::same_as<Parameter, typename Binding::state_type&>;
    }
    return false;
}

template <auto Handler, typename... Bindings, std::size_t... Indices>
consteval bool handler_parameters_match(std::index_sequence<Indices...>)
{
    using traits = function_traits<decltype(Handler)>;
    using parameters = typename traits::argument_types;
    using bindings = std::tuple<Bindings...>;

    return (binding_matches_parameter<
                std::tuple_element_t<Indices, bindings>,
                std::tuple_element_t<Indices, parameters>>() && ...);
}

template <auto Handler, typename... Bindings>
consteval bool handler_matches()
{
    using traits = function_traits<decltype(Handler)>;
    if constexpr (!traits::valid ||
                  !(is_supported_binding_v<Bindings> && ...))
    {
        return false;
    }
    else if constexpr (traits::arity != sizeof...(Bindings))
    {
        return false;
    }
    else
    {
        return handler_parameters_match<Handler, Bindings...>(
            std::index_sequence_for<Bindings...>{});
    }
}

template <typename Child>
struct visible_child_count : std::integral_constant<std::size_t, 1>
{};

template <fixed_string Title, typename... Children>
struct visible_child_count<help_group<Title, Children...>>
    : std::integral_constant<
          std::size_t,
          (visible_child_count<Children>::value + ...)>
{};

template <typename Child>
struct visible_child_appender
{
    template <std::size_t Size>
    static consteval void append(
        std::array<std::string_view, Size>& names,
        std::size_t& index)
    {
        names[index++] = Child::name.view();
    }
};

template <fixed_string Title, typename... Children>
struct visible_child_appender<help_group<Title, Children...>>
{
    template <std::size_t Size>
    static consteval void append(
        std::array<std::string_view, Size>& names,
        std::size_t& index)
    {
        (visible_child_appender<Children>::template append<Size>(names, index),
         ...);
    }
};

template <typename... Children>
consteval bool child_names_are_unique()
{
    constexpr std::size_t child_count =
        (visible_child_count<Children>::value + ...);
    std::array<std::string_view, child_count> names{};
    std::size_t index = 0;
    (visible_child_appender<Children>::template append<child_count>(
         names, index),
     ...);

    for (std::size_t left = 0; left < names.size(); ++left)
    {
        for (std::size_t right = left + 1; right < names.size(); ++right)
        {
            if (names[left] == names[right])
            {
                return false;
            }
        }
    }
    return true;
}
} // namespace detail

template <command_meta Meta, typename... Bindings>
struct command
{
    static_assert(detail::is_function_pointer<Meta.handler>(),
                  "a command handler must be a non-null free function pointer");
    static_assert((detail::is_supported_binding_v<Bindings> && ...),
                  "command contains an unsupported binding type");
    static_assert(detail::handler_matches<Meta.handler, Bindings...>(),
                  "command bindings must exactly match the handler parameters");

    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
    static constexpr auto handler = Meta.handler;

    using bindings = std::tuple<Bindings...>;
};

template <help_command_meta Meta>
struct help_command
{
    static constexpr auto meta = Meta;
    static constexpr auto name = Meta.name;
    static constexpr auto description = Meta.description;
};

template <fixed_string Name, fixed_string Summary, typename... Children>
struct group
{
    static_assert(sizeof...(Children) > 0, "a command group cannot be empty");
    static_assert(detail::child_names_are_unique<Children...>(),
                  "sibling command names must be unique");

    static constexpr auto name = Name;
    static constexpr auto summary = Summary;

    using children = std::tuple<Children...>;
};

namespace detail
{
template <typename T>
struct is_group_spec : std::false_type
{};

template <fixed_string Name, fixed_string Summary, typename... Children>
struct is_group_spec<group<Name, Summary, Children...>> : std::true_type
{};

template <typename T>
inline constexpr bool is_group_spec_v = is_group_spec<T>::value;
} // namespace detail
} // namespace rope::cli
