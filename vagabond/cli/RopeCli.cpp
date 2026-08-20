#include <filesystem>

#include "RopeCli.h"
#include "../../config/config.h"
#include "../core/Environment.h"
#include "../core/Reporter.h"

// INSTRUCTIONS
// Adding a new command:
// 1. Add a new private method (e.g. `void CmdMyCommand();`)
// 2. Register the command in `registerCommands()` by calling the new method
// 3. If the command needs access to the shared Environment, call `getEnv()` inside
//    the command's callback
/**
 * A command-line interface with shared state and chained subcommands for RoPE
 */
RopeCli::RopeCli()
{
    app_.description("Representation of Protein Entities (RoPE) v." + version());
    registerGlobalOptions();
    registerCommands();
}

/**
 * Runs the RopeCli application with the provided command-line arguments.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The exit code of the application.
 */
int RopeCli::run(int argc, char** argv)
{
    try
    {
        app_.parse(argc, argv);
    }
    catch (const CLI::ParseError& e)
    {
        return app_.exit(e);
    }

    // Exit normally if no subcommands were provided
    if (!app_.get_subcommands().empty()) return 0;
    // Otherwise proceed to batch mode over stdin
    return runBatchMode();
}

/**
 * Registers the global options for the RopeCli application.
 */
void RopeCli::registerGlobalOptions()
{
    // Any options defined here should be mapped to variables that are
    // accessible from the RopeCli scope
    app_.add_flag_function(
        "-v,--verbose",
        [this](std::int64_t v) -> void
            { console.setVerbosity(static_cast<int>(v)); },
        "Print additional information"
    );

    app_.add_flag_function(
        "-d,--debug",
        [this](bool d) -> void
            { console.setDebug(d); },
        "Print debug information"
    );

    app_.add_option(
        "-e,--env",
        env_file_,
        "Path to rope.json file"
    )->check(CLI::ExistingFile);
}

/**
 * Registers the commands for the RopeCli application.
 */
void RopeCli::registerCommands()
{
    // General panel
    CmdVersion();

    // Report panel
    CmdReport();
}

/**
 * Runs the RopeCli application in batch mode, reading commands from stdin.
 *
 * @return The exit code of the application.
 */
int RopeCli::runBatchMode()
{
    std::string line;
    int lineNo = 0;
    while (std::getline(std::cin, line)) {
        ++lineNo;
        auto first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;

        app_.clear(); // reset parsed values/subcommand state before reparsing
        try {
            app_.parse(line);
        } catch (const CLI::ParseError& e) {
            // Fail fast
            console.print(Level::Error, "Error on line {}: \"{}\"", lineNo, line);
            return app_.exit(e); // fail fast
        } catch (const std::runtime_error& e) {
            console.print(Level::Error, "Error on line {}: {}", lineNo, e.what());
            return 1;
        }
    }
    return 0;
}

/**
 * Returns the version of the RoPE application.
 * If the version is not defined, it returns "UNK" (unknown).
 *
 * @return The version string of the RoPE application.
 */
std::string RopeCli::version()
{
    if (ROPE_VERSION != nullptr) {
        return std::string(ROPE_VERSION);
    }
    return "UNK";
}

/**
 * Returns a reference to the Environment singleton, loading it from the specified file if necessary.
 * If an environment file is not specified, it prints an error message and exits the program.
 *
 * @return Reference to the Environment object.
 */
Environment& RopeCli::getEnv()
{
    if (env_loaded_) return Environment::env();

    if (env_file_.empty())
    {
        console.print(Level::Error, "No environment file specified. "
                                    "Use -e or --env to specify the path to rope.json.");
        exit(1);
    }

    console.print("Loading environment: {}",
                  std::filesystem::absolute(env_file_).string());
    Environment& env = Environment::env();
    env.load(env_file_.string());
    env_loaded_ = true;
    return env;
}

/**
 * Get the version of RoPE
 */
void RopeCli::CmdVersion()
{
    auto* cmd = app_.add_subcommand(
        "version",
        "Print version info"
    );
    cmd->group("General");
    cmd->callback(
        [this]() -> void
            { console.print("RoPE {}", version()); }
        );
}

/**
 * Report various statistics on a RoPE environment.
 */
void RopeCli::CmdReport()
{
    auto* cmd = app_.add_subcommand(
        "report",
        "Report various statistics on the existing environment. Useful for debugging."
    );
    cmd->group("Report");
    cmd->callback(
        [this]() -> void
        {
            Environment& env = getEnv();
            Reporter reporter;
            reporter.report();
        }
        );
}
