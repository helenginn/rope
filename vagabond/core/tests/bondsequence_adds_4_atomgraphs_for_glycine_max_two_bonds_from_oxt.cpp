#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/GLY.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(anchor, 2);
	int added = sequence->addedAtomsCount();

	if (added != 4)
	{
		std::cout << "Incorrect number of added atoms, " << added << std::endl;
		return 1;
	}

	delete atoms;

	return 0;
}

