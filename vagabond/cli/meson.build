clifiles = [
'Console.cpp',
'RopeCli.cpp',

'Console.h',
'RopeCli.h',
]

cli2 = static_library(
   'rope_cli2', clifiles,
   cpp_args: [compile_args],
   link_args: [links],
   include_directories: ['../..', '../core'],
   dependencies : [cli_deps], install: false
)