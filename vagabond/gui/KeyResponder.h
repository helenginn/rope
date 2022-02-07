#ifndef __practical__KeyResponder__
#define __practical__KeyResponder__

#include <string>
#include <SDL2/SDL.h>

class KeyResponder
{
public:
	KeyResponder() {};
	virtual ~KeyResponder() {};
	virtual void keyPressed(char key) = 0;
	virtual void keyPressed(SDL_Keycode other) = 0;
};


#endif

