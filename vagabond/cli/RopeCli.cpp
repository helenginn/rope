#include <filesystem>

#include "RopeCli.h"
#include "../../config/config.h"
#include "../core/Environment.h"
#include "../core/FileManager.h"
#include "../core/Metadata.h"
#include "../core/Reporter.h"

// INSTRUCTIONS
// Adding a new command:
// 1. Add a new private method (e.g. `void CmdMyCommand();`)
// 2. Register the command in `registerCommands()` by calling the new method
// 3. If the command needs access to the shared Environment, call `getEnv()` inside
//    the command's callback
// 4. If the command has any options, declare the variables to hold their values
//    as shared pointers (i.e. `auto myOption = std::make_shared<std::string>();`)
//    in order to survive the scope of the callback
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

    // Files panel
    CmdLoadFiles();
    CmdLoadMetadata();

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
 * Load data files into the RoPE environment.
 */
void RopeCli::CmdLoadFiles()
{
    auto files = std::make_shared<std::vector<std::filesystem::path>>();

    auto* cmd = app_.add_subcommand(
        "load_files",
        "Load files into the RoPE environment"
    );
    cmd->group("Files");
    cmd->add_option("files", *files, "Files to load")
        ->required()
        ->expected(1, -1)
        ->check(CLI::ExistingFile);
    cmd->callback(
        [this, files]() -> void
            {
                Environment& env = getEnv();

                console.print("Loading data file(s) into the RoPE environment...");
                FileManager* fm = env.fileManager();

                for (const auto& file : *files)
                {
                    console.print("Loading file: {}", file.string());
                    if (!fm->acceptFile(file.string()))
                    {
                        console.print(Level::Error, "Failed to load file: {}", file.string());
                    }
                }
                console.print("{} file(s) currently loaded in the environment.", fm->filteredCount());
            }
        );
}

/**
 * Load metadata files into the RoPE environment.
 */
void RopeCli::CmdLoadMetadata()
{
    auto files = std::make_shared<std::vector<std::filesystem::path>>();

    auto* cmd = app_.add_subcommand(
        "load_meta",
        "Load metadata into the RoPE environment"
    );
    cmd->group("Files");
    cmd->add_option("files", *files, "Files to load")
        ->required()
        ->expected(1, -1)
        ->check(CLI::ExistingFile);
    cmd->callback(
        [this, files]() -> void
            {
                Environment& env = getEnv();

                console.print("Loading metadata file(s) into the RoPE environment...");
                for (const auto& file : *files)
                {
                    File *f = File::loadUnknown(file.string());
                    File::Type type = File::Nothing;

                    if (f)
                    {
                        type = f->cursoryLook();
                    }

                    if (type & File::Meta)
                    {
                        *env.metadata() += *f->metadata();
                        console.print("Loaded metadata from file: {}", file.string());
                    }
                    else
                    {
                        if (type == File::Nothing)
                        {
                            console.print(Level::Error, "File {} is not readable.", file.string());
                        }
                        else
                        {
                            console.print(Level::Error, "File {} is not a metadata file. Detected type: {}",
                                file.string(), static_cast<int>(type));
                        }
                    }
                }
            }
        );
};

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
