#include <stdio.h>

#include "VagWindow.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#include <curl/curl.h>
#endif

float test = 0;

int main (int argc, char **argv)
{
#ifdef __EMSCRIPTEN__
	  EM_ASM({ Module.wasmTable = wasmTable; });
#endif
#ifndef __EMSCRIPTEN__
	curl_global_init(CURL_GLOBAL_ALL);
#endif
	{
		VagWindow window;
		window.setup(argc, argv);

#ifdef __EMSCRIPTEN__
		emscripten_set_main_loop(Window::window_tick, -1, 1);
#else
		while (window.tick())
		{
			
		}
#endif
	}

#ifndef __EMSCRIPTEN__
	curl_global_cleanup();
#endif
	
	// for leaks
//	fscanf(stdin, "c");

	return 0;
}
