
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include <vagabond/gui/elements/Renderable.h>
#include <vagabond/core/Atom.h>
#include <atomic>

class Icosahedron;
class GuiBond;
class GuiBalls;
class GuiRibbon;
class GuiRepresentation;
class AtomGroup;

class Atom;

class GuiAtom : public Renderable
{
public:
	GuiAtom();
	~GuiAtom();
	
	void watchAtom(Atom *a);
	void watchAtoms(AtomGroup *as);
	
	void checkAtoms();
	bool checkAtom(Atom *a);
	void startBackgroundWatch();

	glm::vec3 getCentre();

	void setMulti(bool m)
	{
		_multi = m;
	}

	void stop();
	
	const std::vector<Atom *> &atoms() const
	{
		return _atoms;
	}

	virtual void render(SnowGL *gl);
private:
	static void backgroundWatch(GuiAtom *what);
	
	GuiBalls *_balls = nullptr;
	GuiRibbon *_ribbon = nullptr;
	
	std::vector<GuiRepresentation *> _representations;

	std::vector<Atom *> _atoms;
	
	std::thread *_watch = nullptr;

	std::atomic<bool> _finish{false};
	
	bool _multi = true;
};

#endif
