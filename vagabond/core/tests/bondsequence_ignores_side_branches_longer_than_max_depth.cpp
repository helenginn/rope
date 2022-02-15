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
	sequence->reflagDepth(0, INT_MAX, 0);
	
	size_t nf = sequence->flagged();
	
	delete atoms;
	if (nf != 6)
	{
		std::cout << "Incorrect number of flagged atoms: " << nf << std::endl;
		return 1;
	}

	return 0;
}


