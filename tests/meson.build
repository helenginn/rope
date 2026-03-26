doctest_dep = dependency('doctest', required: true)

tests_exe = executable(
  'rope_doctest',
  ['doctest_main.cpp', 'smoke_test.cpp'],
  link_whole: [core, vagutils, cluster4x],
  dependencies: [doctest_dep],
  include_directories: include_directories('..'),
)

test('rope_doctest', tests_exe)
