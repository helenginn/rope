#pragma once

#include <array>
#include <concepts>
#include <cstdlib>
#include <tuple>
#include <type_traits>

namespace rope::cli::detail
{
template <typename... States>
struct unique_state_types : std::true_type
{};

template <typename First, typename... Rest>
struct unique_state_types<First, Rest...>
    : std::bool_constant<
          ((!std::same_as<std::remove_cvref_t<First>,
                          std::remove_cvref_t<Rest>>) && ...) &&
          unique_state_types<Rest...>::value>
{};

template <typename... States>
class state_pack
{
public:
    explicit state_pack(States&... states)
        : states_{&states...}
    {
        static_assert(unique_state_types<States...>::value,
                      "each application state type may be supplied only once");
        static_assert((!std::is_volatile_v<States> && ...),
                      "volatile application state is not supported");
    }

    template <typename Requested>
    [[nodiscard]] const Requested& read()
    {
        constexpr std::size_t count = match_count<Requested>();
        if constexpr (count == 1)
        {
            return *std::get<match_index<Requested>()>(states_);
        }
        else
        {
            static_assert(count == 1,
                          "requested application state must be supplied exactly once");
            std::abort();
        }
    }

    template <typename Requested>
    [[nodiscard]] Requested& mutate()
    {
        constexpr std::size_t count = match_count<Requested>();
        if constexpr (count == 1)
        {
            constexpr std::size_t index = match_index<Requested>();
            using supplied_type =
                std::tuple_element_t<index, std::tuple<States...>>;
            if constexpr (std::is_const_v<supplied_type>)
            {
                static_assert(!std::is_const_v<supplied_type>,
                              "mutable application state cannot be supplied as const");
                std::abort();
            }
            else
            {
                return *std::get<index>(states_);
            }
        }
        else
        {
            static_assert(count == 1,
                          "requested application state must be supplied exactly once");
            std::abort();
        }
    }

private:
    template <typename Requested>
    static consteval std::size_t match_count()
    {
        return (std::size_t{0} + ... +
                (std::same_as<Requested, std::remove_cvref_t<States>>
                     ? std::size_t{1}
                     : std::size_t{0}));
    }

    template <typename Requested>
    static consteval std::size_t match_index()
    {
        constexpr std::array<bool, sizeof...(States)> matches{
            std::same_as<Requested, std::remove_cvref_t<States>>...};
        for (std::size_t index = 0; index < matches.size(); ++index)
        {
            if (matches[index])
            {
                return index;
            }
        }
        return 0;
    }

    std::tuple<States*...> states_;
};
} // namespace rope::cli::detail
