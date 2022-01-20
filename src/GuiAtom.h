
#ifndef __vagabond__GuiAtom__
#define __vagabond__GuiAtom__

#include "Renderable.h"

class Icosahedron;

class GuiAtom : public Renderable
{
public:
	GuiAtom();

	void addPosition(glm::vec3 position);
	
	size_t verticesPerAtom();
	virtual void render(SnowGL *gl);
private:
	Icosahedron *_template;

};

#endif
