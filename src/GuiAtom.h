
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include "Renderable.h"
#include <atomic>

class Icosahedron;
class GuiBond;
class Atom;
class AtomGroup;

class GuiAtom : public Renderable
{
public:
	GuiAtom();
	~GuiAtom();
	
	void watchAtom(Atom *a);
	void watchAtoms(AtomGroup *as);
	
	size_t verticesPerAtom();
	void checkAtoms();
	void startBackgroundWatch();

	virtual void render(SnowGL *gl);
private:
	void setPosition(glm::vec3 position);
	void colourByElement(std::string ele);
	bool checkAtom(Atom *a);
	static void backgroundWatch(GuiAtom *what);

	Icosahedron *_template;
	GuiBond *_bonds;

	std::vector<Atom *> _atoms;
	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
	
	std::thread *_watch = nullptr;
	std::atomic<bool> _finish;
};

#endif
