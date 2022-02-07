#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../BondSequence.h"

int main()
{
	std::string path = "/assets/geometry/HEX.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	BondSequence *sequence = new BondSequence();
	sequence->addToGraph(atoms->possibleAnchor(0), UINT_MAX);
	
	delete sequence;
	delete atoms;

	return 0;
}

