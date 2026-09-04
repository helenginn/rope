#pragma once

#include <optional>
#include <string>
#include <vagabond/core/Environment.h>

namespace rope::cli
{
class ProjectState
{
public:
    ProjectState() : environment_{Environment::env()} {}

    [[nodiscard]] Environment& environment()
    {
        return environment_;
    }

    [[nodiscard]] const Environment& environment() const
    {
        return environment_;
    }

    [[nodiscard]] bool loaded() const
    {
        return loaded_from_.has_value();
    }

    [[nodiscard]] const std::optional<std::string>& loaded_from() const
    {
        return loaded_from_;
    }

    void mark_loaded(std::string filename)
    {
        loaded_from_ = std::move(filename);
    }

private:
    Environment& environment_;
    std::optional<std::string> loaded_from_;
};
} // namespace rope::cli
