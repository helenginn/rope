
#ifndef __vagabond__Display__
#define __vagabond__Display__

#include "Scene.h"

class GuiAtom;

class Display : public Scene
{
public:
	Display(Scene *prev = NULL);
	~Display();

	virtual void setup();
private:
	GuiAtom *_atoms;

};

#endif
