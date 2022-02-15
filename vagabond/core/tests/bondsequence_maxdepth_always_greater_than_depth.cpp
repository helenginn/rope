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
	sequence->addToGraph(anchor);
	
	for (size_t i = 0; i < sequence->atomGraphCount(); i++)
	{
		int diff = sequence->remainingDepth(i);
		if (diff < 0)
		{
			sequence->checkAtomGraph(i);
			return 1;
		}
	}

	return 0;
}



