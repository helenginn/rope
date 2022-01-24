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
	Atom *anchor = atoms->firstAtomWithName("OXT");

	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(anchor);
	
	int cBonds = sequence->numBondsForBlock(2);
	int nBonds = sequence->numBondsForBlock(7);
	
	if (cBonds != 3) return 1;
	if (nBonds != 4) return 1;

	delete atoms;

	return 0;
}

