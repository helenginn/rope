#pragma once

#include "../../utils/ResultType.h"

#include <string>
#include <type_traits>

namespace rope::cli
{
struct command_error
{
    std::string message;
};

template <typename T = void>
using command_result = rust_type::Result<T, command_error>;

namespace detail
{
template <typename T>
struct result_traits
{
    static constexpr bool is_result = false;
};

template <typename Value, typename Error>
struct result_traits<rust_type::Result<Value, Error>>
{
    static constexpr bool is_result = true;

    using error_type = Error;
};

template <typename T>
using result_traits_t = result_traits<std::remove_cvref_t<T>>;

template <typename T>
inline constexpr bool is_result_v = result_traits_t<T>::is_result;
} // namespace detail
} // namespace rope::cli
