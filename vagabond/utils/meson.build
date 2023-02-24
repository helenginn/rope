utilfiles = [
'Canonical.cpp',
'FileReader.cpp',
'LookupTable.cpp',
'svd/svdcmp.cpp',
'svd/PCA.cpp',
'svd/matrix.cpp',
'ProbDist.cpp',
'Hypersphere.cpp',
'maths.cpp',
'version.h',
'polyfit.h',
'Hypersphere.h',
'LookupTable.h',
'ProbDist.h',
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

