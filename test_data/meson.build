# All unit tests in test_data
torsion_grad_exe = executable(
  'test_dihedral_gradient',
  'test_dihedral_gradient.cpp',
  '../vagabond/core/matrix_functions.cpp',
  include_directories: [
    include_directories('../vagabond/core'),
  ],
  cpp_args: ['-std=c++17']
)

test('torsion_gradient', torsion_grad_exe)
