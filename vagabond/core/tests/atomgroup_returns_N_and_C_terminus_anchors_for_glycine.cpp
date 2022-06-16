#include <iostream>
#include "../AtomGroup.h"
#include "../CifFile.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *glycine = geom.atoms();
	
	size_t anchors = glycine->possibleAnchorCount();
	
	if (anchors != 3)
	{
		std::cout << "Incorrect number of anchors: " << anchors << std::endl;

		for (size_t i = 0; i < anchors; i++)
		{
			Atom *a = glycine->possibleAnchor(i);
			std::cout << a->atomName() << " ";

		}
		std::cout << std::endl;
		return 1;
	}
	
	bool found[] = {false, false, false};
	std::string labels[] = {"N", "O", "OXT"};

	for (size_t i = 0; i < anchors; i++)
	{
		Atom *a = glycine->possibleAnchor(i);
		for (size_t j = 0; j < 3; j++)
		{
			if (a->atomName() == labels[j])
			{
				found[j] = true;
			}
		}
	}
	
	for (size_t i = 0; i < 3; i++)
	{
		if (!found[i])
		{
			return 1;
		}
	}
	
	delete glycine;
	return 0;
}
