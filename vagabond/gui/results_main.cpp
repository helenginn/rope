#include <stdio.h>

#include "VagWindow.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

float test = 0;

int main (int argc, char **argv)
{
	{
		VagWindow window;
		window.setup_special();

#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(Window::window_tick, -1, 1);
#else
		while (window.tick())
		{
			
		}
#endif
	}

//	fscanf(stdin, "c");

	return 0;
}
