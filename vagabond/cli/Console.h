#ifndef ROPE_CONSOLE_H
#define ROPE_CONSOLE_H

#include <format>
#include <string>

enum class Level : int
{
    Error   = -2,
    Warn    = -1,
    Info    =  0,
    Verbose =  1,
    Trace   =  2,
    Debug   =  3,
};

class Console
{
public:
    explicit Console(int verbosity = 0, bool debug = false);
    int verbosity() const;
    int width() const;

    template<typename... Args>
    void print(int level, std::format_string<Args...> fmt, Args&&... args) const
    {
        print_impl(level, fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args>
    void print(Level level, std::format_string<Args...> fmt, Args&&... args) const
    {
        print(static_cast<int>(level), fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void print(std::format_string<Args...> fmt, Args&&... args) const
    {
        print(static_cast<int>(Level::Info), fmt, std::forward<Args>(args)...);
    }

private:
    int verbosity_;
    bool debug_;

    mutable int width_ = 80; // Default width for formatting

    void print_impl(int level, std::string_view fmt, std::format_args args) const;
    static std::string wrap(std::string_view text, int width);
};


#endif //ROPE_CONSOLE_H
