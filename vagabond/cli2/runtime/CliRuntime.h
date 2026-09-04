#pragma once

#include <CLI/CLI.hpp>

#include <algorithm>
#include <iostream>
#include <istream>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "CommandExecution.h"
#include "CommandSpec.h"
#include "HelpFormatter.h"
#include "RootOptions.h"
#include "RuntimeNode.h"
#include "StatePack.h"

namespace rope::cli
{
namespace detail
{
[[nodiscard]] inline bool ignored_session_line(const std::string& line)
{
    const std::size_t first = line.find_first_not_of(" \t\r");
    return first == std::string::npos || line[first] == '#';
}

template <typename Root, typename RootOptions, typename... States>
    requires is_root_options_v<RootOptions>
class command_runner
{
public:
    enum class mode
    {
        command_line,
        session,
    };

    command_runner(mode runner_mode, States&... states)
        : runner_mode_{runner_mode},
          app_{std::string(Root::summary.view()),
               std::string(Root::name.view())},
          states_{states...}
    {
        app_.formatter(make_help_formatter());
        app_.prefix_command(true);
        if (runner_mode == mode::command_line)
        {
            RootOptions::configure(app_);
        }
        runtime_.lower_root(app_, states_, plan_);
        if (runner_mode == mode::session)
        {
            CLI::App* exit = app_.add_subcommand(
                "exit", "Exit the current command session");
            exit->group("Commands");
            exit->subcommand_fallthrough(false);
            exit->parse_complete_callback(
                [this]() { plan_.template add<exit_invocation>(); });
            exit->immediate_callback();
        }
        else
        {
            const std::string name{Root::name.view()};
            app_.footer(
                "Modes:\n" + name +
                " shell: Start an interactive shell\n" + name +
                " -: Read commands from standard input");
        }
    }

    [[nodiscard]] execution_result execute(
        int argc,
        char** argv,
        console& output)
    {
        std::vector<std::string> arguments;
        arguments.reserve(argc > 1
                              ? static_cast<std::size_t>(argc - 1)
                              : 0);
        for (int index = 1; index < argc; ++index)
        {
            arguments.emplace_back(argv[index]);
        }
        return execute_tokens(std::move(arguments), output);
    }

    [[nodiscard]] execution_result execute(
        const std::string& line,
        console& output)
    {
        try
        {
            std::vector<std::string> arguments = CLI::detail::split_up(line);
            arguments.erase(
                std::remove(arguments.begin(), arguments.end(), ""),
                arguments.end());
            CLI::detail::remove_quotes(arguments);
            return execute_tokens(std::move(arguments), output);
        }
        catch (const std::invalid_argument& error)
        {
            return present_parse_error(
                app_,
                CLI::ParseError{
                    error.what(), CLI::ExitCodes::InvalidError},
                output);
        }
    }

private:
    [[nodiscard]] execution_result execute_tokens(
        std::vector<std::string> tokens,
        console& output)
    {
        std::vector<std::string> command_tokens;
        bool saw_separator = false;
        for (std::string& token : tokens)
        {
            if (token != "++")
            {
                command_tokens.push_back(std::move(token));
                continue;
            }

            if (command_tokens.empty())
            {
                return separator_error(output);
            }
            if (auto error = parse_commands(
                    std::move(command_tokens), output))
            {
                return *error;
            }
            command_tokens.clear();
            saw_separator = true;
        }

        if (command_tokens.empty())
        {
            return saw_separator ? separator_error(output) : dispatch(output);
        }
        if (auto error = parse_commands(std::move(command_tokens), output))
        {
            return *error;
        }
        return dispatch(output);
    }

