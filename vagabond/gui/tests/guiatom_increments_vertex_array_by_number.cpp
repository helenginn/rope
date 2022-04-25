#include <vagabond/gui/GuiAtom.h>
#include <vagabond/core/Atom.h>

int main()
{
	GuiAtom *gui = new GuiAtom();
	size_t v = gui->verticesPerAtom();

	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
	double init_b = 30;
	Atom *atom = new Atom();
	atom->setInitialPosition(start, init_b);
	
	gui->watchAtom(atom);
	
	return !(gui->vertexCount() == v);
}
