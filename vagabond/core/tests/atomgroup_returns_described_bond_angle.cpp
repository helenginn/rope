#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondAngle.h"

int main()
{
	std::string path = "/assets/geometry/H2S.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();

	Atom *s = atoms->firstAtomWithName("S");
	Atom *hs1 = atoms->firstAtomWithName("HS1");
	Atom *hs2 = atoms->firstAtomWithName("HS2");
	
	BondAngle *angle = atoms->findBondAngle(hs1, s, hs2);
	
	return !(angle && angle->atomIsCentral(s));
	
}

