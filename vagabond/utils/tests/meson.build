boost_files = [
'test_hypersphere.cpp',
'test_lookuptable.cpp',
'test_brain_layers.cpp',
'test_simplex.cpp',
]

boost_unit_test = dependency('boost', modules: ['unit_test_framework'])

utils_test = executable('boost_test_utils', boost_files, 
link_with : [vagutils],
dependencies : boost_unit_test,
 cpp_args : ['-I/usr/local/include/vaginclude', '-std=c++11'], 
 include_directories: '../../../')
