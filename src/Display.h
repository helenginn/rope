
#ifndef __vagabond__Display__
#define __vagabond__Display__

#include "Scene.h"

class GuiAtom;
class AtomGroup;

class Display : public Scene
{
public:
	Display(Scene *prev = NULL);
	~Display();

	virtual void mouseMoveEvent(double x, double y);
	virtual void mousePressEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void mouseReleaseEvent(double x, double y, SDL_MouseButtonEvent button);
	virtual void buttonPressed(std::string tag, Button *button);

	virtual void keyReleaseEvent(SDL_Keycode pressed);
	virtual void keyPressEvent(SDL_Keycode pressed);

	virtual void setup();
	void loadCif(std::string path);
	void recalculateAtoms();
private:
	void interpretMouseButton(SDL_MouseButtonEvent button, bool dir);
	void interpretControlKey(SDL_Keycode pressed, bool dir);
	GuiAtom *_guiAtoms;
	AtomGroup *_atoms;

};

#endif
