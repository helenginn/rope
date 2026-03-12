mabscapefiles = [
'AntigenSetup.h',
'AntigenSetup.cpp',
'AssignChains.h',
'AssignChains.cpp',
'CompetitionSetup.h',
'CompetitionSetup.cpp',
'FiducialSetup.h',
'FiducialSetup.cpp',
'Mab.h',
'Mab.cpp',
'MabscapeWindow.h',
'MabscapeWindow.cpp',
'MabscapeSetup.h',
'MabscapeSetup.cpp',
'Mesh.h',
'Mesh.cpp',
'PrepWorkView.h',
'PrepWorkView.cpp',
'main.cpp',
]

executable('mabscape', mabscapefiles,
    link_with : [gui, core],
    link_args: [links],
	include_directories: ['../..'],
    dependencies : gui_deps,
    install: false)

