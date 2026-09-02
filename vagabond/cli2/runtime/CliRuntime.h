#pragma once

#include <CLI/CLI.hpp>

#include <array>
#include <concepts>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <istream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "CommandSpec.h"

namespace rope::cli
{
namespace detail
{
template <typename T>
concept printable = requires(std::ostream& output, const T& value)
{
    output << value;
};

inline void print_help(std::ostream& output, const CLI::App& app)
{
    std::string help = app.help();
    output << help;
    if (help.empty() || help.back() != '\n')
    {
        output << '\n';
    }
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

inline void print_local_help(std::ostream& output, const CLI::App& app)
{
    std::string help = app.get_formatter()->make_help(
        &app, command_path(app), CLI::AppFormatMode::Normal);
    output << help;
    if (help.empty() || help.back() != '\n')
    {
        output << '\n';
    }
}

struct execution_result
{
    int exit_code = 0;
    bool exit_requested = false;
};

class planned_invocation
{
public:
    virtual ~planned_invocation() = default;
    [[nodiscard]] virtual execution_result dispatch(std::ostream& output) = 0;
};

class help_invocation final : public planned_invocation
{
public:
    explicit help_invocation(const CLI::App& app)
        : app_{&app}
    {}

    [[nodiscard]] execution_result dispatch(std::ostream& output) override
    {
        print_local_help(output, *app_);
        return {};
    }

private:
    const CLI::App* app_;
};

class exit_invocation final : public planned_invocation
{
public:
    [[nodiscard]] execution_result dispatch(std::ostream&) override
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

    [[nodiscard]] execution_result dispatch(std::ostream& output)
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

struct no_binding_storage
{};

template <typename Binding, bool = is_value_binding_v<Binding>>
struct binding_storage
{
    using type = no_binding_storage;
};

template <typename Binding>
struct binding_storage<Binding, true>
{
    using type = typename Binding::value_type;
};

template <typename Binding>
using binding_storage_t = typename binding_storage<Binding>::type;

template <typename Spec, typename StatePack>
class runtime_node;

template <command_meta Meta,
          typename StatePack,
          typename... Bindings>
class runtime_node<command<Meta, Bindings...>, StatePack>
{
public:
    runtime_node() = default;

    CLI::App* lower(CLI::App& parent,
                    StatePack& states,
        execution_plan& plan)
    {
        app_ = parent.add_subcommand(
            std::string(Meta.name.view()),
            std::string(Meta.description.view()));
        states_ = &states;
        plan_ = &plan;
        lower_bindings(std::index_sequence_for<Bindings...>{});
        app_->parse_complete_callback([this]() { snapshot_invocation(); });
        app_->immediate_callback();
        return app_;
    }

private:
    using storage_type = std::tuple<binding_storage_t<Bindings>...>;

    static_assert((std::default_initializable<binding_storage_t<Bindings>> && ...),
                  "CLI value types must be default-initializable in this slice");
    static_assert(std::is_move_assignable_v<storage_type>,
                  "CLI value storage must be move-assignable in this slice");

    class invocation final : public planned_invocation
    {
    public:
        invocation(StatePack& states, storage_type storage)
            : states_{&states}, storage_{std::move(storage)}
        {}

        [[nodiscard]] execution_result dispatch(std::ostream& output) override
        {
            return {
                .exit_code = runtime_node::invoke(
                    storage_, *states_, output,
                    std::index_sequence_for<Bindings...>{}),
                .exit_requested = false};
        }

    private:
        StatePack* states_;
        storage_type storage_;
    };

    template <typename Binding>
    [[nodiscard]] static std::string option_names()
    {
        std::string names;
        if constexpr (Binding::short_name != '\0')
        {
            names = "-";
            names.push_back(Binding::short_name);
            names += ",";
        }
        names += "--";
        names += Binding::name.view();
        return names;
    }

    template <typename Binding, typename Storage>
    void lower_binding(Storage& storage)
    {
        if constexpr (is_positional_v<Binding>)
        {
            app_->add_option(
                    std::string(Binding::name.view()),
                    storage,
                    std::string(Binding::description.view()))
                ->required();
        }
        else if constexpr (is_option_v<Binding>)
        {
            using parsed_type = typename Binding::parsed_type;
            app_->template add_option_function<parsed_type>(
                option_names<Binding>(),
                [&storage](const parsed_type& value)
                {
                    storage = value;
                },
                std::string(Binding::description.view()));
        }
        else if constexpr (is_required_option_v<Binding>)
        {
            app_->add_option(
                    option_names<Binding>(),
                    storage,
                    std::string(Binding::description.view()))
                ->required();
        }
        else if constexpr (is_flag_v<Binding>)
        {
            app_->add_flag(
                option_names<Binding>(),
                storage,
                std::string(Binding::description.view()));
        }
    }

    template <std::size_t... Indices>
    void lower_bindings(std::index_sequence<Indices...>)
    {
        (lower_binding<Bindings>(std::get<Indices>(storage_)), ...);
    }

    void snapshot_invocation()
    {
        plan_->template add<invocation>(*states_, std::move(storage_));
        storage_ = storage_type{};
    }

    template <typename Binding, std::size_t Index>
    static decltype(auto) project_binding(storage_type& storage,
                                          StatePack& states)
    {
        if constexpr (is_value_binding_v<Binding>)
        {
            return std::move(std::get<Index>(storage));
        }
        else if constexpr (is_read_state_v<Binding>)
        {
            return states.template read<typename Binding::state_type>();
        }
        else
        {
            static_assert(is_mutate_state_v<Binding>);
            return states.template mutate<typename Binding::state_type>();
        }
    }

    template <std::size_t... Indices>
    static int invoke(storage_type& storage,
                      StatePack& states,
                      std::ostream& output,
                      std::index_sequence<Indices...>)
    {
        using return_type =
            typename function_traits<decltype(Meta.handler)>::return_type;

        if constexpr (std::is_void_v<return_type>)
        {
            std::invoke(
                Meta.handler,
                project_binding<Bindings, Indices>(storage, states)...);
        }
        else
        {
            static_assert(printable<return_type>,
                          "a non-void command result must be printable");
            output << std::invoke(
                          Meta.handler,
                          project_binding<Bindings, Indices>(storage, states)...)
                   << '\n';
        }
        return 0;
    }

    CLI::App* app_ = nullptr;
    StatePack* states_ = nullptr;
    execution_plan* plan_ = nullptr;
    storage_type storage_{};
};

template <help_command_meta Meta, typename StatePack>
class runtime_node<help_command<Meta>, StatePack>
{
public:
    runtime_node() = default;

    CLI::App* lower(CLI::App& parent,
                    StatePack&,
                    execution_plan& plan)
    {
        parent_ = &parent;
        plan_ = &plan;
        app_ = parent.add_subcommand(
            std::string(Meta.name.view()),
            std::string(Meta.description.view()));
        app_->parse_complete_callback(
            [this]()
            {
                plan_->template add<help_invocation>(*parent_);
            });
        app_->immediate_callback();
        return app_;
    }

private:
    CLI::App* app_ = nullptr;
    CLI::App* parent_ = nullptr;
    execution_plan* plan_ = nullptr;
};

template <fixed_string Title,
          typename StatePack,
          typename... Children>
class runtime_node<help_group<Title, Children...>, StatePack>
{
public:
    runtime_node() = default;

    void lower(CLI::App& parent,
               StatePack& states,
               execution_plan& plan)
    {
        std::apply(
            [&parent, &states, &plan](auto&... child)
            {
                (child.lower(parent, states, plan)
                     ->group(std::string(Title.view())),
                 ...);
            },
            children_);
    }

private:
    std::tuple<runtime_node<Children, StatePack>...> children_{};
};

template <fixed_string Name,
          fixed_string Summary,
          typename StatePack,
          typename... Children>
class runtime_node<group<Name, Summary, Children...>, StatePack>
{
public:
    runtime_node() = default;

    CLI::App* lower(CLI::App& parent,
                    StatePack& states,
                    execution_plan& plan)
    {
        app_ = parent.add_subcommand(
            std::string(Name.view()), std::string(Summary.view()));
        states_ = &states;
        plan_ = &plan;
        configure_group();
        lower_children();
        app_->parse_complete_callback(
            [this]()
            {
                if (app_->get_subcommands().empty())
                {
                    plan_->template add<help_invocation>(*app_);
                }
            });
        app_->immediate_callback();
        return app_;
    }

    void lower_root(CLI::App& root,
                    StatePack& states,
                    execution_plan& plan)
    {
        app_ = &root;
        states_ = &states;
        plan_ = &plan;
        configure_group();
        lower_children();
    }

private:
    void configure_group()
    {
        app_->require_subcommand(0, 0);
        app_->subcommand_fallthrough(true);
    }

    void lower_children()
    {
        std::apply(
            [this](auto&... child)
            {
                (child.lower(*app_, *states_, *plan_), ...);
            },
            children_);
    }

    CLI::App* app_ = nullptr;
    StatePack* states_ = nullptr;
    execution_plan* plan_ = nullptr;
    std::tuple<runtime_node<Children, StatePack>...> children_{};
};

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
        return dispatch(output);
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
        return dispatch(output);
    }

private:
    [[nodiscard]] execution_result dispatch(std::ostream& output)
    {
        if (plan_.empty())
        {
            print_help(output, app_);
            return {};
        }
        return plan_.dispatch(output);
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
