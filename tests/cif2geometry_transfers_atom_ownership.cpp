#include "../src/Cif2Geometry.h"
#include "../src/AtomGroup.h"

int main()
{
	Cif2Geometry *geom = new Cif2Geometry("");
	AtomGroup *atoms = geom->atoms();
	delete geom;
	
	int count = atoms->size();
	
	return !(count == 0);
}
