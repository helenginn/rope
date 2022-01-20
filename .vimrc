set path=src/**

command! Tags !ctags -R libgui/* libsrc/*
command! Ninja :wa|!ninja -C build/current
command! Winja :wa|!ninja -C build/website

command! Doxy !doxygen Doxyfile



