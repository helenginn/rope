c4xfiles = [
'Data.h',
'Data.cpp',
'DegreeTypedData.h',
'Cluster.h',
'Cluster.cpp',
'ClusterSVD.h',
'ClusterSVD.cpp',
'ClusterTSNE.h',
'ClusterTSNE.cpp',
'Posular.h',
'TypedData.h',
]

cluster4x = static_library('cluster4x', c4xfiles, 
	cpp_args: ['-I/usr/local/include', compile_args],
	link_args: ['-L/usr/local/lib', links],
	link_with: vagutils,
	include_directories: ['../..'],
	dependencies : deps, install: false)
