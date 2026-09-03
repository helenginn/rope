#pragma once

#include <CLI/CLI.hpp>

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "CommandExecution.h"
#include "CommandSpec.h"
#include "HelpFormatter.h"
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

struct runtime_arguments
{
    std::vector<char*> values;
    bool plain = false;
};

[[nodiscard]] inline runtime_arguments extract_runtime_arguments(
    int argc,
    char** argv)
{
    runtime_arguments arguments;
    if (argc > 0)
    {
        arguments.values.reserve(static_cast<std::size_t>(argc));
        arguments.values.push_back(argv[0]);
    }

    bool options_enabled = true;
    for (int index = 1; index < argc; ++index)
    {
        const std::string_view argument{argv[index]};
        if (options_enabled && argument == "--plain")
        {
            arguments.plain = true;
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

template <typename Root, typename... States>
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
        runtime_.lower_root(app_, states_, plan_);
        if (runner_mode == mode::session)
        {
            CLI::App* exit = app_.add_subcommand(
                "exit", "Exit the current command session");
            exit->group("Commands");
            exit->parse_complete_callback(
                [this]() { plan_.template add<exit_invocation>(); });
            exit->immediate_callback();
        }
        else
        {
            app_.add_flag(
                "--plain", "Print command results without terminal formatting");
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
        try
        {
            app_.parse(argc, argv);
        }
        catch (const CLI::ParseError& error)
        {
            return present_parse_error(app_, error, output);
        }
        return dispatch(output);
    }

    [[nodiscard]] execution_result execute(
        const std::string& line,
        console& output)
    {
        try
        {
            app_.parse(line);
        }
        catch (const CLI::ParseError& error)
        {
            return present_parse_error(app_, error, output);
        }
        return dispatch(output);
    }

private:
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

        command_runner<Root, States...> runner{
            command_runner<Root, States...>::mode::session,
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

template <typename Root, typename... States>
    requires is_group_spec_v<Root>
int run_with_console(int argc,
                     char** argv,
                     std::istream& input,
                     console& output,
                     States&... states)
{
    runtime_arguments arguments = extract_runtime_arguments(argc, argv);
    if (arguments.plain)
    {
        output.set_style(console_style::plain);
    }

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

    command_runner<Root, States...> runner{
        command_runner<Root, States...>::mode::command_line,
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

template <typename Root, typename... States>
    requires detail::is_group_spec_v<Root>
int run_with_streams(int argc,
                     char** argv,
                     std::istream& input,
                     std::ostream& standard_output,
                     std::ostream& error_output,
                     States&... states)
{
    detail::console output{standard_output, error_output};
    return detail::run_with_console<Root>(
        argc, argv, input, output, states...);
}

template <typename Root, typename... States>
    requires detail::is_group_spec_v<Root>
int run(int argc, char** argv, States&... states)
{
    detail::console output{
        std::cout,
        std::cerr,
        detail::standard_console_context()};
    return detail::run_with_console<Root>(
        argc, argv, std::cin, output, states...);
}
} // namespace rope::cli
