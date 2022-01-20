#include "../src/GuiAtom.h"

int main()
{
	GuiAtom *atom = new GuiAtom();
	size_t v = atom->verticesPerAtom();
	
	return !(v > 0);
}
