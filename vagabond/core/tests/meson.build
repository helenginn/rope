boost_files = [
'all_tests.cpp',
]

boost_unit_test = dependency('boost', modules: ['unit_test_framework'])

boost_test = executable('boost_test_core', boost_files, 
link_with : [core],
dependencies : [boost_unit_test, deps],
 cpp_args : ['-std=c++11', '-fno-access-control'],
 include_directories: ['..', '../../../'])

