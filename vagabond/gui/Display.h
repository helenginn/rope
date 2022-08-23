
#ifndef __vagabond__Display__
#define __vagabond__Display__

#include "vagabond/gui/elements/Mouse3D.h"
#include <vagabond/core/Responder.h>
#include <atomic>

class ImageButton;
class GuiAtom;
class GuiRefls;
class GuiDensity;
class AtomGroup;
class Diffraction;
class ArbitraryMap;
class Entity;

class Display : 
public Mouse3D, 
public HasResponder<Responder<Display>>,
public Responder<Entity>
{
public:
	Display(Scene *prev = nullptr);
	virtual ~Display();

	virtual void setup();

	void loadAtoms(AtomGroup *atoms);
	void stop();

	void loadDiffraction(Diffraction *diff);
	void makeMapFromDiffraction();
	void recalculateAtoms();
	virtual void tieButton();
	void wedgeButtons();
	void densityButton();
	void mechanicsButton();
	
	void setOwnsAtoms(bool owns)
	{
		_owns = owns;
	}
	
	void setAtoms(AtomGroup *grp)
	{
		_toLoad = grp;
	}
	
	void densityFromDiffraction(Diffraction *diff);
	
	virtual void buttonPressed(std::string tag, Button *button);
	void fftButton();
protected:
	AtomGroup *_atoms = nullptr;

	GuiAtom *_guiAtoms = nullptr;
	GuiRefls *_guiRefls = nullptr;
private:
	void resetDensityMap();

	GuiDensity *_guiDensity = nullptr;
	AtomGroup *_toLoad = nullptr;

	ImageButton *_halfWedge = nullptr;
	ImageButton *_wedge = nullptr;
	ImageButton *_density = nullptr;
	ImageButton *_mechanics = nullptr;
	
	Diffraction *_diff = nullptr;
	ArbitraryMap *_map = nullptr;
	
	bool _owns = true;
	bool _reciprocal = false;
	std::atomic<bool> _finish{false};
};

#endif
