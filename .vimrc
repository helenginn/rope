set path=vagabond/core/**,vagabond/gui/**,vagabond/gui/elements/**,vagabond/utils/**,vagabond/cmd/**,vagabond/c4x/**,

command! Tags !ctags -R vagabond/core/* vagabond/gui/* vagabond/utils/*

command! Ninja :wa|!ninja -C build/current vagabond.gui
command! Dinja :wa|!ninja -C build/debug vagabond.gui
command! Winja :wa|!ninja -C build/website

command! Unit :wa|!ninja -C build/current vagabond/core/tests/boost_test_core &&  build/current/vagabond/core/tests/boost_test_core; ninja -C build/current vagabond/utils/tests/boost_test_utils &&  build/current/vagabond/utils/tests/boost_test_utils; 
command! Ge :wa|!ninja -C build/current vagabond/gui/elements/tests/boost_test_gui_elements &&  build/current/vagabond/gui/elements/tests/boost_test_gui_elements;
command! Dunit :wa|!cd build/debug; meson test --suite=unit
command! Integration :wa|!cd build/current; meson test --suite=integration
command! Dintegration :wa|!cd build/debug; meson test --suite=integration

" `u to compile unit tests
:imap `u :Tinja
:nmap `u :Tinja

:imap `N :AsyncRun ninja -C build/current vagabond.gui
:nmap `N :AsyncRun ninja -C build/current vagabond.gui

:imap `D :AsyncRun ninja -C build/debug vagabond.gui
:nmap `D :AsyncRun ninja -C build/debug vagabond.gui

command! Doxy !doxygen Doxyfile

function! MakeTest()
	let testname = input("name your test: ")
	:execute "!./make_test " . testname
	:execute "edit " . fnameescape("tests/" . testname . ".cpp")
endfunction

command Test call MakeTest()
