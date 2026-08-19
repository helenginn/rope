#include <algorithm>
#include <iostream>
#include <sstream>

#include "../utils/os.h"
#include "Console.h"

#ifdef OS_WINDOWS
    #include <windows.h>
#endif
#ifdef OS_UNIX
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif


Console::Console(int verbosity, bool debug)
{
    setDebug(debug);
    setVerbosity(debug_ ? static_cast<int>(Level::Debug) : verbosity);
}

void Console::print_impl(int level, std::string_view fmt, std::format_args args) const {
    if (level > verbosity_) return;
    std::string message = std::vformat(fmt, args);
    std::cout << wrap(message, width()) << '\n';
}

int Console::width() const
{
    #ifdef OS_WINDOWS
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        width_ = info.srWindow.Right - info.srWindow.Left + 1;
    }
    #endif
    #ifdef OS_UNIX
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        width_ = ws.ws_col;
    }
    #endif

    return width_;
}

std::string Console::wrap(std::string_view text, int width) {
    if (width <= 0) width = 80; // avoid pathological/zero widths

    std::string result;
    std::istringstream words{std::string(text)};
    std::string word;

    int col = 0;
    bool firstWord = true;

    while (words >> word) {
        int wordLen = static_cast<int>(word.size());

        if (!firstWord && col + 1 + wordLen > width)
        {
            result += '\n';
            col = 0;
            firstWord = true;
        }

        if (!firstWord)
        {
            result += ' ';
            ++col;
        }

        result += word;
        col += wordLen;
        firstWord = false;
    }

    return result;
}
