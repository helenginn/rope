arg_list = []

# cmtzlib / csymlib: SYMINFO-variable-independent distributions needs separate compiling
# May or may not need the maths library on various systems

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

lib_ccp4 = static_library('ccp4',
'ccp4_array.cpp',
'ccp4_general.cpp',
'ccp4_parser.cpp',
'ccp4_program.cpp',
'ccp4_unitcell.cpp',
'cmap_accessor.cpp',
'cmap_close.cpp',
'cmap_data.cpp',
'cmap_header.cpp',
'cmap_labels.cpp',
'cmap_open.cpp',
'cmap_skew.cpp',
'cmap_stats.cpp',
'cmap_symop.cpp',
'cmtzlib.cpp',
'csymlib.cpp',
'cvecmat.cpp',
'library_err.cpp',
'library_file.cpp',
'library_utils.cpp',
'pack_c.cpp',
dependencies: m_dep,
cpp_args: arg_list, link_args: arg_list, install: true)
inc_ccp4 = include_directories('.')
dep_ccp4 = declare_dependency(link_with: lib_ccp4, include_directories: inc_ccp4)
