arg_list = []

#if (host_machine.system() == 'darwin')
#  arg_list += '-mmacosx-version-min=10.14'
#endif

add_global_arguments(arg_list, language : ['cpp', 'c'])
add_global_arguments('-std=c++11', language : ['cpp'])


# cmtzlib / csymlib: SYMINFO-variable-independent distributions needs separate compiling
# May or may not need the maths library on various systems

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

lib_ccp4 = static_library('ccp4',
'ccp4_array.c',
'ccp4_general.c',
'ccp4_parser.c',
'ccp4_program.c',
'ccp4_unitcell.c',
'cmap_accessor.c',
'cmap_close.c',
'cmap_data.c',
'cmap_header.c',
'cmap_labels.c',
'cmap_open.c',
'cmap_skew.c',
'cmap_stats.c',
'cmap_symop.c',
'cmtzlib.c',
'csymlib.c',
'cvecmat.c',
'library_err.c',
'library_file.c',
'library_utils.c',
'pack_c.c',
dependencies: m_dep,
cpp_args: arg_list, link_args: arg_list, install: true)
inc_ccp4 = include_directories('.')
dep_ccp4 = declare_dependency(link_with: lib_ccp4, include_directories: inc_ccp4)
