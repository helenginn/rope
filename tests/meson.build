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

test('rope_doctest', tests_exe, args: ['--test-suite-exclude=force-e2e'])

test(
  'force-e2e',
  tests_exe,
  args: ['--test-suite=force-e2e'],
  suite: 'local',
  workdir: meson.current_build_dir(),
  env: {
    'ROPE_DATA_DIR': meson.project_source_root(),
    'ROPE_FORCES_FIXTURES': join_paths(
      meson.project_source_root(),
      'external',
      'reference_geometry',
      ),
    },
  )

add_test_setup(
  'normal',
  is_default: true,
  exclude_suites: ['local'],
)
