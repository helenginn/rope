untanglefiles = [
'ClashView.h',
'ClashView.cpp',
'NonBonds.h',
'NonBonds.cpp',
'Points.h',
'Points.cpp',
'TangledBond.h',
'TangledBond.cpp',
'UndoStack.h',
'UndoStack.cpp',
'UntangleWindow.h',
'UntangleWindow.cpp',
'UntangleView.h',
'UntangleView.cpp',
'Untangle.h',
'Untangle.cpp',
'Visual.h',
'Visual.cpp',
'Write.h',
'Write.cpp',
'main.cpp',
]

executable('untangle', untanglefiles,
    link_with : [gui, core],
    link_args: [links],
	include_directories: ['../..'],
    dependencies : gui_deps,
    install: false)
