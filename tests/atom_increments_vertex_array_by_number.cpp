#include "../src/GuiAtom.h"

int main()
{
	GuiAtom *atom = new GuiAtom();
	size_t v = atom->verticesPerAtom();
	
	atom->addPosition(glm::vec3(0, 0, 0));
	
	return !(atom->vertexCount() == v);
}
