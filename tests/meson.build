tests_exe = executable(
  'rope_doctest',
  [
    'doctest_main.cpp',
    'smoke_test.cpp',
    'force_analysis_tests.cpp',
  ],
  link_whole: [core, vagutils, cluster4x, cli2],
  dependencies: [cli_deps],
  include_directories: include_directories('..'),
)

test('rope_doctest', tests_exe)
