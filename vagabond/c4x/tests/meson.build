
unit_tests = [
'cluster_can_svd_on_datagroup',
'datagroup_calculates_average',
'datagroup_calculates_correlation_matrix',
'datagroup_calculates_distance_matrix',
'datagroup_calculates_differences',
'datagroup_normalises_units',
]

integration_tests = [
]

foreach entry : unit_tests
  t = executable(entry, entry + '.cpp', link_with : [cluster4x],
 cpp_args : ['-I/usr/local/include/vaginclude', '-std=c++11'], 
 include_directories: '../../../')
  test(entry, t, suite: 'unit')
endforeach

foreach entry : integration_tests
  t = executable(entry, entry + '.cpp', link_with : [cluster4x],
	  cpp_args : ['-I/usr/local/include/vaginclude', '-std=c++11'], 
	  include_directories: '../../../')
  test(entry, t, suite: 'integration')
endforeach
