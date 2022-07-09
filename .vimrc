set path=vagabond/core/**,vagabond/gui/**,vagabond/gui/elements/**,vagabond/utils/**,vagabond/cmd/**,vagabond/c4x/**,

command! Tags !ctags -R vagabond/core/* vagabond/gui/*
command! Ninja :wa|!ninja -C build/current vagabond.gui
command! Dinja :wa|!ninja -C build/debug vagabond.gui
command! Winja :wa|!ninja -C build/website
command! Unit :wa|!cd build/current; meson test --suite=unit
command! Dunit :wa|!cd build/debug; meson test --suite=unit
command! Integration :wa|!cd build/current; meson test --suite=integration
command! Dintegration :wa|!cd build/debug; meson test --suite=integration

" `u to compile unit tests
:imap `u :Tinja
:nmap `u :Tinja

:imap `N :AsyncRun ninja -C build/current vagabond.gui
:nmap `N :AsyncRun ninja -C build/current vagabond.gui

command! Doxy !doxygen Doxyfile

function! MakeTest()
	let testname = input("name your test: ")
	:execute "!./make_test " . testname
	:execute "edit " . fnameescape("tests/" . testname . ".cpp")
endfunction

command Test call MakeTest()
