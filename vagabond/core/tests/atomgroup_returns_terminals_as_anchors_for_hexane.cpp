#include <iostream>
#include "../AtomGroup.h"
#include "../Cif2Geometry.h"

int main()
{
	std::string path = "/assets/geometry/HEX.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *hexane = geom.atoms();
	
	size_t anchors = hexane->possibleAnchorCount();
	
	if (anchors != 2)
	{
		std::cout << "Incorrect number of anchors: " << anchors << std::endl;

		for (size_t i = 0; i < anchors; i++)
		{
			Atom *a = hexane->possibleAnchor(i);
			std::cout << a->atomName() << " ";

		}
		std::cout << std::endl;
		return 1;
	}

	for (size_t i = 0; i < anchors; i++)
	{
		Atom *a = hexane->possibleAnchor(i);
		if (a->atomName() != "C1" && a->atomName() != "C6")
		{
			return 1;
		}
	}
	
	delete hexane;
	return 0;
}
