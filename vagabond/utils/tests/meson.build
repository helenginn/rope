
unit_tests = [
'base_filename_shortens_extension',
'base_filename_without_path_shortens_extension',
'pca_does_not_allow_fewer_rows_than_columns',
'pca_matrix_returns_same_result_as_glm_matrix',
'seqalign_correctly_aligns_sequences',
'seqalign_correctly_aligns_sequences_reversed',
'seqalign_finds_max_sequence_gap_at_start',
'seqalign_finds_max_sequence_when_alignments_inverted',
'seqalign_finds_max_sequence_gap_in_middle_of_sequence',
'seqalign_finds_zero_sequence_gap_at_start',
'seqalign_writes_sequence_gap_in_middle_of_sequence',
'seqalign_writes_gap_code_in_middle_of_sequence',
'seqalign_writes_different_gaps_in_middle_of_sequence',
'split_by_comma_leaves_dangling_strings',
'split_by_comma_starts_with_dangling_string',
]


integration_tests = [
]

foreach entry : unit_tests
  t = executable(entry, entry + '.cpp', link_with : vagutils, cpp_args : ['-std=c++11'])
  test(entry, t, suite: 'unit')
endforeach

foreach entry : integration_tests
  t = executable(entry, entry + '.cpp', link_with : vagutils, cpp_args : ['-std=c++11'])
  test(entry, t, suite: 'integration')
endforeach
