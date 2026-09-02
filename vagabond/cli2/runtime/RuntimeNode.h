#pragma once

#include <CLI/CLI.hpp>

#include <concepts>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "CommandExecution.h"
#include "CommandSpec.h"
#include "StatePack.h"

namespace rope::cli::detail
{
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

        [[nodiscard]] execution_result dispatch(command_output& output) override
        {
            return runtime_node::invoke(
                storage_, *states_, output,
                std::index_sequence_for<Bindings...>{});
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
    static execution_result invoke(storage_type& storage,
                                   StatePack& states,
                                   command_output& output,
                                   std::index_sequence<Indices...>)
    {
        auto result = invoke_command(
            Meta.handler,
            project_binding<Bindings, Indices>(storage, states)...);
        return present_command_result(std::move(result), output);
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
} // namespace rope::cli::detail
