#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	for (size_t i = 0; i < atoms->possibleAnchorCount(); i++)
	{
		BondSequence *sequence = new BondSequence();
		sequence->addToGraph(atoms->possibleAnchor(i), UINT_MAX);
		int added = sequence->addedAtomsCount();

		if (added != 10)
		{
			std::cout << "Incorrect number of added atoms, " << added << std::endl;
			return 1;
		}
	}
	
	delete atoms;

	return 0;
}

