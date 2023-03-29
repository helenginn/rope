boost_files = [
'boost_tests.cpp',
'test_list.cpp',
]

boost_unit_test = dependency('boost', modules: ['unit_test_framework'])

boost_test = executable('boost_test_gui_elements', boost_files, 
link_with : [vagutils, guielements],
dependencies : boost_unit_test,
 cpp_args : ['-std=c++11'], 
 include_directories: ['../../../..'])

