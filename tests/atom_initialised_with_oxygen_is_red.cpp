#include "../src/Atom.h"
#include "../src/GuiAtom.h"

int main()
{
	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
	double init_b = 30;
	Atom *atom = new Atom();
	atom->setElementSymbol("O");
	atom->setInitialPosition(start, init_b);

	GuiAtom *gui = new GuiAtom();
	gui->watchAtom(atom);
	
	for (size_t i = 0; i < gui->vertexCount(); i++)
	{
		const Vertex &v = gui->vertex(i);
		if (v.color.r <= v.color.g || v.color.r <= v.color.b)
		{
			return 1;
		}
	}

	return 0;
}



