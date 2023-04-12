boost_files = [
'boost_tests.cpp',
'test_surface.cpp',
'test_sequence.cpp',
'test_molecule.cpp',
'test_atomgroup.cpp',
'test_molrefiner.cpp',
'test_ramachandran.cpp',
'test_atomsfromsequence.cpp',
]

boost_unit_test = dependency('boost', modules: ['unit_test_framework'])

boost_test = executable('boost_test_core', boost_files, 
link_with : [core],
dependencies : boost_unit_test,
 cpp_args : ['-I/usr/local/include/vaginclude', '-std=c++11'], 
 include_directories: ['..', '../../../'])

