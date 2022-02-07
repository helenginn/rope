
unit_tests = [
'guiatom_initialised_and_pushed_to_gui_is_renderable',
'guiatom_initialised_with_nitrogen_is_blue',
'guiatom_initialised_with_oxygen_is_red',
'guiatom_initialised_with_sulphur_is_yellow',
'guiatom_changes_atom_location_when_updated',
'guiatom_does_not_accept_anumeric_positions',
'guiatom_increments_vertex_array_by_number',
'guiatom_vertices_per_atom_above_zero',
'guiatom_can_use_projection',
'box_does_not_use_projection',
'button_highlights_when_mouse_over',
'button_without_sender_does_nothing',
'empty_renderable_envelope_radius_is_zero',
'filemanager_accepts_files_when_fileview_not_assigned',
'inert_button_does_nothing_when_mouse_over',
'renderable_centroid_is_average_position',
'renderable_does_not_add_same_object_twice',
'renderable_envelope_radius_is_most_maximal',
'renderable_returns_vertex_count_and_size',
]

integration_tests = [
]

foreach entry : unit_tests
  t = executable(entry, entry + '.cpp', dependencies : deps, 
link_with : gui, cpp_args : ['-std=c++11', '-I../../'])
  test(entry, t, suite: 'unit')
endforeach

foreach entry : integration_tests
  t = executable(entry, entry + '.cpp', link_with : gui, cpp_args : ['-std=c++11'])
  test(entry, t, suite: 'integration')
endforeach
