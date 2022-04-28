#include <vagabond/core/Atom.h>
#include <iostream>
#include <vagabond/gui/GuiAtom.h>

int main()
{
//	glm::vec3 start = glm::vec3(0.5, 0.5, 0.5);
//	double init_b = 30;
	Atom *atom = new Atom();
//	atom->setCode("BLA");
//	atom->setInitialPosition(start, init_b);
	atom->setElementSymbol("N");

	GuiAtom *gui = new GuiAtom();
	gui->watchAtom(atom);
	std::cout << atom->elementSymbol() << std::endl;
	
	for (size_t i = 0; i < gui->vertexCount(); i++)
	{
		const Vertex &v = gui->vertex(i);
		std::cout << glm::to_string(v.color) << std::endl;
		if (v.color.b <= v.color.r || v.color.b <= v.color.g)
		{
			return 1;
		}
	}

	return 0;
}



