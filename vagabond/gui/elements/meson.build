glm = dependency('glm', required: false)
gl = dependency('GL', method : 'system', required: false)
sdl2 = dependency('SDL2', required: false)
sdl2image = dependency('SDL2_image', required: false)

if (links == [])
  deps += [gl, sdl2, sdl2image]
endif

guielementfiles = [
'AskYesNo.cpp',
'BadChoice.cpp',
'Box.cpp',
'Button.cpp',
'Choice.cpp',
'ChoiceGroup.cpp',
'Draggable.cpp',
'HasRenderables.cpp',
'Icosahedron.cpp',
'Image.cpp',
'Library.cpp',
'ListView.cpp',
'Modal.cpp',
'Renderable.cpp',
'Scene.cpp',
'Slider.cpp',
'SnowGL.cpp',
'Text.cpp',
'TextEntry.cpp',
'TextManager.cpp',
'Window.cpp',
]

install_headers([
'AskYesNo.h',
'BadChoice.h',
'Box.h',
'Button.h',
'Choice.h',
'ChoiceGroup.h',
'Draggable.h',
'HasRenderables.h',
'Icosahedron.h',
'Image.h',
'ImageButton.h',
'Library.h',
'ListView.h',
'Modal.h',
'Renderable.h',
'Scene.h',
'Slider.h',
'SnowGL.h',
'Text.h',
'TextButton.h',
'TextEntry.h',
'TextManager.h',
'Window.h',
],
subdir : 'vagabond/gui/elements')

vag = []

if (links == []) # not emscripten

  guielements = static_library('vagguielements', guielementfiles,
  cpp_args: ['-I/usr/local/include', '-std=c++11'],
  link_args: ['-L/usr/local/lib', links],
  dependencies : deps, link_with : vag, install: true)
endif

