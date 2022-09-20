clifiles = [
'CmdWorker.cpp',
'Dictator.cpp',

'CmdWorker.h',
'Dictator.h',
]

cli = static_library('rope_cli', clifiles,
	cpp_args: ['-std=c++11', compile_args],
	link_args: [links],
	dependencies : deps, 
    include_directories: ['../..', '../core'],
	link_with : [core],
	install: false)
