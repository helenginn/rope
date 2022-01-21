
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include "Renderable.h"

class Icosahedron;
class Atom;
class AtomGroup;

class GuiAtom : public Renderable
{
public:
	GuiAtom();

	
	void watchAtom(Atom *a);
	void watchAtoms(AtomGroup *as);
	
	size_t verticesPerAtom();
	virtual void render(SnowGL *gl);
private:
	void setPosition(glm::vec3 position);
	void colourByElement(std::string ele);
	void checkAtoms();
	Icosahedron *_template;

	std::vector<Atom *> _atoms;
	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
};

#endif
