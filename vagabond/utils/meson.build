utilfiles = [
'Canonical.cpp',
'FileReader.cpp',
'svd/svdcmp.cpp',
'svd/PCA.cpp',
'svd/matrix.cpp',
'maths.cpp',
]

install_headers([
'Canonical.h',
'FileReader.h',
'glm_import.h',
'glm_json.h',
'seqalign.h',
'maths.h',
'version.h',
'svd/PCA.h',
],
subdir : 'vagabond/utils')

vagutils = static_library('vagutils', utilfiles, 
	cpp_args: ['-I/usr/local/include', '-std=c++11', compile_args],
	link_args: [links],
	dependencies : deps, install: false)

