//
// Created by dtriand on 19/08/2026.
//

#ifndef ROPE_ROPECLI_H
#define ROPE_ROPECLI_H

#include <CLI/CLI.hpp>

#include "../core/Environment.h"
#include "Console.h"

class RopeCli
{
public:
    RopeCli();
    Console console{};
    int run(int argc, char** argv);

private:
    /** CLI11 application instance **/
    CLI::App app_{};
    void registerGlobalOptions();
    void registerCommands();
    int runBatchMode();
    static std::string version();

    // Environment management
    /** The path to the environment file (rope.json) (if specified) **/
    std::filesystem::path env_file_;
    /** Track whether the Environment has been loaded **/
    bool env_loaded_ = false;
    Environment& getEnv();

    // Command registration methods
    // General panel
    void CmdVersion();

    // Report panel
    void CmdReport();
};


#endif //ROPE_ROPECLI_H
