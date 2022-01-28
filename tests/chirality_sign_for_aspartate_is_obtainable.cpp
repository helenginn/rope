#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"
#include "../src/Chirality.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	Cif2Geometry geom = Cif2Geometry(path);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	Atom *ca = atoms->firstAtomWithName("CA");
	Atom *cb = atoms->firstAtomWithName("CB");
	Atom *n = atoms->firstAtomWithName("N");
	Atom *c = atoms->firstAtomWithName("C");
	Atom *h = atoms->firstAtomWithName("HA");
	
	Chirality *ch = ca->chirality(0);

	int sign = ch->get_sign(&h, &cb, &n, &c);
	int rsign = ch->get_sign(&h, &n, &cb, &c);
	
	std::cout << "Sign: " << sign << std::endl;
	std::cout << "Reverse sign: " << rsign << std::endl;
	
	return !(rsign * sign < 0);
}
