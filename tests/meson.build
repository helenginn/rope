doctest_dep = dependency('doctest', required: false)
if not doctest_dep.found()
  doctest_dep = declare_dependency(
    include_directories: include_directories('../vagabond/utils', is_system: true)
  )
endif

tests_exe = executable(
  'rope_doctest',
  ['doctest_main.cpp', 'smoke_test.cpp', 'export_superposed_test.cpp'],
  link_whole: [core, vagutils, cluster4x],
  dependencies: [doctest_dep, cli_deps],
  include_directories: include_directories('..')
)

test('rope_doctest', tests_exe)
