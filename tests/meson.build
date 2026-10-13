doctest_dep = dependency('doctest', required: true)

tests_exe = executable(
  'rope_doctest',
  ['doctest_main.cpp', 'smoke_test.cpp', 'occupancy_propagation_test.cpp',
   'hbond_electrostatics_test.cpp', 'planar_acceptance_test.cpp',
   'hydrogen_placement_test.cpp', 'energy_tally_test.cpp',
   'count_helpers_test.cpp', 'path_groups_test.cpp'],
  link_whole: [core, vagutils, cluster4x],
  dependencies: [doctest_dep, cli_deps],
  # core's own headers (e.g. protonic/hnet.h) include siblings by bare name,
  # which only resolves from vagabond/core itself
  include_directories: include_directories('..', '../vagabond/core'),
)

test('rope_doctest', tests_exe)
