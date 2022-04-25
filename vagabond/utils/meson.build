utilfiles = [
'FileReader.cpp',
'svd/svdcmp.cpp',
'svd/PCA.cpp',
'svd/matrix.cpp',
]

install_headers([
'FileReader.h',
'glm_import.h',
'seqalign.h',
'svd/PCA.h',
],
subdir : 'vagabond/utils')

install_headers([
'svd/PCA.h',
],
subdir : 'svd')

vagutils = static_library('vagutils', utilfiles, 
	cpp_args: ['-I/usr/local/include', '-std=c++11', compile_args],
	link_args: ['-L/usr/local/lib', links],
	dependencies : deps, install: true)

