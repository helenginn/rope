guielementfiles = [
cfile,
'AskYesNo.cpp',
'AskForText.cpp',
'AskMultipleChoice.cpp',
'AskMultipleChoice.h',
'AlterFloatView.cpp',
'AlterFloatView.h',
'BadChoice.cpp',
'Box.cpp',
'Button.cpp',
'ButtonResponder.cpp',
'ChooseRange.cpp',
'Draggable.cpp',
'FloatingImage.h',
'FloatingImage.cpp',
'FloatingText.h',
'FloatingText.cpp',
'ForwardBackward.cpp',
'HasRenderables.cpp',
'Icosahedron.cpp',
'IndexResponseView.cpp',
'Image.cpp',
'Library.cpp',
'Line.cpp',
'Line.h',
'ListView.cpp',
'list/ItemLine.cpp',
'list/LineGroup.cpp',
'Menu.cpp',
'Modal.cpp',
'Mouse3D.cpp',
'Mouse2D.cpp',
'Mouse2D.h',
'Quad.cpp',
'Renderable.cpp',
'Scene.cpp',
'ScrollBox.cpp',
'ScrollBox.h',
'SelectionBox.cpp',
'Slider.cpp',
'SnowGL.cpp',
'Text.cpp',
'TextEntry.cpp',
'TextManager.cpp',
'ThickLine.cpp',
'ThickLine.h',
'TickBoxes.cpp',
'Window.cpp',

'ShaderGetsVertices.cpp',
'ShaderGetsVertices.h',
'ShaderGetsPrimitives.cpp',
'ShaderGetsPrimitives.h',
'ShaderGets.h',

'AskYesNo.h',
'AskForText.h',
'BadChoice.h',
'Box.h',
'Button.h',
'ButtonResponder.h',
'canvas.h',
'Draggable.h',
'ForwardBackward.h',
'HasRenderables.h',
'KeyResponder.h',
'Icosahedron.h',
'Image.h',
'ImageButton.h',
'IndexResponseView.h',
'Library.h',
'ListView.h',
'list/ItemLine.h',
'list/LineGroup.h',
'Modal.h',
'Mouse3D.h',
'Renderable.h',
'Scene.h',
'Slider.h',
'SnowGL.h',
'Text.h',
'TextButton.h',
'TextEntry.h',
'TextManager.h',
'TickBoxes.h',
'upload.h',
'Window.h',
]

install_headers([
#'AskYesNo.h',
#'AskForText.h',
#'BadChoice.h',
#'Box.h',
#'Button.h',
#'ButtonResponder.h',
#'canvas.h',
#'Choice.h',
#'ChoiceGroup.h',
#'Draggable.h',
#'ForwardBackward.h',
#'HasRenderables.h',
#'KeyResponder.h',
#'Icosahedron.h',
#'Image.h',
#'ImageButton.h',
#'IndexResponseView.h',
#'Library.h',
#'ListView.h',
#'Modal.h',
#'Mouse3D.h',
#'Renderable.h',
#'Scene.h',
#'Slider.h',
#'SnowGL.h',
#'Text.h',
#'TextButton.h',
#'TextEntry.h',
#'TextManager.h',
#'upload.h',
'Window.h',
],
subdir : 'vagabond/gui/elements')

pkg = import('pkgconfig')

guielements = static_library('vagguielements', guielementfiles,
	cpp_args: [compile_args],
	include_directories: ['../../..'],
	link_args: [links],
	dependencies : [header_deps, graphics_deps], install: true)

pkg.generate([guielements],
             subdirs : [''],
             version : '1.0',
             name : 'guielements',
             filebase : 'guielements',
             description : 'Library for helen guielements')
