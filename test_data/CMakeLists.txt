add_executable(test_bond_rotation_on_torsion_gradient
    ${CMAKE_SOURCE_DIR}/test_data/test_bond_rotation_on_torsion_gradient.cpp
    ${CMAKE_SOURCE_DIR}/core/matrix_functions.cpp
)
target_include_directories(test_bond_rotation_on_torsion_gradient PRIVATE
    ${CMAKE_SOURCE_DIR}/core
    ${CMAKE_SOURCE_DIR}/external/glm
)
