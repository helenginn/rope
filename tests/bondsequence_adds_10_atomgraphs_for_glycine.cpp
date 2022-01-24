#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
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

