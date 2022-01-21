
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include "Renderable.h"

class Icosahedron;
class Atom;

class GuiAtom : public Renderable
{
public:
	GuiAtom();

	void addPosition(glm::vec3 position);
	
	void watchAtom(Atom *a);
	
	size_t verticesPerAtom();
	virtual void render(SnowGL *gl);
private:
	void checkAtoms();
	Icosahedron *_template;

	std::vector<Atom *> _atoms;
	std::map<Atom *, int> _atomIndex;
	std::map<Atom *, glm::vec3> _atomPos;
};

#endif
