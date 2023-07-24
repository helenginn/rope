c4xfiles = [
'DegreeDataGroup.cpp',
'ClusterTSNE.cpp',
'Cluster.h',
'Posular.h',
'DataGroup.h',
]

cluster4x = static_library('cluster4x', c4xfiles, 
	cpp_args: ['-I/usr/local/include', '-std=c++11', compile_args],
	link_args: ['-L/usr/local/lib', links],
	link_with: vagutils,
	include_directories: ['../..'],
	dependencies : deps, install: false)
