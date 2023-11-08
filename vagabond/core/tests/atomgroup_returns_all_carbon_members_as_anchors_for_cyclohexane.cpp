#include <iostream>
#include "../AtomGroup.h"
#include "../files/CifFile.h"

int main()
{
	std::string path = "/assets/geometry/CHX.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *hexane = geom.atoms();
	
	size_t anchors = hexane->possibleAnchorCount();
	
	if (anchors != 6)
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
		
		if (a->elementSymbol() != "C")
		{
			return 1;
		}
	}
	
	delete hexane;
	return 0;
}
