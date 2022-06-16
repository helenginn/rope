#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/HEX.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(atoms->possibleAnchor(0), UINT_MAX);
	
	delete sequence;
	delete atoms;

	return 0;
}

