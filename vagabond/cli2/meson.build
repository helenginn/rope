rope_cli2_runtime_sources = files(
  'runtime/Console.cpp',
  'runtime/CommandExecution.cpp',
  'runtime/CommandSpec.cpp',
  'runtime/CliRuntime.cpp',
  'runtime/HelpFormatter.cpp',
  'runtime/RuntimeNode.cpp',
  'runtime/StatePack.cpp',
  'commands/general/Version.cpp',
  'commands/test_commands/add.cpp',
)

cli2 = static_library(
  'rope_cli2',
  rope_cli2_runtime_sources,
  cpp_args: [compile_args],
  dependencies: cli_deps,
  include_directories: include_directories('../..'),
)
