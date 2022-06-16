#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondLength.h"

int main()
{
	std::string path = "/assets/geometry/H2S.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();

	Atom *s = atoms->firstAtomWithName("S");
	Atom *hs1 = atoms->firstAtomWithName("HS1");
	
	BondLength *length = atoms->findBondLength(hs1, s);
	
	return !(length != nullptr);
	
}

