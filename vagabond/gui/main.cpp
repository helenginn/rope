#include <stdio.h>

#include "Window.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

float test = 0;

int main (int argc, char **argv)
{
	Window window(argc, argv);

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(Window::tick, -1, 1);
#else
	while (true)
	{
		window.tick();
	}
#endif

	return 0;
}
