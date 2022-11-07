
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include <thread>
#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Atom.h>
#include <atomic>

class Icosahedron;
class GuiBond;
class GuiBalls;
class GuiRibbon;
class AtomGroup;
class GuiHelices;
class GuiThickBond;
class GuiRepresentation;
class VisualPreferences;

class Atom;

class GuiAtom : public Renderable
{
public:
	GuiAtom();
	~GuiAtom();
	
	void watchAtoms(AtomGroup *as);
	
	void checkAtoms();
	bool checkAtom(Atom *a);
	void startBackgroundWatch();

	glm::vec3 getCentre();

	void setMultiBond(bool m);

	void stop();
	
	const std::vector<Atom *> &atoms() const
	{
		return _atoms;
	}
	
	void setDisableBalls(bool dis);
	void setDisableRibbon(bool dis);

	virtual void render(SnowGL *gl);

	void applyVisuals(VisualPreferences *vp);
private:
	static void backgroundWatch(GuiAtom *what);
	
	GuiBalls *_balls = nullptr;
	GuiRibbon *_ribbon = nullptr;
	GuiHelices *_helices = nullptr;
	GuiThickBond *_thickBonds = nullptr;
	
	std::vector<GuiRepresentation *> _representations;

	std::vector<Atom *> _atoms;
	
	std::thread *_watch = nullptr;

	std::atomic<bool> _finish{false};
	
	bool _multi = true;
};

#endif
