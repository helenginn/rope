rope_cli2_runtime_sources = files(
  'runtime/CliRuntime.cpp',
  'commands/general/Version.cpp',
)

cli2 = static_library(
  'rope_cli2',
  rope_cli2_runtime_sources,
  cpp_args: [compile_args],
  dependencies: cli_deps,
  include_directories: include_directories('../..'),
)
