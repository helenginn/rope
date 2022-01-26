#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/BondLength.h"

int main()
{
	std::string path = "/assets/geometry/H2S.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();

	Atom *s = atoms->firstAtomWithName("S");
	Atom *hs1 = atoms->firstAtomWithName("HS1");
	
	BondLength *length = atoms->findBondLength(hs1, s);
	
	return !(length != nullptr);
	
}

