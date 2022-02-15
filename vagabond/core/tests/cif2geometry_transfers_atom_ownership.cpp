#include "../CifFile.h"
#include "../AtomGroup.h"

int main()
{
	CifFile *geom = new CifFile("");
	AtomGroup *atoms = geom->atoms();
	delete geom;
	
	int count = atoms->size();
	
	return !(count == 0);
}
