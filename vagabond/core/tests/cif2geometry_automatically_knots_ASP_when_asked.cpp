#include "../CifFile.h"
#include "../AtomGroup.h"

int main()
{
	std::string path = "/assets/geometry/ASP.cif";

	CifFile geom = CifFile(path);
	geom.setAutomaticKnot(true);
	geom.parse();
	
	AtomGroup *atoms = geom.atoms();
	
	return !(atoms->bondstraintCount() > 0);
}
