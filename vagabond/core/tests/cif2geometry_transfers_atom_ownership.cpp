#include "../Cif2Geometry.h"
#include "../AtomGroup.h"

int main()
{
	Cif2Geometry *geom = new Cif2Geometry("");
	AtomGroup *atoms = geom->atoms();
	delete geom;
	
	int count = atoms->size();
	
	return !(count == 0);
}
