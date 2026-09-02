#pragma once

#include <CLI/CLI.hpp>

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include "CommandExecution.h"
#include "CommandSpec.h"
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
        : app_{std::string(Root::summary.view()),
               std::string(Root::name.view())},
          states_{states...}
    {
        runtime_.lower_root(app_, states_, plan_);
        if (runner_mode == mode::session)
        {
            CLI::App* exit = app_.add_subcommand(
                "exit", "Exit the current command session");
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
        std::ostream& output,
        std::ostream& error_output)
    {
        try
        {
            app_.parse(argc, argv);
        }
        catch (const CLI::ParseError& error)
        {
            return {
                .exit_code = app_.exit(error, output, error_output),
                .exit_requested = false};
        }
        return dispatch(output, error_output);
    }

    [[nodiscard]] execution_result execute(
        const std::string& line,
        std::ostream& output,
        std::ostream& error_output)
    {
        try
        {
            app_.parse(line);
        }
        catch (const CLI::ParseError& error)
        {
            return {
                .exit_code = app_.exit(error, output, error_output),
                .exit_requested = false};
        }
        return dispatch(output, error_output);
    }

private:
    [[nodiscard]] execution_result dispatch(std::ostream& output,
                                            std::ostream& error_output)
    {
        if (plan_.empty())
        {
            print_help(output, app_);
            return {};
        }
        command_output command_streams{output, error_output};
        return plan_.dispatch(command_streams);
    }

    CLI::App app_;
    state_pack<States...> states_;
    execution_plan plan_;
    runtime_node<Root, state_pack<States...>> runtime_;
};
} // namespace detail

template <typename Root, typename... States>
    requires detail::is_group_spec_v<Root>
int run_session_with_streams(std::istream& input,
                             std::ostream& output,
                             std::ostream& error_output,
                             bool interactive,
                             States&... states)
{
    std::string line;
    while (true)
    {
        if (interactive)
        {
            output << Root::name.view() << "> " << std::flush;
        }

        if (!std::getline(input, line))
        {
            return 0;
        }
        if (detail::ignored_session_line(line))
        {
            continue;
        }

        detail::command_runner<Root, States...> runner{
            detail::command_runner<Root, States...>::mode::session,
            states...};
        detail::execution_result result =
            runner.execute(line, output, error_output);

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
    requires detail::is_group_spec_v<Root>
int run_with_streams(int argc,
                     char** argv,
                     std::istream& input,
                     std::ostream& output,
                     std::ostream& error_output,
                     States&... states)
{
    if (argc > 1)
    {
        const std::string_view entry_mode{argv[1]};
        if (entry_mode == "shell" || entry_mode == "-")
        {
            if (argc != 2)
            {
                error_output << "'" << entry_mode
                             << "' must be used alone\n";
                return 1;
            }
            return run_session_with_streams<Root>(
                input,
                output,
                error_output,
                entry_mode == "shell",
                states...);
        }
    }

    detail::command_runner<Root, States...> runner{
        detail::command_runner<Root, States...>::mode::command_line,
        states...};
    return runner.execute(argc, argv, output, error_output).exit_code;
}

template <typename Root, typename... States>
    requires detail::is_group_spec_v<Root>
int run(int argc, char** argv, States&... states)
{
    return run_with_streams<Root>(
        argc,
        argv,
        std::cin,
        std::cout,
        std::cerr,
        states...);
}
} // namespace rope::cli
