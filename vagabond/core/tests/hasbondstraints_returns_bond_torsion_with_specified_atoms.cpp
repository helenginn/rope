#include "../Cif2Geometry.h"
#include "../AtomGroup.h"
#include "../BondTorsion.h"

int main()
{
	std::string path = "/assets/geometry/TYR.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	Atom *n = atoms->firstAtomWithName("N");
	Atom *ca = atoms->firstAtomWithName("CA");
	Atom *cb = atoms->firstAtomWithName("CB");
	Atom *cg = atoms->firstAtomWithName("CG");

	BondTorsion *t = atoms->findBondTorsion(n, ca, cb, cg);
	BondTorsion *rt = atoms->findBondTorsion(cg, cb, ca, n);
	
	return !(t != nullptr && rt != nullptr && rt == t);
}

