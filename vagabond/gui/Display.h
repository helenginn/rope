
#ifndef __vagabond__Display__
#define __vagabond__Display__

#include <vagabond/gui/elements/Scene.h>

class ImageButton;
class GuiAtom;
class GuiRefls;
class GuiDensity;
class AtomGroup;
class Diffraction;

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
	void loadAtoms(AtomGroup *atoms);
	void loadDiffraction(Diffraction *diff);
	void recalculateAtoms();
	void tieButton();
	void wedgeButtons();
	void densityButton();
	
private:
	void interpretMouseButton(SDL_MouseButtonEvent button, bool dir);
	void interpretControlKey(SDL_Keycode pressed, bool dir);

	GuiAtom *_guiAtoms = nullptr;
	GuiRefls *_guiRefls = nullptr;
	GuiDensity *_guiDensity = nullptr;
	AtomGroup *_atoms = nullptr;

	void updateSpinMatrix();
	
	glm::mat3x3 _spin;
	
	ImageButton *_halfWedge = nullptr;
	ImageButton *_wedge = nullptr;
	ImageButton *_density = nullptr;
};

#endif