    [[nodiscard]] std::optional<execution_result> parse_commands(
        std::vector<std::string> arguments,
        console& output)
    {
        std::reverse(arguments.begin(), arguments.end());
        while (!arguments.empty())
        {
            // Prefix parsing leaves the next root-qualified command untouched.
            const std::size_t planned_before = plan_.size();
            try
            {
                app_.parse(arguments);
            }
            catch (const CLI::ParseError& error)
            {
                return present_parse_error(app_, error, output);
            }

            arguments = app_.remaining_for_passthrough(true);
            if (plan_.size() == planned_before)
            {
                std::vector<std::string> unexpected{arguments.rbegin(),
                                                    arguments.rend()};
                return present_parse_error(
                    app_, CLI::ExtrasError{unexpected}, output);
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] execution_result separator_error(console& output)
    {
        return present_parse_error(
            app_,
            CLI::ParseError{
                "'++' must separate complete commands",
                CLI::ExitCodes::InvalidError},
            output);
    }

    [[nodiscard]] execution_result dispatch(console& output)
    {
        const console_mode initial_mode = output.context().mode;
        const bool chained_shell_command =
            runner_mode_ == mode::session &&
            initial_mode == console_mode::shell &&
            plan_.size() > 1;

        if (runner_mode_ == mode::command_line)
        {
            output.set_mode(plan_.size() > 1
                                ? console_mode::chained
                                : console_mode::one_shot);
        }
        else if (chained_shell_command)
        {
            output.set_mode(console_mode::chained);
        }
        if (plan_.empty())
        {
            output.help(help_text(app_));
            return {};
        }

        execution_result result = plan_.dispatch(output);
        if (chained_shell_command)
        {
            output.set_mode(initial_mode);
        }
        return result;
    }

    mode runner_mode_;
    CLI::App app_;
    state_pack<States...> states_;
    execution_plan plan_;
    runtime_node<Root, state_pack<States...>> runtime_;
};

template <typename Root, typename... States>
    requires is_group_spec_v<Root>
int run_session(std::istream& input,
                console& output,
                bool interactive,
                States&... states)
{
    std::string line;
    while (true)
    {
        if (interactive)
        {
            output.prompt(Root::name.view());
        }

        if (!std::getline(input, line))
        {
            return 0;
        }
        if (ignored_session_line(line))
        {
            continue;
        }

        command_runner<Root, root_options<>, States...> runner{
            command_runner<Root, root_options<>, States...>::mode::session,
            states...};
        execution_result result = runner.execute(line, output);

        if (result.exit_requested)
        {
            return result.exit_code;
        }
        if (result.exit_code != 0 && !interactive)
        {
            return result.exit_code;
        }
    }
}

template <typename Root, typename RootOptions, typename... States>
    requires is_group_spec_v<Root> && is_root_options_v<RootOptions>
int run_with_console(int argc,
                     char** argv,
                     std::istream& input,
                     console& output,
                     States&... states)
{
    root_arguments arguments =
        extract_root_arguments<RootOptions>(argc, argv, output);

    const int runtime_argc = static_cast<int>(arguments.values.size());
    char** runtime_argv = arguments.values.data();
    if (runtime_argc > 1)
    {
        const std::string_view entry_mode{runtime_argv[1]};
        if (entry_mode == "shell" || entry_mode == "-")
        {
            if (runtime_argc != 2)
            {
                output.error("'" + std::string(entry_mode) +
                             "' must be used alone");
                return 1;
            }

            const bool interactive = entry_mode == "shell";
            output.set_mode(interactive
                                ? console_mode::shell
                                : console_mode::batch);
            return run_session<Root>(input, output, interactive, states...);
        }
    }

    command_runner<Root, RootOptions, States...> runner{
        command_runner<Root, RootOptions, States...>::mode::command_line,
        states...};
    return runner.execute(runtime_argc, runtime_argv, output).exit_code;
}
} // namespace detail

template <typename Root, typename... States>
    requires detail::is_group_spec_v<Root>
int run_session_with_streams(std::istream& input,
                             std::ostream& standard_output,
                             std::ostream& error_output,
                             bool interactive,
                             States&... states)
{
    detail::console output{
        standard_output,
        error_output,
        {.mode = interactive
                     ? detail::console_mode::shell
                     : detail::console_mode::batch}};
    return detail::run_session<Root>(input, output, interactive, states...);
}

template <typename Root, typename RootOptions, typename... States>
    requires detail::is_group_spec_v<Root> &&
             detail::is_root_options_v<RootOptions>
int run_with_streams(int argc,
                     char** argv,
                     std::istream& input,
                     std::ostream& standard_output,
                     std::ostream& error_output,
                     States&... states)
{
    detail::console output{standard_output, error_output};
    return detail::run_with_console<Root, RootOptions>(
        argc, argv, input, output, states...);
}

template <typename Root, typename RootOptions, typename... States>
    requires detail::is_group_spec_v<Root> &&
             detail::is_root_options_v<RootOptions>
int run(int argc, char** argv, States&... states)
{
    detail::console output{
        std::cout,
        std::cerr,
        detail::standard_console_context()};
    return detail::run_with_console<Root, RootOptions>(
        argc, argv, std::cin, output, states...);
}
} // namespace rope::cli
