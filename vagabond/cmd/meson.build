clifiles = [
'CmdWorker.cpp',
'Socket.cpp',
'Dictator.cpp',

'CmdWorker.h',
'Socket.h',
'Dictator.h',
]

cli = static_library('rope_cli', clifiles,
	cpp_args: [compile_args],
	link_args: [links],
	dependencies : deps, 
    include_directories: ['../..', '../core'],
	link_with : [core],
	install: false)
