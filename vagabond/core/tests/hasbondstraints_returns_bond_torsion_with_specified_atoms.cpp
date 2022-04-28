#include "../CifFile.h"
#include "../AtomGroup.h"
#include "../BondTorsion.h"

int main()
{
	std::string path = "/assets/geometry/TYR.cif";

	CifFile geom = CifFile(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *n = atoms->firstAtomWithName("N");
	Atom *ca = atoms->firstAtomWithName("CA");
	Atom *cb = atoms->firstAtomWithName("CB");
	Atom *cg = atoms->firstAtomWithName("CG");

	BondTorsion *t = atoms->findBondTorsion(n, ca, cb, cg);
	BondTorsion *rt = atoms->findBondTorsion(cg, cb, ca, n);
	
	std::cout << t << std::endl;
	std::cout << rt << std::endl;
	
	return !(t != nullptr && rt != nullptr && rt == t);
}

