mabscapefiles = [
'AntigenSetup.h',
'AntigenSetup.cpp',
'CompetitionSetup.h',
'CompetitionSetup.cpp',
'MabscapeWindow.h',
'MabscapeWindow.cpp',
'MabscapeSetup.h',
'MabscapeSetup.cpp',
'main.cpp',
]

executable('mabscape', mabscapefiles,
    link_with : [gui, core],
    link_args: [links],
	include_directories: ['../..'],
    dependencies : gui_deps,
    install: false)

