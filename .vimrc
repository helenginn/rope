set path=src/**

command! Tags !ctags -R libgui/* libsrc/*
command! Ninja :wa|!ninja -C build/current vagabond
command! Winja :wa|!ninja -C build/website
command! Winja :wa|!ninja -C build/website

command! Doxy !doxygen Doxyfile

function! MakeTest()
	let testname = input("name your test: ")
	:execute "!./make_test " . testname
	:execute "edit " . fnameescape("tests/" . testname . ".cpp")
endfunction

command Test call MakeTest()
