#pragma once

#include <CLI/CLI.hpp>

#include <concepts>
#include <cstdlib>
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "CommandResult.h"
#include "Console.h"

namespace rope::cli::detail
{
template <typename T>
concept printable = requires(std::ostream& output, const T& value)
{
    output << value;
};

[[nodiscard]] inline std::string help_text(const CLI::App& app)
{
    return app.help();
}

[[nodiscard]] inline std::string command_path(const CLI::App& app)
{
    const CLI::App* parent = app.get_parent();
    if (parent == nullptr)
    {
        return app.get_name();
    }
    return command_path(*parent) + " " + app.get_name();
}

[[nodiscard]] inline std::string local_help_text(const CLI::App& app)
{
    return app.get_formatter()->make_help(
        &app, command_path(app), CLI::AppFormatMode::Normal);
}

struct execution_result
{
    int exit_code = 0;
    bool exit_requested = false;
};

[[nodiscard]] inline execution_result present_parse_error(
    CLI::App& app,
    const CLI::ParseError& error,
    console& output)
{
    std::ostringstream standard;
    std::ostringstream error_output;
    const int exit_code = app.exit(error, standard, error_output);
    output.parser_output(standard.str(), error_output.str());
    return {.exit_code = exit_code, .exit_requested = false};
}

template <typename Handler, typename... Arguments>
[[nodiscard]] auto invoke_command(Handler handler, Arguments&&... arguments)
{
    using return_type = std::invoke_result_t<Handler, Arguments...>;
    using unqualified_return_type = std::remove_cvref_t<return_type>;

    if constexpr (std::is_void_v<return_type>)
    {
        std::invoke(handler, std::forward<Arguments>(arguments)...);
        return command_result<void>{rust_type::Ok<void>{}};
    }
    else if constexpr (is_result_v<unqualified_return_type>)
    {
        using error_type =
            typename result_traits_t<unqualified_return_type>::error_type;
        static_assert(std::same_as<error_type, command_error>,
                      "a command Result must use rope::cli::command_error");
        static_assert(!std::is_reference_v<return_type>,
                      "a command Result must be returned by value");
        return std::invoke(
            handler, std::forward<Arguments>(arguments)...);
    }
    else
    {
        auto value = std::invoke(
            handler, std::forward<Arguments>(arguments)...);
        return command_result<unqualified_return_type>{
            rust_type::Ok(std::move(value))};
    }
}

template <typename Value>
[[nodiscard]] execution_result present_command_result(
    command_result<Value>&& result,
    console& output)
{
    if (result.is_err())
    {
        command_error error = std::move(result).unwrap_err();
        output.error(error.message);
        return {.exit_code = EXIT_FAILURE, .exit_requested = false};
    }

    if constexpr (!std::is_void_v<Value>)
    {
        static_assert(printable<Value>,
                      "a command success value must be printable");
        std::ostringstream rendered;
        rendered << std::move(result).unwrap();
        output.result(rendered.str());
    }
    return {};
}

class planned_invocation
{
public:
    virtual ~planned_invocation() = default;
    [[nodiscard]] virtual execution_result dispatch(console& output) = 0;
};

class help_invocation final : public planned_invocation
{
public:
    explicit help_invocation(const CLI::App& app)
        : app_{&app}
    {}

    [[nodiscard]] execution_result dispatch(console& output) override
    {
        output.help(local_help_text(*app_));
        return {};
    }

private:
    const CLI::App* app_;
};

class exit_invocation final : public planned_invocation
{
public:
    [[nodiscard]] execution_result dispatch(console&) override
    {
        return {.exit_code = 0, .exit_requested = true};
    }
};

class execution_plan
{
public:
    template <typename Invocation, typename... Arguments>
    void add(Arguments&&... arguments)
    {
        invocations_.push_back(std::make_unique<Invocation>(
            std::forward<Arguments>(arguments)...));
    }

    [[nodiscard]] bool empty() const
    {
        return invocations_.empty();
    }

    [[nodiscard]] std::size_t size() const
    {
        return invocations_.size();
    }

    [[nodiscard]] execution_result dispatch(console& output)
    {
        for (const std::unique_ptr<planned_invocation>& invocation : invocations_)
        {
            execution_result result = invocation->dispatch(output);
            if (result.exit_code != 0 || result.exit_requested)
            {
                return result;
            }
        }
        return {};
    }

private:
    std::vector<std::unique_ptr<planned_invocation>> invocations_;
};
} // namespace rope::cli::detail
